#pragma once
#include <string>

namespace gt {
    extern std::string version;
	extern std::string pofversion;
	extern std::string pofstatus;
    extern std::string flag;
    extern bool resolving_uid2;
    extern bool connecting;
    extern bool in_game;
    extern bool ghost;
    void send_log(std::string text);
    void SolveCaptcha(std::string text);
}
