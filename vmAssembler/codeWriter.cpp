#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

#include "CodeWriter.h"
#include "Parser.h"
#include "Utils.h"

namespace fs = std::filesystem;


constexpr char REG_D = 'D';
constexpr char REG_A = 'A';
constexpr char REG_M = 'M';
constexpr char AT = '@';
constexpr char EQUALS_TO = '=';
constexpr char BRAC_OP = '(';
constexpr char BRAC_CLE = ')';
constexpr char ZERO = '0';

const char* REG_SP = "SP";
const char* REG_R13 = "R13";
const char* REG_R14 = "R14";
const char* SEG_HIDDEN = "hidden";




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
        writePushPop(Parser::Command::C_POP, SEG_HIDDEN, 1);
        writePushPop(Parser::Command::C_POP, SEG_HIDDEN, 0, true);
        implementArith(utils::symbolMap[cmd]);
        writePushPop(Parser::Command::C_PUSH, SEG_HIDDEN, 0, true);
    }
    else if (cmdType == Parser::Command::C_ARITHMETIC_UN)
    {
        writePushPop(Parser::Command::C_POP, SEG_HIDDEN, 0, true);
        implementArith(utils::symbolMap[cmd], false);
        writePushPop(Parser::Command::C_PUSH, SEG_HIDDEN, 0, true);
    }
    else if (cmdType == Parser::Command::C_COMPARISON)
    {
        writePushPop(Parser::Command::C_POP, SEG_HIDDEN, 1);
        writePushPop(Parser::Command::C_POP, SEG_HIDDEN, 0, true);
        implementArith("-", true, utils::symbolMap[cmd]);
        writePushPop(Parser::Command::C_PUSH, SEG_HIDDEN, 0, true);
    }
}

void CodeWriter::writePushPop(Parser::Command cmd, const std::string& segment, int index, bool ld_frm_d)
{
    auto found{ utils::segmentMap.find(segment) };

    if (cmd == Parser::Command::C_POP)
    {
        // Decrement stack point
        wrtBaseCmd(REG_SP, REG_M, REG_M, '-', '1');

        if (found != utils::segmentMap.end())
        {
            accessIdxAddrOrLdMem(index, found->second);
            wrtBaseCmd(REG_R13, REG_M, REG_D);

            wrtBaseCmd(REG_SP, REG_A, REG_M);
            wrtBaseCmd("", REG_D, REG_M, false);

            wrtBaseCmd(REG_R13, REG_A, REG_M);
            wrtBaseCmd("", REG_M, REG_D, false);
        }

        else
        {
            wrtBaseCmd(REG_SP, REG_A, REG_M);
            wrtBaseCmd("", REG_D, REG_M, false);

            if (!ld_frm_d)
            {
                std::string temp{ directQualName(index, segment) };
                wrtBaseCmd(temp, REG_M, REG_D);
            }
        }
    }
    else if (cmd == Parser::Command::C_PUSH)
    {
        if (found != utils::segmentMap.end())
        {
            accessIdxAddrOrLdMem(index, found->second);
            wrtBaseCmd("", REG_A, REG_D, false);
            wrtBaseCmd("", REG_D, REG_M, false);
        }
        else if (!ld_frm_d)
        {
            std::string temp{ directQualName(index, segment) };

            if (segment == "constant")
                wrtBaseCmd(index, REG_D, REG_A);
            else
                wrtBaseCmd(temp, REG_D, REG_M);
        }

        wrtBaseCmd(REG_SP, REG_A, REG_M);
        wrtBaseCmd("", REG_M, REG_D, false);
        // Increment stack pointer
        wrtBaseCmd(REG_SP, REG_M, REG_M, '+', '1');
    }
}

void CodeWriter::close() { mFile.close(); }

// Beginning of overloaded functions for generating Hack assembly commands.
void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char from, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << seg << '\n';
    mFile << to << EQUALS_TO << from << '\n';
}

void CodeWriter::wrtBaseCmd(int seg, char to, char op1, char op, char op2, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << seg << '\n';
    mFile << to << EQUALS_TO << op1 << op << op2 << '\n';
}

void CodeWriter::wrtBaseCmd(int segment, char to, char from, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << segment << '\n';
    mFile << to << EQUALS_TO << from << '\n';
}
void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char op1, char op, char op2, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << seg << '\n';
    mFile << to << EQUALS_TO << op1 << op << op2 << '\n';
}

void CodeWriter::wrtBaseCmd(const std::string& seg, char comp_val, const std::string& comp_op, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << seg << '\n';
    mFile << comp_val << ';' << comp_op << '\n';
}

void CodeWriter::wrtBaseCmd(const std::string& seg, char to, char op, char op1, bool ld_seg)
{
    if (ld_seg)
        mFile << AT << seg << '\n';
    mFile << to << EQUALS_TO << op << op1 << '\n';
}
// end of overloaded functions


void CodeWriter::accessIdxAddrOrLdMem(int index, const std::string& seg)
{
    if (!index)
        wrtBaseCmd(seg, REG_D, REG_M);
    else
    {
        wrtBaseCmd(index, REG_D, REG_A);
        wrtBaseCmd(seg, REG_D, REG_M, '+', REG_D);
    }
}

void CodeWriter::implementArith(const std::string& sign, bool binary_op, const std::string& cmp_sign)
{
    static int comp_sign_counter{};
    std::string r13{ REG_R13 };

    if (binary_op)
        wrtBaseCmd(REG_R14, REG_D, REG_D, sign.front(), REG_M);
    else
        wrtBaseCmd("", REG_D, sign.front(), REG_D, false);

    if (!cmp_sign.empty())
    {
        std::string compLabel{ "COMP_" + cmp_sign + "_" + std::to_string(comp_sign_counter) };
        std::string exitCompLabel{ "EXIT_" + compLabel };

        wrtBaseCmd(compLabel, REG_D, cmp_sign);

        wrtBaseCmd(r13, REG_D, ZERO);

        wrtBaseCmd(exitCompLabel, ZERO, "JMP");

        mFile << BRAC_OP << compLabel << BRAC_CLE << '\n';
        wrtBaseCmd(r13, REG_D, '-', '1');
        mFile << BRAC_OP << exitCompLabel << BRAC_CLE << '\n';

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
    else if (segment == SEG_HIDDEN)
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
        wrtBaseCmd(const_val, REG_D, REG_A);
        if (tempName.empty())
        {
            wrtBaseCmd(found->second, REG_A, REG_M);
            wrtBaseCmd("", REG_M, REG_D, false);
        }
        else
            wrtBaseCmd(tempName, REG_M, REG_D);
        return;
    }

    wrtBaseCmd(REG_R13, REG_M, REG_D);
    wrtBaseCmd(const_val, REG_D, REG_A);
    wrtBaseCmd(REG_R13, REG_A, REG_M);
    wrtBaseCmd("", REG_M, REG_D, false);
}

void CodeWriter::writeLabel(const std::string& label)
{
    mFile << BRAC_OP << label << BRAC_CLE << '\n';
}

void CodeWriter::writeGoto(const std::string& label)
{
    wrtBaseCmd(label, ZERO, "JMP");
}

void CodeWriter::writeIf(const std::string& label)
{
    wrtBaseCmd(REG_SP, REG_M, REG_M, '-', '1');
    wrtBaseCmd(REG_SP, REG_A, REG_M);
    wrtBaseCmd("", REG_D, REG_M, false);
    wrtBaseCmd(label, REG_D, "JNE");
}

void CodeWriter::setFileName(const std::string& file_name)
{
    mName = file_name;
}
