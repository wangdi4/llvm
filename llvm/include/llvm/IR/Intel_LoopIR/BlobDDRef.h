//===--- BlobDDRef.h - Data dependency node for blobs in HIR ----*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/IR/Intel_LoopIR/DDRef.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

namespace llvm {

namespace loopopt {

class RegDDRef;
class HLDDNode;

/// \brief Represents a blob present in a canonical expr of a RegDDRef
///
/// This DDRef is associated with a RegDDRef to expose data dependencies
/// present due to blobs.
class BlobDDRef final : public DDRef {
private:
  CanonExpr *CE;
  RegDDRef *ParentDDRef;

protected:
  explicit BlobDDRef(unsigned Index, unsigned Level);
  virtual ~BlobDDRef() override {}

  /// \brief Copy constructor used by cloning.
  BlobDDRef(const BlobDDRef &BlobDDRefObj);

  friend class RegDDRef;
  friend class DDRefUtils;

  /// \brief Sets the HLDDNode of BlobDDRef.
  void setHLDDNode(HLDDNode *HNode) override;

  /// \brief Sets the parent DDRef of BlobDDRef.
  void setParentDDRef(RegDDRef *Ref) { ParentDDRef = Ref; }

  /// Restrict access to base class's public member. Blob DDRef can be modified
  /// to represent a different blob using the interface replaceBlob(). The
  /// symbase is automatically replaced.
  using DDRef::setSymbase;

public:
  /// \brief Returns HLDDNode this DDRef is attached to.
  HLDDNode *getHLDDNode() const override;

  /// \brief Prints BlobDDRef in a simple format.
  virtual void print(formatted_raw_ostream &OS,
                     bool Detailed = false) const override;

  /// \brief Returns the canonical form associated with the blob.
  const CanonExpr *getCanonExpr() const { return CE; }

  /// \brief Returns the blob index associated with this BlobDDRef.
  unsigned getBlobIndex() const { return CE->getSingleBlobIndex(); }

  /// \brief Returns the RegDDRef this is attached to.
  RegDDRef *getParentDDRef() { return ParentDDRef; }
  const RegDDRef *getParentDDRef() const { return ParentDDRef; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef *Ref) {
    return Ref->getDDRefID() == DDRef::BlobDDRefVal;
  }

  /// clone() - Create a copy of 'this' BlobDDRef that is identical in all
  /// ways except the following:
  ///   * The Parent RegDDRef needs to be set explicitly
  BlobDDRef *clone() const override;

  /// \brief Returns the src element type associated with this DDRef.
  Type *getSrcType() const override { return CE->getSrcType(); }
  /// \brief Returns the dest element type associated with this DDRef.
  Type *getDestType() const override { return CE->getDestType(); }

  /// \brief Returns true if the blob DDRef represents a self-blob like (1 * %t)
  /// which should always be true.
  bool isSelfBlob() const override { return true; }

  /// \brief Returns true if this blob DDRef represents an undef blob.
  bool containsUndef() const override { return CE->containsUndef(); }

  /// \brief Used to represent a different blob by replacing the existing blob
  /// index with the new one. Symbase is automatically updated.
  void replaceBlob(unsigned NewIndex);

  /// \brief Sets defined at level for the blob.
  void setDefinedAtLevel(unsigned Level) { CE->setDefinedAtLevel(Level); }

  /// \brief Marks the blob as non-linear.
  void setNonLinear() { CE->setNonLinear(); }

  /// \brief Verifies BlobDDRef integrity.
  virtual void verify() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
