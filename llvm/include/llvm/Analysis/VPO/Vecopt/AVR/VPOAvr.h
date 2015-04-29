//===---------- VectorAVR.h - Abstract Vector Representation------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Vectorizer's AVR node.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_H
#define LLVM_ANALYSIS_VPO_AVR_H

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "avr"

namespace intel { // VPO Vectorizer Namespace

using namespace llvm;

class AVRLoop;

/// \brief Abstract Vector Representation Node base class
///
/// This represents a node of the vectorizer AVR. It is used to represent
/// the incoming LLVM IR or incoming LoopOpt HIR.
///
/// This class (hierarchy) disallows creating objects on stack.
/// Objects are created/destroyed using AVRUtils friend class.
class AVR : public ilist_node<AVR> {
private:

  /// \brief Make class uncopyable.
  //void operator=(const AVR &) LLVM_DELETED_FUNCTION;

  /// AVR Subclass Identifier
  const unsigned char SubClassID;

  /// Lexical parent of AVR
  AVR *Parent;

  /// Unique ID for AVR node.
  unsigned Number;

  /// \brief Destroys all objects of this class. Only called after Vectorizer 
  /// phase code generation.
  static void destroyAll();

  /// Sets unique ID for this AVR Node.
  void setNumber();

protected:
  AVR(unsigned SCID);
  AVR(const AVR &AVRObj);

  //friend class AVRUtils;
  /// \brief Destroys the object.
  void destroy();


public:
  //ToDo implement virtual destructor
  //virtual ~AVR() {}
  ~AVR() {}

  /// Virtual Clone Method
  virtual AVR *clone() const = 0;
  virtual void dump() const { print(); }
  virtual void print() const;

  /// \brief Returns the immediate lexical parent of the AVR.
  AVR *getParent() const { return Parent; }

  /// \brief Returns the parent loop of this node, if one exists.
  AVRLoop *getParentLoop() const;

  /// \brief Returns the strictly lexical parent loop of this node, if one
  /// exists.
  /// This is different for HLInsts which are located in loop
  /// preheader/postexit.
  AVRLoop *getLexicalParentLoop() const;

  /// \brief Return an ID for the concrete type of this object.
  ///
  /// This is used to implement the classof checks in LLVM and should't
  ///  be used for any other purpose.
  unsigned getAVRID() const { return SubClassID; }

  /// \brief Sets the lexical parent of this AVR.
  void setParent(AVR *ParentNode) { Parent = ParentNode; }

  /// \brief Enumeration for the concrete subclasses of AVR.
  enum AVRVal {
    AVRLoopNode,
    AVRFunctionNode,
    AVRIfNode,
    AVRAssignNode,
    AVRExprNode,
    AVRLabelNode,
    AVRPhiNode,
    AVRCallNode,
    AVRFBranchNode,
    AVRBackEdgeNode,
    AVREntryNode,
    AVRReturnNode
  };

};

} // End VPO Vectorizer Namspace

namespace llvm { 

// ilist templates
template <>
struct ilist_traits<intel::AVR>
  : public ilist_default_traits<intel::AVR> {

  intel::AVR *createSentinel() const {
    return static_cast<intel::AVR *>(&Sentinel);
  }

  static void destroySentinel(intel::AVR *) {}

  intel::AVR *provideInitialHead() const { return createSentinel(); }
  intel::AVR *ensureHead(intel::AVR *) const {
    return createSentinel();
  }
  static void noteHead(intel::AVR *, intel::AVR *) {}

  static intel::AVR *createNode(const intel::AVR &) {
    llvm_unreachable("AVR should be explicitly created via AVRUtils"
                     "class");

    return nullptr;
  }
  static void deleteNode(intel::AVR *) {}

private:
  mutable ilist_half_node<intel::AVR> Sentinel;
};

}  // End llvm Namespace


namespace intel { // VPO Vectorizer Namespace
typedef iplist<AVR> AVRContainerTy;
// TODO: Remove this.
extern AVRContainerTy AVRFunctions;

} // End VPO Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_H


