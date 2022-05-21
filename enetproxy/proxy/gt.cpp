#include "gt.hpp"
#include "packet.h"
#include "server.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>

std::string gt::version = "3.89";
std::string gt::pofversion = "0.1";
std::string gt::pofstatus = "Updated";
std::string gt::flag = "tr";
bool gt::resolving_uid2 = false;
bool gt::connecting = false;
bool gt::in_game = false;
bool gt::ghost = false;


using namespace std;

    void gt::send_log(std::string text) 
    {
    g_server->send(true, "action|log\nmsg|" + text, NET_MESSAGE_GAME_MESSAGE);
    }
