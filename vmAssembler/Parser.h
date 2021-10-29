#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <map>
#include <string>
#include <fstream>
#include <sstream>


class Parser
{
public:
    enum class Command
    {
        C_ARITHMETIC_BI,
        C_ARITHMETIC_UN,
        C_COMPARISON,
        C_PUSH,
        C_POP,
        C_LABEL,
        C_GOTO,
        C_IF,
        C_FUNCTION,
        C_CALL,
        C_RETURN,
        C_NOT_IMPLEMENTED,
    };

public:
    Parser(const std::string& fileName);

    /*
    * Returns true if open file still has lines to process
    * false otherwise.
    */
    inline bool hasMoreLines() { return !mFile.eof(); };

    /*
    * Advances the file pointer to the next command, skips
    * whitespace and comments.
    */
    void advance();
    void __advance(std::stringstream& strm);

    /*
    * Returns the type of command the current line implements.
    */
    Command commandType();
    Command peekNxtCommandType();

    /*
    * Returns the arg1 of the current line.
    */
    std::string arg1();

    /*
    * Returns the arg1 of the current line.
    */
    int arg2();

    /*
    * Returns the current line, used for implementing comments in source code.
    */
    std::string returnCommand();


private:
    std::ifstream mFile;

    /*
    * Current command.
    */
    std::stringstream mCommand;
    std::stringstream mNxtCommand;

    /*
    * Resets the stream to beginning after seeking.
    */
    void resetStream(std::stringstream& strm);
};


#endif // PARSER_H_INCLUDED
