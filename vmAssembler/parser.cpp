#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Utils.h"
#include "Parser.h"


Parser::Parser(const std::string& fileName)
    : mFile{ fileName }
    , mCommand{}
    , mNxtCommand{}
{
    __advance(mNxtCommand);
}

void Parser::__advance(std::stringstream& strm)
{
    strm.clear();
    strm.str("");

    while (hasMoreLines())
    {
        std::string temp;
        std::getline(mFile, temp);

        utils::removeComments(temp);

        if (!temp.empty())
        {
            strm << temp;
            break;
        }
    }
}

void Parser::advance()
{
    mCommand.clear();
    mCommand.str("");
    mCommand << mNxtCommand.str();
    if (hasMoreLines())
        __advance(mNxtCommand);
}

Parser::Command Parser::commandType()
{
    std::string cmd;
    mCommand >> cmd;
    auto type{ utils::commandMap.find(cmd) };
    resetStream(mCommand);

    if (type != utils::commandMap.end())
        return type->second;
    else
        return Command::C_NOT_IMPLEMENTED;
}

Parser::Command Parser::peekNxtCommandType()
{
    std::string cmd;
    mNxtCommand >> cmd;
    auto type{ utils::commandMap.find(cmd) };
    resetStream(mNxtCommand);

    if (type != utils::commandMap.end())
        return type->second;
    else
        return Command::C_NOT_IMPLEMENTED;
}

std::string Parser::arg1()
{
    Command cmdType{ commandType() };
    std::string cmd;
    std::string arg1{};
    mCommand >> cmd >> arg1;
    if (cmdType == Command::C_ARITHMETIC_BI ||
        cmdType == Command::C_ARITHMETIC_UN ||
        cmdType == Command::C_COMPARISON)
        return cmd;
    else
        return arg1;
}

int Parser::arg2()
{
    int arg2;
    mCommand >> arg2;
    return arg2;
}

void Parser::resetStream(std::stringstream& strm)
{
    strm.seekg(0, std::ios_base::beg);
}

std::string Parser::returnCommand()
{
    return mCommand.str();
}

