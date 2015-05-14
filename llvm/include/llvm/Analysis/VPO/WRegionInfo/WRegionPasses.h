//===----- WRegionPasses.h - Constructors for WRegionInfo  ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
