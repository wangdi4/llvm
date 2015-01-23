//===------- BlobDDRef.h - Data dependency node in HIR ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the BlobDDRef node in high level IR.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_IR_INTEL_LOOPIR_BLOBDDREF_H
#define LLVM_IR_INTEL_LOOPIR_BLOBDDREF_H

#include "llvm/IR/Intel_LoopIR/DDRef.h"

namespace llvm {

namespace loopopt {

class CanonExpr;
class RegDDRef;
class HLNode;

/// \brief Represents a blob present in a canonical expr of a RegDDRef
///
/// This DDRef is associated with a RegDDref to expose data dependncies
/// present due to blobs.
class BlobDDRef : public DDRef {
private:
  CanonExpr* CExpr;
  RegDDRef* ParentDDRef;

protected:
  explicit BlobDDRef(int SB, CanonExpr* CE, RegDDRef* Parent);
  ~BlobDDRef() { }

  friend class DDRefUtils;

  BlobDDRef* clone_impl() const override;

  void setHLNode(HLNode* HNode) override;

public:
  /// \brief Returns HLNode this DDRef is attached to.
  HLNode* getHLNode() const override;

  /// TODO implementation
  Value* getLLVMValue() const override { return nullptr; }
 
  /// \brief Returns the canonical form associated with the blob.
  CanonExpr* getCanonExpr() { return CExpr; }
  const CanonExpr* getCanonExpr() const { return CExpr; }

  /// \brief Returns the RegDDRef this is attached to.
  RegDDRef* getParentDDRef() { return ParentDDRef; }
  const RegDDRef* getParentDDRef() const { return ParentDDRef; }

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const DDRef* Ref) {
    return Ref->getDDRefID() == DDRef::BlobDDRefVal;
  }

};

} // End namespace loopopt

} // End namespace llvm

#endif
