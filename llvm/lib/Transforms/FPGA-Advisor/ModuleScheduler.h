#ifndef LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_MS_H
#define LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_MS_H

#include "AdvisorCommon.h"

#include <map>

#include "llvm/Pass.h"

namespace llvm {
class BasicBlock;
class Instruction;
class ModulePass;
}

using namespace llvm;

namespace fpga {

class ModuleScheduler : public ModulePass, public InstVisitor<ModuleScheduler> {
public:
  static char ID;
  static void *analyzerLibHandle;
  static int (*getBlockLatency)(BasicBlock *BB);
  static int (*getBlockII)(BasicBlock *BB);
  static bool useDefault;
  ModuleScheduler();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // AU.addPreserved<AliasAnalysis>();
    // AU.addPreserved<MemoryDependenceAnalysis>();
    // AU.addPreserved<DependenceGraph>();
    AU.setPreservesAll();
  }

  bool runOnModule(Module &M) override;
  std::map<BasicBlock *, LatencyStruct> &getFPGALatencyTable() {
    return LatencyTableFPGA;
  }

  static int
  getBasicBlockLatencyAccelerator(std::map<BasicBlock *, LatencyStruct> &LT,
                                  BasicBlock *BB) {
    auto search = LT.find(BB);
    assert(search != LT.end());

    return search->second.acceleratorLatency;
  }

  static int getBasicBlockLatencyCpu(std::map<BasicBlock *, LatencyStruct> &LT,
                                     BasicBlock *BB) {
    auto search = LT.find(BB);
    assert(search != LT.end());

    return search->second.cpuLatency;
  }

  int getInstructionLatency(Instruction *I);

  void visitBasicBlock(BasicBlock &BB);

  std::map<BasicBlock *, LatencyStruct> LatencyTableFPGA;

}; // end class ModuleScheduler

} // end namespace fpga

#endif // file include
