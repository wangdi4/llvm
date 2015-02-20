//===-------- HIRCodeGen.h - HIR to LLVM IR conversion  ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the TODO
// License.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose the 
// HIRCG pass in the Intel Transforms library. 
//
//===----------------------------------------------------------------------===//



#ifndef LLVM_TRANSFORMS_HIRCG_H
#define LLVM_TRANSFORMS_HIRCG_H
namespace llvm {

  class Loop;
  class LoopInfo;
  class ScalarEvolution;
  class SCEV;
  class SCEVConstant;
FunctionPass *createHIRCodeGenPass();
}
#endif
