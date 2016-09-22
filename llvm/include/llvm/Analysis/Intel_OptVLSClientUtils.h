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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRVLSClient.h"
#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/Debug.h"

namespace llvm {

/// Common VLS-Client Cost-model utilities.
/// The VLS-engine needs clients to provide this information in order to 
/// compute the cost of a VLS group.
class OVLSTTICostModel : public OVLSCostModelAnalysis {
protected:
  /// \brief A handle to Target Information
  const TargetTransformInfo &TTI;

  /// \brief A handle to the LLVM Context
  LLVMContext &C;

public:
  OVLSTTICostModel(const TargetTransformInfo &TTI, LLVMContext &C)
      : TTI(TTI), C(C) {}

  /// \brief Returns target-specific cost for \p I based on the
  /// Target Transformation Information (TTI) interface.
  uint64_t getInstructionCost(const OVLSInstruction *I) const;

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

/// Implementation for the HIR vectorizer client, operating on
/// HIRVLSClientMemrefs (scalar memrefs).
class OVLSTTICostModelHIR : public OVLSTTICostModel {
public:
  OVLSTTICostModelHIR(const TargetTransformInfo &TTI, LLVMContext &C)
      : OVLSTTICostModel(TTI, C) {}

  Type *getMrfDataType(const OVLSMemref *Mrf) const override {
    assert(isa<HIRVLSClientMemref>(Mrf) && "Expecting HIR Memref.\n");
    return (cast<HIRVLSClientMemref>(Mrf))->getRef()->getSrcType();
  }
  unsigned getMrfAddressSpace(const OVLSMemref &Mrf) const override;
  uint64_t getGatherScatterOpCost(const OVLSMemref &Mrf) const override;
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
}
#endif
