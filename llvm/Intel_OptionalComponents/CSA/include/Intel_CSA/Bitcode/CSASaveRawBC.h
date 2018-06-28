//===- CSASaveRawBC.h - CSA Raw BC Interface --------------------*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// The CSASaveRawBC pass saves the IR for a module. It is intended to be as
/// close to the Bitcode emitted by the -flto option as possible.
///
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

// Declare routine to create the pass which will save a copy of the original
// IR. This is called from PassManagerBuilder.cpp to add the pass very early
// in the initialization sequence
ImmutablePass *createCSASaveRawBCPass();


class CSASaveRawBC : public ImmutablePass {
public:
  // The ID used to identify the pass. LLVM will actually use the address,
  // not the value, so it's just initialized to 0 in CSASaveRawBC.cpp
  static char ID;

  // The raw IR serialized as a BC file
  static std::string BcData;

  // Constructor
  CSASaveRawBC();

  // Initialization - Pretty much all that we have to work with. Called before
  // any module or function passes. Save away a copy of the raw (unoptimized)
  // IR so we have it if anybody wants it later
  bool doInitialization(Module &M) override;

  // Get the raw IR serialized as a BC file. Note that this is *NOT* a string!
  // It's a sequence of bytes, some of which may be 0.
  const std::string &getRawBC() const;

private:
  void dumpBC(StringRef modName);

}; // class CSASaveRawBC

}  // namespace llvm
