//===- LoopSPMDization.cpp - Loop SPMDization pass          ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements ...
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include <vector>

using namespace llvm;


namespace {
  class LoopSPMDization : public LoopPass {
    LLVMContext Context;
  public:
    static char ID;
    LoopSPMDization() : LoopPass(ID) {
      initializeLoopSPMDizationPass(*PassRegistry::getPassRegistry());
    }
  private:
    bool FixReductionsIfAny(Loop *L, BasicBlock *AfterLoop, int PE, int NPEs, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, std::vector<Instruction *> *OldInst);
    bool FindReductionVariables(Loop *L, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig);
    PHINode *getInductionVariable(Loop *L, ScalarEvolution *SE);
    bool TransformLoopInitandStep(Loop *L, ScalarEvolution *SE, int PE, int NPEs);
    bool AddParallelIntrinsicstoLoop(Loop *L, LLVMContext& context, Module *M, BasicBlock *OrigPH, BasicBlock *E);
    IntrinsicInst* detectSPMDIntrinsic(Loop *L, LoopInfo *LI);

    bool runOnLoop(Loop *L, LPPassManager &) override {
      LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
      DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
      ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
      
      LLVMContext& context = L->getHeader()->getContext();
      Function *F = L->getHeader()->getParent();
      Module *M = F->getParent() ;
      
      ValueToValueMapTy VMap;
      BasicBlock *OrigPH = L->getLoopPreheader();
      Loop *OrigL = L;
      //return true;
      IntrinsicInst* found_spmd = detectSPMDIntrinsic(L, LI);
      if (found_spmd) {
	Value *NPEs_val = found_spmd->getOperand(0);
	int NPEs = (dyn_cast<ConstantInt>(NPEs_val))->getZExtValue();
	found_spmd->eraseFromParent();
	
	//Fix me: We assume a maximum of 16 reductions in the loop
	std::vector<PHINode *> Reductions(16);
	std::vector<Value *> ReduceVarExitOrig(16);
	std::vector<Instruction *> ReduceVarOrig(16);
	//there is OldInst foreach reduction variable
	std::vector<Instruction *> OldInsts(16);
	FindReductionVariables(L, &Reductions, &ReduceVarExitOrig, &ReduceVarOrig);
	BasicBlock *PH = SplitBlock(OrigPH, OrigPH->getTerminator(), DT, LI);
	PH->setName(L->getHeader()->getName() + ".ph");
	
	BasicBlock *OrigE = L->getExitBlock();
	//Fixme: should return all successors
	BasicBlock *AfterLoop = OrigE->getSingleSuccessor();
	Instruction *i = dyn_cast<Instruction>(OrigE->begin());
	BasicBlock *E = SplitBlock(OrigE, i, DT, LI);
	OrigE->setName(L->getHeader()->getName() + ".e");
	//TODO: I need to update the other phi nodes in F->blocks to point to OrigE
	 
	
	//Add CSA parallel intrinsics:
	AddParallelIntrinsicstoLoop(L, context, M, OrigPH, E);
       
	TransformLoopInitandStep(L, SE, 0, NPEs);
	SmallVector<Value *, 128> NewReducedValues;//should be equal to NPEs
	for(int PE=1; PE<NPEs; PE++) {
	  SmallVector<BasicBlock *, 8> NewLoopBlocks;
	  BasicBlock *Exit = L->getExitBlock();
	  //clone the exit block, to be attached to the cloned loop
	  BasicBlock *NewE = CloneBasicBlock(Exit, VMap, ".PE" + std::to_string(PE), F);
	  VMap[Exit] = NewE;
	  Loop *NewLoop =
	    cloneLoopWithPreheader(PH, OrigPH, L, VMap,
				   ".PE" + std::to_string(PE), LI, 
                                   DT, NewLoopBlocks);
	  
	  NewLoopBlocks.push_back(NewE);
	  remapInstructionsInBlocks(NewLoopBlocks, VMap);
	  // Update LoopInfo.
	  if(OrigL->getParentLoop())
	    OrigL->getParentLoop()->addBasicBlockToLoop(NewE, *LI);
	  // Add DominatorTree node, update to correct IDom.
	  DT->addNewBlock(NewE, NewLoop->getLoopPreheader());
	  Instruction *ExitTerm = Exit->getTerminator();
	  BranchInst::Create(NewLoop->getLoopPreheader(), Exit);
	  ExitTerm->eraseFromParent();
	  
	  TransformLoopInitandStep(NewLoop, SE, 1, NPEs);
       
	  L = NewLoop;
	  
	  FixReductionsIfAny(L, AfterLoop, PE, NPEs, &Reductions, &ReduceVarExitOrig, &ReduceVarOrig, &OldInsts);	  
	}
      }
      return true;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      getLoopAnalysisUsage(AU);
    }
  };
}

#define DEBUG_TYPE "spmdization"
char LoopSPMDization::ID = 0;

INITIALIZE_PASS_BEGIN(LoopSPMDization, DEBUG_TYPE, "Loop SPMDization", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopAccessLegacyAnalysis)
INITIALIZE_PASS_DEPENDENCY(LoopPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_END(LoopSPMDization, DEBUG_TYPE, "Loop SPMDization", false, false)

Pass *llvm::createLoopSPMDizationPass() {
  return new LoopSPMDization();
}

bool LoopSPMDization::FindReductionVariables(Loop *L, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig) {
  int r = 0;
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    RecurrenceDescriptor RedDes;
    if (RecurrenceDescriptor::isReductionPHI(Phi, L, RedDes)) {
      (*Reductions)[r] = Phi; 
      if (Phi->getIncomingBlock(0) == L->getLoopPreheader()){
	(*ReduceVarOrig)[r] = dyn_cast<Instruction>(Phi->getIncomingValue(1));
      }
      else {
	(*ReduceVarOrig)[r] = dyn_cast<Instruction>(Phi->getIncomingValue(0));
      }
      
      BasicBlock::iterator i, ie; 
      for (i = L->getExitBlock()->begin(), ie = L->getExitBlock()->end();  (i != ie); ++i) {
	PHINode *PhiExit = dyn_cast<PHINode>(&*i);
	if (!PhiExit)
	  continue;
	Instruction *ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(0));
	if(dyn_cast<Value>(ReduceVarExit) == dyn_cast<Value>((*ReduceVarOrig)[r])) {
	  (*ReduceVarExitOrig)[r] = dyn_cast<Value>(PhiExit);
	}
      }
      r++;
    }
  }
  return true;
}

//Handling of reductions
bool LoopSPMDization::FixReductionsIfAny(Loop *L, BasicBlock *AfterLoop, int PE, int NPEs, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, std::vector<Instruction *> *OldInsts) {
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    for(uint r=0; r<Reductions->size(); r++){
      if(!Reductions->at(r))	
	break;
      //Use the new names in the new loops generated by the clone function
      std::string RedName = (*Reductions)[r]->getName();
      for(int pe=1; pe<=PE; pe++)
	RedName = RedName +".PE" + std::to_string(pe);  
      if(Phi->getName().str().compare(RedName)==0) {
	Instruction *ReduceVar;
	if (Phi->getIncomingBlock(0) == L->getLoopPreheader())
	  ReduceVar = dyn_cast<Instruction>(Phi->getIncomingValue(1));
	else 
	  ReduceVar = dyn_cast<Instruction>(Phi->getIncomingValue(0));
	BasicBlock::iterator i, ie; 
	for (i = AfterLoop->begin(), ie = AfterLoop->end();  (i != ie); ++i) {
	  PHINode *PhiExit = dyn_cast<PHINode>(&*i);
	  if (!PhiExit)
	    continue;
	  Instruction *ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(1));
	  
	  if(dyn_cast<Value>(ReduceVarExit) == (*ReduceVarExitOrig)[r]) {
	    //add phi node
	    Instruction *NewInstPhi = PhiExit->clone();
	    PHINode *NewPhi = dyn_cast<PHINode>(NewInstPhi);
	    NewPhi->setIncomingValue(1, ReduceVar);
	    IRBuilder<> B(AfterLoop->getFirstNonPHI());
	    AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInstPhi);
	    //Phi corresponding to first loop is already there
	    if(PE == 1) {
	      (*OldInsts)[r] = PhiExit;
	      B.SetInsertPoint(AfterLoop->getFirstNonPHI());
	    }
	    else
	      B.SetInsertPoint((*OldInsts)[r]->getNextNode());
	    
	    Instruction *NewInst = (*ReduceVarOrig)[r]->clone();
	    (*OldInsts)[r]->replaceAllUsesWith(NewInst);
	    NewInst->setOperand(1, dyn_cast<Value>((*OldInsts)[r]));
	    NewInst->setOperand(0, dyn_cast<Value>(NewInstPhi));
	    AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInst);
	    (*OldInsts)[r] = NewInst;
	  }
	}
      }
    }
  }
  return true;
}


/* This routine should made generic and be declared somewhere as public to be used here and in csa backend (Target/CSA/CSALoopIntrinsicExpander.cpp)*/
IntrinsicInst* LoopSPMDization::detectSPMDIntrinsic(Loop *L, LoopInfo *LI) {
  // Start iterating backwards at the preheader
  for (BasicBlock *cur_block = L->getLoopPreheader();
       cur_block and LI->getLoopFor(cur_block) == LI->getLoopFor(L->getLoopPreheader());
       cur_block = cur_block->getSinglePredecessor()
       ) {
    
    // Look for intrinsic calls with the right ID.
    for (Instruction& inst : *cur_block)
      if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
	if (intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization)//|| intr_inst->getIntrinsicID() == Intrinsic::x86_spmdization)
	  return intr_inst;
  }
  return nullptr;
}

/* This routine has been copied from LoopInterchange.cpp where it is declared static */
PHINode *LoopSPMDization::getInductionVariable(Loop *L, ScalarEvolution *SE) {
  PHINode *InnerIndexVar = L->getCanonicalInductionVariable();
  if (InnerIndexVar)
    return InnerIndexVar;
  if (L->getLoopLatch() == nullptr || L->getLoopPredecessor() == nullptr)
    return nullptr;
  for (BasicBlock::iterator I = L->getHeader()->begin(); isa<PHINode>(I); ++I) {
    PHINode *PhiVar = cast<PHINode>(I);
    Type *PhiTy = PhiVar->getType();
    if (!PhiTy->isIntegerTy() && !PhiTy->isFloatingPointTy() &&
	!PhiTy->isPointerTy())
      return nullptr;
    if (!PhiTy->isIntegerTy())
      continue;
    if(!SE->isSCEVable(PhiVar->getType()))
      continue;
    const SCEVAddRecExpr *AddRec =
      dyn_cast<SCEVAddRecExpr>(SE->getSCEV(PhiVar));
    if (!AddRec || !AddRec->isAffine())
      continue;
    const SCEV *Step = AddRec->getStepRecurrence(*SE);
    if (!isa<SCEVConstant>(Step))
      continue;
    // Found the induction variable.
    // FIXME: Handle loops with more than one induction variable. Note that,
    // currently, legality makes sure we have only one induction variable.
    return PhiVar;
  }
  return nullptr;
}

bool LoopSPMDization::TransformLoopInitandStep(Loop *L, ScalarEvolution *SE, int PE, int NPEs) {
  
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  if (!InductionPHI) {
    DEBUG(dbgs() << "Failed to find the loop induction variable \n");
    return false;
  }
  Instruction *InnerIndexVar;
  Value *InitVar;
  if (InductionPHI->getIncomingBlock(0) == PreHeader){
    InnerIndexVar = dyn_cast<Instruction>(InductionPHI->getIncomingValue(1));
    InitVar = InductionPHI->getIncomingValue(0);
  }
  else {
    InnerIndexVar = dyn_cast<Instruction>(InductionPHI->getIncomingValue(0));
    InitVar = InductionPHI->getIncomingValue(1);
  }
  IRBuilder<> B2(InnerIndexVar);
  Value *NewInc = B2.CreateAdd(InductionPHI,
			       ConstantInt::get(InductionPHI->getType(), NPEs), 
			       InductionPHI->getName()+".newnext");
  
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  Value *Cond = LatchBR->getCondition();
  Instruction *CondI = dyn_cast<Instruction>(Cond);
  Value *TripCount = CondI->getOperand(1);
  Value *IdxCmp;
  if (LatchBR->getSuccessor(0) == L->getHeader())
    IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_ULT, 
			     NewInc,
			     TripCount, 
			     Cond->getName());
  
  else
    IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_UGE, 
			     NewInc,
			     TripCount, 
			     Cond->getName());
  ReplaceInstWithInst(CondI, dyn_cast<Instruction>(IdxCmp));

  IRBuilder<> B(PreHeaderBR);
  Value *NewInitV = B.CreateAdd(InitVar,
				ConstantInt::get(InductionPHI->getType(), PE), 
				InductionPHI->getName()+
				".init", dyn_cast<Instruction>(NewInc));
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    InductionPHI->setIncomingValue(0, NewInitV );
    InductionPHI->setIncomingValue(1, NewInc );
  }
  else {
    InductionPHI->setIncomingValue(1, NewInitV );
    InductionPHI->setIncomingValue(0, NewInc );
  }
 
  return true;
}

bool LoopSPMDization::AddParallelIntrinsicstoLoop(Loop *L, LLVMContext& context, Module *M, BasicBlock *OrigPH, BasicBlock *E) {
  Function* FIntr = Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_region_entry); 
  int next_token = 1;
  Instruction*const header_terminator = OrigPH->getTerminator();
  Instruction*const preheader_terminator = L->getLoopPreheader()->getTerminator();
  CallInst *region_entry = IRBuilder<>{header_terminator}.CreateCall(
    FIntr, 
    ConstantInt::get(IntegerType::get(context, 32), next_token++), 
    "parallel_region_entry"
  );
  
  CallInst *section_entry = IRBuilder<>{preheader_terminator}.CreateCall(
    Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_section_entry), 
    region_entry, 
    "parallel_section_entry"
  );
  
  IRBuilder<>{preheader_terminator}.CreateCall(
	      Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_loop));
  
  // The csa.parallel.region.exit intrinsic goes at the beginning of the loop exit.
  SmallVector<BasicBlock*, 2> exits;
  L->getExitBlocks(exits);
  for (BasicBlock *const exit : exits) {
    IRBuilder<>{exit->getFirstNonPHI()}.CreateCall(
      Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_section_exit),
      section_entry
    );
  }
  IRBuilder<>{E->getFirstNonPHI()}.CreateCall(
    Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_region_exit), 
    region_entry
  );
  
  return true;
}


