#include "gt.hpp"
#include "packet.h"
#include "server.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

std::string gt::version = "3.89";
std::string gt::pofversion = "0.1";
std::string gt::pofstatus = "Updated";
std::string gt::flag = "tr";
bool gt::resolving_uid2 = false;
bool gt::connecting = false;
bool gt::in_game = false;
bool gt::ghost = false;

vector<string> split (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

using namespace std;

    void gt::send_log(std::string text) 
    {
    g_server->send(true, "action|log\nmsg|" + text, NET_MESSAGE_GAME_MESSAGE);
    }

void gt::SolveCaptcha(std::string text)
{
    //thanks to HeySurfer
  auto spcaptch = split(text, "|");
  std::string captchaid = spcaptch[1];
  utils::replace(captchaid, "0098/captcha/generated/", "");
  utils::replace(captchaid, "PuzzleWithMissingPiece.rttex", "");
  captchaid = captchaid.substr(0, captchaid.size() - 1);
  http::Request request{"http://api.surferstealer.com/captcha/index?CaptchaID"+captchaid};
  const auto response = request.send("GET");
  std::string captchaAnswer = std::string{response.body.begin(), response.body.end()};
  if (captchaAnswer.find("Failed") != std::string::npos) 
      std::cout << "Captcha Failed!" << '\n';
      gt::send_log("`bCaptcha Failed!");
  else if (captchaAnswer.find("Answer|") != std::string::npos) 
  {
        std::cout << "Captcha Success!" << '\n';
        std::cout << captchaAnswer<< '\n';
        gt::send_log("`2Captcha Success!");
        return captchaAnswer;
  }
    return "Fail";
}
