//===------ DDRef.h - Data dependency node in HIR -------*- C++ -*---------===//
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
  /// \brief Make class uncopyable.
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

  /// \brief Virtual set HLDDNode
  virtual void setHLDDNode(HLDDNode *HNode) = 0;

  /// \brief Implements get*Type() functionality.
  Type *getTypeImpl(bool IsSrc) const;

public:
  /// Returns parent DDRefUtils object.
  DDRefUtils &getDDRefUtils() const { return DDRU; }

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() const;

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() const;

  /// \brief Virtual Clone Method
  virtual DDRef *clone() const = 0;

  /// \brief Dumps DDRef.
  void dump(bool Detailed) const;
  /// \brief Dumps DDRef in a simple format.
  void dump() const;

  /// \brief Prints DDRef in a simple format.
  virtual void print(formatted_raw_ostream &OS, bool Detailed = false) const;

  /// \brief Returns the parent HLDDNode.
  virtual HLDDNode *getHLDDNode() const = 0;

  /// \brief Returns the Level of parent HLDDNode Level.
  unsigned getNodeLevel() const;

  /// \brief Returns the src element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose src base type is [7 x
  /// [101 x float]]*, we will return float.
  /// TODO: extend to handle struct types.
  virtual Type *getSrcType() const = 0;
  /// \brief Returns the dest element type associated with this DDRef.
  /// For example, for a 2 dimensional GEP DDRef whose dest base type is [7 x
  /// [101 x int32]]*, we will return int32.
  /// TODO: extend to handle struct types.
  virtual Type *getDestType() const = 0;

  /// \brief Returns the symbol number used to disambiguate references.
  unsigned getSymbase() const { return Symbase; };
  void setSymbase(unsigned SB) { Symbase = SB; }

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getDDRefID() const { return SubClassID; }

  /// \brief An enumeration to keep track of the concrete subclasses of DDRef
  enum DDRefVal { RegDDRefVal, BlobDDRefVal };

  /// \brief Returns true if the DDRef represents a self-blob like (1 * %t). In
  /// addition DDRef's symbase should be the same as %t's symbase. This is so
  /// because for some livein copies %t1 = %t2, lval %t1 is parsed as 1 * %t2.
  /// But since %t1 has a different symbase than %t2 we still need to add a blob
  /// DDRef for %t2 to the DDRef.
  virtual bool isSelfBlob() const = 0;

  /// Returns true if this ref looks like 1 * undef.
  virtual bool isStandAloneUndefBlob() const = 0;

  /// \brief Returns true if this DDRef contains undefined canon expressions.
  virtual bool containsUndef() const = 0;

  /// \brief Verifies DDRef integrity.
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

  /// \brief  Returns ParentLoop of DDRef.
  HLLoop *getParentLoop() const;

  /// Returns lexical ParentLoop of DDRef.
  HLLoop *getLexicalParentLoop() const;
};

} // End loopopt namespace

} // End llvm namespace

#endif
