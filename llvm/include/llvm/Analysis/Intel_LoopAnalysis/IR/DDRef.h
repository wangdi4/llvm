//===------ DDRef.h - Data dependency node in HIR -------*- C++ -*---------===//
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
// This file defines the data dependency node in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_DDREF_H
#define LLVM_IR_INTEL_LOOPIR_DDREF_H

#include "llvm/Support/Compiler.h"
#include <set>

namespace llvm {

class Value;
class Type;
class formatted_raw_ostream;

namespace loopopt {

class CanonExpr;
class HLDDNode;
class HLLoop;
class DDRefUtils;
class CanonExprUtils;
class BlobUtils;

/// \brief Base class for encapsulating Values/References which can cause
/// data dependencies and/or for which we need to generate code using the
/// canonical form.
///
/// This class (hierarchy) disallows creating objects on stack.
/// Objects are created/destroyed using DDRefUtils friend class.
class DDRef {
private:
  /// Make class uncopyable.
  void operator=(const DDRef &) = delete;

  /// Reference to parent utils object. This is needed to access util functions.
  DDRefUtils &DDRU;

  const unsigned char SubClassID;
  unsigned Symbase;

protected:
  DDRef(DDRefUtils &DDRU, unsigned SCID, unsigned SB);
  DDRef(const DDRef &DDRefObj);
  virtual ~DDRef() {}

  friend class DDRefUtils;

  /// Virtual set HLDDNode
  virtual void setHLDDNode(HLDDNode *HNode) = 0;

  /// Implements get*Type() functionality.
  Type *getTypeImpl(bool IsSrc) const;

public:
  /// Returns parent DDRefUtils object.
  DDRefUtils &getDDRefUtils() const { return DDRU; }

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const;

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() const;

  /// Virtual Clone Method
  virtual DDRef *clone() const = 0;

  /// Dumps DDRef.
  void dump(bool Detailed) const;
  /// Dumps DDRef in a simple format.
  void dump() const;

  /// Prints DDRef in a simple format.
  virtual void print(formatted_raw_ostream &OS, bool Detailed = false) const;

  /// Returns the parent HLDDNode.
  virtual const HLDDNode *getHLDDNode() const = 0;

  virtual HLDDNode *getHLDDNode() = 0;

  /// Returns the Level of parent HLDDNode Level.
  unsigned getNodeLevel() const;

  /// Returns the src element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose src base type is [7 x
  /// [101 x float]]*, we will return float.
  /// TODO: extend to handle struct types.
  virtual Type *getSrcType() const = 0;

  /// Returns the dest element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose dest base type is [7 x
  /// [101 x int32]]*, we will return int32.
  /// TODO: extend to handle struct types.
  virtual Type *getDestType() const = 0;

  /// Returns the symbol number used to disambiguate references.
  unsigned getSymbase() const { return Symbase; };
  void setSymbase(unsigned SB) { Symbase = SB; }

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getDDRefID() const { return SubClassID; }

  /// An enumeration to keep track of the concrete subclasses of DDRef
  enum DDRefVal { RegDDRefVal, BlobDDRefVal };

  /// Returns true if the DDRef represents a self-blob like (1 * %t). In
  /// addition DDRef's symbase should be the same as %t's symbase. This is so
  /// because for some livein copies %t1 = %t2, lval %t1 is parsed as 1 * %t2.
  /// But since %t1 has a different symbase than %t2 we still need to add a blob
  /// DDRef for %t2 to the DDRef.
  virtual bool isSelfBlob() const = 0;

  /// Returns true if DDRef corresponds to terminal ref.
  /// BlobDDRef is always terminal
  virtual bool isTerminalRef() const = 0;

  /// Returns true if DDRef is lvalue.
  virtual bool isLval() const = 0;

  /// Returns true if DDRef is rvalue.
  bool isRval() const { return !isLval(); }

  /// Returns true if this ref looks like 1 * undef.
  virtual bool isStandAloneUndefBlob() const = 0;

  /// Returns true if this DDRef represents a metadata.
  /// If true, metadata is returned in Val.
  virtual bool isMetadata(MetadataAsValue **Val = nullptr) const = 0;

  /// Returns single CanonExpr (important special case for terminal refs)
  virtual const CanonExpr *getSingleCanonExpr() const = 0;

  virtual CanonExpr *getSingleCanonExpr() = 0;

  /// Returns true if this DDRef contains undefined canon expressions.
  virtual bool containsUndef() const = 0;

  /// Verifies DDRef integrity.
  virtual void verify() const;

  /// Returns true if temp DDRef is live out of Region.
  /// Assets if this DDRef does not represent a temp.
  bool isLiveOutOfRegion() const;

  /// Returns true if temp DDRef is live into parent loop.
  /// Asserts if this DDRef does not represent a temp.
  bool isLiveIntoParentLoop() const;

  /// Returns true if temp DDRef is live out of parent loop.
  /// Asserts if this DDRef does not represent a temp.
  bool isLiveOutOfParentLoop() const;

  /// Returns ParentLoop of DDRef.
  const HLLoop *getParentLoop() const;

  HLLoop *getParentLoop();

  /// Returns lexical ParentLoop of DDRef.
  const HLLoop *getLexicalParentLoop() const;

  HLLoop *getLexicalParentLoop();
};

} // namespace loopopt

} // namespace llvm

#endif
