#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/MockHIR.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "mhir"

using namespace llvm;
using namespace llvm::loopopt;
namespace {
    struct MockHIR : public FunctionPass {
        static char ID;
        AliasAnalysis *AA;
        ScalarEvolution *SE;
        LoopInfo *LI;
        Function *F;

        MockHIR() : FunctionPass(ID) {
        }
        bool functionMatchesSimpleLoop();
        void createMockHIRSimpleLoop();

        bool runOnFunction(Function &F) override {
            errs() << "Starting the static linked mock for ";
            errs().write_escaped(F.getName()) << "\n";
            //loopopt::HLNodeUtils *util;
            //loopopt::HLSwitch *sw = util->createHLSwitch(nullptr);

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

}

char MockHIR::ID = 0;
static RegisterPass<MockHIR> X("MockHIR", "Mock HIR Construction", false, false);

void MockHIR::createMockHIRSimpleLoop() {
  
    Function::iterator curBlock = F->begin();
    BasicBlock *EntryBlock = curBlock;
    BasicBlock *PrehdrBlock = ++curBlock;
    BasicBlock *LoopBlock = ++curBlock;
    BasicBlock *ExitBlock = ++curBlock;

    std::set< BasicBlock* > OrigBBs;

    for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
        OrigBBs.insert(i);
    }

    //set up basic region
    HLRegion *Region = HLNodeUtils::createHLRegion(OrigBBs, EntryBlock, 
            ExitBlock);

    //set up ztt if
    HLIf *Ztt = HLNodeUtils::createHLIf(Region);
    HLNodeUtils::setSimpleLoopZtt(Ztt, PrehdrBlock);

    //set up loop
    HLLoop *Loop = HLNodeUtils::createHLLoop(Region, Ztt, false, 1);
   
    //regions child is loop
    HLNodeUtils::dbgPushBackChild(Region, Loop);

    //set up instruction(s) inside
    BasicBlock::iterator CurInst = LoopBlock->begin();

    Instruction *LoadGEP = CurInst;
    Instruction *LoadInst = ++CurInst;
    Instruction *StoreGEP = ++CurInst;
    Instruction *StoreInst = ++CurInst;

    DEBUG(errs() << "The Load GEP is " << *LoadGEP << "\n");
    DEBUG(errs() << "The Load is " << *LoadInst << "\n");
    DEBUG(errs() << "The Store GEP is " << *StoreGEP << "\n");
    DEBUG(errs() << "The Store is " << *StoreInst << "\n");
}


/*
lookinig for a function like this
pretty much for(..) {A[i] = B[i]}
define i32 @_Z3foov() #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i64 %i.0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [10 x i32]* @B, i32 0, i64 %i.0
  %0 = load i32* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds [10 x i32]* @A, i32 0, i64 %i.0
  store i32 %0, i32* %arrayidx1, align 4, !tbaa !1
  %inc = add nsw i64 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %1 = load i32* getelementptr inbounds ([10 x i32]* @A, i32 0, i64 2), align 4, !tbaa !1
  ret i32 %1
}

*/
bool MockHIR::functionMatchesSimpleLoop() {
    int numBlocks = 0;
    
    for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
        errs() << "Basic block (name=" << i->getName() << ") has "
                         << i->size() << " instructions.\n";
        numBlocks++;
    }
    if(numBlocks != 4) { 
        DEBUG(errs() << "Incorrect number of blocks " << numBlocks << "\n");
        return false;
    }

    BasicBlock *first = (F->begin());
    BasicBlock *second = ++(F->begin());
    BasicBlock *third = ++(++(F->begin()));
    //first block has only branch
    if(first->size() != 1) {
        DEBUG(errs () << "first block did not meet criteria " << 
                first->size() << "\n");
        return false;
    }

    //second block is pre header
    if(second->size() != 3)  { 
        DEBUG(errs () << "second block did not meet criteria " << 
                second->size() << "\n");
        return false;
    }
    
    //third block is body
    if(third->size() != 6)  { 
        DEBUG(errs () << "third block did not meet criteria " << 
                third->size() << "\n");
        return false;
    }

    DEBUG(errs() << "function matched simple loop\n");
    return true;
}

FunctionPass *llvm::createMockHIRPass() {
  return new MockHIR();
}
