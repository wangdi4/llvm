//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanSLP class which incorporates SLP related methods
/// that are run on top of VPlan IR.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm {

namespace vpo {

class VPBasicBlock;
class VPValue;
class VPInstruction;
class VPInstructionCost;
class VPlanTTICostModel;
class VPLoadStoreInst;

// VPlanSLPNodeTy defines the types of vectors VSLP detects and is able to
// estimate the cost of vectorization for.
using VPlanSLPNodeTy = enum {
  NotVector,        // Not a vector. Not vectorizable.
  ConstVector,      // Vector of immediate values of the same type.
  SplatVector,      // Vector of the very same scalar value broadcasted to all
                    // lanes.
  InstVector,       // Vector of (vectorizable) VPInstructions of the same
                    // opcode.
  InstShuffleVector // Vector of instructions of two different opcodes, so
                    // an extra shuffle is needed to vectorize it.
};

class VPlanSLP {
  // VPlanSLPNodeElement is mostly a wrapper for VPValue. It contains VPValue
  // reference as data member as well as some additional data that is used for
  // SLP modelling.
  // Specifically, it has extra opcode value which effectively overrides the
  // opcode value of associated VPValue/VPInstruction. It is used to implement
  // transformations on top of VPlan IR w/o modifing IR.
  class VPlanSLPNodeElement {
    const VPValue *Value;
    const VPValue *Op[2];
    unsigned AltOpcode;

  public:
    VPlanSLPNodeElement(const VPValue *Value) : Value(Value) {
      setOpsAndAltOpcode(nullptr /* Op[0] */, nullptr /* Op[1] */,
                         0 /* AltOpcode */);
    }
    ~VPlanSLPNodeElement() = default;

    const VPValue *getValue() const { return Value; }
    unsigned getOpcode() const {
      if (AltOpcode != 0)
        return AltOpcode;
      return cast<VPInstruction>(Value)->getOpcode();
    }

    // Trivial setter for private fields.
    void setOpsAndAltOpcode(const VPValue *Op0, const VPValue *Op1,
                            unsigned Opcode) {
      AltOpcode = Opcode;
      Op[0] = Op0;
      Op[1] = Op1;
    }

    /// getOperand() is a proxy for VPUser::getOperand() method. Thereby it
    /// asserts that 'Value' is of VPInstruction type.
    ///
    /// VPlanSLPNodeElement::getOperand() can return values different from what
    /// VPUser::getOperand() returns.
    ///
    /// For example it happens if a + b * (-1) is folded into a - b virtually.
    /// In this case AltOpcode is set to '-', Op[0] is set to 'a' and Op[1] is
    /// set to 'b' in the node corresponding to '+'.
    const VPValue *getOperand(unsigned N) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    // To call from gdb and for debugging dumps.
    void dump(raw_ostream &OS) const;
    void dump() const;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  };

  // Shorthands for some frequently used types.
  using ElemVectorImplTy = SmallVectorImpl<VPlanSLPNodeElement>;
  using ElemVectorTy = SmallVector<VPlanSLPNodeElement, 32>;
  using ElemArrayRef = ArrayRef<VPlanSLPNodeElement>;
  using ElemMutableArrayRef = MutableArrayRef<VPlanSLPNodeElement>;

  VPlanTTICostModel *CM;
  // Only instructions belonging BB are subject for SLP.
  const VPBasicBlock *BB;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Used in debugging dumps only.
  unsigned BundleID;
  unsigned GraphID;
  // Formatted print of 'Values' into dbgs() stream.
  static void printVector(ElemArrayRef);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // TODO:
  // We might want to introduce a new class to represent SLP Node and move some
  // methods from below to the new class.

  // Tries to find values in InValues array that are the same and copies those
  // values in OutValues. Return true if 2 or more same values are found.
  //
  // NOTE:
  // There can be more than one splat vector in InValues. The utility can detect
  // only one and the caller handles the rest elements.
  bool getSplatVector(ElemArrayRef InValues, ElemVectorImplTy &OutValues) const;

  // Picks all values in InValues array that are constants and copies those
  // values in OutValues. Return true if 2 or more same values are found.
  //
  // NOTE:
  // The method ignores the type of the constants. Checking the type complicates
  // the code significantly since several groups of constants of different types
  // are allowed in InValues. However, we expect those constants to be arguments
  // to instructions that are vectorizable, i.e. of the same opcode and thereby
  // the same type of their arguments. The exception here could be vectorizable
  // scalar instructions that have two or more immediate arguments of different
  // type.
  bool getConstVector(ElemArrayRef InValues, ElemVectorImplTy &OutValues) const;

  // Return true if at least two values from InValues array can be represented
  // with a vector instruction.
  //
  // The utility populates OutValues with vectorizable VPInstructions.
  bool getVecInstsVector(ElemArrayRef InValues,
                         ElemVectorImplTy &OutValues) const;

  // Selects values from InValues array that can be vectorized legally and
  // copy those values into OutValues array.
  //
  // Returns the type of vector detected. OutValues is guaranteed to contain two
  // elements at least if returned vector type is anything else but NotVector.
  // OutValues is not valid to read if return value is NotVector.
  //
  // It is caller responsibility to handle other elements of InValues that are
  // not copied into OutValues but may construct another vector yet.
  VPlanSLPNodeTy getVectorizableValues(ElemArrayRef InValues,
                                       ElemVectorImplTy &OutValues) const;

  // Returns true iff VPLoadStoreInst FromInst can be moved to ToInst.
  // TODO: It might be duplication of VPVLSClientMemref::canMoveTo().
  // We want these utilities to be merged once VPlanSLP matures.
  bool canMoveTo(const VPLoadStoreInst *FromInst,
                 const VPLoadStoreInst *ToInst) const;

  // The method swaps elements from Op1 with elements from Op2 on the same index
  // position in order to make vectorization of Op1 and/or Op2 more probable or
  // profitable.
  void tryReorderOperands(ElemMutableArrayRef Op1,
                          ElemMutableArrayRef Op2) const;

  // Tries to build an SLP tree starting at 'Seed' list and returns the
  // difference (vectorized_graph_cost - scalar_graph_cost). Thereby if the
  // cost is negative, it is profitable to vectorize. If the cost is Invalid,
  // the graph is not vectorizable with SLP.
  VPInstructionCost buildGraph(ElemArrayRef Seed);

  // Searches for SLP patters that start at 'Seed' as seed instructions.
  // The utility modifies input Seed vector sorting it and it is truncated
  // at function's exit.
  //
  // Return the sum of the gain costs of profitable to SLP subgraphs only.
  VPInstructionCost searchSLPPatterns(ElemVectorImplTy &Seed);

  // Utility collects the constant distances between Memrefs in Values array
  // and 'BaseMem' memory reference.
  //
  // Only constant distances are collected. Only load and stores are expected
  // in Insts array. Every load/store in input array is expected to have
  // DDRef node associated.
  static void collectMemRefDistances(const VPLoadStoreInst *BaseMem,
                                     ElemArrayRef Values,
                                     SmallVectorImpl<ssize_t> &Distances);

  // The utility sorts 'Distances' array and checks if it is indexes for
  // consequent in memory elements. Such, it return true for
  // 0, 1, 2, 3
  // -2, -1, 0, 1
  // 3, 2, 1, 0
  //
  // but false for:
  // 0, 2, 4, 6
  // 3, 2, 1, -1
  //
  // The number of elements is an arbitrary (it is not nessesary power of two).
  bool isUnitStrideMemRef(SmallVectorImpl<ssize_t> &Distances) const;

  // Calculates the cost of a vector instruction that would be produced from
  // scalar instruction Base extending it for VF elements. If Base is a memory
  // reference 'IsUnitMemref' indicates when the extention of the memory
  // results a whole chunk of memory access. VF which is a power of two is
  // supported only.
  // Non power-of-two memref size is indicated with IsMasked.
  VPInstructionCost getVectorCost(const VPInstruction *Base, unsigned VF,
                                  bool IsUnitMemref, bool IsMasked) const;

  // Returns the difference (vectorized_cost - scalar_cost), where
  // scalar_cost - the sum of all costs of scalar instructions to represent
  // Values in input array,
  // vectorized_cost - the cost of vector instruction to represent the vector
  // value that is constructed from the scalar Values.
  VPInstructionCost estimateSLPCostDifference(ElemArrayRef Values,
                                              VPlanSLPNodeTy NType) const;

  // The method tries to bundle 'similar' instructions in groups and see if
  // those groups are profitable to SLP. The group is discarded (not stored
  // in OutSeed) if it is vectorizable. Not vectorized groups are stored in
  // OutSeed array if OutSeed is not null.
  //
  // 'similarity' of instructions inside InSeed is determined by Compare
  // comparator, which return true when two instructions are similar enough to
  // have a potential for vectorization in a single vector. Please note that
  // this is a quick heuristics based check rather than a legality check.
  //
  // The costs of profitable to vectorize groups is accumulated and returned.
  VPInstructionCost
  formAndCostBundles(ElemArrayRef InSeed,
                     std::function<bool(const VPlanSLPNodeElement &,
                                        const VPlanSLPNodeElement &)>
                         Compare,
                     ElemVectorImplTy *OutSeed = nullptr);

public:
  VPlanSLP(VPlanTTICostModel *CM, const VPBasicBlock *BB)
    : CM(CM), BB(BB)
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      ,
      BundleID(0), GraphID(0)
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  {}

  ~VPlanSLP() = default;

  // Tries to detect SLP patterns and return the cost gain if found. The
  // result is either a negative cost, which is a gain for SLP, or 0 if
  // no pattern is found.
  VPInstructionCost estimateSLPCostDifference();
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSLP_H
