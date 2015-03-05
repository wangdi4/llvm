//===------ DDRef.h - Data dependency node in HIR ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the data dependency node in high level IR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_DDREF_H
#define LLVM_IR_INTEL_LOOPIR_DDREF_H

#include "llvm/Support/Compiler.h"
#include "llvm/IR/Intel_LoopIR/HLDDNode.h"
#include <set>

namespace llvm {

class Value;

namespace loopopt {

class CanonExpr;

/// \brief Base class for encapsulating Values/References which can cause
/// data dependencies and/or for which we need to generate code using the
/// canonical form.
///
/// This class (hierarchy) disallows creating objects on stack.
/// Objects are created/destroyed using DDRefUtils friend class.
class DDRef {
private:
  /// \brief Make class uncopyable.
  void operator=(const DDRef &) LLVM_DELETED_FUNCTION;

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set<DDRef *> Objs;

  const unsigned char SubClassID;
  int SymBase;

protected:
  DDRef(unsigned SCID, int SB);
  DDRef(const DDRef &DDRefObj);
  virtual ~DDRef() {}

  friend class DDRefUtils;

  /// \brief Virtual set HLDDNode
  virtual void setHLDDNode(HLDDNode *HNode) = 0;

  /// \brief Required to access setHLDDNode().
  friend class HLDDNode;

  /// \brief Destroys the object.
  void destroy();

public:
  /// Virtual Clone Method
  virtual DDRef *clone() const = 0;
  /// TBD how to do this
  void dump() const;
  /// TBD how to do this
  void print() const;

  /// \brief Returns the HLDDNode this DDRef is attached to.
  virtual HLDDNode *getHLDDNode() const = 0;

  /// \brief Returns the underlying value this DDRef represents.
  virtual Value *getLLVMValue() const = 0;

  /// \brief Returns the symbol number used to disambiguate references.
  int getSymBase() const { return SymBase; };
  void setSymBase(int Sbase) { SymBase = Sbase; }

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getDDRefID() const { return SubClassID; }

  /// \brief An enumeration to keep track of the concrete subclasses of DDRef
  enum DDRefVal { ConstDDRefVal, RegDDRefVal, BlobDDRefVal };
};

} // End loopopt namespace

} // End llvm namespace

#endif
