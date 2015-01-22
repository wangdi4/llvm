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
#include <set>

namespace llvm {

class Value;

namespace loopopt {

class CanonExpr;
class HLNode;

/// \brief Base class for encamsulating Values/References which can cause 
/// data dependencies and/or for which we need to generate code using the 
/// canonical form.
///
/// This class (hierarchy) disallows creating objects on stack. 
/// Objects are created/destroyed using DDRefUtils friend class.
class DDRef {
private:
  /// \brief Make class uncopyable.
  DDRef(const DDRef &) LLVM_DELETED_FUNCTION;
  void operator=(const DDRef &) LLVM_DELETED_FUNCTION;

  /// \brief Destroys all objects of this class. Should only be
  /// called after code gen.
  static void destroyAll();
  /// Keeps track of objects of this class.
  static std::set< DDRef* >Objs;

  const unsigned char SubClassID;
  int SymBase;

protected:
  DDRef(unsigned SCID, int SB);
  virtual ~DDRef() { }

  friend class DDRefUtils;

  virtual void setHLNode(HLNode* HNode) = 0;
  virtual DDRef* clone_impl() const = 0;   

  /// \brief Destroys the object.
  void destroy();

public:
  DDRef* clone() const;
  /// TBD how to do this
  void dump() const;
  /// TBD how to do this
  void print() const;

  /// \brief Returns the HLNode this DDRef is attached to.
  virtual HLNode* getHLNode() const = 0;

  /// \brief Returns the underlying value this DDRef represents.
  virtual Value* getLLVMValue() const = 0;

  /// \brief Returns the symbol number used to disambiguate references.
  int getSymBase() const { return SymBase; };
  void setSymBase(int Sbase) { SymBase = Sbase; }

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getDDRefID() const { return SubClassID; }

  /// \brief An enumeration to keep track of the concrete subclasses of DDRef
  enum DDRefVal {
    ConstDDRefVal,
    RegDDRefVal,
    BlobDDRefVal
  };

};

} // End loopopt namespace

} // End llvm namespace

#endif

