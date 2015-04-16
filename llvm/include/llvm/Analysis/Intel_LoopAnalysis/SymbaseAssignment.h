//===----------- SymbaseAssignment.h - Creates HIR nodes-------*-- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// TODO LICENSE
//===----------------------------------------------------------------------===//
// This pass is responsible for initial assignment of symbases to ddrefs
// DDRefs sharing a symbase may alias with any other ddref with the same symbase
// but are guaranteed not to alias with a DDref with another symbase. Similar
// in many respects to LLVM's alias sets concept. Will probably use it for
// implementation in some form or fashion
//
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
  unsigned int getNewSymbase() { return ++MaxSymbase; }

  unsigned int getSymbaseForConstants() { return CONSTANT_SYMBASE; }

private:
  unsigned int MaxSymbase = CONSTANT_SYMBASE + 1;
  AliasAnalysis *AA;

  // this symbase is reserved for ConstantDDRefs which require no dd testing
  const unsigned int CONSTANT_SYMBASE = 1;
  Function *F;
};
}
}
#endif
