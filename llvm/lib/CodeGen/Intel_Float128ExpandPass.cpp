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
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/ScopedHashTable.h"
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
  ScopedHTType VNT;
  DenseMap<BasicBlock *, ScopeType *> ScopeMap;
  DenseMap<BasicBlock *, SmallVector<Instruction *, 4>> BBWorkList;
  MapVector<PHINode *, PHINode *> NewPHI2OldPHI;
  DenseMap<Value *, Instruction *> Value2Ptr;

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
  auto AllocaAlignment = MaybeAlign(DL.getPrefTypeAlignment(FP128Ty));
  unsigned AllocaAS = DL.getAllocaAddrSpace();
  AllocaInst *AllocaRes =
      new AllocaInst(FP128Ty, AllocaAS, "", &F.getEntryBlock().front());
  AllocaRes->setAlignment(MaybeAlign(AllocaAlignment));
  return AllocaRes;
}

static bool isUsedByFP128Libcall(Value *Val) {
  for (auto UI : Val->users()) {
    Instruction *I = cast<Instruction>(UI);
    switch (I->getOpcode()) {
    default:
      break;
    case Instruction::FAdd:
    case Instruction::FSub:
    case Instruction::FMul:
    case Instruction::FDiv:
    case Instruction::FNeg:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
    case Instruction::FPToSI:
    case Instruction::FPToUI:
    case Instruction::FCmp:
      return true;
    }
  }
  return false;
}

// in our first traverse, if the oldphi is used by  a libcall,
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
      if (IncomingPtr == nullptr) {
        IRBuilder<> Builder(&*NewPhi->getParent()->getFirstInsertionPt());
        AllocaInst *AllocaForPhi =
            CreateFP128AllocaInst(Builder, NewPhi->getParent());
        Builder.CreateAlignedStore(OldPhi, AllocaForPhi,
                                   MaybeAlign(AllocaForPhi->getAlignment()));
        NewPhi->replaceAllUsesWith(AllocaForPhi);
        NewPhi->eraseFromParent();

        break;
      }
      BasicBlock *IncomingBB = OldPhi->getIncomingBlock(i);
      NewPhi->addIncoming(IncomingPtr, IncomingBB);
    }
  }
  return;
}

Value *Float128Expand::expandToLibCall(IRBuilder<> &Builder, Instruction *I,
                                       StringRef LibCallName, Type *RetTy,
                                       ArrayRef<Value *> Ops) {
  LLVMContext &Ctx = Builder.getContext();
  Type *FP128Ty = Type::getFP128Ty(Ctx);
  Module *M = I->getModule();
  SmallVector<Value *, 3> Args;

  AllocaInst *AllocaDst = nullptr;
  AllocaInst *AllocaOp0 = nullptr;
  AllocaInst *AllocaOp1 = nullptr;


  // If the return is FP128, create an alloca and push it to the arg list.
  if (RetTy->isFP128Ty()) {
    AllocaDst = CreateFP128AllocaInst(Builder, I->getParent());
    Args.push_back(AllocaDst);
    // Replace the type with void.
    RetTy = Type::getVoidTy(Ctx);
  }

  // If the first argument is an FP128, create an alloca and push it
  // to the arg list.
  assert(Ops.size() > 0 && Ops.size() <= 2 && "Unexpected number of operands");
  if (Ops[0]->getType()->isFP128Ty()) {
    if (!VNT.count(Ops[0])) {
      AllocaOp0 = CreateFP128AllocaInst(Builder, I->getParent());
      Builder.CreateAlignedStore(Ops[0], AllocaOp0,
                                 MaybeAlign(AllocaOp0->getAlignment()));
      Args.push_back(AllocaOp0);
      VNT.insert(Ops[0], AllocaOp0);
      Value2Ptr.insert({Ops[0], AllocaOp0});
    } else {
      Instruction *Op0 = VNT.lookup(Ops[0]);
      Args.push_back(Op0);
    }

  } else {
    // Otherwise just push the argument.
    Args.push_back(Ops[0]);
  }

  // If there is a second argument it will always be an fp128.
  if (Ops.size() >= 2) {

    if (!VNT.count(Ops[1])) {
      assert(Ops[1]->getType()->isFP128Ty() && "Unexpected type!");
      AllocaOp1 = CreateFP128AllocaInst(Builder, I->getParent());
      Builder.CreateAlignedStore(Ops[1], AllocaOp1,
                                 MaybeAlign(AllocaOp1->getAlignment()));
      Args.push_back(AllocaOp1);
      VNT.insert(Ops[1], AllocaOp1);
      Value2Ptr.insert({Ops[1], AllocaOp1});
    } else {
      Instruction *Op1 = VNT.lookup(Ops[1]);
      Args.push_back(Op1);
    }
  }

  SmallVector<Type *, 3> ArgTys;
  for (Value *Arg : Args)
    ArgTys.push_back(Arg->getType());
  FunctionType *FnType = FunctionType::get(RetTy, ArgTys, false);
  FunctionCallee LibcallFn = M->getOrInsertFunction(LibCallName, FnType);
  Value *Result = Builder.CreateCall(LibcallFn, Args);

  // If we had fp128 operands, we're done with their allocas now.
  if (AllocaOp0) {
    // TODO How to handle the life time end?
    // Builder.CreateLifetimeEnd(AllocaOp0, SizeVal64);
  }
  if (AllocaOp1) {
    // TODO How to handle the life time end?
    // Builder.CreateLifetimeEnd(AllocaOp1, SizeVal64);
  }

  if (AllocaDst) {
    Result = Builder.CreateAlignedLoad(FP128Ty, AllocaDst,
                                       MaybeAlign(AllocaDst->getAlignment()));
    // TODO How to handle the life time end?
    // Builder.CreateLifetimeEnd(AllocaDst, SizeVal64);
    // Record the Result maping to AllocaDst so that we can resue it later.
    VNT.insert(Result, AllocaDst);
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
  // create Store at the entry bb
  if (BB == &(F->getEntryBlock())) {
    for (Argument &Arg : F->args()) {
      if (!Arg.getType()->isFP128Ty() || !isUsedByFP128Libcall(&Arg))
        continue;
      IRBuilder<> Builder(getFirstNonAllocaInTheEntryBlock(*F));
      AllocaInst *AllocaArg = CreateFP128AllocaInst(Builder, BB);

      Builder.CreateAlignedStore(&Arg, AllocaArg,
                                 MaybeAlign(AllocaArg->getAlignment()));
      VNT.insert(&Arg, AllocaArg);
      Value2Ptr.insert({&Arg, AllocaArg});
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
    const std::vector<DomTreeNode *> &Children = Node->getChildren();
    OpenChildren[Node] = Children.size();
    for (DomTreeNode *Child : Children)
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

bool Float128Expand::runOnFunction(Function &F) {
  using namespace llvm::PatternMatch;

  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC)
    return false;

  auto &TM = TPC->getTM<TargetMachine>();
  if (!TM.Options.IntelLibIRCAllowed)
    return false;

  bool MadeChange = false;
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
  // clear local record
  NewPHI2OldPHI.clear();
  Value2Ptr.clear();
  return MadeChange;
}
