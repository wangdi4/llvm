//===- llvm/Analysis/LPUSaveRawBC.h - LPU Raw BC Interface -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The LPUSaveRawBC pass saves the IR for a module. It is intended to be as
// close to the Bitcode emitted by the -flto option as possible.
//
//===----------------------------------------------------------------------===//

// Declare routine to create the pass which will save a copy of the original
// IR. This is called from PassManagerBuilder.cpp to add the pass very early
// in the initialization sequence
namespace llvm {
ImmutablePass *createLPUSaveRawBCPass();
}
