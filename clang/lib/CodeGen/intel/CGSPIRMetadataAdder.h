//===- SPIRMetadataAdder.h - Add SPIR related module scope metadata -------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Module.h"
#include <string>

#ifndef CLANG_CODEGEN_SPIRMETADATAADDER_H
#define CLANG_CODEGEN_SPIRMETADATAADDER_H

namespace clang {

namespace CodeGen {

  void addSPIRMetadata(llvm::Module &M, int OCLVersion,
                       std::string SPIROptions);

} // end namespace CodeGen
} // end namespace clang
#endif
