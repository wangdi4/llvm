//===-- MICCodeGenerationEngine.h - Drive MIC JIT code gen ------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains interface for driving JIT code generation of full
// module. Used for MIC.
//
//===----------------------------------------------------------------------===//

#ifndef __MIC_CODE_GENERATION_ENGINE__H__
#define __MIC_CODE_GENERATION_ENGINE__H__

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CodeGen.h"

#include "stddef.h"

#include <string>

namespace llvm {
class IFunctionAddressResolver;
class LLVMModuleJITHolder;
class Module;
class TargetMachine;
class Type;

//===--------------------------------------------------------------------===//
// MICCodeGenerationEngine - Drives code generation by the target machine
//
class MICCodeGenerationEngine {
public:

 static TargetMachine *selectTarget(Module *Mod,
                              StringRef MArch,
                              StringRef MCPU,
                              const SmallVectorImpl<std::string>& MAttrs,
                              Reloc::Model RM,
                              CodeModel::Model CM,
                              std::string *ErrorStr);


  MICCodeGenerationEngine(TargetMachine &tm, CodeGenOpt::Level optlvl, const IFunctionAddressResolver* resolver);

  virtual ~MICCodeGenerationEngine();

  /////////////////////////////////////////
  //Return: the size(in bits) of the given type
  /////////////////////////////////////////
  size_t sizeOf(llvm::Type* t) const;

  /////////////////////////////////////////////////////////////////////
  //Return: A ModuleJITHolder which holds the JIT code and other 
  //metadata corresponding the given module
  //Note: the returned value is a heap-allocated object.
  //The ownership on this object is transfered to the caller of that method.
  /////////////////////////////////////////////////////////////////////
  LLVMModuleJITHolder* getModuleHolder(llvm::Module& m, const std::string& outAsmFile) const;

private:
  TargetMachine &TM;
  CodeGenOpt::Level optLevel;
  const IFunctionAddressResolver* Resolver;

};

}

#endif //__MIC_CODE_GENERATION_ENGINE__H__
