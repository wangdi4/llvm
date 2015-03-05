//===--- HIRCodeGen.cpp - Implements HIRCodeGen class ----- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under TODO license
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRCodeGen.h"
#include "llvm/Transforms/Intel_LoopTransforms/MockHIR.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"

#include "llvm/Support/Debug.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/IR/Intel_LoopIR/HIRVisitor.h"
// TODO audit includes
#define DEBUG_TYPE "hircg"

using namespace llvm;
using namespace llvm::loopopt;
namespace {

// TODO export type
typedef std::vector<const SCEV *> BlobTableTy;

class HIRCodeGen : public FunctionPass {
private:
  ScalarEvolution *SE;
  Function *F;
  MockHIR *HIR;

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
    Value *visitDDRef(DDRef *Ref);

    Value *visitRegion(HLRegion *R);
    Value *visitLoop(HLLoop *L);
    Value *visitIf(HLIf *I) { llvm_unreachable("Unimpl CG for If"); }

    Value *visitSwitch(HLSwitch *S) {
      llvm_unreachable("Unimpl CG for Switch");
    }

    Value *visitInst(HLInst *I);

    Value *visitGoto(HLGoto *G) { llvm_unreachable("Unimpl CG for GOTO"); }
    Value *visitLabel(HLLabel *L) { llvm_unreachable("Unimpl CG for Label"); }

    // any client shouldhave used visit(node), this function is used as a
    // fallback
    // when visitXXX couldnt be found for an hlnode of type XXX
    Value *visitHLNode(HLNode *Node) {
      llvm_unreachable("Unknown HIR type in CG");
    }

    CGVisitor(Function *CurFunc, ScalarEvolution *SE, BlobTableTy &BlobTbl)
        : F(CurFunc), Blobs(BlobTbl) {
      Builder = new IRBuilder<>(F->getContext());
      // TODO possibly IV conflict if scev blobs contain IV
      Expander = new SCEVExpander(*SE, "i");
    }

  private:
    // \brief returns a value representing the summation of all coef*blob pairs
    Value *sumBlobs(CanonExpr *CE);

    // \brief for a canon expr of form c_1 * i_1[ + c_2 *i_2 + ...] + c_0 + blob
    // return a value representing ONLY the summation of c_n * i_n pairs where
    // c_i
    // is a constant and i_n is an induction variable
    Value *sumIV(CanonExpr *CE);

    // \brief Return a value for blob corresponding to BlobIdx
    Value *getBlobValue(int BlobIdx, Type *Ty) {
      Value *Blob = Expander->expandCodeFor(getBlobSCEV(BlobIdx), Ty,
                                            Builder->GetInsertPoint());
      return Blob;
    }

    // \brief TODO blobs are reprsented by scev with some caveats
    SCEV *getBlobSCEV(int BlobIdx) {
      return const_cast<SCEV *>(Blobs[BlobIdx]);
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
    BlobTableTy &Blobs;

    // keep track of our mem allocs. Only IV atm
    std::map<std::string, AllocaInst *> NamedValues;

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

  HIRCodeGen() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    DEBUG(errs() << "Starting the code gen for ");
    DEBUG(errs().write_escaped(F.getName()) << "\n");
    DEBUG(F.dump());

    this->F = &F;
    SE = &getAnalysis<ScalarEvolution>();
    HIR = &getAnalysis<MockHIR>();

    // generate code
    CGVisitor CG(&F, SE, HIR->getBlobTable());
    // TODO enable region iterator
    CG.visit(HIR->TopRegion);
    // reg2mem, instcombine etc
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    // consider all analysis invalidated, however we lost a req analysis
    // eithout this..not sure
    // AU.setPreservesAll();

    // AU.addRequiredTransitive<ScalarEvolution>();
    AU.addRequired<ScalarEvolution>();
    AU.addRequired<MockHIR>();
  }
};
}

FunctionPass *llvm::createHIRCodeGenPass() { return new HIRCodeGen(); }

char HIRCodeGen::ID = 0;
static RegisterPass<HIRCodeGen> X("HIRCG", "HIR Code Generation", false, false);

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
  assert((Ref->getNumDimensions() == 1) &&
         "Cannot handle multiple dimensions!");

  Value *BaseV = visitCanonExpr(Ref->getBaseCE());
  // Create zero for the first GEP index
  Value *Zero =
      ConstantInt::getSigned(IntegerType::get(F->getContext(), 64), 0);

  std::vector<Value *> IndexV;
  IndexV.push_back(Zero);
  IndexV.push_back(visitCanonExpr(*(Ref->canon_begin())));

  if (Ref->isInBounds()) {
    return Builder->CreateInBoundsGEP(BaseV, IndexV, "arrayIdx");
  } else {
    return Builder->CreateGEP(BaseV, IndexV, "arrayIdx");
  }
}

Value *HIRCodeGen::CGVisitor::visitDDRef(DDRef *Ref) {
  if (ConstDDRef *CRef = dyn_cast<ConstDDRef>(Ref)) {
    return visitCanonExpr(CRef->getCanonExpr());
  } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Ref)) {
    return visitCanonExpr(BRef->getCanonExpr());
  } else if (RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref)) {
    return visitRegDDRef(RegRef);
  }
  llvm_unreachable("Unimpl CG for unknown ddref type");
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

  // TODO patch up predecessor to region entry bblock
  // Doing this makes the predecessor's successor unreachable.
  // this often results in invalid llvm ir, but if we call simplifycfg
  // the bblocks corresponding to this region are cleanly replaced
  // by the ones we just created.
  // Alternate impl is to create a cond br with true branch jumping to
  // our new region entry and false jumping to old, but cond always being
  // true. We end on valid IR but must call some form of pred opt to remove
  // old code
  BranchInst *RegionBranch = BranchInst::Create(RegionEntry);
  Instruction *Term = R->getPredBBlock()->getTerminator();
  BasicBlock::iterator ii(Term);
  ReplaceInstWithInst(Term->getParent()->getInstList(), ii, RegionBranch);

  // current insertion point is at end of region, add jump to successor
  // and we are done
  // TODO can there be no successor?
  if (!R->getSuccBBlock())
    llvm_unreachable("no successor block to region");

  Builder->CreateBr(R->getSuccBBlock());
  // DEBUG(F->dump());
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

  Value *StartVal = visitDDRef(L->getLowerDDRef());

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

  Value *StepVal = visitDDRef(L->getStrideDDRef());
  Value *TC = visitDDRef(L->getTripCountDDRef());

  // increment IV
  Value *CurVar = Builder->CreateLoad(Alloca);
  Value *NextVar = Builder->CreateAdd(CurVar, StepVal, "nextvar");
  Builder->CreateStore(NextVar, Alloca);

  Value *EndCond = Builder->CreateICmpSLE(NextVar, TC, "loopcond");

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
Value *HIRCodeGen::CGVisitor::visitInst(HLInst *I) {
  // CG the operands
  Value *Ops[3];
  int OpIdx;
  for (auto R = I->ddref_begin(), E = I->ddref_end(); R != E; OpIdx++, R++) {
    Ops[OpIdx] = visitDDRef(*R);
  }

  // create the inst
  // TODO if we want to match llvm ir more clsely, the store must
  // be created before other operands are cg'd
  if (isa<StoreInst>(I->getLLVMInstruction())) {
    // TODO change twine?
    LoadInst *Load = Builder->CreateLoad(Ops[0], "gepload");
    Builder->CreateStore(Load, Ops[1]);
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
