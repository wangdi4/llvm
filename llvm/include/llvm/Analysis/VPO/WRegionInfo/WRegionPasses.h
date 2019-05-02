#if INTEL_COLLAB // -*- C++ -*-
//===----- WRegionPasses.h - Constructors for WRegionInfo  ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file This header file defines prototypes for accessor functions that expose
/// passes in the WRegionInfo library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONINFO_PASSES_H
#define LLVM_ANALYSIS_VPO_WREGIONINFO_PASSES_H

namespace llvm {

class FunctionPass;

/// \brief This creates a pass that collects W-Regions.
FunctionPass *createWRegionCollectionWrapperPassPass();

/// \brief This creates a pass that forms W-Region nodes.
FunctionPass *createWRegionInfoWrapperPassPass();

}
#endif // LLVM_ANALYSIS_VPO_WREGIONINFO_PASSES_H
#endif // INTEL_COLLAB
