#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "CodeWriter.h"
#include "Parser.h"
#include "Utils.h"
#include "VMTranslator.h"

namespace fs = std::filesystem;

void translateVMFile(const std::string& name)
{
    Parser parser{ name };
    CodeWriter cwriter{ name };

    std::cout << "Translating " << fs::path(name).filename().string() << '\n';

    while (parser.hasMoreLines())
    {
        parser.advance();
        auto cmd{ parser.commandType() };
        std::string arg1{};

        if (cmd != Parser::Command::C_RETURN)
            arg1 = parser.arg1();

        if (cmd == Parser::Command::C_GOTO)
        {
            cwriter.writeComment(parser.returnCommand());
            cwriter.writeGoto(arg1);
        }
        else if (cmd == Parser::Command::C_IF)
        {
            cwriter.writeComment(parser.returnCommand());
            cwriter.writeIf(arg1);
        }
        else if (cmd == Parser::Command::C_LABEL)
        {
            cwriter.writeLabel(arg1);
        }

        else if (cmd == Parser::Command::C_ARITHMETIC_BI ||
            cmd == Parser::Command::C_ARITHMETIC_UN ||
            cmd == Parser::Command::C_COMPARISON)
        {
            cwriter.writeComment(parser.returnCommand());
            cwriter.writeArithmetic(arg1);
        }
        else if (cmd == Parser::Command::C_PUSH ||
            cmd == Parser::Command::C_POP)
        {
            auto arg2{ parser.arg2() };

            if (arg1 == "constant" && parser.peekNxtCommandType() == Parser::Command::C_POP)
            {
                // This is a straight assignment syntax of assigning a
                // constant value to a place in memory so we can optimize
                // the generated code by bypassing the stack as a whole,
                // assigning the constant value to be pushed directly to memory
                // N.B. Assignment/Pushing from another value in memory creates
                // more complications and is handled the normal way.
                parser.advance();
                auto seg_pop{ parser.arg1() };
                auto arg2_pop{ parser.arg2() };
                std::stringstream s;
                s << "assignment " << arg1 << ' ' << arg2 << " to " << seg_pop << ' ' << arg2_pop;
                cwriter.writeComment(s.str());
                cwriter.opt_assignment_op(arg2, seg_pop, arg2_pop);
            }
            else
            {
                cwriter.writeComment(parser.returnCommand());
                cwriter.writePushPop(cmd, arg1, arg2);
            }
        }
    }
    cwriter.writeInfiniteLoop();

    std::cout << "Finished Translating " << fs::path(name).filename().string() << '\n';
}

void translate_VM_files(const std::string& f)
{
    std::vector<std::string> files{};

    if (fs::is_directory(fs::absolute(f)))
    {
        std::cout << "Finding '.vm' files in current directory..." << '\n' << '\n';

        for (const auto& entry : fs::recursive_directory_iterator(f))
        {
            if (!entry.is_directory() && utils::isVMFile(fs::path(entry).filename().string()))
                files.push_back(fs::absolute(entry).string());
        }
    }
    else
    {
        if (utils::isVMFile(fs::path(f).filename().string()))
            files.push_back(fs::absolute(f).string());
    }

    if (files.empty())
    {
        std::cout << "Did not find any '.vm' file in current directory..." << '\n';
        return;
    }

    for (const auto& g : files)
        translateVMFile(g);
}
