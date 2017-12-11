//===- LoopSPMDization.cpp - Loop SPMDization pass          ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements the Loop SPMDization transformation that generates multiple loops from one loop. These loops can run in parallel. Two approaches are implemented here: the cyclic approach where each loop has a stride of k and teh blocking approach where each loop iterates over contiguous #iterations/NPEs iterations.
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

#define SPMD_CYCLIC 1
#define SPMD_BLOCKING 2

namespace {
  class LoopSPMDization : public LoopPass {
    LLVMContext Context;
  public:
    static char ID;
    LoopSPMDization() : LoopPass(ID) {
      initializeLoopSPMDizationPass(*PassRegistry::getPassRegistry());
    }
  private:
    int next_token;
    Value *steptimesk; 
    Value *StepPE0;
    Value *NewInitV; 
    Value *Cond;
    Value *nbyk;
    Value *UpperBound;
    Value * LowerBound;
    bool FixReductionsIfAny(Loop *L, Loop *OrigL, BasicBlock *E, BasicBlock *AfterLoop, int PE, int NPEs, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, std::vector<Instruction *> *OldInst);
    bool FindReductionVariables(Loop *L, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig);
    PHINode *getInductionVariable(Loop *L, ScalarEvolution *SE);
    bool TransformLoopInitandStep(Loop *L, ScalarEvolution *SE, int PE, int NPEs);
    bool TransformLoopInitandBound(Loop *L, ScalarEvolution *SE, int PE, int NPEs);
    bool ZeroTripCountCheck(Loop *L, ScalarEvolution *SE, int PE, int NPEs, BasicBlock *AfterLoop, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, DominatorTree *DT, LoopInfo *LI); 
    bool AddParallelIntrinsicstoLoop(Loop *L, LLVMContext& context, Module *M, BasicBlock *OrigPH, BasicBlock *E);
    IntrinsicInst* detectSPMDIntrinsic(Loop *L, LoopInfo *LI);
    IntrinsicInst* detectSPMDExitIntrinsic(Loop *L, LoopInfo *LI);

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
      unsigned spmd_approach = 0;
      IntrinsicInst* found_spmd = detectSPMDIntrinsic(L, LI);
      if (found_spmd) {
        Value *NPEs_val = found_spmd->getOperand(0);
        Value *approachV  = found_spmd->getOperand(1);
        if(ConstantExpr *expr = dyn_cast<ConstantExpr>(approachV)) {
          if (expr->getOpcode() == Instruction::GetElementPtr) {
            GlobalVariable *glob_arg = dyn_cast<GlobalVariable>(expr->getOperand(0));
            if (glob_arg and glob_arg->isConstant() and glob_arg->getInitializer()) {
              // Unlike C, Fortran string has no null byte at the end
              StringRef user_approach;
              if(dyn_cast<ConstantDataArray>(glob_arg->getInitializer())->isCString())
                user_approach = dyn_cast<ConstantDataArray>(glob_arg->getInitializer())->getAsCString();
              else
                user_approach = dyn_cast<ConstantDataArray>(glob_arg->getInitializer())->getAsString();
           
              if(user_approach.compare_lower("cyclic")==0) {
                spmd_approach = SPMD_CYCLIC;
              }
              else if(user_approach.compare_lower("blocked")==0 || user_approach.compare_lower("blocking")==0) {
                spmd_approach = SPMD_BLOCKING;
              }
              else if(user_approach.compare_lower("hybrid")==0) {
                errs() << "\n";
                errs().changeColor(raw_ostream::BLUE, true);
                errs() << "!! WARNING: Hybrid Approach of SPMD is not supported yet !!";
                errs().resetColor();
                return false;
              }
              else {
                errs() << "\n";
                errs().changeColor(raw_ostream::BLUE, true);
                errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
                errs().resetColor();
                errs() << " Second argument should be Cyclic, Blocked, Blocking, or Hybrid.\n"
                  "This call will be ignored.\n\n";
                return false;
              }
            }
            else {
              errs() << "\n";
              errs().changeColor(raw_ostream::BLUE, true);
              errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
              errs().resetColor();
              return false;
            }
          }     
          else {
            errs() << "\n";
            errs().changeColor(raw_ostream::BLUE, true);
            errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
            errs().resetColor();
            return false;
          }
        }
        else {
          errs() << "\n";
          errs().changeColor(raw_ostream::BLUE, true);
          errs() << "!! WARNING: BAD CSA SPMD INTRINSIC !!";
          errs().resetColor();
          return false;
        }
        int NPEs = (dyn_cast<ConstantInt>(NPEs_val))->getZExtValue();
        IntrinsicInst* found_spmd_exit = detectSPMDExitIntrinsic(L, LI);
        if(found_spmd_exit) {
          for (auto UA = (dyn_cast<Value>(found_spmd))->user_begin(), EA = (dyn_cast<Value>(found_spmd))->user_end(); UA != EA;) {
            Instruction *spmd_use = cast<Instruction>(*UA++);
            spmd_use->eraseFromParent();
          }
          found_spmd->eraseFromParent();
        }
        if(!L->getExitBlock()) {
          errs() << "\n";
          errs().changeColor(raw_ostream::BLUE, true);
          errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
          errs().resetColor();
          errs() << R"help(The SPMDization loop body has unstructured code.

Branches to or from an OpenMP structured block are illegal

)help";
          return false;
        }
        
        //Fix me: We assume a maximum of 16 reductions in the loop
        std::vector<PHINode *> Reductions(16);
        std::vector<Value *> ReduceVarExitOrig(16);
        std::vector<Instruction *> ReduceVarOrig(16);
        //there is OldInst foreach reduction variable
        std::vector<Instruction *> OldInsts(16);
        FindReductionVariables(L, &Reductions, &ReduceVarExitOrig, &ReduceVarOrig);
        if(spmd_approach == SPMD_CYCLIC) {
          if(!TransformLoopInitandStep(L, SE, 0, NPEs)) {
            return false;
          } 
        }
        else if(spmd_approach == SPMD_BLOCKING) { 
          const DataLayout &DL = L->getHeader()->getModule()->getDataLayout();
          SCEVExpander Expander(*SE, DL, "loop-SPMDization");
          BranchInst *PreHeaderBR = cast<BranchInst>(L->getLoopPreheader()->getTerminator());
          const SCEV *BECountSC = SE->getBackedgeTakenCount(L);
          const SCEV *TripCountSC =
            SE->getAddExpr(BECountSC, SE->getConstant(BECountSC->getType(), 1));
          Value *TripCountV = Expander.expandCodeFor(TripCountSC, 
                                                     TripCountSC->getType(),    
                                                     PreHeaderBR);
     
          IRBuilder<> BPR(L->getLoopPreheader()->getTerminator());
          nbyk = BPR.CreateUDiv(TripCountV,
                                ConstantInt::get(BECountSC->getType(), NPEs), 
                                ".nbyk");
          TransformLoopInitandBound(L, SE, 0, NPEs);
        }
     

        BasicBlock *PH = SplitBlock(OrigPH, OrigPH->getTerminator(), DT, LI);
        PH->setName(L->getHeader()->getName() + ".ph");
        BasicBlock *OrigE = L->getExitBlock();
        
        BasicBlock *AfterLoop = OrigE->getSingleSuccessor();
        Instruction *i = dyn_cast<Instruction>(OrigE->begin());
        BasicBlock *E = SplitBlock(OrigE, i, DT, LI);
        OrigE->setName(L->getHeader()->getName() + ".e");
        if(!AfterLoop)
          AfterLoop = E;
        
        //Add CSA parallel intrinsics:
        AddParallelIntrinsicstoLoop(L, context, M, OrigPH, E);
        SmallVector<Value *, 128> NewReducedValues;//should be equal to NPEs
        for(int PE = 1; PE < NPEs; PE++) {
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
          
          if(spmd_approach == SPMD_CYCLIC) 
            TransformLoopInitandStep(NewLoop, SE, 1, NPEs);
          else if(spmd_approach == SPMD_BLOCKING) 
            TransformLoopInitandBound(NewLoop, SE, PE, NPEs);
       
          ZeroTripCountCheck(NewLoop, SE, PE, NPEs, AfterLoop, &Reductions, &ReduceVarExitOrig, &ReduceVarOrig, DT, LI);
          //This assumes -ffp-contract=fast is set
          FixReductionsIfAny(NewLoop, OrigL, E, AfterLoop, PE, NPEs, &Reductions, &ReduceVarExitOrig, &ReduceVarOrig, &OldInsts); 

          L = NewLoop;
          
        }
        //Fix missed Phi operands in AfterLoop
        BasicBlock::iterator bi, bie; 
        for (bi = AfterLoop->begin(), bie = AfterLoop->end();  (bi != bie); ++bi) {
          PHINode *RedPhi = dyn_cast<PHINode>(&*bi);
          if(!RedPhi)
            continue;
          Value *RedV;
          if(AfterLoop == L->getExitBlock()->getSingleSuccessor()) {
            if(RedPhi->getBasicBlockIndex(L->getExitBlock()) == -1) 
              //Afterloop did not have a phi node
              RedPhi->setIncomingBlock(0, L->getExitBlock());
            
            RedV = RedPhi->getIncomingValueForBlock(L->getExitBlock());
          }
          else {
            //if(RedPhi->getBasicBlockIndex(L->getExitBlock()->getSinglePredecessor()) == -1) 
            //Afterloop did not have a phi node
            //RedPhi->setIncomingBlock(0, L->getExitBlock()->getSinglePredecessor());
            RedV = RedPhi->getIncomingValueForBlock(L->getExitBlock()->getSingleSuccessor());
          }
          for (auto it = pred_begin(AfterLoop), et = pred_end(AfterLoop); it != et; ++it) {
            BasicBlock* predecessor = *it;
            if(RedPhi->getBasicBlockIndex(predecessor) == -1) {
              RedPhi->addIncoming(RedV, predecessor);
            }
          }
        }
      }
      return true;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      //getLoopAnalysisUsage(AU);
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addPreserved<DominatorTreeWrapperPass>();
      AU.addRequired<LoopInfoWrapperPass>();
      AU.addPreserved<LoopInfoWrapperPass>();
      AU.addRequired<ScalarEvolutionWrapperPass>();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequiredID(LoopSimplifyID);
      AU.addRequiredID(LCSSAID);
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
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
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
      Value *ReduceVar;
      PHINode *Phiop = Phi;
      PHINode *redoperation;
      if (Phi->getIncomingBlock(0) == L->getLoopPreheader()) {
        ReduceVar = dyn_cast<Value>(Phi->getIncomingValue(1));
        redoperation = dyn_cast<PHINode>(Phiop->getIncomingValue(1));
        (*ReduceVarOrig)[r]= dyn_cast<Instruction>(Phiop->getIncomingValue(1));
      }
      else {
        ReduceVar = dyn_cast<Value>(Phi->getIncomingValue(0));
        redoperation = dyn_cast<PHINode>(Phiop->getIncomingValue(0));
        (*ReduceVarOrig)[r]= dyn_cast<Instruction>(Phiop->getIncomingValue(0));
      }
      while(redoperation) {
        Phiop = redoperation;
        //We could choose 0 or 1 values but we test both to avoid cyclic Phis
        redoperation = dyn_cast<PHINode>(dyn_cast<Instruction>(Phiop->getIncomingValue(0)));
        if(redoperation) { 
          redoperation = dyn_cast<PHINode>(dyn_cast<Instruction>(Phiop->getIncomingValue(1)));
          (*ReduceVarOrig)[r]= dyn_cast<Instruction>(Phiop->getIncomingValue(1));
        }
        else
          (*ReduceVarOrig)[r]= dyn_cast<Instruction>(Phiop->getIncomingValue(0));
      }
      BasicBlock::iterator i, ie; 
      for (i = L->getExitBlock()->begin(), ie = L->getExitBlock()->end();  (i != ie); ++i) {
        PHINode *PhiExit = dyn_cast<PHINode>(&*i);
        if (!PhiExit)
          continue;
        Instruction *ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(0));
        if(dyn_cast<Value>(ReduceVarExit) == dyn_cast<Value>(ReduceVar)) {
          
          (*ReduceVarExitOrig)[r] = dyn_cast<Value>(PhiExit);
        }
      }
      r++;
    }
  }
  return true;
}

//Handling of reductions
bool LoopSPMDization::FixReductionsIfAny(Loop *L, Loop *OrigL, BasicBlock *E, BasicBlock *AfterLoop, int PE, int NPEs, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, std::vector<Instruction *> *OldInsts) {
  BasicBlock *pred_AfterLoop;
  if(AfterLoop == L->getExitBlock()->getSingleSuccessor())
    pred_AfterLoop = L->getExitBlock();
  else
    pred_AfterLoop = L->getExitBlock()->getSinglePredecessor();  
          
  for (Instruction &I : *L->getHeader()) {
    PHINode *Phi = dyn_cast<PHINode>(&I);
    if (!Phi)
      continue;
    for(unsigned r=0; r<Reductions->size(); r++) {
      if(!Reductions->at(r))        
        break;
      //Use the new names in the new loops generated by the clone function
      std::string RedName = (*Reductions)[r]->getName();
      for(int pe = 1; pe <= PE; pe++)
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
          Instruction *NewInstPhi; 
          PHINode *NewPhi;
          Instruction *ReduceVarExit;
           IRBuilder<> B(AfterLoop->getFirstNonPHI());
          bool found_p = false;
          //look for use of the reduced value
          if (!PhiExit) {
            for(unsigned m = 0; m < i->getNumOperands(); m++) {
              if(i->getOperand(m) == (*ReduceVarExitOrig)[r]) {
                ReduceVarExit = dyn_cast<Instruction>(i->getOperand(m));
                if(PE == 1) {
                  PhiExit = B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "orig");
                  PhiExit->addIncoming((*ReduceVarExitOrig)[r], pred_AfterLoop);
                  i->setOperand(m, PhiExit);
                }
                NewPhi = B.CreatePHI(ReduceVar->getType(), 1, Phi->getName() + "red");
                NewPhi->addIncoming(ReduceVar, pred_AfterLoop);//->getSinglePredecessor());
                NewInstPhi = dyn_cast<Instruction>(NewPhi);
                found_p = true;
              }
            }
          }
          else {// There is an actual Phi node for the reduction var
            if(PhiExit->getNumIncomingValues() >= 2)// == 2) zero trip change
              ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(1));
            else
              ReduceVarExit = dyn_cast<Instruction>(PhiExit->getIncomingValue(0));
            if(dyn_cast<Value>(ReduceVarExit) == (*ReduceVarExitOrig)[r]) {
              NewInstPhi = PhiExit->clone();
              NewPhi = dyn_cast<PHINode>(NewInstPhi);
              
              if(PhiExit->getNumIncomingValues() >= 2) // ==2
                NewPhi->setIncomingValue(1, ReduceVar);
              else
                NewPhi->setIncomingValue(0, ReduceVar);
      
              AfterLoop->getInstList().insert(B.GetInsertPoint(), NewInstPhi);
              found_p = true;
            } 
          }
          if (found_p) {
            // Handling of the new branches related to the zero trip count
            BasicBlock* BB = AfterLoop;
            for (auto it = pred_begin(BB), et = pred_end(BB); it != et; ++it) {
              BasicBlock* predecessor = *it;
              //if(predecessor == L->getLoopPreheader()->getSinglePredecessor()) 
              {// this is the predecessor coming from the zero trip count gard block
                if(NewPhi->getBasicBlockIndex(predecessor) == -1 && predecessor != pred_AfterLoop) {
                  Value *Ident;
                  Type *Ty = NewPhi->getType();
                  switch ((*ReduceVarOrig)[r]->getOpcode()) {
                  case Instruction::Add:
                  case Instruction::FAdd:
                    {
                      Ident =  Constant::getNullValue(Ty);
                      break;
                    }
                  case Instruction::Or:
                  case Instruction::Xor:{
                    Ident =  Constant::getNullValue(Ty);
                    break;
                  }
                  case Instruction::Mul:{
                    Ident = ConstantInt::get(Ty, 1);
                    break;
                  }
                  case Instruction::And:{
                    Ident =  Constant::getAllOnesValue(Ty);
                    break;
                  }
                  default:{
                    // Doesn't have an identity.
                    errs() << "\n";
                    errs().changeColor(raw_ostream::BLUE, true);
                    errs() << "!! ERROR: COULD NOT PERFORM SPMDization !!\n";
                    errs().resetColor();
                    errs() << R"help(
Failed to find the identity element of the reduction operation.

)help";
                    Ident =  nullptr;
                    break;
                  }
                  }
                  NewPhi->addIncoming(Ident, predecessor);
                }
              }
            } 
            //Phi corresponding to first cloned loop is already there
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
            break;
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
        if (intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization_entry)//|| intr_inst->getIntrinsicID() == Intrinsic::x86_spmdization)
          return intr_inst;
  }
  return nullptr;
}
IntrinsicInst* LoopSPMDization::detectSPMDExitIntrinsic(Loop *L, LoopInfo *LI) {
  SmallVector<BasicBlock*, 2> exits;
  L->getExitBlocks(exits);
  for (BasicBlock *const exit : exits) {
    for (Instruction& inst : *exit)
      if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
        if (intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization_exit)//|| intr_inst->getIntrinsicID() == Intrinsic::x86_spmdization)
          return intr_inst;
    
    BasicBlock *afterexit = exit->getSingleSuccessor();
    //Optimizations might move the exit intrinsic to the next exit
    for (Instruction& inst : *afterexit)
      if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
        if (intr_inst->getIntrinsicID() == Intrinsic::csa_spmdization_exit)//|| intr_inst->getIntrinsicID() == Intrinsic::x86_spmdization)
          return intr_inst;
  }
  return nullptr;
}


/* This routine has been copied from LoopInterchange.cpp. It has then been modified to accomodate the type of induction variables we are insterested in handling for SPMDization  */
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

bool LoopSPMDization::TransformLoopInitandBound(Loop *L, ScalarEvolution *SE, int PE, int NPEs) {
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  if (!InductionPHI) {
    DEBUG(dbgs() << "Failed to find the loop induction variable \n");
    return false;
  }
  IRBuilder<> B(PreHeaderBR);
  Cond = LatchBR->getCondition();
  Instruction *CondI = dyn_cast<Instruction>(Cond);
  if(PE == 0) {
    if (InductionPHI->getIncomingBlock(0) == PreHeader){
      LowerBound = InductionPHI->getIncomingValue(0);
    }
    else {
      LowerBound = InductionPHI->getIncomingValue(1);
    }
    UpperBound = CondI->getOperand(1);
    if(dyn_cast<IntegerType>(nbyk->getType())->getBitWidth() != dyn_cast<IntegerType>(LowerBound->getType())->getBitWidth())
      nbyk = B.CreateZExtOrTrunc(nbyk, LowerBound->getType(), nbyk->getName()+".trex"); 
  }
  //i = i+PE ==> i+ (k-1)n/NPEs ==> i+(k-1)*nbyNPEs
  Value *ktimesnbyk = B.CreateMul(ConstantInt::get(nbyk->getType(), PE),
                                  nbyk,
                                  InductionPHI->getName()+
                                  ".ktimesnbyk"
                                  );
  Value *kplus1 = B.CreateAdd(ConstantInt::get(nbyk->getType(), PE),
                              ConstantInt::get(nbyk->getType(), 1), 
                              InductionPHI->getName()+
                              ".kplus1");
  Value *kplus1timesnbyk = B.CreateMul(kplus1, 
                                       nbyk,
                                       InductionPHI->getName()+
                                       ".k+1xnbyk"
                                       );
  Value *kplus1timesnbyk2 = B.CreateAdd(kplus1timesnbyk, 
                                        LowerBound,
                                        InductionPHI->getName()+
                                        ".k+1xnbyk2"
                                        );
  NewInitV = B.CreateAdd(LowerBound,
                         ktimesnbyk, 
                         InductionPHI->getName()+
                         ".init");
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    InductionPHI->setIncomingValue(0, NewInitV );
  }
  else {
    InductionPHI->setIncomingValue(1, NewInitV );
  }
  //change bound (cond)
  if(dyn_cast<IntegerType>(kplus1timesnbyk2->getType())->getBitWidth() != dyn_cast<IntegerType>(UpperBound->getType())->getBitWidth())
    kplus1timesnbyk2 = B.CreateZExtOrTrunc(kplus1timesnbyk2, UpperBound->getType(), kplus1timesnbyk2->getName()+".trex"); 
  
  if(PE == NPEs-1) 
    kplus1timesnbyk2 = UpperBound;
  
  CondI->setOperand(1, kplus1timesnbyk2);
  return true;
}



bool LoopSPMDization::TransformLoopInitandStep(Loop *L, ScalarEvolution *SE, int PE, int NPEs) {
  
  PHINode *InductionPHI = getInductionVariable(L, SE);
  BasicBlock *PreHeader = L->getLoopPreheader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  if (!InductionPHI) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
    errs().resetColor();
    errs() << R"help(
Failed to find the loop induction variable.

)help";
    DEBUG(dbgs() << "Failed to find the loop induction variable \n");
    return false;
  }
  Instruction *OldInc;
  Value *InitVar;
  if (InductionPHI->getIncomingBlock(0) == PreHeader) {
    OldInc = dyn_cast<Instruction>(InductionPHI->getIncomingValue(1));
    InitVar = InductionPHI->getIncomingValue(0);
  }
  else {
    OldInc = dyn_cast<Instruction>(InductionPHI->getIncomingValue(0));
    InitVar = InductionPHI->getIncomingValue(1);
  }
  IRBuilder<> B2(OldInc);
  if(PE == 0) {
    StepPE0 = OldInc->getOperand(1);
    steptimesk = B2.CreateMul(OldInc->getOperand(1),
                              ConstantInt::get(InductionPHI->getType(), NPEs),
                              InductionPHI->getName()+
                              ".steptimesk"
                              );
  }
  Value *NewInc = B2.CreateAdd(InductionPHI,
                               steptimesk,
                               InductionPHI->getName()+".next.spmd");
  
  IRBuilder<> B(PreHeaderBR);
  Value *steptimespe = B.CreateMul(StepPE0,
                                   ConstantInt::get(InductionPHI->getType(), PE),
                                   InductionPHI->getName()+
                                   ".steptimesPE"
                                   );
  /*Value **/NewInitV = B.CreateAdd(InitVar,
                                    steptimespe,
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
  
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  /*Value **/Cond = LatchBR->getCondition();
  Instruction *CondI = dyn_cast<Instruction>(Cond);
  bool cond_found_p = false;
  if(CondI->getOperand(0) == dyn_cast<Value>(OldInc) || CondI->getOperand(1) == dyn_cast<Value>(OldInc))
    cond_found_p = true;
  else if(CondI->getOperand(0) == dyn_cast<Value>(InductionPHI) || CondI->getOperand(1) == dyn_cast<Value>(InductionPHI)) {
    cond_found_p = true;
    OldInc = dyn_cast<Instruction>(InductionPHI); 
    NewInc = dyn_cast<Value>(InductionPHI);
  }
  else {
    for (auto UA = (dyn_cast<Value>(OldInc))->user_begin(), EA = (dyn_cast<Value>(OldInc))->user_end(); UA != EA;) {
      Instruction *User_OldInc = cast<Instruction>(*UA++);
      if(CondI->getOperand(0) == dyn_cast<Value>(User_OldInc) || CondI->getOperand(1) == dyn_cast<Value>(User_OldInc)) {
        cond_found_p = true;
        for(unsigned m = 0; m < User_OldInc->getNumOperands(); m++) 
          if(User_OldInc->getOperand(m) == dyn_cast<Value>(OldInc)) { 
            User_OldInc->setOperand(m, NewInc);
            //OldInc->replaceAllUsesWith(NewInc);
            NewInc = User_OldInc;
            OldInc = User_OldInc;
          } 
      }
    }
  }
  if(!cond_found_p) {
    errs() << "\n";
    errs().changeColor(raw_ostream::BLUE, true);
    errs() << "!! WARNING: COULD NOT PERFORM SPMDization !!\n";
    errs().resetColor();
    errs() << R"help(
Failed to find the loop latch condition.

)help";
    return false;
  }
  else {
    Value *TripCount = CondI->getOperand(1);
    Value *IdxCmp;
    CmpInst *CmpCond = dyn_cast<CmpInst>(Cond);
    Value *NewCondOp0, *NewCondOp1;
    if(CondI->getOperand(0) == dyn_cast<Value>(OldInc)) {
      NewCondOp0 = NewInc;
      NewCondOp1 = TripCount;
    }
    else {
      NewCondOp1 = NewInc;
      NewCondOp0 = TripCount;
    }   
    if (CmpCond->getPredicate() == CmpInst::ICMP_EQ || CmpCond->getPredicate() == CmpInst::ICMP_NE) {  
      if(LatchBR->getSuccessor(0) == L->getHeader())
        IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLT, 
                                 NewCondOp0, 
                                 NewCondOp1,  
                                 Cond->getName());
    
      else 
        IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGE, 
                                 NewCondOp0, 
                                 NewCondOp1, 
                                 Cond->getName());
      ReplaceInstWithInst(CondI, dyn_cast<Instruction>(IdxCmp));
    }
    else { //in other cases, we keep the same predicate
      if(CondI->getOperand(0) == dyn_cast<Value>(OldInc)) 
        CondI->setOperand(0, NewInc);
      else
        CondI->setOperand(1, NewInc);
    }
  }
  return true;
}


bool LoopSPMDization::ZeroTripCountCheck(Loop *L, ScalarEvolution *SE, int PE, int NPEs, BasicBlock *AfterLoop, std::vector<PHINode *> *Reductions, std::vector<Value *> *ReduceVarExitOrig, std::vector<Instruction *> *ReduceVarOrig, DominatorTree *DT, LoopInfo *LI) {
  
  BasicBlock *PreHeader = L->getLoopPreheader();
  BranchInst *PreHeaderBR = cast<BranchInst>(PreHeader->getTerminator());
  BasicBlock *Latch = L->getLoopLatch();
  IRBuilder<> B(PreHeaderBR);
  BranchInst *LatchBR = cast<BranchInst>(Latch->getTerminator());
  Instruction *CondI = dyn_cast<Instruction>(Cond);
  
  Value *TripCount = CondI->getOperand(1);
  Value *IdxCmp;
  CmpInst *CmpCond = dyn_cast<CmpInst>(Cond);
  Instruction *CmpZeroTrip;
  Value *NewCondOp0, *NewCondOp1;
  if(dyn_cast<IntegerType>(NewInitV->getType())->getBitWidth() > dyn_cast<IntegerType>(TripCount->getType())->getBitWidth()){
    auto *Trunc = B.CreateTrunc(NewInitV, TripCount->getType(), NewInitV->getName()+".trunk");
    NewInitV = Trunc;  
  }
  else {
    auto *Trunc = B.CreateSExt(NewInitV, TripCount->getType(), NewInitV->getName()+".sext");
    NewInitV = Trunc;  
  }
  NewCondOp1 = NewInitV; 
  NewCondOp0 = TripCount;
  if (CmpCond->getPredicate() == CmpInst::ICMP_EQ || CmpCond->getPredicate() == CmpInst::ICMP_NE) {  
    if((LatchBR->getSuccessor(0) == L->getHeader() && CmpCond->getPredicate() == CmpInst::ICMP_EQ) || (LatchBR->getSuccessor(0) != L->getHeader() && CmpCond->getPredicate() == CmpInst::ICMP_NE))
      IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SLT, 
                               NewCondOp0, 
                               NewCondOp1,  
                               Cond->getName());
    
    else 
      IdxCmp = CmpInst::Create(Instruction::ICmp, CmpInst::ICMP_SGE, 
                               NewCondOp0, 
                               NewCondOp1, 
                               Cond->getName());
    CmpZeroTrip = dyn_cast<Instruction>(IdxCmp);
  }
  else { //in other cases, we keep the same predicate
    CmpZeroTrip = CondI->clone();
    IdxCmp = dyn_cast<Value>(CmpZeroTrip);
    if(dyn_cast<Instruction>(IdxCmp)->getOperand(1) == TripCount){ 
      dyn_cast<Instruction>(IdxCmp)->setOperand(0, NewInitV); 
      dyn_cast<Instruction>(IdxCmp)->setOperand(1, TripCount); 
    }
    else {
      dyn_cast<Instruction>(IdxCmp)->setOperand(1, NewInitV);
      dyn_cast<Instruction>(IdxCmp)->setOperand(0, TripCount); 
    }
  }
  PreHeader->getInstList().insert(B.GetInsertPoint(), dyn_cast<Instruction>(CmpZeroTrip));
              
  //need to distringuish cases
  if(LatchBR->getSuccessor(0) == PreHeaderBR->getSuccessor(0))
    B.CreateCondBr(IdxCmp, PreHeaderBR->getSuccessor(0), AfterLoop);
  else
    B.CreateCondBr(IdxCmp, AfterLoop, PreHeaderBR->getSuccessor(0));
  
  PreHeaderBR->eraseFromParent();
  
  BasicBlock *NewPH = InsertPreheaderForLoop(L, DT, LI, true);
  // Move section entry from .e block to the  new preheader to avoid bad section placement
  IRBuilder<> BPH(NewPH->getFirstNonPHI());
  
  for (Instruction& inst : *NewPH->getSinglePredecessor())
    if (IntrinsicInst *intr_inst = dyn_cast<IntrinsicInst>(&inst))
      if (intr_inst->getIntrinsicID() == Intrinsic::csa_parallel_section_entry) {
        (&inst)->moveBefore(NewPH->getFirstNonPHI());
        break;
      }
  
  return true;
}


bool LoopSPMDization::AddParallelIntrinsicstoLoop(Loop *L, LLVMContext& context, Module *M, BasicBlock *OrigPH, BasicBlock *E) {
  Function* FIntr = Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_region_entry); 
  Instruction*const header_terminator = OrigPH->getTerminator();
  Instruction*const preheader_terminator = L->getLoopPreheader()->getTerminator();
  CallInst *region_entry = IRBuilder<>{header_terminator}.CreateCall(
                      FIntr, 
                      ConstantInt::get(IntegerType::get(context, 32), 1), 
                      "spmd_pre"
                                                                     );
  std::string RegionName = region_entry->getName();
  next_token = context.getMDKindID(RegionName) + 1000;
  region_entry->setOperand(0, ConstantInt::get(IntegerType::get(context, 32), next_token));
  CallInst *section_entry = IRBuilder<>{preheader_terminator}.CreateCall(
          Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_section_entry), 
          region_entry, 
          "spmd_pse"
                                                                         );
  
  //IRBuilder<>{preheader_terminator}.CreateCall(
  //              Intrinsic::getDeclaration(M, Intrinsic::csa_parallel_loop));
  
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


