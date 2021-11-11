#ifndef VMTRANSLATOR_H_INCLUDED
#define VMTRANSLATOR_H_INCLUDED

#include <string>
#include "CodeWriter.h"

void translateVMFile(const std::string& name, CodeWriter& cwriter);
void translate_VM_files(const std::string& f);

#endif // VMTRANSLATOR_H_INCLUDED

