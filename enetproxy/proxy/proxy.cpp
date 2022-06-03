#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include "enet/include/enet.h"
#include "http.h"
#include "server.h"
#include "proton/rtparam.hpp"
#include <fstream>
#include "HTTPRequest.hpp"
#include "gt.hpp"

using namespace std;
std::ofstream slog("logs.txt");
server* g_server = new server();
string line;
#ifdef _WIN32
BOOL WINAPI exit_handler(DWORD dwCtrlType) {
    try {
        std::ofstream temizleh("C:\\Windows\\System32\\drivers\\etc\\hosts");

        switch (dwCtrlType) {
        case CTRL_BREAK_EVENT || CTRL_CLOSE_EVENT || CTRL_C_EVENT:
            if (temizleh.is_open()) {
                temizleh << "";
                temizleh.close();
            }
            return TRUE;

        default: return FALSE;
        }

        return TRUE;
    }
    catch (int e) {}
}
#endif

void pofproxy()
{
    try
    {
        http::Request request1{ "https://raw.githubusercontent.com/ProjectPOF/POFProxy/main/pofproxy" };
        http::Request request2{ "https://raw.githubusercontent.com/ProjectPOF/POFProxy/main/status" };
        http::Request request3{ "https://raw.githubusercontent.com/ProjectPOF/POFProxy/main/gameversion.txt" };
        const auto response1 = serverdata.send("POST", "version=1&protocol=158", { "Content-Type: application/x-www-form-urlencoded" });
        const auto response2 = serverdata.send("POST", "version=1&protocol=158", { "Content-Type: application/x-www-form-urlencoded" });
        const auto response3 = serverdata.send("POST", "version=1&protocol=158", { "Content-Type: application/x-www-form-urlencoded" });
        std::string pofversion = { response1.body.begin(), response1.body.end() };
        std::string pofstatus = { response2.body.begin(), response2.body.end() };
        std::string gtversion = { response3.body.begin(), response3.body.end() };
        cout << "1: " + gtversion + "\n";
        cout << "2: " + pofversion + "\n";
        cout << "3: " + pofstatus + "\n";*/
        gt::version = gtversion;
        gt::pofversion = pofversion; 
        gt::pofstatus = pofstatus;
        cout << "Growtopia version: " + gt::version + "\n";
        cout << "POFProxy version: " + gt::pofversion + "\n";
        cout << "POFProxy status: " + gt::pofstatus + "\n";
        if (slog.is_open()) {
            slog << "#POFProxyCurrentVersion=" + gt::pofversion + "\n#POFProxyCurrentStatus=" + gt::pofstatus;
            slog.close();
        }
    }
    catch (std::exception) {}
}
void edithost()
{
    try {
        std::ofstream shost("C:\\Windows\\System32\\drivers\\etc\\hosts");

        if (shost.is_open()) {
            cout << "Editing Hosts\n";
            shost << "127.0.0.1 growtopia1.com\n127.0.0.1 growtopia2.com";
            shost.close();
        }
    }
    catch (std::exception) {}
}
void serverdata() {
    try
    {
        http::Request serverdata{ "http://api.surferstealer.com/system/growtopiaapi?getall" };
        const auto response = serverdata.send("POST", "version=1&protocol=158", { "Content-Type: application/x-www-form-urlencoded" });
        rtvar var = rtvar::parse({ response.body.begin(), response.body.end() });

        var.serialize();
        if (var.get("server") == "127.0.0.1") {
            return;
        }
        if (var.find("server")) {
            g_server->m_server = var.get("server");
            g_server->m_port = std::stoi(var.get("port"));
            g_server->meta = var.get("meta");
        }
    } catch (std::exception) {}

}

int main() {
#ifdef _WIN32
    SetConsoleTitleA("POFProxy");
    SetConsoleCtrlHandler(exit_handler, true);//auto host
#endif
    printf("Discord: ProB1#0100\n");
    printf("Enet Proxy By Ama Fixed Shadowban and added features by ProB1\n");

    pofproxy();
    serverdata();
    edithost();
    std::thread http(http::run, "127.0.0.1", g_server->m_port); //"17191"
    http.detach();
    printf("HTTP server is running.\n");
    enet_initialize();
    if (g_server->start()) {
        printf("Server & client proxy is running.\n");
        while (true) {
            g_server->poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    else
        if (slog.is_open()) {//                                                     
            slog << "Failed to start server or proxy.\n";
            slog.close();
        }
    
}
