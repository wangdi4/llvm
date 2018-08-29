//===--- BlobDDRef.h - Data dependency node for blobs in HIR ----*- C++ -*-===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the BlobDDRef node in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_BLOBDDREF_H
#define LLVM_IR_INTEL_LOOPIR_BLOBDDREF_H

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/DDRef.h"

namespace llvm {

namespace loopopt {

class RegDDRef;
class HLDDNode;

/// Represents a blob present in a canonical expr of a RegDDRef
///
/// This DDRef is associated with a RegDDRef to expose data dependencies
/// present due to blobs.
class BlobDDRef final : public DDRef {
private:
  CanonExpr *CE;
  RegDDRef *ParentDDRef;

  explicit BlobDDRef(DDRefUtils &DDRU, unsigned Index, unsigned Level);
  virtual ~BlobDDRef() override {}

  /// Copy constructor used by cloning.
  BlobDDRef(const BlobDDRef &BlobDDRefObj);

  friend class RegDDRef;
  friend class DDRefUtils;
  friend class DDUtils;

  /// Sets the HLDDNode of BlobDDRef.
  void setHLDDNode(HLDDNode *HNode) override;

  /// Sets the parent DDRef of BlobDDRef.
  void setParentDDRef(RegDDRef *Ref) { ParentDDRef = Ref; }

  /// Only const-method is allowed
  HLDDNode *getHLDDNode() override;

  /// Restrict access to base class's public member. Blob DDRef can be modified
  /// to represent a different blob using the interface replaceBlob(). The
  /// symbase is automatically replaced.
  using DDRef::setSymbase;

public:
  /// Returns HLDDNode this DDRef is attached to.
  const HLDDNode *getHLDDNode() const override;

  /// Prints BlobDDRef in a simple format.
  virtual void print(formatted_raw_ostream &OS,
                     bool Detailed = false) const override;

  /// Returns the canonical form associated with the blob.
  const CanonExpr *getSingleCanonExpr() const override { return CE; }
  CanonExpr *getSingleCanonExpr() override { return CE; }

  /// Returns the blob index associated with this BlobDDRef.
  unsigned getBlobIndex() const { return CE->getSingleBlobIndex(); }

  /// Returns the RegDDRef this is attached to.
  RegDDRef *getParentDDRef() { return ParentDDRef; }
  const RegDDRef *getParentDDRef() const { return ParentDDRef; }

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef *Ref) {
    return Ref->getDDRefID() == DDRef::BlobDDRefVal;
  }

  /// clone() - Create a copy of 'this' BlobDDRef that is identical in all
  /// ways except the following:
  ///   * The Parent RegDDRef needs to be set explicitly
  BlobDDRef *clone() const override;

  /// Returns the src element type associated with this DDRef.
  Type *getSrcType() const override { return CE->getSrcType(); }
  /// Returns the dest element type associated with this DDRef.
  Type *getDestType() const override { return CE->getDestType(); }

  /// Returns true if the blob DDRef represents a self-blob like (1 * %t)
  /// which should always be true.
  bool isSelfBlob() const override { return true; }

  /// Returns true if DDRef corresponds to temp blob
  /// self-blobs are subset of terminal refs
  bool isTerminalRef() const override { return true; }

  /// Returns false because BlobDDRef is never lvalue
  bool isLval() const override { return false; }

  /// Returns true if this ref looks like 1 * undef.
  bool isStandAloneUndefBlob() const override {
    return CE->isStandAloneUndefBlob();
  }

  /// Returns false because BlobDDRef never represents a metadata.
  bool isMetadata(MetadataAsValue **Val = nullptr) const override {
    return false;
  }

  /// Returns true if this blob DDRef represents an undef blob.
  bool containsUndef() const override { return CE->containsUndef(); }

  /// Used to represent a different blob by replacing the existing blob
  /// index with the new one. Symbase is automatically updated.
  void replaceBlob(unsigned NewIndex);

  /// Returns defined at level of the blob.
  unsigned getDefinedAtLevel() const { return CE->getDefinedAtLevel(); }
  /// Sets defined at level for the blob.
  void setDefinedAtLevel(unsigned Level) { CE->setDefinedAtLevel(Level); }

  /// Returns true if blob is non-linear.
  bool isNonLinear() const { return CE->isNonLinear(); }
  /// Marks the blob as non-linear.
  void setNonLinear() { CE->setNonLinear(); }

  /// Verifies BlobDDRef integrity.
  virtual void verify() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
