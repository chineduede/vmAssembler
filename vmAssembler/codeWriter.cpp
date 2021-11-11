#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <exception>

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
constexpr char PLUS = '+';
constexpr char MINUS = '-';

const char* REG_SP = "SP";
const char* REG_R13 = "R13";
const char* REG_R14 = "R14";
const char* REG_LOCAL = "LCL";
const char* REG_ARG = "ARG";
const char* REG_THIS = "THIS";
const char* REG_THAT = "THAT";

const char* EMPTY = "";

const char* SEG_HIDDEN = "hidden";
const char* SEG_FRAME = "frame";

CodeWriter::CodeWriter(const std::string& name)
    : mFile{ name }
    , mName{ EMPTY }
{
    if (!mFile)
        throw std::exception{ "Could not open file." };
    init();

}


void CodeWriter::init()
{
    wrtBaseCmd(256, REG_D, REG_A);
    wrtBaseCmd(REG_SP, REG_M, REG_D);
    //wrtBaseCmd("Sys.init", ZERO, "JMP");
    writeCall("Sys.init", 0);
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
        wrtBaseCmd(REG_SP, REG_M, REG_M, MINUS, '1');

        if (found != utils::segmentMap.end())
        {
            accessIdxAddrOrLdMem(index, found->second);
            wrtBaseCmd(REG_R13, REG_M, REG_D);

            wrtBaseCmd(REG_SP, REG_A, REG_M);
            wrtBaseCmd(EMPTY, REG_D, REG_M, false);

            wrtBaseCmd(REG_R13, REG_A, REG_M);
            wrtBaseCmd(EMPTY, REG_M, REG_D, false);
        }

        else
        {
            wrtBaseCmd(REG_SP, REG_A, REG_M);
            wrtBaseCmd(EMPTY, REG_D, REG_M, false);

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
            wrtBaseCmd(EMPTY, REG_A, REG_D, false);
            wrtBaseCmd(EMPTY, REG_D, REG_M, false);
        }
        else if (!ld_frm_d)
        {
            std::string temp{ directQualName(index, segment) };

            if (segment == "constant")
                wrtBaseCmd(index, REG_D, REG_A);
            else if (segment.empty())
                wrtBaseCmd(segment, REG_D, REG_A);
            else
                wrtBaseCmd(temp, REG_D, REG_M);
        }

        wrtBaseCmd(REG_SP, REG_A, REG_M);
        wrtBaseCmd(EMPTY, REG_M, REG_D, false);
        // Increment stack pointer
        wrtBaseCmd(REG_SP, REG_M, REG_M, PLUS, '1');
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
        wrtBaseCmd(seg, REG_D, REG_M, PLUS, REG_D);
    }
}

void CodeWriter::implementArith(const std::string& sign, bool binary_op, const std::string& cmp_sign)
{
    static int comp_sign_counter{};
    std::string r13{ REG_R13 };

    if (binary_op)
        wrtBaseCmd(REG_R14, REG_D, REG_D, sign.front(), REG_M);
    else
        wrtBaseCmd(EMPTY, REG_D, sign.front(), REG_D, false);

    if (!cmp_sign.empty())
    {
        std::string compLabel{ "COMP_" + cmp_sign + "_" + std::to_string(comp_sign_counter) };
        std::string exitCompLabel{ "EXIT_" + compLabel };

        wrtBaseCmd(compLabel, REG_D, cmp_sign);

        wrtBaseCmd(r13, REG_D, ZERO);

        wrtBaseCmd(exitCompLabel, ZERO, "JMP");

        mFile << BRAC_OP << compLabel << BRAC_CLE << '\n';
        wrtBaseCmd(r13, REG_D, MINUS, '1');
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
        temp = (index) ? REG_THAT : REG_THIS;
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
            wrtBaseCmd(EMPTY, REG_M, REG_D, false);
        }
        else
            wrtBaseCmd(tempName, REG_M, REG_D);
        return;
    }

    wrtBaseCmd(REG_R13, REG_M, REG_D);
    wrtBaseCmd(const_val, REG_D, REG_A);
    wrtBaseCmd(REG_R13, REG_A, REG_M);
    wrtBaseCmd(EMPTY, REG_M, REG_D, false);
}

std::string CodeWriter::__gen_label_name(const std::string& label, bool add_prefix)
{
    return (add_prefix) ? std::string{currFunctionName + "$" + label} : std::string{label};
}

void CodeWriter::writeLabel(const std::string& label, bool add_prefix)
{
    mFile << BRAC_OP << __gen_label_name(label, add_prefix) << BRAC_CLE << '\n';
}

void CodeWriter::writeGoto(const std::string& label, bool add_prefix)
{
    wrtBaseCmd(__gen_label_name(label, add_prefix), ZERO, "JMP");
}

void CodeWriter::writeIf(const std::string& label, bool add_prefix)
{
    wrtBaseCmd(REG_SP, REG_M, REG_M, MINUS, '1');
    wrtBaseCmd(REG_SP, REG_A, REG_M);
    wrtBaseCmd(EMPTY, REG_D, REG_M, false);
    wrtBaseCmd(__gen_label_name(label, add_prefix), REG_D, "JNE");
}

void CodeWriter::setFileName(const std::string& file)
{
    mName = fs::path(file).filename().replace_extension().string();
}

void CodeWriter::writeFunction(const std::string& func_name, int nVars)
{
    currFunctionName = func_name;
    writeLabel(func_name, false);
    for (int i = 0; i < nVars; i++)
    {
        wrtBaseCmd(REG_SP, REG_A, REG_M);
        wrtBaseCmd(EMPTY, REG_M, '0', false);
        wrtBaseCmd(REG_SP, REG_M, REG_M, PLUS, '1');
    }
}

void CodeWriter::writeReturn()
{
    // Pushes LCL pointer to a temporary variable
    wrtBaseCmd(REG_LOCAL, REG_D, REG_M);
    wrtBaseCmd(SEG_FRAME, REG_M, REG_D);

    // Pushes return address to a temporary variable
    __restorePointer("ret", 5);

    //Pops the topmost value of the stack into the caller argument
    writePushPop(Parser::Command::C_POP, "argument", 0);

    //Repositions stack pointer for the caller
    wrtBaseCmd(REG_ARG, REG_D, REG_M, PLUS, '1');
    wrtBaseCmd(REG_SP, REG_M, REG_D);

    const char* reg_temp[]{ REG_THAT, REG_THIS, REG_ARG, REG_LOCAL };

    for (int i = 0; i < 4; ++i)
        __restorePointer(reg_temp[i], i + 1);

    wrtBaseCmd("ret", REG_A, REG_M);
    wrtBaseCmd(EMPTY, '0', "JMP", false);
}

void CodeWriter::writeCall(const std::string& func_name, int nVars)
{
    static int retStaticVar{};
    //Add return address to global stack
    std::string label{ func_name + "$ret." + std::to_string(retStaticVar++) };
    __push(label);

    //pushes LCL, ARG, THIS, THAT to the global stack
    const char* reg_temp[]{ REG_LOCAL, REG_ARG, REG_THIS, REG_THAT };

    for (int i = 0; i < 4; ++i)
        __push(reg_temp[i]);

    //repositions ARG for the caller
    wrtBaseCmd(5, REG_D, REG_A);
    if (nVars != 0)
        wrtBaseCmd(nVars, REG_D, REG_D, PLUS, REG_A);
    wrtBaseCmd(REG_SP, REG_D, REG_M, MINUS, REG_D);
    wrtBaseCmd(REG_ARG, REG_M, REG_D);
  
    //repositions LCL for the caller
    wrtBaseCmd(REG_SP, REG_D, REG_M);
    wrtBaseCmd(REG_LOCAL, REG_M, REG_D);

    //jump to function and add return label
    writeGoto(func_name, false);
    writeLabel(label, false);
}

void CodeWriter::__push(const char* segment, bool ret_addr)
{
    if (ret_addr) 
        wrtBaseCmd(segment, REG_D, REG_A);
    else 
        wrtBaseCmd(segment, REG_D, REG_M);
    wrtBaseCmd(REG_SP, REG_A, REG_M);
    wrtBaseCmd(EMPTY, REG_M, REG_D, false);
    // Increment stack pointer
    wrtBaseCmd(REG_SP, REG_M, REG_M, PLUS, '1');
}

void CodeWriter::__push(const std::string& segment)
{
    __push(segment.c_str(), true);
}

void CodeWriter::__restorePointer(const char* segment, int index)
{
    wrtBaseCmd(index, REG_D, REG_A);
    wrtBaseCmd(SEG_FRAME, REG_A, REG_M, MINUS, REG_D);
    wrtBaseCmd(0, REG_D, REG_M, false);
    wrtBaseCmd(segment, REG_M, REG_D);
}

