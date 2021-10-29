#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string>
#include <map>

#include "Parser.h"

namespace utils
{
    // trim from both ends (in place)
    void trim(std::string& s);
    void removeComments(std::string& s);
    bool isVMFile(const std::string& f);
    extern std::map<std::string, Parser::Command> commandMap;
    extern std::map<std::string, std::string> segmentMap;
    extern std::map<std::string, std::string> symbolMap;
}

#endif // UTILS_H_INCLUDED
