#include <algorithm>
#include <cctype>
#include <string>
#include <map>

#include "Utils.h"
#include "Parser.h"

namespace utils
{
    bool ends_with(const std::string& value, const std::string& ending)
    {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    bool isVMFile(const std::string& f)
    {
        return std::isupper(f[0])
            && (std::count_if(f.begin(), f.end(), [](char c) { return c == '.'; }) == 1)
            && ends_with(f, ".vm") && (f.length() >= 4);
    }

    void ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    // trim from end (in place)
    void rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(std::string& s)
    {
        ltrim(s);
        rtrim(s);
    }

    void removeComments(std::string& s)
    {
        ltrim(s);
        auto res = std::find(s.begin(), s.end(), '/');

        if (res != s.end())
            s.erase(res, s.end());

        rtrim(s);
    }

    std::map<std::string, Parser::Command> commandMap{
        { "pop", Parser::Command::C_POP },
        { "push", Parser::Command::C_PUSH },
        { "add", Parser::Command::C_ARITHMETIC_BI },
        { "sub", Parser::Command::C_ARITHMETIC_BI },
        { "neg", Parser::Command::C_ARITHMETIC_UN },
        { "eq", Parser::Command::C_COMPARISON },
        { "gt", Parser::Command::C_COMPARISON },
        { "lt", Parser::Command::C_COMPARISON },
        { "and", Parser::Command::C_ARITHMETIC_BI },
        { "or", Parser::Command::C_ARITHMETIC_BI },
        { "not", Parser::Command::C_ARITHMETIC_UN },
    };

    std::map<std::string, std::string> symbolMap{
        { "add", "+" },
        { "sub", "-" },
        { "neg", "-" },
        { "and", "&" },
        { "or", "|" },
        { "not", "!" },
        { "eq", "JEQ" },
        { "gt", "JGT" },
        { "lt", "JLT" },
    };

    std::map<std::string, std::string> segmentMap{
        { "argument", "ARG" },
        { "local", "LCL" },
        { "this", "THIS" },
        { "that", "THAT" },
    };
}

