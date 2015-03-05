
#ifndef LLVM_IR_HLVISITOR_H
#define LLVM_IR_HLVISITOR_H
#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

#include "llvm/Support/ErrorHandling.h"

namespace llvm {

/// @brief Base class for HIR visitor
///
/// HIR visitors are used when you want to perform different actions
/// for different kinds of HIR without having to use lots of casts
/// and a big switch statement (in your code, that is).
///
/// Note that the traversal logic itself is in the visitXXX code, this template
/// does not visit children on its own. Main client in mind was hircg, for which
/// the traversal order may be very unique
///
/// To define your own visitor, inherit from this class, specifying your
/// new type for the 'SubClass' template parameter, and "override" visitXXX
/// functions in your class. I say "override" because this class is defined
/// in terms of statically resolved overloading, not virtual functions.
///
/// For example, here is a visitor that counts the number of loops
///
///  /// Declare the class.  Note that we derive from InstVisitor instantiated
///  /// with _our new subclasses_ type.
///  ///
///  struct CountLoopVisitor : public HIRVisitor<CountLoopVisitor> {
///    unsigned Count;
///    CountLoopVisitor() : Count(0) {}
///
///    void visitLoop(HLLoop &L) { ++Count; }
///  };
///
///  And this class would be used like this:
///    CountAllocaVisitor CAV;
///    CAV.visit(Region);
///    NumLoops = CAV.Count;
///
/// The optional second template argument specifies the type that instruction
/// visitation functions should return. If you specify this, you *MUST* provide
/// an implementation of visitHLNode though!.
///
/// Note that this class is specifically designed as a template to avoid
/// virtual function call overhead.  Defining and using an HIRVisitor is just
/// as efficient as having your own switch statement
//
template <typename SubClass, typename RetTy = void> class HIRVisitor {

public:
  // Generic visit method - Allow visitation to all HIR in a range
  /*TODO enable
  template<class Iterator>
  void visit(Iterator Start, Iterator End) {
    while (Start != End)
      static_cast<SubClass*>(this)->visit(*Start++);
  }
 */
  RetTy visit(HLNode &Node) { return visit(&Node); }
  RetTy visit(HLNode *Node) {
    if (HLRegion *R = dyn_cast<HLRegion>(Node)) {
      return static_cast<SubClass *>(this)->visitRegion(R);
    } else if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
      return static_cast<SubClass *>(this)->visitLoop(L);
    } else if (HLSwitch *S = dyn_cast<HLSwitch>(Node)) {
      return static_cast<SubClass *>(this)->visitSwitch(S);
    } else if (HLInst *I = dyn_cast<HLInst>(Node)) {
      return static_cast<SubClass *>(this)->visitInst(I);
    } else if (HLGoto *G = dyn_cast<HLGoto>(Node)) {
      return static_cast<SubClass *>(this)->visitGoto(G);
    } else if (HLIf *If = dyn_cast<HLIf>(Node)) {
      return static_cast<SubClass *>(this)->visitIf(If);
    } else if (HLLabel *L = dyn_cast<HLLabel>(Node)) {
      return static_cast<SubClass *>(this)->visitLabel(L);
    } else {
      llvm_unreachable("Unknown HIR type encountered!");
    }
  }

  //===--------------------------------------------------------------------===//
  // Visitation functions... these functions provide default fallbacks in case
  // the user does not specify what to do for a particular HIR type.
  // The default behavior is to generalize the instruction type to its subtype
  // and try visiting the subtype.  All of this should be inlined perfectly,
  // because there are no virtual functions to get in the way.
  //
  // Define HIR specific visitor functions that can be overridden to
  // handle SPECIFIC HIRs.

  RetTy visitLoop(HLLoop *L) { visitHLNode(L); }
  RetTy visitRegion(HLRegion *R) { visitHLNode(R); }
  RetTy visitIf(HLIf *I) { visitHLNode(I); }
  RetTy visitSwitch(HLSwitch *S) { visitHLNode(S); }
  RetTy visitInst(HLInst *I) { visitHLNode(I); }
  RetTy visitLabel(HLLabel *L) { visitHLNode(L); }
  RetTy visitGoto(HLGoto *G) { visitHLNode(G); }

  // If the user wants a 'default' case, they can choose to override this
  // function.  If this function is not overloaded in the user's subclass, then
  // this instruction just gets ignored.
  //
  // Note that you MUST override this function if your return type is not void.
  //
  void visitHLNode(HLNode *Node) {} // Ignore unhandled instructions

private:
};
}
#endif
