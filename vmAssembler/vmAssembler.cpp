#include <iostream>

#include "VMTranslator.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
        std::cout << "Usage: " << argv[0] << "   <filename>\n";
    else
        translate_VM_files({ argv[1] });

    return 0;
}
