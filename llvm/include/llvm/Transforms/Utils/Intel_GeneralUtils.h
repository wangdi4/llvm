//===--------- Intel_GeneralUtils.h - Class definition -*- C++ -*----------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// General purpose set of utilities that are visible within Intel_VPO and to
/// the community.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H
#define LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H

#include "llvm/ADT/SmallVector.h"

namespace llvm {

class Constant;
class Loop;
class LoopInfo;
class BasicBlock;
class Type;
class LLVMContext;

/// \brief This class provides a set of general utility functions that can be
/// used for a variety of purposes.
class IntelGeneralUtils {

public:

  /// \brief Returns a floating point or integer constant depending on Ty.
  template <typename T>
  static Constant* getConstantValue(Type *Ty, LLVMContext &Context, T Val);

  /// \brief Returns Loop in LoopInfo corresponding to the WRN's EntryBB
  static Loop* getLoopFromLoopInfo(LoopInfo* LI, BasicBlock *WRNEntryBB);

  /// \brief Generates BB set in sub CFG for a given WRegionNode.
  /// The entry basic bblock 'EntryBB' and the exit basic
  /// block 'ExitBB' are the inputs, and 'BBSet' is the output containing all
  /// the basic blocks that belong to this region. It guarantees that the
  /// first item in BBSet is 'EntryBB' and the last item is 'ExitBB'.
  static void collectBBSet(BasicBlock *EntryBB, BasicBlock *ExitBB,
                           SmallVectorImpl<BasicBlock *> &BBSet);

};

} // end llvm namespace

#endif // LLVM_TRANSFORM_UTILS_INTEL_GENERALUTILS_H
