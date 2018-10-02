//===--- LoopOptReport.h ----------------------------------------*- C++ -*-===//
//
// Copyright (C) 2018-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares LoopOptReport class.
//
// Detailed description is located at: docs/Intel/OptReport.rst
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_LOOPOPTREPORT_H
#define LLVM_ANALYSIS_LOOPOPTREPORT_H

#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Metadata.h"

namespace llvm {

class DILocation;
class LoopOptReportRemark;

struct LoopOptReportTag {
  static constexpr const char *Root = "llvm.loop.optreport";
  static constexpr const char *Proxy = "intel.loop.optreport";
  static constexpr const char *DebugLoc = "intel.optreport.debug_location";
  static constexpr const char *Origin = "intel.optreport.origin";
  static constexpr const char *Remarks = "intel.optreport.remarks";
  static constexpr const char *Remark = "intel.optreport.remark";
  static constexpr const char *NextSibling = "intel.optreport.next_sibling";
  static constexpr const char *FirstChild = "intel.optreport.first_child";
};

/// \brief Helper for representing a single OptReport remark.
///  Example usecase:
///
///  LoopOptReport OptReport;
///  for (const LoopOptReportRemark R: OptReport.remarks()) { ... }
///
///  It serves to abstract the format of the underlying metadata. Particularly,
///  it checks that the first (idx 0) operand is the LoopOptReportTag::Remark
///  and getOperand interface of this class skips this tag and starts counting
///  directly from the first actual argument of the remark.
class LoopOptRemark {
  MDTuple *Remark;

public:
  LoopOptRemark(MDTuple *R = nullptr) : Remark(R) {
    assert((!R || isOptRemarkMetadata(R)) && "Bad remark metadata");
  }
  LoopOptRemark(const MDOperand &R) : LoopOptRemark(cast<MDTuple>(R)) {}

  template <typename... Args>
  static LoopOptRemark get(LLVMContext &Context, Args &&... args) {
    SmallVector<Metadata *, 4> Ops;
    populateMDTupleOperands(Ops, Context, LoopOptReportTag::Remark,
                            std::forward<Args>(args)...);
    MDTuple *Tuple = MDTuple::get(Context, Ops);
    return Tuple;
  }

  const Metadata *getOperand(unsigned Idx) const;
  unsigned getNumOperands() const;

  explicit operator bool() const { return Remark; }
  MDTuple *get() const { return Remark; }

  static bool isOptRemarkMetadata(const Metadata *R);

private:
  static void populateMDTupleOperands(SmallVectorImpl<Metadata *> &V,
                                      LLVMContext &C) {
    return;
  }

  template <typename... Args>
  static void populateMDTupleOperands(SmallVectorImpl<Metadata *> &V,
                                      LLVMContext &C, StringRef S,
                                      Args &&... args) {
    V.push_back(MDString::get(C, S));
    populateMDTupleOperands(V, C, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void populateMDTupleOperands(SmallVectorImpl<Metadata *> &V,
                                      LLVMContext &C, int I, Args &&... args) {
    ConstantInt *CI = ConstantInt::get(Type::getInt32Ty(C), I);
    V.push_back(ConstantAsMetadata::get(CI));
    populateMDTupleOperands(V, C, std::forward<Args>(args)...);
  }
};

/// \brief Helper class to simplify processing of OptReport metadata.
///
/// Basically, it is simply a wrapper around a pointer to MDTuple, but it adds a
/// few convenient methods to populate optreports and to read its content.
///
/// Implementation of optimization reports is based on MDTuple class. It is
/// important to keep in mind that MDTuple does not support resizing. That is,
/// operations like adding a remark invalidate old tuple of remarks and create a
/// new one. That's why LoopOptReport keeps a pointer to the root node of
/// optimization report which points to an additional proxy node and looks like
///     !{!"llvm.loop.optreport", !0}
/// The root node is never extended (pointer is never invalidated), but its
/// operand is replaced after some operations. That is, pointers to various
/// internal fields of OptReports may be invalidated by any instance of
/// LoopOptReport and shouldn't be stored across invocations. Only root node is
/// safe to keep.
///
/// A proper solution to the problem of invalidated metadata would be to add a
/// new type of resizeable metadata to IR (MDList), but it looks like an
/// overkill for now.
class LoopOptReport {
  MDTuple *OptReport;

public:
  LoopOptReport(MDTuple *OR = nullptr) : OptReport(OR) {
    assert(!OptReport || isOptReportMetadata(OR));
  }
  static LoopOptReport createEmptyOptReport(LLVMContext &Context);

  using op_iterator = MDTuple::op_iterator;
  using op_range = MDTuple::op_range;

  // Setters.
  void addOrigin(LoopOptRemark Origin) const;
  void setDebugLoc(DILocation *Location) const;
  void addRemark(LoopOptRemark Remark) const;
  void addChild(LoopOptReport Child) const;
  void addSibling(LoopOptReport Sibling) const;
  void eraseSiblings() const;

  // Getters.
  op_range origin() const;
  const DILocation *debugLoc() const;
  op_range remarks() const;
  const LoopOptReport nextSibling() const;
  const LoopOptReport firstChild() const;

  explicit operator bool() const { return OptReport; }
  MDTuple *get() const { return OptReport; }

  /// \brief Checks if metadata is an instance of LoopOptReport.
  ///
  /// Checks if metadata \M is a tuple tagged with LoopOptReportTag::Root.
  static bool isOptReportMetadata(const Metadata *M) {
    const MDTuple *T = dyn_cast<MDTuple>(M);
    if (!T)
      return false;

    if (T->getNumOperands() == 0)
      return false;

    MDString *S = dyn_cast<MDString>(T->getOperand(0));
    if (!S)
      return false;

    return S->getString() == LoopOptReportTag::Root;
  }


  /// \brief Construct LoopOptReport object from LoopID.
  ///
  /// Traverses LoopID metadata looking for OptReport. Null is returned if no
  /// OptReport is found or if \p LoopID is null.
  static LoopOptReport findOptReportInLoopID(MDNode *LoopID);

  /// \brief Remove OptReport from LoopID metadata.
  ///
  /// Creates a copy of \p LoopID metadata with OptReport excluded. Returns
  /// nullptr if OptReport is the only entry in LoopID or if \p LoopID is null.
  /// Otherwise, returns a pointer to the new LoopID metadata.
  static MDNode *eraseOptReportFromLoopID(MDNode *LoopID, LLVMContext &C);

  /// \brief Attach OptReport to LoopID metadata.
  ///
  /// Creates a copy of \p LoopID metadata with OptReport added to it. Creates a
  /// new LoopID metadata from scratch if nullptr is passed as the old LoopID.
  /// Returns a pointer to the new LoopID metadata.
  static MDNode *addOptReportToLoopID(MDNode *LoopID, LoopOptReport OR,
                                      LLVMContext &C);
};

} // namespace llvm

#endif // LLVM_ANALYSIS_LOOPOPTREPORT_H
