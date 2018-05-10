#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Analysis/GlobalsModRef.h"    // INTEL
#include "llvm/Analysis/Intel_AggInline.h"  // INTEL
#include "llvm/Analysis/Intel_Andersens.h"  // INTEL
#include "llvm/Analysis/Intel_WP.h"         // INTEL
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar/SCCP.h"

using namespace llvm;

PreservedAnalyses IPSCCPPass::run(Module &M, ModuleAnalysisManager &AM) {
  const DataLayout &DL = M.getDataLayout();
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  if (!runIPSCCP(M, DL, &TLI))
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();        // INTEL
  PA.preserve<WholeProgramAnalysis>();  // INTEL
  PA.preserve<GlobalsAA>();             // INTEL
  PA.preserve<AndersensAA>();           // INTEL
  PA.preserve<InlineAggAnalysis>();     // INTEL
  return PA;                            // INTEL
  return PreservedAnalyses::none();
}

namespace {

//===--------------------------------------------------------------------===//
//
/// IPSCCP Class - This class implements interprocedural Sparse Conditional
/// Constant Propagation.
///
class IPSCCPLegacyPass : public ModulePass {
public:
  static char ID;

  IPSCCPLegacyPass() : ModulePass(ID) {
    initializeIPSCCPLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    const DataLayout &DL = M.getDataLayout();
    const TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
    return runIPSCCP(M, DL, TLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();     // INTEL
    AU.addPreserved<GlobalsAAWrapperPass>();        // INTEL
    AU.addPreserved<AndersensAAWrapperPass>();      // INTEL
    AU.addPreserved<InlineAggressiveWrapperPass>(); // INTEL
  }
};

} // end anonymous namespace

char IPSCCPLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(IPSCCPLegacyPass, "ipsccp",
                      "Interprocedural Sparse Conditional Constant Propagation",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(IPSCCPLegacyPass, "ipsccp",
                    "Interprocedural Sparse Conditional Constant Propagation",
                    false, false)

// createIPSCCPPass - This is the public interface to this file.
ModulePass *llvm::createIPSCCPPass() { return new IPSCCPLegacyPass(); }
