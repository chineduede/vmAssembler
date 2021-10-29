#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

#include "CodeWriter.h"
#include "Parser.h"
#include "Utils.h"

namespace fs = std::filesystem;

CodeWriter::CodeWriter(const std::string& name)
    : mFile{ fs::path(name).replace_extension(".asm").string() }
    , mName{ fs::path(name).filename().replace_extension().string() }
{
    if (!mFile)
        std::cout << "Couldn't open " << name;
}

void CodeWriter::writeArithmetic(const std::string& cmd)
{
    const Parser::Command cmdType{ utils::commandMap[cmd] };

    if (cmdType == Parser::Command::C_ARITHMETIC_BI)
    {
        writePushPop(Parser::Command::C_POP, "hidden", 1);
        writePushPop(Parser::Command::C_POP, "hidden", 0, true);
        implementArith(utils::symbolMap[cmd]);
        writePushPop(Parser::Command::C_PUSH, "hidden", 0, true);
    }
    else if (cmdType == Parser::Command::C_ARITHMETIC_UN)
    {
        writePushPop(Parser::Command::C_POP, "hidden", 0, true);
        implementArith(utils::symbolMap[cmd], false);
        writePushPop(Parser::Command::C_PUSH, "hidden", 0, true);
    }
    else if (cmdType == Parser::Command::C_COMPARISON)
    {
        writePushPop(Parser::Command::C_POP, "hidden", 1);
        writePushPop(Parser::Command::C_POP, "hidden", 0, true);
        implementArith("-", true, utils::symbolMap[cmd]);
        writePushPop(Parser::Command::C_PUSH, "hidden", 0, true);
    }
}

void CodeWriter::writePushPop(Parser::Command cmd, const std::string& segment, int index, bool ld_frm_d)
{
    auto found{ utils::segmentMap.find(segment) };

    if (cmd == Parser::Command::C_POP)
    {
        // Decrement stack point
        wrtBaseCmd("SP", 'M', 'M', '-', '1');

        if (found != utils::segmentMap.end())
        {
            accessIdxAddrOrLdMem(index, found->second);
            wrtBaseCmd("R13", 'M', 'D');

            wrtBaseCmd("SP", 'A', 'M');
            wrtBaseCmd("", 'D', 'M', false);

            wrtBaseCmd("R13", 'A', 'M');
            wrtBaseCmd("", 'M', 'D', false);
        }

        else
        {
            wrtBaseCmd("SP", 'A', 'M');
            wrtBaseCmd("", 'D', 'M', false);

            if (!ld_frm_d)
            {
                std::string temp{ directQualName(index, segment) };
                wrtBaseCmd(temp, 'M', 'D');
            }
        }
    }
    else if (cmd == Parser::Command::C_PUSH)
    {
        if (found != utils::segmentMap.end())
        {
            accessIdxAddrOrLdMem(index, found->second);
            wrtBaseCmd("", 'A', 'D', false);
            wrtBaseCmd("", 'D', 'M', false);
        }
        else if (!ld_frm_d)
        {
            std::string temp{ directQualName(index, segment) };

            if (segment == "constant")
                wrtBaseCmd(index, 'D', 'A');
            else
                wrtBaseCmd(temp, 'D', 'M');
        }

        wrtBaseCmd("SP", 'A', 'M');
        wrtBaseCmd("", 'M', 'D', false);
        // Increment stack pointer
        wrtBaseCmd("SP", 'M', 'M', '+', '1');
    }
}

void CodeWriter::close() { mFile.close(); }

// Beginning of overloaded functions for generating Hack assembly commands.
void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char from, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << seg << '\n';
    mFile << to << '=' << from << '\n';
}

void CodeWriter::wrtBaseCmd(int seg, char to, char op1, char op, char op2, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << seg << '\n';
    mFile << to << '=' << op1 << op << op2 << '\n';
}

void CodeWriter::wrtBaseCmd(int segment, char to, char from, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << segment << '\n';
    mFile << to << '=' << from << '\n';
}
void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char op1, char op, char op2, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << seg << '\n';
    mFile << to << '=' << op1 << op << op2 << '\n';
}

void CodeWriter::wrtBaseCmd(const std::string& seg, char comp_val, const std::string& comp_op, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << seg << '\n';
    mFile << comp_val << ';' << comp_op << '\n';
}

void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char op, char op1, bool ld_seg)
{
    if (ld_seg)
        mFile << '@' << seg << '\n';
    mFile << to << '=' << op << op1 << '\n';
}
// end of overloaded functions


void CodeWriter::accessIdxAddrOrLdMem(int index, const std::string& seg)
{
    if (!index)
        wrtBaseCmd(seg, 'D', 'M');
    else
    {
        wrtBaseCmd(index, 'D', 'A');
        wrtBaseCmd(seg, 'D', 'M', '+', 'D');
    }
}

void CodeWriter::implementArith(const std::string& sign, bool binary_op, const std::string& cmp_sign)
{
    static int comp_sign_counter{};
    std::string r13{ "R13" };

    if (binary_op)
        wrtBaseCmd("R14", 'D', 'D', sign.front(), 'M');
    else
        wrtBaseCmd("", 'D', sign.front(), 'D', false);

    if (!cmp_sign.empty())
    {
        std::string compLabel{ "COMP_" + cmp_sign + "_" + std::to_string(comp_sign_counter) };
        std::string exitCompLabel{ "EXIT_" + compLabel };

        wrtBaseCmd(compLabel, 'D', cmp_sign);

        wrtBaseCmd(r13, 'D', '0');

        wrtBaseCmd(exitCompLabel, '0', "JMP");

        mFile << '(' << compLabel << ')' << '\n';
        wrtBaseCmd(r13, 'D', '-', '1');
        mFile << '(' << exitCompLabel << ')' << '\n';

        comp_sign_counter++;
    }
}

std::string CodeWriter::directQualName(int index, const std::string& segment)
{
    std::string temp;
    if (segment == "temp")
        temp = { 'R' + std::to_string(5 + index) };
    else if (segment == "static")
        temp = { mName + '.' + std::to_string(index) };
    else if (segment == "pointer")
        temp = (index) ? std::string{ "THAT" } : std::string{ "THIS" };
    else if (segment == "hidden")
        temp = { 'R' + std::to_string(13 + index) };
    else
        temp = {};
    return temp;
}

void CodeWriter::opt_assignment_op(int const_val, const std::string& seg, int index)
{
    std::string tempName{ directQualName(index, seg) };
    auto found{ utils::segmentMap.find(seg) };

    if (found != utils::segmentMap.end() && index != 0)
        accessIdxAddrOrLdMem(index, found->second);
    else
    {
        wrtBaseCmd(const_val, 'D', 'A');
        if (tempName.empty())
        {
            wrtBaseCmd(found->second, 'A', 'M');
            wrtBaseCmd("", 'M', 'D', false);
        }
        else
            wrtBaseCmd(tempName, 'M', 'D');
        return;
    }

    wrtBaseCmd("R13", 'M', 'D');
    wrtBaseCmd(const_val, 'D', 'A');
    wrtBaseCmd("R13", 'A', 'M');
    wrtBaseCmd("", 'M', 'D', false);
}
