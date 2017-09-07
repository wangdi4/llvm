#ifndef LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_DG_H
#define LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_DG_H

#include "AdvisorCommon.h"

#include <boost/graph/adjacency_list.hpp>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

namespace llvm {
class AAResultsWrapperPass;
class BasicBlock;
class DominatorTree;
class DominatorTreeWrapperPass;
class Instruction;
class MemoryDependenceResults;
class MemoryDependenceWrapperPass;
}

using namespace llvm;

namespace fpga {

class DependenceGraph : public ModulePass {

public:
  static char ID;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<MemoryDependenceWrapperPass>();
    AU.addRequiredTransitive<AAResultsWrapperPass>();
    AU.setPreservesAll();
    // AU.addRequiredTransitive<MemoryDependenceWrapperPass>();
  }
  DependenceGraph() : ModulePass(ID) {
    initializeBasicAAWrapperPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override;
  DepGraph &getDepGraph() { return DG; }
  static DepGraph::vertex_descriptor
  get_vertex_descriptor_for_basic_block(BasicBlock *BB, DepGraph &depGraph);
  static bool isBasicBlockDependent(BasicBlock *BB1, BasicBlock *BB2,
                                    DepGraph &DG);
  static void getAllBasicBlockDependencies(DepGraph &depGraph, BasicBlock *BB,
                                           std::vector<BasicBlock *> &deps);
  static bool isBasicBlockDependenceTrue(BasicBlock *BB1, BasicBlock *BB2,
                                         DepGraph &DG);

private:
  void addVertices(Function &F);
  void addEdges();
  void
  insertDependentBasicBlock(std::vector<std::pair<BasicBlock *, bool>> &list,
                            BasicBlock *BB, bool trueDep);
  void insertDependentBasicBlockAllMemory(
      std::vector<std::pair<BasicBlock *, bool>> &list, bool trueDep);
  bool unsupportedMemoryInstruction(Instruction *I);
  void output_graph_to_file(raw_ostream *outputFile);
  bool dgRunOnFunction(Function &F);

  Function *func;
  MemoryDependenceResults *MDA;
  DominatorTree *DT;
  DepGraph DG;
  std::vector<std::string> NameVec;
  // a list of basic blocks that may read or write memory
  std::vector<BasicBlock *> MemoryBBs;
}; // end class DependenceGraph
}

#endif // file include
