//===- Intel_Float128Expand.cpp - Expand FP128 operations -----------------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass (at IR level) to replace fp128 instructions with
// library calls.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/RecyclingAllocator.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "float128-expand"

namespace {

class Float128Expand : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  Float128Expand() : FunctionPass(ID) {
    initializeFloat128ExpandPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;

private:
  using AllocatorTy =
      RecyclingAllocator<BumpPtrAllocator,
                         ScopedHashTableVal<Value *, Instruction *>>;
  using ScopedHTType = ScopedHashTable<Value *, Instruction *,
                                       DenseMapInfo<Value *>, AllocatorTy>;
  using ScopeType = ScopedHTType::ScopeTy;
  struct SCCNode {
    DenseSet<BasicBlock *> BBList;
    bool HasLoop = false;
    DenseSet<SCCNode *> Preds;
    DenseSet<SCCNode *> Succs;
  };
  ScopedHTType VNT;
  DenseMap<BasicBlock *, ScopeType *> ScopeMap;
  DenseMap<BasicBlock *, SmallVector<Instruction *, 4>> BBWorkList;
  MapVector<PHINode *, PHINode *> NewPHI2OldPHI;
  DenseMap<Value *, Instruction *> Value2Ptr;
  // SCCList contains SCCs in Reverse-Topological-Order.
  SmallVector<std::unique_ptr<SCCNode>, 4> SCCList;
  DenseMap<BasicBlock *, SCCNode *> BB2SCC;
  DenseMap<std::pair<BasicBlock *, Value *>, Instruction *> LastUse;
  DenseSet<std::pair<SCCNode *, Value *>> IsLive;
  DenseSet<std::pair<SCCNode *, Value *>> HasUse;
  // We use SetVector instead of DenseSet since if we visit FP128PtrList
  // out of order, testcases sometimes fail.
  SetVector<Value *> FP128PtrList;
  void EnterScope(BasicBlock *BB);
  void ExitScope(BasicBlock *BB);
  void ExitScopeIfDone(DomTreeNode *Node,
                       DenseMap<DomTreeNode *, unsigned> &OpenChildren);
  bool ProcessInstruction(Instruction *I);
  bool ProcessBlock(BasicBlock *BB);
  bool PerformFp128Transform(DomTreeNode *Node);
  bool TransformFP128PHI(Instruction *I);
  void PostTransformFP128PHI();
  Value *expandToLibCall(IRBuilder<> &Builder, Instruction *I,
                         StringRef LibCallName, Type *RetTy,
                         ArrayRef<Value *> Ops);
  bool expandArith(IRBuilder<> &Builder, Instruction *I, unsigned Opcode,
                   ArrayRef<Value *> Ops);
  bool expandFPTrunc(IRBuilder<> &Builder, Instruction *I);
  bool expandFPExt(IRBuilder<> &Builder, Instruction *I);
  bool expandIToFP(IRBuilder<> &Builder, Instruction *I);
  bool expandFPToI(IRBuilder<> &Builder, Instruction *I);
  bool expandFCmp(IRBuilder<> &Builder, Instruction *I);
  bool isUsedOutsideLoops(Value *Val, SCCNode *CurSCC);
  BasicBlock *calculateSafePoint(SCCNode *CurSCC);
  void visitSCCAndCreateLE(SCCNode &CurSCC);
};

} // end anonymous namespace

char Float128Expand::ID = 0;

char &llvm::Float128ExpandID = Float128Expand::ID;

INITIALIZE_PASS(Float128Expand, DEBUG_TYPE, "Expand fp128 instructions",
                false, false)

FunctionPass *llvm::createFloat128ExpandPass() { return new Float128Expand(); }

static Instruction *getFirstNonAllocaInTheEntryBlock(Function &F) {
  for (Instruction &I : F.getEntryBlock())
    if (!isa<AllocaInst>(&I))
      return &I;
  llvm_unreachable("No terminator in the entry block");
}

static AllocaInst *CreateFP128AllocaInst(IRBuilder<> &Builder, BasicBlock *BB) {
  LLVMContext &Ctx = Builder.getContext();
  Function &F = *BB->getParent();
  Module *M = BB->getModule();
  const DataLayout &DL = M->getDataLayout();

  Type *FP128Ty = Type::getFP128Ty(Ctx);
  auto AllocaAlignment = DL.getPrefTypeAlign(FP128Ty);
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  AllocaInst *AllocaRes =
      new AllocaInst(FP128Ty, AllocaAS, "", &F.getEntryBlock().front());
  AllocaRes->setAlignment(AllocaAlignment);
  return AllocaRes;
}

static Instruction *CreateFP128LifetimeStart(IRBuilder<> &Builder,
                                             Value *AllocaPtr, Function &F) {
  LLVMContext &Ctx = Builder.getContext();
  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();
  Type *FP128Ty = Type::getFP128Ty(Ctx);
  ConstantInt *SizeVal64 = Builder.getInt64(DL.getTypeStoreSize(FP128Ty));
  return Builder.CreateLifetimeStart(AllocaPtr, SizeVal64);
}

static Instruction *CreateFP128LifetimeEnd(IRBuilder<> &Builder,
                                           Value *AllocaPtr, Function &F) {
  LLVMContext &Ctx = Builder.getContext();
  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();
  Type *FP128Ty = Type::getFP128Ty(Ctx);
  ConstantInt *SizeVal64 = Builder.getInt64(DL.getTypeStoreSize(FP128Ty));
  return Builder.CreateLifetimeEnd(AllocaPtr, SizeVal64);
}

static bool canLowerToFP128Libcall(Instruction *I, bool CheckType = false) {
  switch (I->getOpcode()) {
  default:
    return false;
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FNeg:
  case Instruction::FPExt:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
    if (CheckType)
      return I->getType()->getScalarType()->isFP128Ty();
    return true;
  case Instruction::FCmp:
  case Instruction::FPTrunc:
  case Instruction::FPToSI:
  case Instruction::FPToUI:
    if (CheckType)
      return I->getOperand(0)->getType()->getScalarType()->isFP128Ty();
    return true;
  }
}

static bool isUsedByFP128Libcall(Value *Val) {
  for (auto UI : Val->users())
    if (canLowerToFP128Libcall(cast<Instruction>(UI)))
      return true;
  return false;
}

// Check if Val is used by PHI.
static bool isUsedbyPHI(Value *Val) {
  for (auto *UI : Val->users()) {
    if (isa<PHINode>(UI))
      return true;
  }

  return false;
}

// In our first traverse, if the oldphi is used by a libcall,
// then we will create newphi(without incomings) for it.
bool Float128Expand::TransformFP128PHI(Instruction *I) {
  IRBuilder<> Builder(I->getParent()->getFirstNonPHI());
  LLVMContext &Ctx = Builder.getContext();
  Type *FP128PtrTy = Type::getFP128PtrTy(Ctx);
  PHINode *Phi = cast<PHINode>(I);
  PHINode *NewPhi = Builder.CreatePHI(FP128PtrTy, Phi->getNumIncomingValues());
  NewPHI2OldPHI.insert({NewPhi, Phi});
  VNT.insert(I, NewPhi);
  Value2Ptr.insert({I, NewPhi});
  return true;
}

// After we finish the first traverse, we will add incoming value&block for
// NewPhi. If we can't add incoming value&block, in spite of OldPhi's being used
// by Libcall, we will create a alloca(AllocaForPhi) for OldPhi, store OldPhi
// into AllocaForPhi, erase NewPhi and replace NewPhi's use with AllocaForPhi.
// We won't erase the OldPhi, since some function calls might use it.
void Float128Expand::PostTransformFP128PHI() {
  for (auto PairI : NewPHI2OldPHI) {
    PHINode* NewPhi = PairI.first;
    PHINode *OldPhi = PairI.second;

    for (unsigned i = 0; i != OldPhi->getNumIncomingValues(); ++i) {
      Value *IncomingValue = OldPhi->getIncomingValue(i);
      Value *IncomingPtr = Value2Ptr[IncomingValue];
      // We won't create a NewPhi for OldPhi which has a constant Operand.
      //
      // Assume we create a NewPhi for OldPhi which has a constant Operand,
      // In the following Example, When reach bb3, we create PK for 0.111
      // i.e. Value2Ptr[0.111]=PK. So PostTransformFP128PHI might falsely use
      // PK for
      //    %0 = phi fp128 [0.111, %bb1],[%x, %bb2],
      // and falsely create
      //    P0' = phi fp128*[PK, bb1], [PX, bb2]
      // In fact, we store 0.111 into PK in bb3. So, When the program is running
      // from bb1 to bb2, PK hasn't been stored by 0.111 yet, which means %1
      // will be calculated wrongly.
      //
      // Example:
      //        bb1
      //   /          \
      //  /            \
      //  |               bb2
      //  |             /   %0 = phi fp128 [0.111, %bb1],[%x, %bb2]
      //  |            |    %1 = fadd fp128 %x, %0
      //  |             \   br i1 undef, label %bb2, label %bb4
      // bb3                            |
      // %2 = fsub fp128 %x,0.111       |
      // ret 0                          |
      //                               bb4
      // A workaround method is to find out all the constant fp128 and create
      // AllocaInst/StoreInst for them in the entry basicblock but it needs
      // another traverse and makes this Pass even more sophisticated.
      if (IncomingPtr == nullptr || isa<Constant>(IncomingValue)) {
        IRBuilder<> Builder(&*NewPhi->getParent()->getFirstInsertionPt());
        AllocaInst *AllocaForPhi =
            CreateFP128AllocaInst(Builder, NewPhi->getParent());
        Builder.CreateAlignedStore(OldPhi, AllocaForPhi,
                                   MaybeAlign(AllocaForPhi->getAlign()));
        NewPhi->replaceAllUsesWith(AllocaForPhi);
        // Update the Value2Ptr by Replacing NewPhi in Value2Ptr with
        // AllocaForPhi
        // Example#1:
        //     bb1
        //       \
        //        \
        //         bb2
        //        phi1 = phi fp128 [...,bb1], [...,bb4]
        //        phi2 = phi fp128 [...,bb1], [...,bb4]
        //          /     \   \
        //         /       \   \
        //        |         |   |
        //        bb3       |   |
        //         \       /   /
        //          \     /   /
        //              bb4
        //        phi3 = phi fp128 [phi1,bb3], [phi2, bb4]
        // After we finish the first traverse, we may have Value2Ptr[phi1] =
        // pphi1(fp128*). But when we do PostTransformFP128PHI, we may find that
        // we can't set valid incoming value for pphi1, so AllocaForPhi1 will be
        // created to replace pphi1's uses. We also update the Value2Ptr by
        // replacing pphi1 with AllocaForPhi1, otherwise, pphi3 will falsely use
        // pphi1 to set its incomings.
        for (auto VPI : Value2Ptr) {
          if (VPI.second == NewPhi)
            Value2Ptr[VPI.first] = AllocaForPhi;
        }
        NewPhi->eraseFromParent();
        break;
      }
      BasicBlock *IncomingBB = OldPhi->getIncomingBlock(i);
      NewPhi->addIncoming(IncomingPtr, IncomingBB);
    }
  }
  return;
}

static bool isFPToILib(StringRef LibCallName) {
  return LibCallName == "__qtoi" || LibCallName == "__qtou" ||
         LibCallName == "__qtoj" || LibCallName == "__qtok" ||
         LibCallName == "__qtom" || LibCallName == "__qton";
}

Value *Float128Expand::expandToLibCall(IRBuilder<> &Builder, Instruction *I,
                                       StringRef LibCallName, Type *RetTy,
                                       ArrayRef<Value *> Ops) {
  LLVMContext &Ctx = Builder.getContext();
  Type *FP128Ty = Type::getFP128Ty(Ctx);
  Module *M = I->getModule();
  SmallVector<Value *, 3> Args;

  AllocaInst *AllocaDst = nullptr;
  Value *VOp0 = nullptr;
  Value *VOp1 = nullptr;
  auto *CurSCC = BB2SCC[I->getParent()];
  // If the return is FP128, create an alloca and push it to the arg list.
  if (RetTy->isFP128Ty()) {
    AllocaDst = CreateFP128AllocaInst(Builder, I->getParent());
    // As for LifetimeStart, normally, we will create LifetimeStart before
    // Store. But if the a fp128 value is defined inside a loop and used outside
    // the loop, we will hoist LifetimeStart outside the loop safely.
    //
    // Here are two examples:
    // Example#1:
    //     bb1
    //    /   \
    //   /     \
    // bb2      bb3(PX's def)
    //          / \
    //         |   |
    //          \ /
    //           bb4
    //            \
    //             \
    //              bb5(use PX)
    //
    // We will hoist LifetimeStart to bb1, since PX is defined in a loop
    // and is used in bb5 which is outside the loop.
    //
    // Example#2:
    //     bb1
    //     /
    //    /
    //   bb2
    //   / \
    //  |   |
    //   \ /
    //   bb3
    //     \
    //      \
    //       bb4(PX's def)
    //       / \
    //      |   |
    //       \ /
    //        bb5
    //         \
    //          \
    //           bb6(use PX)
    // We won't create LifetimeStart for PX since bb3 is in a loop
    // Thus, we won't create PX's LifetimeEnd for PX.
    bool HasCreatedLS = false;
    if (CurSCC->HasLoop && isUsedOutsideLoops(I, CurSCC)) {
      if (BasicBlock *TargetBB = calculateSafePoint(CurSCC)) {
        Instruction *InsertPt =
            TargetBB == &TargetBB->getParent()->getEntryBlock()
                ? getFirstNonAllocaInTheEntryBlock(*TargetBB->getParent())
                : &*TargetBB->getFirstInsertionPt();
        IRBuilder<> NewBuilder(InsertPt);
        CreateFP128LifetimeStart(NewBuilder, AllocaDst, *I->getFunction());
        HasCreatedLS = true;
      }
    } else {
      CreateFP128LifetimeStart(Builder, AllocaDst, *I->getFunction());
      HasCreatedLS = true;
    }
    Args.push_back(AllocaDst);
    // Assume we have %newphi = phi fp128*, [%px, bb1], [%py, bb2],
    // %px's Lifetime will be extended by newphi's uses.
    // So it's better ignore fp128ptr's LifetimeMarker if its value is related
    // with PHI. Besides, if we haven't create LifetimeStart for PX, we won't
    // create LifetimeEnd for it.
    if (!isUsedbyPHI(I) && HasCreatedLS)
      FP128PtrList.insert(AllocaDst);
    // Replace the type with void.
    RetTy = Type::getVoidTy(Ctx);
  }

  // If the first argument is an FP128, create an alloca and push it
  // to the arg list.
  assert(Ops.size() > 0 && Ops.size() <= 2 && "Unexpected number of operands");
  if (Ops[0]->getType()->isFP128Ty()) {
    if (!VNT.count(Ops[0])) {
      AllocaInst *AllocaOp0 = CreateFP128AllocaInst(Builder, I->getParent());
      CreateFP128LifetimeStart(Builder, AllocaOp0, *I->getFunction());
      Builder.CreateAlignedStore(Ops[0], AllocaOp0,
                                 MaybeAlign(AllocaOp0->getAlign()));
      Args.push_back(AllocaOp0);
      VNT.insert(Ops[0], AllocaOp0);
      Value2Ptr.insert({Ops[0], AllocaOp0});
      VOp0 = AllocaOp0;
      if (!isUsedbyPHI(Ops[0]))
        FP128PtrList.insert(VOp0);
    } else {
      VOp0 = VNT.lookup(Ops[0]);
      Args.push_back(VOp0);
    }
    // Set the mode to MODE_TOWARDZERO
    if (isFPToILib(LibCallName))
      Args.push_back(ConstantInt::get(Type::getInt32Ty(Ctx), 0, true));
  } else {
    // Otherwise just push the argument.
    Args.push_back(Ops[0]);
  }

  // If there is a second argument it will always be an fp128.
  if (Ops.size() >= 2) {
    if (!VNT.count(Ops[1])) {
      assert(Ops[1]->getType()->isFP128Ty() && "Unexpected type!");
      AllocaInst *AllocaOp1 = CreateFP128AllocaInst(Builder, I->getParent());
      CreateFP128LifetimeStart(Builder, AllocaOp1, *I->getFunction());
      Builder.CreateAlignedStore(Ops[1], AllocaOp1,
                                 MaybeAlign(AllocaOp1->getAlign()));
      Args.push_back(AllocaOp1);
      VNT.insert(Ops[1], AllocaOp1);
      Value2Ptr.insert({Ops[1], AllocaOp1});
      VOp1 = AllocaOp1;
      if (!isUsedbyPHI(Ops[1]))
        FP128PtrList.insert(VOp1);
    } else {
      VOp1 = VNT.lookup(Ops[1]);
      Args.push_back(VOp1);
    }
  }

  SmallVector<Type *, 3> ArgTys;
  for (Value *Arg : Args)
    ArgTys.push_back(Arg->getType());
  FunctionType *FnType = FunctionType::get(RetTy, ArgTys, false);
  FunctionCallee LibcallFn = M->getOrInsertFunction(LibCallName, FnType);
  Instruction *Result = Builder.CreateCall(LibcallFn, Args);

  // If we had fp128 operands, we're done with their allocas now.
  if (VOp0) {
    LastUse[{I->getParent(), VOp0}] = Result;
    HasUse.insert({CurSCC, VOp0});
  }
  if (VOp1) {
    LastUse[{I->getParent(), VOp1}] = Result;
    HasUse.insert({CurSCC, VOp1});
  }

  if (AllocaDst) {
    Result = Builder.CreateAlignedLoad(FP128Ty, AllocaDst,
                                       MaybeAlign(AllocaDst->getAlign()));
    // Record the Result maping to AllocaDst so that we can resue it later.
    VNT.insert(Result, AllocaDst);
    LastUse[{I->getParent(), AllocaDst}] = Result;
    HasUse.insert({CurSCC, AllocaDst});
    Value2Ptr.insert({Result, AllocaDst});
  }

  return Result;
}

// Expand a binary operator and fneg on fp128 into a library call.
bool Float128Expand::expandArith(IRBuilder<> &Builder, Instruction *I,
                                 unsigned Opcode, ArrayRef<Value *> Ops) {
  StringRef LibCallName;
  switch (Opcode) {
  default: llvm_unreachable("Unexpected operation");
  case Instruction::FAdd: LibCallName = "__addq"; break;
  case Instruction::FSub: LibCallName = "__subq"; break;
  case Instruction::FMul: LibCallName = "__mulq"; break;
  case Instruction::FDiv: LibCallName = "__divq"; break;
  case Instruction::FNeg: LibCallName = "__negq"; break;
  }

  Value *Result = expandToLibCall(Builder, I, LibCallName, I->getType(), Ops);

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

bool Float128Expand::expandFPTrunc(IRBuilder<> &Builder, Instruction *I) {
  if (!I->getOperand(0)->getType()->isFP128Ty())
    return false;

  StringRef LibCallName;
  switch (I->getType()->getTypeID()) {
  default: return false;
  case Type::FloatTyID:    LibCallName = "__qtof"; break;
  case Type::DoubleTyID:   LibCallName = "__qtod"; break;
  case Type::X86_FP80TyID: LibCallName = "__qtol"; break;
  }

  Value *Result = expandToLibCall(Builder, I, LibCallName, I->getType(),
                                  I->getOperand(0));

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

bool Float128Expand::expandFPExt(IRBuilder<> &Builder, Instruction *I) {
  if (!I->getType()->isFP128Ty())
    return false;

  Value *In = I->getOperand(0);

  StringRef LibCallName;
  switch (In->getType()->getTypeID()) {
  default: return false;
  case Type::FloatTyID:    LibCallName = "__ftoq"; break;
  case Type::DoubleTyID:   LibCallName = "__dtoq"; break;
  case Type::X86_FP80TyID: LibCallName = "__ltoq"; break;
  }

  Value *Result = expandToLibCall(Builder, I, LibCallName, I->getType(), In);

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

bool Float128Expand::expandIToFP(IRBuilder<> &Builder, Instruction *I) {
  unsigned BitWidth = I->getOperand(0)->getType()->getIntegerBitWidth();
  bool IsSigned = I->getOpcode() == Instruction::SIToFP;

  StringRef LibCallName;
  if (BitWidth <= 32) {
    BitWidth = 32;
    LibCallName = IsSigned ? "__itoq" : "__utoq";
  } else if (BitWidth <= 64) {
    BitWidth = 64;
    LibCallName = IsSigned ? "__jtoq" : "__ktoq";
  } else if (BitWidth <= 128) {
    BitWidth = 128;
    LibCallName = IsSigned ? "__mtoq" : "__ntoq";
  } else
    return false; // We don't support more than 128 bits.

  // Cast to a supported type.
  Type *CastTy = Builder.getIntNTy(BitWidth);
  Value *Cast = Builder.CreateIntCast(I->getOperand(0), CastTy, IsSigned);

  Value *Result = expandToLibCall(Builder, I, LibCallName, I->getType(), Cast);

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

bool Float128Expand::expandFPToI(IRBuilder<> &Builder, Instruction *I) {
  unsigned BitWidth = I->getType()->getIntegerBitWidth();
  bool IsSigned = I->getOpcode() == Instruction::FPToSI;

  StringRef LibCallName;
  if (BitWidth <= 32) {
    BitWidth = 32;
    LibCallName = IsSigned ? "__qtoi" : "__qtou";
  } else if (BitWidth <= 64) {
    BitWidth = 64;
    LibCallName = IsSigned ? "__qtoj" : "__qtok";
  } else if (BitWidth <= 128) {
    BitWidth = 128;
    LibCallName = IsSigned ? "__qtom" : "__qton";
  } else
    return false; // We don't support more than 128 bits.

  Type *ResTy = Builder.getIntNTy(BitWidth);
  Value *Result = expandToLibCall(Builder, I, LibCallName, ResTy,
                                  I->getOperand(0));

  // Cast the result type if needed.
  if (Result->getType() != I->getType()) {
    Result = Builder.CreateTrunc(Result, I->getType());
  }

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

bool Float128Expand::expandFCmp(IRBuilder<> &Builder, Instruction *I) {
  StringRef LibCallName;
  bool Invert = false;
  switch (cast<FCmpInst>(I)->getPredicate()) {
  default: return false;
  case FCmpInst::FCMP_OEQ: LibCallName = "__eqq"; break;
  case FCmpInst::FCMP_UNE: LibCallName = "__neq"; break;
  case FCmpInst::FCMP_OLT: LibCallName = "__ltq"; break;
  case FCmpInst::FCMP_OGT: LibCallName = "__gtq"; break;
  case FCmpInst::FCMP_OLE: LibCallName = "__leq"; break;
  case FCmpInst::FCMP_OGE: LibCallName = "__geq"; break;
  case FCmpInst::FCMP_ULT: LibCallName = "__geq"; Invert = true; break;
  case FCmpInst::FCMP_UGT: LibCallName = "__leq"; Invert = true; break;
  case FCmpInst::FCMP_ULE: LibCallName = "__gtq"; Invert = true; break;
  case FCmpInst::FCMP_UGE: LibCallName = "__ltq"; Invert = true; break;
  }

  Value *Result = expandToLibCall(Builder, I, LibCallName, Builder.getInt32Ty(),
                                  { I->getOperand(0), I->getOperand(1) });
  Result = Builder.CreateTrunc(Result, I->getType());

  // Invert the result if needed.
  if (Invert)
    Result = Builder.CreateNot(Result);

  I->replaceAllUsesWith(Result);
  I->eraseFromParent();
  return true;
}

void Float128Expand::EnterScope(BasicBlock *BB) {
  LLVM_DEBUG(dbgs() << "Entering: " << BB->getName() << "\n");
  ScopeType *Scope = new ScopeType(VNT);
  ScopeMap[BB] = Scope;
}

void Float128Expand::ExitScope(BasicBlock *BB) {
  LLVM_DEBUG(dbgs() << "Exiting: " << BB->getName() << "\n");
  DenseMap<BasicBlock *, ScopeType *>::iterator SI = ScopeMap.find(BB);
  assert(SI != ScopeMap.end());
  delete SI->second;
  ScopeMap.erase(SI);
}

/// ExitScopeIfDone - Destroy scope for the BB that corresponds to the given
/// dominator tree node if its a leaf or all of its children are done. Walk
/// up the dominator tree to destroy ancestors which are now done.
void Float128Expand::ExitScopeIfDone(
    DomTreeNode *Node, DenseMap<DomTreeNode *, unsigned> &OpenChildren) {
  if (OpenChildren[Node])
    return;

  // Pop scope.
  ExitScope(Node->getBlock());

  // Now traverse upwards to pop ancestors whose offsprings are all done.
  while (DomTreeNode *Parent = Node->getIDom()) {
    unsigned Left = --OpenChildren[Parent];
    if (Left != 0)
      break;
    ExitScope(Parent->getBlock());
    Node = Parent;
  }
}

bool Float128Expand::ProcessInstruction(Instruction *I) {
  using namespace llvm::PatternMatch;
  bool MadeChange = false;
  IRBuilder<> Builder(I);

  switch (I->getOpcode()) {
  default:
    break;
  case Instruction::FSub:
    Value *X;
    if (match(I, m_FNeg(m_Value(X)))) {
      MadeChange |= expandArith(Builder, I, Instruction::FNeg, X);
      break;
    }
    LLVM_FALLTHROUGH;
  case Instruction::FAdd:
  case Instruction::FMul:
  case Instruction::FDiv:
    MadeChange |= expandArith(Builder, I, I->getOpcode(),
                              {I->getOperand(0), I->getOperand(1)});
    break;
  case Instruction::FNeg:
    MadeChange |= expandArith(Builder, I, Instruction::FNeg, I->getOperand(0));
    break;
  case Instruction::FPTrunc:
    MadeChange |= expandFPTrunc(Builder, I);
    break;
  case Instruction::FPExt:
    MadeChange |= expandFPExt(Builder, I);
    break;
  case Instruction::SIToFP:
  case Instruction::UIToFP:
    MadeChange |= expandIToFP(Builder, I);
    break;
  case Instruction::FPToSI:
  case Instruction::FPToUI:
    MadeChange |= expandFPToI(Builder, I);
    break;
  case Instruction::FCmp:
    MadeChange |= expandFCmp(Builder, I);
    break;
  case Instruction::PHI:
    if (isUsedByFP128Libcall(I))
      MadeChange |= TransformFP128PHI(I);
    break;
  }

  return MadeChange;
}

bool Float128Expand::ProcessBlock(BasicBlock *BB) {
  bool MadeChange = false;
  Function *F = BB->getParent();
  // If there is a fp128 variable X which is a function argument, we should
  // create LifetimeStart&Store at the entry bb
  if (BB == &(F->getEntryBlock())) {
    for (Argument &Arg : F->args()) {
      if (!Arg.getType()->isFP128Ty() || !isUsedByFP128Libcall(&Arg))
        continue;
      IRBuilder<> Builder(getFirstNonAllocaInTheEntryBlock(*F));
      AllocaInst *AllocaArg = CreateFP128AllocaInst(Builder, BB);
      CreateFP128LifetimeStart(Builder, AllocaArg, *BB->getParent());
      Builder.CreateAlignedStore(&Arg, AllocaArg,
                                 MaybeAlign(AllocaArg->getAlign()));
      VNT.insert(&Arg, AllocaArg);
      Value2Ptr.insert({&Arg, AllocaArg});
      if (!isUsedbyPHI(&Arg))
        FP128PtrList.insert(AllocaArg);
    }
  }
  for (auto It : BBWorkList[BB]) {
    Instruction *I = &*It;
    if (!I->getType()->isFP128Ty() &&
        !(I->getNumOperands() > 0 && I->getOperand(0)->getType()->isFP128Ty()))
      continue;
    MadeChange |= ProcessInstruction(&*I);
  }
  return MadeChange;
}

bool Float128Expand::PerformFp128Transform(DomTreeNode *Node) {
  SmallVector<DomTreeNode *, 32> Scopes;
  SmallVector<DomTreeNode *, 8> WorkList;
  DenseMap<DomTreeNode *, unsigned> OpenChildren;

  // Perform a DFS walk to determine the order of visit.
  WorkList.push_back(Node);
  do {
    Node = WorkList.pop_back_val();
    Scopes.push_back(Node);
    OpenChildren[Node] = Node->getNumChildren();
    for (DomTreeNode *Child : Node->children())
      WorkList.push_back(Child);
  } while (!WorkList.empty());

  bool Changed = false;
  for (DomTreeNode *Node : Scopes) {
    BasicBlock *BB = Node->getBlock();
    EnterScope(BB);
    Changed |= ProcessBlock(BB);
    // If it's a leaf node, it's done. Traverse upwards to pop ancestors.
    ExitScopeIfDone(Node, OpenChildren);
  }

  return Changed;
}

static bool scalarizeFP128Op(Function &F) {
  bool MadeChange = false;
  SmallVector<Instruction *, 8> ScalarWorkList;
  for (Instruction &I : instructions(F))
    if (isa<FixedVectorType>(I.getType()) &&
        canLowerToFP128Libcall(&I, true))
      ScalarWorkList.push_back(&I);

  for (auto *V : ScalarWorkList) {
    IRBuilder<> Builder(V);
    auto *DstVT = cast<FixedVectorType>(V->getType());
    Value *Res = UndefValue::get(DstVT);
    for (unsigned I = 0, E = DstVT->getNumElements(); I != E; ++I) {
      Value *S0, *S1, *Tmp;
      if (auto *U = dyn_cast<UnaryOperator>(V)) {
        S0 = Builder.CreateExtractElement(V->getOperand(0), I);
        Tmp = Builder.CreateUnOp(U->getOpcode(), S0);
      } else if (auto *C = dyn_cast<CastInst>(V)) {
        S0 = Builder.CreateExtractElement(V->getOperand(0), I);
        Tmp = Builder.CreateCast(C->getOpcode(), S0, DstVT->getScalarType());
      } else if (auto *B = dyn_cast<BinaryOperator>(V)) {
        S0 = Builder.CreateExtractElement(V->getOperand(0), I);
        S1 = Builder.CreateExtractElement(V->getOperand(1), I);
        Tmp = Builder.CreateBinOp(B->getOpcode(), S0, S1);
      } else if (auto *C = dyn_cast<FCmpInst>(V)) {
        S0 = Builder.CreateExtractElement(V->getOperand(0), I);
        S1 = Builder.CreateExtractElement(V->getOperand(1), I);
        Tmp = Builder.CreateFCmp(C->getPredicate(), S0, S1);
      } else {
        llvm_unreachable("Unexpected instruction");
      }
      Res = Builder.CreateInsertElement(Res, Tmp, I);
    }
    V->replaceAllUsesWith(Res);
    MadeChange = true;
  }

  return MadeChange;
}

bool Float128Expand::runOnFunction(Function &F) {
  using namespace llvm::PatternMatch;

  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC)
    return false;

  auto &TM = TPC->getTM<TargetMachine>();
  if (!TM.Options.IntelLibIRCAllowed)
    return false;

  bool MadeChange = false;
  // Expand %d = call fp128 @llvm.fmuladd.f128(fp128 %a, fp128 %b, fp128 %c)
  // into
  // %e = fmul fp128 %a, %b
  // %d = fadd fp128 %e, %c
  SmallVector<Instruction *, 8> ExpandFMAWorkList;
  for (Instruction &I : instructions(F)) {
    IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
    if (II && II->getIntrinsicID() == Intrinsic::fmuladd &&
        II->getType()->getScalarType()->isFP128Ty())
      ExpandFMAWorkList.push_back(II);
  }
  for (Instruction *I : ExpandFMAWorkList) {
    IRBuilder<> Builder(I);
    Value *Res = Builder.CreateFMul(I->getOperand(0), I->getOperand(1));
    Res = Builder.CreateFAdd(Res, I->getOperand(2));
    I->replaceAllUsesWith(Res);
    MadeChange = true;
  }

  MadeChange |= scalarizeFP128Op(F);

  // Calculate each SCC's predecessors and successors in reverse topological
  // order and check if each SCC has a USE of fp128 PX
  for (scc_iterator<Function *> SCCI = scc_begin(&F); !SCCI.isAtEnd(); ++SCCI) {
    const std::vector<BasicBlock *> &CurSCC = *SCCI;
    SCCList.push_back(std::move(std::make_unique<SCCNode>()));
    SCCNode *CurSCCNode = SCCList.back().get();
    for (auto *BI : CurSCC) {
      BB2SCC[BI] = CurSCCNode;
      CurSCCNode->BBList.insert(BI);
    }
    for (auto *BI : CurSCC) {
      for (auto *CurBB : successors(BI)) {
        assert(BB2SCC[CurBB] != nullptr &&
               "Since we calculate each SCC's predecessors and successors in "
               "reverse topological order, BB2SCC[CurBB] should never be "
               "nullptr");
        SCCNode *SCCOut = BB2SCC[CurBB];
        if (SCCOut == CurSCCNode)
          continue;
        CurSCCNode->Succs.insert(SCCOut);
        SCCOut->Preds.insert(CurSCCNode);
      }
      if (SCCI.hasCycle())
        CurSCCNode->HasLoop = true;
    }
  }

  DominatorTree DT(F);
  for (Function::iterator BBI = F.begin(), BBE = F.end(); BBI != BBE; ++BBI) {
    BBWorkList.insert({&*BBI, {}});
    auto &CurBBWorkList = BBWorkList[&*BBI];
    for (BasicBlock::iterator II = BBI->begin(), IE = BBI->end(); II != IE;
         ++II) {
      Instruction *I = &*II;
      if (!I->getType()->isFP128Ty() &&
          !(I->getNumOperands() > 0 &&
            I->getOperand(0)->getType()->isFP128Ty()))
        continue;
      CurBBWorkList.push_back(I);
    }
  }

  MadeChange = PerformFp128Transform(DT.getRootNode());
  PostTransformFP128PHI();
  // Call visitSCCAndCreateLE while visiting SCC in post-order order
  // SCCList is in reverse-topological order.
  for (std::unique_ptr<SCCNode> &UPI : SCCList) {
    visitSCCAndCreateLE(*UPI);
  }

  // Finally, we clear local records.
  BB2SCC.clear();
  LastUse.clear();
  IsLive.clear();
  HasUse.clear();
  FP128PtrList.clear();
  NewPHI2OldPHI.clear();
  Value2Ptr.clear();
  SCCList.clear();
  return MadeChange;
}

// Check if Val is used outside CurSCC.
bool Float128Expand::isUsedOutsideLoops(Value *Val, SCCNode *CurSCC) {
  for (auto *UI : Val->users()) {
    Instruction *UserInst = cast<Instruction>(UI);
    if (BB2SCC[UserInst->getParent()] == CurSCC)
      continue;
    return true;
  }

  return false;
}

// We try to make the whole thing as simple as possible:
// When CurSCC only have only one preceding SCC which has one BB, we will do
// Hoist for LifetimeStart safely. Also, if the TargetBB is catchswitch, we
// won't do Hoist.
BasicBlock *Float128Expand::calculateSafePoint(SCCNode *CurSCC) {
  if (CurSCC->Preds.size() != 1)
    return nullptr;
  SCCNode *PredSCC = *(CurSCC->Preds.begin());
  if (PredSCC->BBList.size() > 1)
    return nullptr;
  BasicBlock *PredBB = *(PredSCC->BBList.begin());
  if (PredBB->isEHPad())
    return nullptr;
  return PredBB;
}

// As for LifetimeEnd, when there are no more uses of a value on a code path, a
// lifetime.end marker can be inserted after the final use.
// In details:
// For PX in all fp128 ptr:
//     If isLive[scc][PX]==false for all scc in CurSCC.successors, then set
//     IsDead=true; otherwise set IsDead=false
//
//     if IsDead && HasUse[CurSCC][PX] && (CurSCC.size == 1):
//              Do LifetimeEnd after LastUse[the only BB of CurSCC][PX]
//     isLive[CurSCC][PX] = !IsDead | HasUse[CurSCC][PX]
//
// Here is two example:
//
// Example#1:
//     bb1(PX's def)
//    /   \
//   /     \
// bb2      bb3(use PX)
// we will create LifetimeEnd after the final PX's use in bb3
// And we won't create LifetimeEnd in bb2.
//
// Example#2:
//     bb1
//       \
//        \
//          bb2(PX's def)
//          / \
//         |   |
//          \ /
//          bb3(use PX)
//            \
//             \
//              bb4
// We won't create PX's LifetimeEnd for PX since bb2&bb3 is in a loop,
// Also we won't create PX's LifetimeEnd in bb4.
void Float128Expand::visitSCCAndCreateLE(SCCNode &CurSCCNode) {
  for (Value *PXI : FP128PtrList) {
    bool IsDead = llvm::none_of(CurSCCNode.Succs, [&](SCCNode *CurSucc) {
      return IsLive.count({CurSucc, PXI});
    });
    if (IsDead && HasUse.count({&CurSCCNode, PXI}) && !CurSCCNode.HasLoop &&
        CurSCCNode.BBList.size() == 1) {
      BasicBlock *TargetBB = *(CurSCCNode.BBList.begin());
      IRBuilder<> Builder(
          LastUse[{TargetBB, PXI}]->getNextNonDebugInstruction());
      CreateFP128LifetimeEnd(Builder, PXI, *TargetBB->getParent());
    }
    // Propagate successors' live state to CurSCCNode.
    if (!IsDead || HasUse.count({&CurSCCNode, PXI}))
      IsLive.insert({&CurSCCNode, PXI});
  }
}
