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

#include "llvm/CodeGen/AsmPrinterHandler.h"
#include <memory>

namespace llvm {

class AsmPrinter;

//===----------------------------------------------------------------------===//
// STIDebug
//
// \brief Interface to emit Symbol and Type Information For Debugging.
//
//===----------------------------------------------------------------------===//

class STIDebug : public AsmPrinterHandler {
public:
  static std::unique_ptr<STIDebug> create(AsmPrinter *Asm);

  virtual ~STIDebug();

  virtual void setSymbolSize(const MCSymbol *Symbol, uint64_t size) override = 0 ;
  virtual void endModule() override = 0;
  virtual void beginFunction(const MachineFunction *MF) override = 0;
  virtual void endFunction(const MachineFunction *MF) override = 0;
  virtual void beginInstruction(const MachineInstr *MI) override = 0;
  virtual void endInstruction() override = 0;

protected:
  STIDebug();
};

} // End of namespace llvm

#endif
