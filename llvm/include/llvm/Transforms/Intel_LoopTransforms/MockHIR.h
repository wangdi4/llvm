
#ifndef LLVM_TRANSFORMS_MOCKHIR_H
#define LLVM_TRANSFORMS_MOCKHIR_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"
namespace llvm {

  class AliasAnalysis;
  class Loop;
  class LoopInfo;
  class ScalarEvolution;
  class SCEV;
  class SCEVConstant;
  using namespace loopopt;

    class MockHIR : public FunctionPass {
        AliasAnalysis *AA;
        ScalarEvolution *SE;
        LoopInfo *LI;
        Function *F;
        typedef std::vector< const SCEV* > BlobTableTy;
        BlobTableTy MockBlobTable;
        public:
        MockHIR() : FunctionPass(ID) {
        }
        static char ID;
        bool functionMatchesSimpleLoop();
        HLNode *TopRegion;

        void createMockHIRSimpleLoop();
        HLNode* getTopRegion() { return TopRegion;}
        BlobTableTy& getBlobTable() {return MockBlobTable;}

        bool runOnFunction(Function &F) override {
            errs() << "Starting the static linked mock for ";
            errs().write_escaped(F.getName()) << "\n";

            this->F = &F;
            AA = &getAnalysis<AliasAnalysis>();
            SE = &getAnalysis<ScalarEvolution>();
            LI = &getAnalysis<LoopInfo>();

            if(functionMatchesSimpleLoop()) {
                createMockHIRSimpleLoop();
            }

            return false;
        }

        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
            AU.addRequiredTransitive<AliasAnalysis>();
            AU.addRequiredTransitive<ScalarEvolution>();
            AU.addRequiredTransitive<LoopInfo>();
        }

    };
FunctionPass *createMockHIRPass();
}
#endif
