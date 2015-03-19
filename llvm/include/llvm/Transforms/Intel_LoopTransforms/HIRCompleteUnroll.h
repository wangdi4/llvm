//===----- HIRCompleteUnroll.h - Complete unrolls a loop -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under TODO:Intel License.
//
//===---------------------------------------------------------------------===//
//
// This header file defines the complete unrolls pass for a HIR loop with
// small trip count.
//
//===---------------------------------------------------------------------===//



#ifndef LLVM_TRANSFORMS_HIRCOMPLETEUNROLL_H
#define LLVM_TRANSFORMS_HIRCOMPLETEUNROLL_H
namespace llvm {

  FunctionPass *createHIRCompleteUnrollPass(int Threshold = -1);
}
#endif
