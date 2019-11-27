//===- Intel_Float128Expand.cpp - Expand FP128 operations -----------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
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

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
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
};

} // end anonymous namespace

char Float128Expand::ID = 0;

char &llvm::Float128ExpandID = Float128Expand::ID;

INITIALIZE_PASS(Float128Expand, DEBUG_TYPE, "Expand fp128 instructions",
                false, false)

FunctionPass *llvm::createFloat128ExpandPass() { return new Float128Expand(); }

static Value *expandToLibCall(IRBuilder<> &Builder, Instruction *I,
                              StringRef LibCallName,
                              Type *RetTy, ArrayRef<Value *> Ops) {
  LLVMContext &Ctx = Builder.getContext();
  Function &F = *I->getFunction();
  Module *M = I->getModule();
  const DataLayout &DL = M->getDataLayout();

  Type *FP128Ty = Type::getFP128Ty(Ctx);
  unsigned AllocaAlignment = DL.getPrefTypeAlignment(FP128Ty);
  ConstantInt *SizeVal64 = Builder.getInt64(DL.getTypeStoreSize(FP128Ty));

  SmallVector<Value *, 3> Args;

  AllocaInst *AllocaDst = nullptr;
  AllocaInst *AllocaOp0 = nullptr;
  AllocaInst *AllocaOp1 = nullptr;

  unsigned AllocaAS = DL.getAllocaAddrSpace();

  // If the return is FP128, create an alloca and push it to the arg list.
  if (RetTy->isFP128Ty()) {
    AllocaDst = new AllocaInst(FP128Ty, AllocaAS, "",
                               &F.getEntryBlock().front());
    AllocaDst->setAlignment(MaybeAlign(AllocaAlignment));

    Builder.CreateLifetimeStart(AllocaDst, SizeVal64);
    Args.push_back(AllocaDst);
    // Replace the type with void.
    RetTy = Type::getVoidTy(Ctx);
  }

  // If the first argument is an FP128, create an alloca and push it
  // to the arg list.
  assert(Ops.size() > 0 && Ops.size() <= 2 && "Unexpected number of operands");
  if (Ops[0]->getType()->isFP128Ty()) {
    AllocaOp0 = new AllocaInst(FP128Ty, AllocaAS, "",
                               &F.getEntryBlock().front());
    AllocaOp0->setAlignment(MaybeAlign(AllocaAlignment));

    Builder.CreateLifetimeStart(AllocaOp0, SizeVal64);
    Builder.CreateAlignedStore(Ops[0], AllocaOp0, AllocaAlignment);
    Args.push_back(AllocaOp0);
  } else {
    // Otherwise just push the argument.
    Args.push_back(Ops[0]);
  }

  // If there is a second argument it will always be an fp128.
  if (Ops.size() >= 2) {
    assert(Ops[1]->getType()->isFP128Ty() && "Unexpected type!");
    AllocaOp1 = new AllocaInst(FP128Ty, AllocaAS, "",
                               &F.getEntryBlock().front());
    AllocaOp1->setAlignment(MaybeAlign(AllocaAlignment));

    Builder.CreateLifetimeStart(AllocaOp1, SizeVal64);
    Builder.CreateAlignedStore(Ops[1], AllocaOp1, AllocaAlignment);
    Args.push_back(AllocaOp1);
  }

  SmallVector<Type *, 3> ArgTys;
  for (Value *Arg : Args)
    ArgTys.push_back(Arg->getType());
  FunctionType *FnType = FunctionType::get(RetTy, ArgTys, false);
  FunctionCallee LibcallFn = M->getOrInsertFunction(LibCallName, FnType);
  Value *Result = Builder.CreateCall(LibcallFn, Args);

  // If we had fp128 operands, we're done with their allocas now.
  if (AllocaOp0)
    Builder.CreateLifetimeEnd(AllocaOp0, SizeVal64);
  if (AllocaOp1)
    Builder.CreateLifetimeEnd(AllocaOp1, SizeVal64);

  if (AllocaDst) {
    Result = Builder.CreateAlignedLoad(FP128Ty, AllocaDst, AllocaAlignment);
    Builder.CreateLifetimeEnd(AllocaDst, SizeVal64);
  }

  return Result;
}

// Expand a binary operator and fneg on fp128 into a library call.
static bool expandArith(IRBuilder<> &Builder, Instruction *I, unsigned Opcode,
                        ArrayRef<Value *> Ops) {
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

static bool expandFPTrunc(IRBuilder<> &Builder, Instruction *I) {
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

static bool expandFPExt(IRBuilder<> &Builder, Instruction *I) {
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

static bool expandIToFP(IRBuilder<> &Builder, Instruction *I) {
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

static bool expandFPToI(IRBuilder<> &Builder, Instruction *I) {
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

static bool expandFCmp(IRBuilder<> &Builder, Instruction *I) {
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

bool Float128Expand::runOnFunction(Function &F) {
  using namespace llvm::PatternMatch;

  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC)
    return false;

  auto &TM = TPC->getTM<TargetMachine>();
  if (!TM.Options.IntelLibIRCAllowed)
    return false;

  bool MadeChange = false;

  SmallVector<Instruction *, 8> Worklist;
  for (Instruction &I : instructions(&F)) {
    // First see if this instruction uses an fp128 type.
    // TODO: Do all relevant instructions have fp128 result or first argument?
    // TODO: What about vectors of fp128?
    if (!I.getType()->isFP128Ty() &&
        !(I.getNumOperands() > 0 && I.getOperand(0)->getType()->isFP128Ty()))
      continue;

    Worklist.push_back(&I);
  }

  for (auto *I : Worklist) {
    IRBuilder<> Builder(I);

    switch (I->getOpcode()) {
    default: break;
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
                                { I->getOperand(0), I->getOperand(1) });
      break;
    case Instruction::FNeg:
      MadeChange |= expandArith(Builder, I, Instruction::FNeg,
                                I->getOperand(0));
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
    }
  }

  return MadeChange;
}
