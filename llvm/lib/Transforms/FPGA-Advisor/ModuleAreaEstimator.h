#ifndef LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_MAE_H
#define LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_MAE_H

#include "llvm/IR/InstVisitor.h"
#include "llvm/Pass.h"

namespace llvm {
class BasicBlock;
class Instruction;
class Module;
}

namespace fpga {

// The ModuleAreaEstimator class performs crude area estimation for the basic
// blocks
// in a function
// The main goal of this class is not to determine the exact area/resources
// required to
// implement the design on an FPGA, the main motivation is to discourage the
// tool to
// suggest putting portions of designs onto the FPGA where there are limited
// resources
// such as operations requiring DSPs, a lot of long routes which may decrease
// the
// clock speed of the design, memory... ?
class ModuleAreaEstimator : public ModulePass,
                            InstVisitor<ModuleAreaEstimator> {
public:
  static char ID;
  static void *analyzerLibHandle;
  static int (*getBlockArea)(BasicBlock *BB);
  static bool useDefault;

  ModuleAreaEstimator();

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addPreserved<MemoryDependenceWrapperPass>();
    // AU.addPreserved<DependenceGraph>();
    AU.setPreservesAll();
  }
  bool runOnModule(Module &M) override;
  static int getBasicBlockArea(std::map<BasicBlock *, int> &AT,
                               BasicBlock *BB) {
    auto search = AT.find(BB);
    assert(search != AT.end());
    return search->second;
  }
  std::map<BasicBlock *, int> &getAreaTable() { return areaTable; }

  void visitBasicBlock(BasicBlock &BB);

  // the area complexity of an instruction is determined by several factors:
  // routing and compute resources on a typical FPGA
  // NOTE: if a basic block purely consists of very basic operations
  // for example integer addition, shifts etc, we won't incur any additional
  // area costs because we want to encourage such designs for the FPGA
  // the instructions which will incur an area cost will be the following types:
  // 1) floating point operations - these are likely to be impl on the FP DSP
  //	units, which are a limited resource
  // 2) memory instructions - this highly depends on the memory architecture,
  //	but we can assume that accesses to global memory all require a lot of
  //	routing and muxing logic
  // 3) switch statement/phi nodes with a large number of inputs
  //	these we can effectively think of as muxes, if there are a large number
  //	of inputs (e.g. 8/16 or more) then the mux will be very large, which we
  //	would want to discourage (FIXME: however, this is really more of a
  // latency
  //	issue than an area issue)
  // 4) ambiguous pointers??? TODO
  int instructionAreaComplexity(Instruction *I);

  bool instructionNeedsFp(Instruction *I);
  bool instructionNeedsGlobalMemory(Instruction *I);
  bool instructionNeedsMuxes(Instruction *I);

  // area estimators
  // I currently have no plan for this, will need to be calibrated
  int getFpAreaCost() { return 1; }

  int getGlobalMemoryAreaCost() {
    // TODO FIXME this should depend on the size of the memory location
    return 1;
  }

  int getMuxAreaCost(Instruction *I);

private:
  std::map<BasicBlock *, int> areaTable;

}; // end class ModuleAreaEstimator
}

#endif // file include
