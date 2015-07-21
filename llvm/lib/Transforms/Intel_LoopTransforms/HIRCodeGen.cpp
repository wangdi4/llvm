//===--- HIRCodeGen.cpp - Implements HIRCodeGen class ----- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief This file implements HIRCodeGen class, used to convert HIR to LLVM IR
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Support/Debug.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/IR/Intel_LoopIR/HIRVisitor.h"
// TODO audit includes
#define DEBUG_TYPE "hircg"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> forceHIRCG("force-HIRCG", cl::init(false), cl::Hidden,
                                cl::desc("forces CodeGen on all HIR regions"));

namespace {

class HIRCodeGen : public FunctionPass {
private:
  ScalarEvolution *SE;
  Function *F;

  // TODO
  // This does the real work of llvm ir cg
  // TODO probably could use a return of void
  class CGVisitor : public HIRVisitor<CGVisitor, Value *> {
  public:
    // following are extra functions not part of visitor
    // ddref and ce cg generates many intermediate inst and values
    // but each one logically represents a single resulting value
    // which is returned
    Value *visitCanonExpr(CanonExpr *CE);
    Value *visitRegDDRef(RegDDRef *Ref);

    Value *visitRegion(HLRegion *R);
    Value *visitLoop(HLLoop *L);
    Value *visitIf(HLIf *I);

    Value *visitSwitch(HLSwitch *S);

    Value *visitInst(HLInst *I);

    Value *visitGoto(HLGoto *G);
    Value *visitLabel(HLLabel *L);
    BasicBlock *getBBlockForLabel(HLLabel *L);

    // any client shouldhave used visit(node), this function is used as a
    // fallback
    // when visitXXX couldnt be found for an hlnode of type XXX
    Value *visitHLNode(HLNode *Node) {
      llvm_unreachable("Unknown HIR type in CG");
    }

    CGVisitor(Function *CurFunc, ScalarEvolution *SE, HIRParser *Parser,
              Pass *CurPass)
        : F(CurFunc), HIRP(Parser), HIRCG(CurPass) {
      Builder = new IRBuilder<>(F->getContext());
      // TODO possibly IV conflict if scev blobs contain IV
      const DataLayout &DL =
          CurFunc->getEntryBlock().getModule()->getDataLayout();
      Expander = new SCEVExpander(*SE, DL, "i");
    }

  private:
    // \brief returns a value representing the summation of all coef*blob pairs
    Value *sumBlobs(CanonExpr *CE);

    // \brief for a canon expr of form c_1 * i_1[ + c_2 *i_2 + ...] + c_0 + blob
    // return a value representing ONLY the summation of c_n * i_n pairs where
    // c_i
    // is a constant and i_n is an induction variable
    Value *sumIV(CanonExpr *CE);

    /// \brief Generates a bool value* representing truth value of HLIf's
    /// predicate(s)
    Value *generateAllPredicates(HLIf *HIf);

    /// \brief Generates a bool value for prediacte in HLIf
    Value *generatePredicate(HLIf *HIf, HLIf::pred_iterator P);

    // \brief Return a value for blob corresponding to BlobIdx
    Value *getBlobValue(int BlobIdx, Type *Ty) {
      Value *Blob = Expander->expandCodeFor(getBlobSCEV(BlobIdx), Ty,
                                            Builder->GetInsertPoint());
      return Blob;
    }

    // \brief TODO blobs are reprsented by scev with some caveats
    SCEV *getBlobSCEV(int BlobIdx) {
      return const_cast<SCEV *>(HIRP->getBlob(BlobIdx));
    }

    //\brief return value for coeff*iv with IV at level
    Value *IVPairCG(CanonExpr::BlobOrConstToVal Pair, int Level, Type *Ty);

    // \brief retutn value for coeff*V
    Value *CoefCG(int Coeff, Value *V);

    //\brief returns value for blobCoeff*blob in <blobidx,coeff> pair
    Value *BlobPairCG(CanonExpr::BlobIndexToCoeff BlobPair, Type *Ty) {
      return CoefCG(BlobPair.Coeff, getBlobValue(BlobPair.Index, Ty));
    }

    Function *F;
    SCEVExpander *Expander;
    // Dont need custom insertion funcs...yet
    IRBuilder<> *Builder;
    HIRParser *HIRP;
    Pass *HIRCG;

    // keep track of our mem allocs. Only IV atm
    std::map<std::string, AllocaInst *> NamedValues;

    // maps internal labels to bblocks. Needed if we encounter "goto Label"
    // before the label itself
    SmallDenseMap<HLLabel *, BasicBlock *> InternalLabels;

    // \brief: Creates a stack allocation of size with name at entry of
    // current func. used for allocs that we expect to regisiterize
    // TODO support custom type
    AllocaInst *CreateEntryBlockAlloca(const std::string &VarName,
                                       Value *size = 0) {
      IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
      return TmpB.CreateAlloca(Type::getInt64Ty(F->getContext()), size,
                               VarName.c_str());
    }

    // HIRCG's considers iv names is iN where N is nesting level
    std::string getIVName(int NestingLevel) {
      return "i" + std::to_string(NestingLevel);
    }
  };

public:
  static char ID;

  HIRCodeGen() : FunctionPass(ID) {
    initializeHIRCodeGenPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    DEBUG(errs() << "Starting the code gen for ");
    DEBUG(errs().write_escaped(F.getName()) << "\n");
    DEBUG(F.dump());

    this->F = &F;
    SE = &getAnalysis<ScalarEvolution>();
    auto HIRP = &getAnalysis<HIRParser>();

    // generate code
    CGVisitor CG(&F, SE, HIRP, this);
    for (auto I = HIRP->hir_begin(), E = HIRP->hir_end(); I != E; I++) {
      if (cast<HLRegion>(I)->shouldGenCode() || forceHIRCG) {
        CG.visit(I);
      }
    }

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {

    // AU.addRequiredTransitive<ScalarEvolution>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<HIRParser>();
  }
};
}

FunctionPass *llvm::createHIRCodeGenPass() { return new HIRCodeGen(); }

char HIRCodeGen::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCodeGen, "HIRCG", "HIR Code Generation", false, false)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRCodeGen, "HIRCG", "HIR Code Generation", false, false)

Value *HIRCodeGen::CGVisitor::visitCanonExpr(CanonExpr *CE) {
  Value *BlobSum = nullptr, *IVSum = nullptr, *C0Value = nullptr;
  BlobSum = sumBlobs(CE);
  IVSum = sumIV(CE);

  int C0 = CE->getConstant();
  Type *Ty = CE->getLLVMType();
  // TODO I dunno about htis more specially a pointer?
  // ie [i32 X 10] for type of base ptr what type to use?
  if (C0) {
    if (isa<SequentialType>(Ty)) {
      Ty = IntegerType::get(F->getContext(), 64);
    }
    C0Value = ConstantInt::getSigned(Ty, C0);
  }

  // combine the blob, const, and ivs into one value
  Value *Res = nullptr;
  if (BlobSum && IVSum) {
    Res = Builder->CreateAdd(BlobSum, IVSum);
  } else {
    Res = IVSum ? IVSum : BlobSum;
  }
  if (Res) {
    Res = C0Value ? Builder->CreateAdd(Res, C0Value) : Res;
  } else {
    Res = C0Value;
  }

  if (!Res) {
    // assert c0 is 0. no iv no blob
    if (CE->hasIV() || CE->hasBlob() || C0 != 0)
      llvm_unreachable("failed to cg IV or blob");
    Res = ConstantInt::getSigned(Ty, C0);
  }

  return Res;
}

/// Only works for single dimension arrays right now.
Value *HIRCodeGen::CGVisitor::visitRegDDRef(RegDDRef *Ref) {
  assert(Ref && " Reference is null.");
  assert((Ref->getNumDimensions() == 1) &&
         "Cannot handle multiple dimensions!");

  if (!Ref->hasGEPInfo()) {
    return visitCanonExpr(Ref->getSingleCanonExpr());
  }

  Value *BaseV = visitCanonExpr(Ref->getBaseCE());

  // Create zero for the first GEP index
  Value *Zero =
      ConstantInt::getSigned((*(Ref->canon_begin()))->getLLVMType(), 0);

  std::vector<Value *> IndexV;
  IndexV.push_back(Zero);
  IndexV.push_back(visitCanonExpr(*(Ref->canon_begin())));

  if (Ref->isInBounds()) {
    return Builder->CreateInBoundsGEP(BaseV, IndexV, "arrayIdx");
  } else {
    return Builder->CreateGEP(BaseV, IndexV, "arrayIdx");
  }
}

Value *HIRCodeGen::CGVisitor::visitRegion(HLRegion *R) {

  // create new bblock for region entry
  BasicBlock *RegionEntry = BasicBlock::Create(F->getContext(), "region", F);
  Builder->SetInsertPoint(RegionEntry);

  // TODO Stack map for live out values

  // Onto children cg
  for (auto It = R->child_begin(), E = R->child_end(); It != E; It++) {
    visit(*It);
  }

  // Patch up predecessor(s) to region entry bblock
  // We do this by splitting the region entry bblock, with first block having
  // original label, but only a br to the second block, with second bblock
  // with the original instructions. Then the br to second block is replaced by
  // a cond br with true branch jumping to our new region'a entry and
  // false jumping to old code(second bblock), but cond always being true.
  // We end on valid IR but must call some form of pred opt to remove old code

  // Save entry and succ fields, these get invalidated once block is split
  BasicBlock *EntryFirstHalf = R->getEntryBBlock();
  BasicBlock *RegionSuccessor = R->getSuccBBlock();

  BasicBlock *EntrySecondHalf =
      SplitBlock(EntryFirstHalf, EntryFirstHalf->begin());

  Instruction *Term = EntryFirstHalf->getTerminator();
  BasicBlock::iterator ii(Term);
  BranchInst *RegionBranch = BranchInst::Create(
      RegionEntry, EntrySecondHalf,
      ConstantInt::get(IntegerType::get(F->getContext(), 1), 1));
  ReplaceInstWithInst(Term->getParent()->getInstList(), ii, RegionBranch);

  // current insertion point is at end of region, add jump to successor
  // and we are done
  // TODO can there be no successor?
  if (!RegionSuccessor)
    llvm_unreachable("no successor block to region");

  Builder->CreateBr(RegionSuccessor);
  // DEBUG(F->dump());
  return nullptr;
}

Value *HIRCodeGen::CGVisitor::generatePredicate(HLIf *HIf,
                                                HLIf::pred_iterator P) {
  Value *CurPred = nullptr;
  Value *LHSVal, *RHSVal;
  RegDDRef *LHSRef = HIf->getPredicateOperandDDRef(P, true);
  RegDDRef *RHSRef = HIf->getPredicateOperandDDRef(P, false);

  if (!LHSRef || !RHSRef) {
    // FCMP_TRUE predicates have no refs but we still need some operands
    assert(!RHSRef && !LHSRef && "Pred ref missing");
    LHSVal = ConstantFP::get(F->getContext(), APFloat(1.0));
    if (*P == CmpInst::FCMP_TRUE) {
      RHSVal = ConstantFP::get(F->getContext(), APFloat(1.0));
    } else if (*P == CmpInst::FCMP_FALSE) {
      RHSVal = ConstantFP::get(F->getContext(), APFloat(0.0));
    } else {
      llvm_unreachable("unexpected predicate");
    }
  } else {
    LHSVal = visitRegDDRef(LHSRef);
    RHSVal = visitRegDDRef(RHSRef);
  }
  assert(LHSVal->getType() == RHSVal->getType() &&
         "HLIf predicate type mismatch");

  if (LHSVal->getType()->isIntegerTy()) {
    CurPred = Builder->CreateICmp(*P, LHSVal, RHSVal, "hir.cmp");
  } else if (LHSVal->getType()->isFloatTy()) {
    CurPred = Builder->CreateFCmp(*P, LHSVal, RHSVal, "hir.cmp");
  } else {
    llvm_unreachable("unknown predicate type in HIRCG");
  }

  return CurPred;
}

Value *HIRCodeGen::CGVisitor::generateAllPredicates(HLIf *HIf) {

  auto FirstPred = HIf->pred_begin();
  Value *CurPred = generatePredicate(HIf, FirstPred);

  for (auto It = HIf->pred_begin() + 1, E = HIf->pred_end(); It != E; ++It) {
    // conjunctions are implicitly AND atm.
    CurPred = Builder->CreateAnd(CurPred, generatePredicate(HIf, It));
  }

  return CurPred;
}

Value *HIRCodeGen::CGVisitor::visitIf(HLIf *HIf) {

  llvm_unreachable("Untested CG for HLIf");
  Value *CondV = generateAllPredicates(HIf);

  BasicBlock *ThenBB = BasicBlock::Create(F->getContext(), "then", F);
  BasicBlock *ElseBB = BasicBlock::Create(F->getContext(), "else");
  BasicBlock *MergeBB = BasicBlock::Create(F->getContext(), "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // generate then block
  Builder->SetInsertPoint(ThenBB);
  for (auto It = HIf->then_begin(), E = HIf->then_end(); It != E; ++It) {
    visit(*It);
  }
  Builder->CreateBr(MergeBB);

  // generate else block
  F->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);
  for (auto It = HIf->else_begin(), E = HIf->else_end(); It != E; ++It) {
    visit(*It);
  }
  Builder->CreateBr(MergeBB);

  // CG resumes at merge block
  F->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);

  return nullptr;
}

Value *HIRCodeGen::CGVisitor::visitLoop(HLLoop *L) {
  if (L->hasZtt())
    llvm_unreachable("Unimpl CG for Loop Ztt");

  // undef ref. TODO define isDoLoop
  // if(!L->isDoLoop()) {
  //  llvm_unreachable("Unimpl CG for non do loops");
  //}

  if (L->hasPreheader())
    llvm_unreachable("Unimpl CG for phdr");

  // set up IV, I think we can reuse the IV allocation across
  // multiple loops of same depth
  AllocaInst *Alloca;
  std::string IVName = getIVName(L->getNestingLevel());
  if (!NamedValues.count(IVName)) {
    Alloca = CreateEntryBlockAlloca(IVName, 0);
    NamedValues[IVName] = Alloca;
  } else {
    Alloca = NamedValues[IVName];
  }

  Value *StartVal = visitRegDDRef(L->getLowerDDRef());

  if (!StartVal || !Alloca)
    llvm_unreachable("Failed to CG IV");

  Builder->CreateStore(StartVal, Alloca);
  BasicBlock *LoopBB = BasicBlock::Create(F->getContext(), "loop", F);

  // explicit fallthru to loop, terminates current bblock
  Builder->CreateBr(LoopBB);
  Builder->SetInsertPoint(LoopBB);

  // CG children
  for (auto It = L->child_begin(), E = L->child_end(); It != E; It++) {
    // a loop might not return anything. Error checking is on callee
    visit(*It);
  }

  Value *StepVal = visitRegDDRef(L->getStrideDDRef());
  Value *Upper = visitRegDDRef(L->getUpperDDRef());

  // increment IV
  Value *CurVar = Builder->CreateLoad(Alloca);
  Value *NextVar = Builder->CreateAdd(CurVar, StepVal, "nextvar");
  Builder->CreateStore(NextVar, Alloca);

  Value *EndCond = Builder->CreateICmpSLE(NextVar, Upper, "loopcond");

  BasicBlock *AfterBB = BasicBlock::Create(F->getContext(), "afterloop", F);

  // latch
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // new code goes after loop
  Builder->SetInsertPoint(AfterBB);

  // set up postexit
  if (L->hasPostexit())
    llvm_unreachable("Unimpl CG for postexit");

  return nullptr;
}

BasicBlock *HIRCodeGen::CGVisitor::getBBlockForLabel(HLLabel *L) {
  if (InternalLabels.count(L))
    return InternalLabels[L];

  BasicBlock *LabelBB = BasicBlock::Create(F->getContext(), "hir.label", F);
  InternalLabels[L] = LabelBB;
  return LabelBB;
}

Value *HIRCodeGen::CGVisitor::visitLabel(HLLabel *L) {
  llvm_unreachable("untested cg for hllabel");
  // if we see label it must be internal, and it must be unique
  BasicBlock *LabelBBlock = getBBlockForLabel(L);
  assert(LabelBBlock->empty() && "label already in use");

  // create a br to L's block. ending current block
  Builder->CreateBr(LabelBBlock);
  Builder->SetInsertPoint(LabelBBlock);
  return nullptr;
}

Value *HIRCodeGen::CGVisitor::visitGoto(HLGoto *G) {
  llvm_unreachable("untested cg for hlgoto");
  // get basic block for G's target
  BasicBlock *TargetBBlock = G->getTargetBBlock();

  // if bblock is null, it must be internal.
  if (!TargetBBlock)
    TargetBBlock = getBBlockForLabel(G->getTargetLabel());

  assert(TargetBBlock && "No bblock target for goto");
  // create a br to target, ending this block
  Builder->CreateBr(TargetBBlock);

  BasicBlock *ContBB = BasicBlock::Create(F->getContext(), "goto.cont", F);

  // set insertion point there, but nodes visited are dead code,
  // until a label is reached.
  Builder->SetInsertPoint(ContBB);
  return nullptr;
}

Value *HIRCodeGen::CGVisitor::visitSwitch(HLSwitch *S) {
  llvm_unreachable("untested hircg for switch");

  Value *CondV = visitRegDDRef(S->getConditionDDRef());
  BasicBlock *DefaultBlock = BasicBlock::Create(F->getContext(), "default");
  BasicBlock *EndBlock = BasicBlock::Create(F->getContext(), "switch.end");

  SwitchInst *LLVMSwitch =
      Builder->CreateSwitch(CondV, DefaultBlock, S->getNumCases());

  // generate default block
  F->getBasicBlockList().push_back(DefaultBlock);
  Builder->SetInsertPoint(DefaultBlock);
  for (auto I = S->default_case_child_begin(), E = S->default_case_child_end();
       I != E; ++I) {
    visit(*I);
  }

  Builder->CreateBr(EndBlock);

  // generate case blocks
  for (unsigned int I = 1; I <= S->getNumCases(); ++I) {
    Value *CaseV = visitRegDDRef(S->getCaseValueDDRef(I));
    // assert its a constant or rely on verifier?
    ConstantInt *CaseInt = cast<ConstantInt>(CaseV);

    BasicBlock *CaseBlock = BasicBlock::Create(F->getContext(), "switch.case");
    F->getBasicBlockList().push_back(CaseBlock);
    Builder->SetInsertPoint(CaseBlock);

    for (auto HNode = S->case_child_begin(I), E = S->case_child_end(I);
         HNode != E; ++HNode) {
      visit(*HNode);
    }

    Builder->CreateBr(EndBlock);
    LLVMSwitch->addCase(CaseInt, CaseBlock);
  }

  F->getBasicBlockList().push_back(EndBlock);
  Builder->SetInsertPoint(EndBlock);
  return nullptr;
}

Value *HIRCodeGen::CGVisitor::visitInst(HLInst *I) {
  // CG the operands
  // TODO change this to match HLInst->getNumOperands() and skip temp lvals.
  Value *Ops[3];
  int OpIdx = 0;
  for (auto R = I->op_ddref_begin(), E = I->op_ddref_end(); R != E;
       OpIdx++, R++) {
    Ops[OpIdx] = visitRegDDRef(*R);
  }

  // create the inst
  // TODO if we want to match llvm ir more clsely, the store must
  // be created before other operands are cg'd
  if (isa<StoreInst>(I->getLLVMInstruction())) {
    // TODO change twine?
    LoadInst *Load = Builder->CreateLoad(Ops[1], "gepload");
    Builder->CreateStore(Load, Ops[0]);
  } else {
    llvm_unreachable("Unimpl CG for inst");
  }

  return nullptr;
}

Value *HIRCodeGen::CGVisitor::sumBlobs(CanonExpr *CE) {
  if (!CE->hasBlob())
    return nullptr;

  auto CurBlobPair = CE->blob_cbegin();
  Type *Ty = CE->getLLVMType();
  Value *res = BlobPairCG(*CurBlobPair, Ty);
  CurBlobPair++;

  for (auto E = CE->blob_cend(); CurBlobPair != E; CurBlobPair++)
    res = Builder->CreateAdd(res, BlobPairCG(*CurBlobPair, Ty));

  return res;
}

Value *HIRCodeGen::CGVisitor::sumIV(CanonExpr *CE) {
  if (!CE->hasIV())
    return nullptr;

  auto CurIVPair = CE->iv_cbegin();
  int Level = 1;
  // start with first summation not of x*0
  for (auto E = CE->iv_cend(); CurIVPair != E; CurIVPair++, Level++) {
    if (CurIVPair->Coeff != 0)
      break;
  }

  if (CurIVPair == CE->iv_cend())
    llvm_unreachable("No iv in CE");

  Type *Ty = CE->getLLVMType();

  Value *res = IVPairCG(*CurIVPair, Level, Ty);
  CurIVPair++;
  Level++;

  // accumulate other pairs
  for (auto E = CE->iv_cend(); CurIVPair != E; CurIVPair++, Level++) {
    if (CurIVPair->Coeff != 0)
      res = Builder->CreateAdd(res, IVPairCG(*CurIVPair, Level, Ty));
  }

  return res;
}

Value *HIRCodeGen::CGVisitor::IVPairCG(CanonExpr::BlobOrConstToVal Pair,
                                       int Level, Type *Ty) {

  Value *IV = Builder->CreateLoad(NamedValues[getIVName(Level)]);

  // pairs are of form <isBlob, coeff>. if its a blob, coeff is the blobidx
  if (Pair.IsBlobCoeff) {
    return Builder->CreateMul(getBlobValue(Pair.Coeff, Ty), IV);
  } else {
    return CoefCG(Pair.Coeff, IV);
  }
}
Value *HIRCodeGen::CGVisitor::CoefCG(int Coeff, Value *V) {

  // do not emit 1*iv, just emit IV
  if (Coeff == 1)
    return V;
  if (Coeff == 0)
    llvm_unreachable("Dead mul in CoefCG");

  return Builder->CreateMul(
      ConstantInt::getSigned(const_cast<Type *>(V->getType()), Coeff), V);
}
