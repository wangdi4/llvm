#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRCodeGen.h"
#include "llvm/Transforms/Intel_LoopTransforms/MockHIR.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/Debug.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"

#include "llvm/IR/LLVMContext.h"
#define DEBUG_TYPE "hircg"


using namespace llvm;
using namespace llvm::loopopt;
namespace {
    //TODO
    class HIRCodeGen : public FunctionPass {
        private:
            HLNode *curr;
            ScalarEvolution *SE;
            LoopInfo *LI;
            Function *F;
            MockHIR *HIR;
        public:
        static char ID;

        HIRCodeGen() : FunctionPass(ID) {
        }

        bool runOnFunction(Function &F) override {
            errs() << "Starting the code gen for ";
            errs().write_escaped(F.getName()) << "\n";

            this->F = &F;
            SE = &getAnalysis<ScalarEvolution>();
            LI = &getAnalysis<LoopInfo>();
            HIR = &getAnalysis<MockHIR>();
       //     genLLVMIR();


            return false;
        }

        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
            AU.addRequiredTransitive<ScalarEvolution>();
            AU.addRequiredTransitive<LoopInfo>();
            AU.addRequiredTransitive<MockHIR>();
        }

    };

}
FunctionPass *llvm::createHIRCodeGenPass() {
  return new HIRCodeGen();
}
char HIRCodeGen::ID = 0;
static RegisterPass<HIRCodeGen> X("HIRCG", "HIR Code Generation", false, false);
