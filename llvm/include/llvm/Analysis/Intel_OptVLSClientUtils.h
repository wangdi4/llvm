/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//===---------------- Intel_OptVLSClientUtils.h ------------------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// Utilities that multiple VLS clients can use.
///
//===---------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTVLSCLIENTUTILS_H
#define LLVM_ANALYSIS_INTEL_OPTVLSCLIENTUTILS_H

#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/IRBuilder.h"

namespace llvm {

/// Common VLS-Client Cost-model utilities.
/// The VLS-engine needs clients to provide this information in order to
/// compute the cost of a VLS group.
class OVLSTTICostModel : public OVLSCostModel {
public:
  OVLSTTICostModel(const TargetTransformInfo &TTI, LLVMContext &C)
      : OVLSCostModel(TTI, C) {}

  /// \brief Returns target-specific cost for \p I based on the
  /// Target Transformation Information (TTI) interface.
  uint64_t getInstructionCost(const OVLSInstruction *I) const override;

  /// \brief Return the internal data type accessed by this memory reference
  // CHECKME: Move this to OVLSMemref?
  virtual Type *getMrfDataType(const OVLSMemref *Mrf) const = 0;

  /// \brief Return the address space accessd by this memref
  virtual unsigned getMrfAddressSpace(const OVLSMemref &Mrf) const = 0;

  /// \brief Helper routine that creates a vector data-type whose number of
  /// elements is as in \p VLSType, and the type of the elements is \p ElemType
  /// if it is not null. It \p ElemType is null this routine will use the
  /// element size as in \p VLSType to create the vector type. (This is
  /// needed for TTI routines such as getMemoryOpCost and getShuffleCost that
  /// require the vector data type).
  Type *getVectorDataType(Type *ElemType, OVLSType &VLSType) const;
};

/// Empty implementation for the (yet non existant) LLVM vectorizer client
/// (which will also operate on scalar Memrefs).
// TODO.
class OVLSTTICostModelLLVMIR : public OVLSTTICostModel {
public:
  OVLSTTICostModelLLVMIR(const TargetTransformInfo &TTI, LLVMContext &C)
      : OVLSTTICostModel(TTI, C) {}

  // TODO
  Type *getMrfDataType(const OVLSMemref *Mrf) const override { return nullptr; }

  // TODO
  unsigned getMrfAddressSpace(const OVLSMemref &Mrf) const override {
    return 0;
  }

  // TODO
  uint64_t getGatherScatterOpCost(const OVLSMemref &Mrf) const override {
    return 0;
  }
};

/// Facilitates conversion between OVLSInstruction and LLVM-IR Instruction.
class OVLSConverter {
public:
  /// \brief Generates LLVM-IR instructions for a set of OVLSInstructions.
  ///
  /// This function generates corresponding LLVM_IR instruction for
  /// OVLSInstructions in \p InstVec. It traverses the \p InstVec from lower
  /// to higher index. Whenever there is a need to generate memory instructions
  /// it uses the pointer address given in \p Addr and the element type in \p
  /// ElemTy. The operands for a generated shufflevector instruction can either
  /// come for a load Instruction or from the passed \p InterleavingShuffleInst.
  /// The latter is used in case of scatters, when the temp from the \p
  /// InterleavingShuffleInst is stored to \p Addr. The function returns a
  /// mapping from OVLSInstruction to the generated LLVM-IR instruction.
  /// TODO: Support heterogeneous elem-types.
  static DenseMap<uint64_t, Value *>
  genLLVMIR(IRBuilder<> &Builder, const OVLSInstructionVector &InstVec,
            ShuffleVectorInst *InterleavingShuffleInst, Value *Addr,
            Type *ElemTy, unsigned Alignment);
};
}
#endif
