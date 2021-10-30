#ifndef CODEWRITER_H_INCLUDED
#define CODEWRITER_H_INCLUDED

#include <fstream>
#include <string>

#include "Parser.h"

class CodeWriter
{
public:
    CodeWriter(const std::string& name);
    /*
    * Writes the resulting arithmetic operation to the file using
    * a series of popping and pushing values to the stack
    */
    void writeArithmetic(const std::string& cmd);


    /*
    * Implements unconditional goto jump, conditional goto jump
    * and inserts labels in the assembly stream.
    */
    void writeGoto(const std::string& label);
    void writeLabel(const std::string& label);
    void writeIf(const std::string& label);

    /*
    * Implements the push and pop syntax.
    */
    void writePushPop(Parser::Command cmd, const std::string& segment, int index, bool ld_frm_d = false);

    /*
    * Generates a more optimize push pop assignment command. Better for assignment
    * when a push command is followed immediately by a pop command.
    */
    void opt_assignment_op(int const_val, const std::string& segment, int index);

    /*
    * Closes the file after writing.
    */
    void close();

    void setFileName(const std::string& file_name);

    /*
    * Writes the stack instruction to file as a comment.
    */
    inline void writeComment(const std::string& str) { mFile << "// " << str << '\n'; }

    /*
    * Ends the whole program by writing an infinite loop.
    */
    inline void writeInfiniteLoop() { mFile << "(INFINITE_LOOP)\n@INFINITE_LOOP\n0;JMP"; }

private:
    /*
    * Opens a file for writing to.
    */
    std::ofstream mFile;

    /*
    * Name of the opened file.
    */
    std::string mName;

    /*
    * Push constant to the stack or access
    * an indexed address from where LCL, ARG,
    * THIS. THAT points to.
    */
    void accessIdxAddrOrLdMem(int index, const std::string& seg);

    /*
    * Generates identifiers for static variables by concatenating
    * the file name and the number separated by a '.'. Also generates
    * identifiers for temp, hidden and pointer access.
    */
    std::string directQualName(int index, const std::string& segment);

    /*
    * Implements every possible combinations of Hack assembly commands
    * using overloaded functions.
    */
    void wrtBaseCmd(const std::string& seg, char to, char from, bool ld_seg = true);
    void wrtBaseCmd(int seg, char to, char op1, char op, char op2, bool ld_seg = true);
    void wrtBaseCmd(int seg, char to, char from, bool ld_seg = true);
    void wrtBaseCmd(const std::string& seg, char to, char op1, char op, char op2, bool ld_seg = true);
    void wrtBaseCmd(const std::string& seg, char comp_val, const std::string& comp_op, bool ld_seg = true);
    void wrtBaseCmd(const std::string& seg, char to, char op, char op1, bool ld_seg = true);

    /*
    * Implements a binary operation on two operands.
    */
    void implementArith(const std::string& sign, bool binary = true, const std::string& cmp_sign = "");

};

#endif // CODEWRITER_H_INCLUDED
