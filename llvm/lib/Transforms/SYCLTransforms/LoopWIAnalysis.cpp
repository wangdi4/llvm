//===- LoopWIAnalysis.cpp - Loop workitem dependency analysis ---*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/LoopWIAnalysis.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-loop-wi-analysis"

static constexpr unsigned MinIndexBitwidthToPreserve = 16;

// Dependency maps which defines dependency according to 2 input deps.
static constexpr LoopWIInfo::Dependency UNI = LoopWIInfo::Dependency::UNIFORM;
static constexpr LoopWIInfo::Dependency STR = LoopWIInfo::Dependency::STRIDED;
static constexpr LoopWIInfo::Dependency RND = LoopWIInfo::Dependency::RANDOM;

static constexpr LoopWIInfo::Dependency
    AddConversion[LoopWIInfo::NumDeps][LoopWIInfo::NumDeps] = {
        /*         UNI, STR, RND */
        /* UNI */ {UNI, STR, RND},
        /* STR */ {STR, STR, RND},
        /* RND */ {RND, RND, RND}};

static constexpr LoopWIInfo::Dependency
    MulConversion[LoopWIInfo::NumDeps][LoopWIInfo::NumDeps] = {
        /*         UNI, STR, RND */
        /* UNI */ {UNI, STR, RND},
        /* STR */ {STR, RND, RND},
        /* RND */ {RND, RND, RND}};

void LoopWIInfo::run(Loop *L, DominatorTree *DT, LoopInfo *LI) {
  if (!L->isLoopSimplifyForm()) {
    LLVM_DEBUG(dbgs() << "Skip " << L->getName()
                      << " which is not simplify form\n");
    return;
  }

  this->L = L;
  this->DT = DT;
  this->LI = LI;

  // Analyze header phi nodes for stride values.
  getHeaderPhiStride();

  // Analyze rest of the loop instructions.
  DomTreeNode *DTNode = DT->getNode(L->getHeader());
  assert(DTNode && "could not get DT node for header");
  // In release, don't try to analyze...
  if (!DTNode)
    return;

  scanLoop(DTNode);
}

bool LoopWIInfo::isUniform(Value *V) {
  // Uniform values might not be calculated on the analysis pre-processing
  // stage, so check the dependency on the fly.
  return getDependency(V) == Dependency::UNIFORM;
}

bool LoopWIInfo::isStrided(Value *V) {
  auto It = Deps.find(V);
  return It != Deps.end() && It->second == Dependency::STRIDED;
}

bool LoopWIInfo::isRandom(Value *V) {
  auto It = Deps.find(V);
  if (It != Deps.end())
    return It->second == Dependency::RANDOM;

  LLVM_DEBUG(dbgs() << *V << " is not found in Deps, isRandom returns true\n");
  return true;
}

bool LoopWIInfo::isStridedIntermediate(Value *V) {
  return StrideIntermediate.count(V);
}

Constant *LoopWIInfo::getConstStride(Value *V) {
  auto It = ConstStrides.find(V);
  return It == ConstStrides.end() ? nullptr : It->second;
}

void LoopWIInfo::clearValDep(Value *V) {
  Deps.erase(V);
  ConstStrides.erase(V);
  StrideIntermediate.remove(V);
}

void LoopWIInfo::setValStrided(Value *V, Constant *ConstStride) {
  Deps[V] = Dependency::STRIDED;
  if (ConstStride)
    ConstStrides[V] = ConstStride;
}

void LoopWIInfo::print(raw_ostream &OS) const {
  unsigned NumSpaces = 2;
  OS << "LoopWIInfo for " << L->getName() << ":\n";
  OS.indent(NumSpaces) << "PreHeader: " << L->getLoopPreheader()->getName()
                       << "\n";
  OS.indent(NumSpaces) << "Header: " << L->getHeader()->getName() << "\n";
  OS.indent(NumSpaces) << "Latch: " << L->getLoopLatch()->getName() << "\n";

  OS.indent(NumSpaces) << "Value dependencies:\n";
  // Iterate over instructions in the loop so that output is deterministic.
  LoopBlocksRPO RPO(L);
  RPO.perform(LI);
  for (BasicBlock *BB : RPO) {
    if (!L->contains(BB) || LI->getLoopFor(BB) != L)
      continue;
    for (auto &I : *BB) {
      auto It = Deps.find(&I);
      assert(It != Deps.end() && "dep not found");
      StringRef D = (It->second == LoopWIInfo::Dependency::UNIFORM)   ? "UNI"
                    : (It->second == LoopWIInfo::Dependency::STRIDED) ? "STR"
                                                                      : "RND";
      OS.indent(2 * NumSpaces) << D << "  " << I << "\n";
    }
  }
  // print non-instruction values' dependencies. Output is non-deterministic.
  for (const auto &Dep : Deps) {
    if (isa<Instruction>(Dep.first))
      continue;
    StringRef D = (Dep.second == Dependency::UNIFORM)   ? "UNI"
                  : (Dep.second == Dependency::STRIDED) ? "STR"
                                                        : "RND";
    OS.indent(2 * NumSpaces) << D << "  " << *Dep.first << "\n";
  }

  OS.indent(NumSpaces) << "StrideIntermediate:\n";
  for (const auto *S : StrideIntermediate)
    OS.indent(2 * NumSpaces) << *S << "\n";

  OS.indent(NumSpaces) << "ConstStrides:\n";
  for (const auto &C : ConstStrides)
    OS.indent(2 * NumSpaces) << *C.second << "  " << *C.first << "\n";
}

void LoopWIInfo::getHeaderPhiStride() {
  // Loop over all the PHI nodes, looking for a canonical indvar.
  for (auto It = L->getHeader()->begin(); isa<PHINode>(&*It); ++It) {
    auto *PN = cast<PHINode>(&*It);
    HeaderPhi.insert(PN);
    // Initialize the phi with random. If it is not found to be strided, it will
    // be updated in the following lines of code.
    Deps[PN] = Dependency::RANDOM;

    // Currently only support scalar phi in the header block.
    if (PN->getType()->isVectorTy())
      continue;

    // The latch entry is an addition.
    Value *LatchVal = PN->getIncomingValueForBlock(L->getLoopLatch());
    auto *I = dyn_cast<Instruction>(LatchVal);
    if (!I)
      continue;
    if (I->getOpcode() != Instruction::Add &&
        I->getOpcode() != Instruction::FAdd)
      continue;

    // Now check that all the added value is loop invariant.
    Value *Stride = nullptr;
    Value *Op0 = I->getOperand(0);
    Value *Op1 = I->getOperand(1);
    // isLoopInvariant means that the operand is either not an instruction or an
    // instruction outside of the loop. Note that we assume LICM run before.
    if (Op0 == PN && L->isLoopInvariant(Op1))
      Stride = Op1;
    else if (Op1 == PN && L->isLoopInvariant(Op0))
      Stride = Op0;
    if (!Stride)
      continue;

    // PN is incremented with invariant values, so it is strided.
    Deps[PN] = Dependency::STRIDED;

    // Try to update the constant stride.
    auto *ConstStride = dyn_cast<Constant>(Stride);
    if (!ConstStride)
      continue;

    // For vector values, this works only if the stride is a splat.
    if (auto *VectorStride = dyn_cast<ConstantDataVector>(ConstStride)) {
      ConstStride = VectorStride->getSplatValue();
      if (!ConstStride)
        continue;
    }

    ConstStrides[PN] = ConstStride;
  }
}

void LoopWIInfo::scanLoop(DomTreeNode *N) {
  assert(N && "invalid dominator tree node");
  BasicBlock *BB = N->getBlock();

  // If this sub-region is outside the current loop exit.
  if (!L->contains(BB))
    return;

  // We don't analyze instructions in sub-loops.
  if (!LoopUtils::inSubLoop(BB, L, LI))
    for (auto It = (BB == L->getHeader())
                       ? BasicBlock::iterator(BB->getFirstNonPHI())
                       : BB->begin(),
              E = BB->end();
         It != E; ++It)
      calculateDep(&*It);

  // Go over blocks recursively according to dominator tree.
  for (DomTreeNode *Child : N->children())
    scanLoop(Child);
}

LoopWIInfo::Dependency LoopWIInfo::getDependency(Value *V) {
  auto It = Deps.find(V);
  if (It != Deps.end())
    return It->second;

  // Go through instructions inside the loop according to dominator tree.
  // Thus the only value that are not encountered before should be invariant
  // or instruction computed inside sub-loops.
  bool IsInvariant = L->isLoopInvariant(V);
  assert((IsInvariant || (isa<Instruction>(V) &&
                          LoopUtils::inSubLoop(cast<Instruction>(V), L, LI))) &&
         "non-invariant value with no dep");

  // Do not assume anything on values computed in sub-loops.
  if (!IsInvariant)
    return Dependency::RANDOM;

  // Non-vector invariants are considered uniform.
  if (!V->getType()->isVectorTy()) {
    Deps[V] = Dependency::UNIFORM;
    return Dependency::UNIFORM;
  }

  // Vectors are considered uniform only if all the elements are the same.
  Dependency Dep = Dependency::RANDOM;
  if (auto *SVI = dyn_cast<ShuffleVectorInst>(V)) {
    if (isBroadcast(SVI))
      Dep = Dependency::UNIFORM;
  } else if (auto *CV = dyn_cast<ConstantVector>(V)) {
    if (CV->getSplatValue())
      Dep = Dependency::UNIFORM;
  } else if (auto *CDV = dyn_cast<ConstantDataVector>(V)) {
    if (CDV->getSplatValue())
      Dep = Dependency::UNIFORM;
  } else if (isa<ConstantAggregateZero>(V)) {
    // All the elements of zeroinitializer are the same.
    Dep = Dependency::UNIFORM;
  }

  Deps[V] = Dep;
  return Dep;
}

void LoopWIInfo::calculateDep(Instruction *I) {
  assert(I && "invalid value");

  // Default is random.
  Dependency Dep = Dependency::RANDOM;

  // Handle supported that can generate stride values.
  if (auto *BO = dyn_cast<BinaryOperator>(I)) {
    Dep = calculateDep(BO);
  } else if (auto *CI = dyn_cast<CastInst>(I)) {
    Dep = calculateDep(CI);
    updateConstStride(CI, CI->getOperand(0));
  } else if (auto *EEI = dyn_cast<ExtractElementInst>(I)) {
    Dep = calculateDep(EEI);
  } else if (auto *SVI = dyn_cast<ShuffleVectorInst>(I)) {
    Dep = calculateDep(SVI);
  }

  Deps[I] = Dep;
}

LoopWIInfo::Dependency LoopWIInfo::calculateDep(BinaryOperator *BO) {
  // Special treatment for generation of sequential index.
  if (BO->getOpcode() == Instruction::Add && isSequentialVector(BO))
    return Dependency::STRIDED;

  // Calculate the dependency type for each of the operands.
  Value *Op0 = BO->getOperand(0);
  Value *Op1 = BO->getOperand(1);
  Dependency Dep0 = getDependency(Op0);
  Dependency Dep1 = getDependency(Op1);

  switch (BO->getOpcode()) {
  case Instruction::Add:
  case Instruction::FAdd:
    if (Dep0 == Dependency::STRIDED && Dep1 == Dependency::UNIFORM)
      updateConstStride(BO, Op0);
    else if (Dep1 == Dependency::STRIDED && Dep0 == Dependency::UNIFORM)
      updateConstStride(BO, Op1);
    return AddConversion[static_cast<int>(Dep0)][static_cast<int>(Dep1)];
  case Instruction::Sub:
  case Instruction::FSub:
    if (Dep0 == Dependency::STRIDED && Dep1 == Dependency::UNIFORM)
      updateConstStride(BO, Op0);
    else if (Dep1 == Dependency::STRIDED && Dep0 == Dependency::UNIFORM)
      updateConstStride(BO, Op1, /* Negate */ true);
    return AddConversion[static_cast<int>(Dep0)][static_cast<int>(Dep1)];
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::Shl:
    return MulConversion[static_cast<int>(Dep0)][static_cast<int>(Dep1)];
  default:
    break;
  }

  return Dependency::RANDOM;
}

LoopWIInfo::Dependency LoopWIInfo::calculateDep(CastInst *CI) {
  Dependency Dep0 = getDependency(CI->getOperand(0));

  switch (CI->getOpcode()) {
  case Instruction::SExt:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::AddrSpaceCast:
    return Dep0;
  case Instruction::ZExt:
  case Instruction::BitCast:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
    return Dependency::RANDOM;
  case Instruction::Trunc: {
    Type *DestTy = CI->getDestTy();
    if (DestTy->isVectorTy())
      DestTy = cast<VectorType>(DestTy)->getElementType();
    auto *IntTy = dyn_cast<IntegerType>(DestTy);
    return (IntTy && IntTy->getBitWidth() >= MinIndexBitwidthToPreserve)
               ? Dep0
               : Dependency::RANDOM;
  }
  default:
    llvm_unreachable("unexpected cast inst opcode");
  }

  return Dependency::RANDOM;
}

// On extract element, the scalar value has the same type as the vector type.
LoopWIInfo::Dependency LoopWIInfo::calculateDep(ExtractElementInst *EEI) {
  Value *VectorOp = EEI->getVectorOperand();
  updateConstStride(EEI, VectorOp);
  return getDependency(VectorOp);
}

LoopWIInfo::Dependency LoopWIInfo::calculateDep(ShuffleVectorInst *SVI) {
  if (!isBroadcast(SVI))
    return Dependency::RANDOM;

  using namespace PatternMatch;
  Value *V;
  if (!match(SVI, m_Shuffle(m_InsertElt(m_Value(), m_Value(V), m_ZeroInt()),
                            m_Value(), m_ZeroMask())))
    return Dependency::RANDOM;

  auto *IEI = cast<InsertElementInst>(SVI->getOperand(0));
  if (!IEI->hasOneUse())
    return Dependency::RANDOM;

  Dependency Dep = getDependency(V);
  if (Dep == Dependency::UNIFORM) {
    Deps[IEI] = Dependency::UNIFORM;
    return Dep;
  }

  if (Dep != Dependency::STRIDED)
    return Dependency::RANDOM;

  // We further demand that the shuffle is broadcast of strided value with the
  // same width of the vector.
  auto It = ConstStrides.find(V);
  if (It == ConstStrides.end())
    return Dependency::RANDOM;
  auto *Stride = dyn_cast<ConstantInt>(It->second);
  if (!Stride || !Stride->equalsInt(
                     cast<FixedVectorType>(SVI->getType())->getNumElements()))
    return Dependency::RANDOM;

  updateConstStride(SVI, V);
  if (Dep == Dependency::STRIDED) {
    StrideIntermediate.insert(IEI);
    updateConstStride(IEI, V);
  }

  return Dependency::STRIDED;
}

// clang-format off
// Sequential IDs instruction example (%c):
//   %a = insertelement <8 x i64> undef, i64 stride_val, i32 0
//   %b = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> zeroinitializer
//   %c = add <8 x i64> %b, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
// Another example (%c):
//   %a = insertelement <8 x i32> poison, i32 stride_val, i64 0
//   %b = shufflevector <8 x i32> %a, <8 x i32> poison, <8 x i32> zeroinitializer
//   %c = add <8 x i32> %b, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7>
// clang-format on
bool LoopWIInfo::isSequentialVector(Instruction *I) {
  assert(I &&
         (I->getOpcode() == Instruction::Add ||
          I->getOpcode() == Instruction::ICmp) &&
         "expected add or icmp inst");
  auto *VTy = dyn_cast<FixedVectorType>(I->getType());
  if (!VTy)
    return false;

  // Pattern of sequantial IDs is addition of constant <0,1,2,...> with
  // broadcast using shuffle.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);
  ShuffleVectorInst *SVI = nullptr;
  if (isConsecutiveConstVector(Op0))
    SVI = dyn_cast<ShuffleVectorInst>(Op1);
  else if (isConsecutiveConstVector(Op1))
    SVI = dyn_cast<ShuffleVectorInst>(Op0);
  if (!SVI)
    return false;

  auto It = ConstStrides.find(SVI);
  if (It == ConstStrides.end())
    return false;

  // Fill data structure.
  LLVM_DEBUG(dbgs() << "Found sequential IDs inst: " << *I
                    << ", Stride: " << *It->second << "\n");
  ConstStrides[I] = It->second;

  return true;
}

bool LoopWIInfo::isConsecutiveConstVector(Value *V) {
  assert(V && "invalid value");
  const auto *CV = dyn_cast<ConstantDataVector>(V);
  if (!CV)
    return false;

  auto *VTy = cast<FixedVectorType>(CV->getType());
  if (!VTy->isIntOrIntVectorTy())
    return false;

  auto IsConsecutive = [&](bool Negate) {
    for (unsigned I = 0, E = VTy->getNumElements(); I != E; ++I) {
      int64_t C = cast<ConstantInt>(CV->getAggregateElement(I))->getSExtValue();
      if (Negate)
        C = -C;
      // If this is not sequence 0,1,2,... return false.
      if (C != (int64_t)I)
        return false;
    }
    return true;
  };

  return IsConsecutive(false) || IsConsecutive(true);
}

// We do not count undef masks as different.
bool LoopWIInfo::isBroadcast(ShuffleVectorInst *SVI) {
  assert(SVI && "invalid shuffle vector inst");
  auto *VTy = dyn_cast<FixedVectorType>(SVI->getType());
  assert(VTy && "shuffle should have vector type");
  int Ind = SVI->getMaskValue(0);
  for (unsigned I = 1, E = VTy->getNumElements(); I != E; ++I) {
    // For undef, getMaskValue() returns -1.
    int MaskI = SVI->getMaskValue(I);
    if (MaskI != Ind && MaskI != -1)
      return false;
  }

  return true;
}

void LoopWIInfo::updateConstStride(Value *ToUpdate, Value *UpdateBy,
                                   bool Negate) {
  if (!UpdateBy->getType()->isIntOrIntVectorTy())
    return;

  auto It = ConstStrides.find(UpdateBy);
  if (It == ConstStrides.end())
    return;

  auto *StrideConst = dyn_cast<ConstantInt>(It->second);
  assert(StrideConst && "stride is expected to be constant scalar");
  int64_t Stride = StrideConst->getSExtValue();
  if (Negate)
    Stride = -Stride;
  Constant *NewStrideConst = nullptr;
  auto *ToUpdateTy = ToUpdate->getType();
  if (ToUpdateTy->isIntOrIntVectorTy())
    NewStrideConst = ConstantInt::get(ToUpdateTy->getScalarType(), Stride);
  else if (ToUpdateTy->isFPOrFPVectorTy())
    NewStrideConst = ConstantFP::get(ToUpdateTy->getScalarType(),
                                     static_cast<double>(Stride));
  if (NewStrideConst) {
    LLVM_DEBUG(dbgs() << "Const stride of " << *ToUpdate
                      << " is updated to stride of " << *NewStrideConst
                      << "\n");
    ConstStrides[ToUpdate] = NewStrideConst;
  } else {
    LLVM_DEBUG(dbgs() << "Unsupported constant stride\n");
  }
}

AnalysisKey LoopWIAnalysis::Key;

LoopWIInfo LoopWIAnalysis::run(Loop &L, LoopAnalysisManager &,
                               LoopStandardAnalysisResults &AR) {
  LoopWIInfo WIInfo;
  WIInfo.run(&L, &AR.DT, &AR.LI);
  return WIInfo;
}

PreservedAnalyses LoopWIAnalysisPrinter::run(Loop &L, LoopAnalysisManager &AM,
                                             LoopStandardAnalysisResults &AR,
                                             LPMUpdater &) {
  AM.getResult<LoopWIAnalysis>(L, AR).print(OS);
  return PreservedAnalyses::all();
}
