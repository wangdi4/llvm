//===- SPIRMetadataAdder.cpp - Add SPIR related module scope metadata -----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "CGSPIRMetadataAdder.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Module.h"
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace CodeGen;

void clang::CodeGen::addSPIRMetadata(Module &M, int OCLVersion,
                                     const std::string &SPIROptions) {
  // Add build options
  NamedMDNode *OCLCompOptsMD =
      M.getOrInsertNamedMetadata("opencl.compiler.options");
  SmallVector<llvm::Metadata *, 5> OCLBuildOptions;

  std::stringstream SOpts(SPIROptions);
  std::string SOpt;
  while (SOpts >> SOpt)
    OCLBuildOptions.push_back(MDString::get(M.getContext(), SOpt));
  OCLCompOptsMD->addOperand(MDNode::get(M.getContext(), OCLBuildOptions));
}
