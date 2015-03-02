//===-- STIDebug.h - Debug Symbol And Type Information -*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file contains support for writing symbol and type information
// compatible with Visual Studio.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_STIDEBUG_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_STIDEBUG_H

#include "../AsmPrinterHandler.h"

namespace llvm {

class AsmPrinter;

//===----------------------------------------------------------------------===//
// STIDebug
//
// \brief Interface to emit Symbol and Type Information For Debugging.
//
//===----------------------------------------------------------------------===//

class STIDebug : public AsmPrinterHandler {
private:
  AsmPrinter *_Asm;

public:
  static STIDebug *create(AsmPrinter *Asm);

  virtual ~STIDebug();

  virtual void setSymbolSize(const MCSymbol *Symbol, uint64_t size) = 0;
  virtual void endModule() = 0;
  virtual void beginFunction(const MachineFunction *MF) = 0;
  virtual void endFunction(const MachineFunction *MF) = 0;
  virtual void beginInstruction(const MachineInstr *MI) = 0;
  virtual void endInstruction() = 0;

protected:
  STIDebug();
};

} // End of namespace llvm

#endif
