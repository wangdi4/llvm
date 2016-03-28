//===----- WRegionPasses.h - Constructors for WRegionInfo  ------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the WRegionInfo library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONINFO_PASSES_H
#define LLVM_ANALYSIS_VPO_WREGIONINFO_PASSES_H

namespace llvm {

class FunctionPass;

/// createWRegionCollectionPass - This creates a pass that collects W-Regions.
FunctionPass *createWRegionCollectionPass();

/// createWRegionInfoPass - This creates a pass that forms W-Region nodes.
FunctionPass *createWRegionInfoPass();

}
#endif
