//===----------- SymbaseAssignment.h - Creates HIR nodes-------*-- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// TODO LICENSE
//===----------------------------------------------------------------------===//
// This pass is responsible for initial assignment of symbases to ddrefs
// DDRefs sharing a symbase may alias with any other ddref with the same
// symbase but are guaranteed not to alias with a DDref with another
// symbase. Similar in many respects to LLVM's alias sets concept.
// Will probably use it for implementation in some form or fashion
// Temps and blobs should have their symbases assigned by parsing
//
// Current implementation operates by using AliasSetTracker to classify ld/sts
// into disjoint sets of memory accesses. This is still possible in HIR
// because this pass is run prior to any HIR optimization. We can still
// reliably map HIR to LLVM IR, and so should be able
// to find a representative ptr for each RegDDRef to use in
// AliasAnalysis.
//
// The usual caveats for AliasAnalysis apply here, namely that the
// effectiveness and repeatability of this pass depends on the specific
// AA implementation(s) used and their chain order. Also, AliasSetTracker
// is known to be non deterministic.
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_SYMBASE
#define INTEL_LOOPANALYSIS_SYMBASE

#include "llvm/Pass.h"
namespace llvm {

class AliasAnalysis;
class Function;
namespace loopopt {

/// \brief TODO
class SymbaseAssignment : public FunctionPass {
public:
  SymbaseAssignment() : FunctionPass(ID) {}
  static char ID;
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const;

  // Returns a new unused symbase ID
  unsigned int getNewSymBase() { return MaxSymBase++; }

  unsigned int getSymBaseForConstants() { return CONSTANT_SYMBASE; }

private:
  // this symbase is reserved for DDRefs representing constnts which require 
  // no dd testing
  const unsigned int CONSTANT_SYMBASE = 1;

  unsigned int MaxSymBase = CONSTANT_SYMBASE + 1;

  Function *F;
};
}
}
#endif
