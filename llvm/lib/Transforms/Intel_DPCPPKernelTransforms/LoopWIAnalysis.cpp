//===- LoopWIAnalysis.cpp - Loop workitem dependency analysis ---*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LoopWIAnalysis.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-loop-wi-analysis"

static constexpr unsigned MinIndexBitwidthToPreserve = 16;

// Dependency maps which defines dependency according to 2 input deps.
static constexpr LoopWIInfo::Dependency UNI = LoopWIInfo::Dependency::UNIFORM;
static constexpr LoopWIInfo::Dependency STR = LoopWIInfo::Dependency::STRIDED;
static constexpr LoopWIInfo::Dependency RND = LoopWIInfo::Dependency::RANDOM;

static constexpr LoopWIInfo::Dependency
    AddConversion[LoopWIInfo::Dependency::NumDeps]
                 [LoopWIInfo::Dependency::NumDeps] = {
                     /*         UNI, STR, RND */
                     /* UNI */ {UNI, STR, RND},
                     /* STR */ {STR, STR, RND},
                     /* RND */ {RND, RND, RND}};

static constexpr LoopWIInfo::Dependency
    MulConversion[LoopWIInfo::Dependency::NumDeps]
                 [LoopWIInfo::Dependency::NumDeps] = {
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
  StrideIntermediate.erase(V);
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
  for (const auto &Dep : Deps) {
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
    return AddConversion[Dep0][Dep1];
  case Instruction::Sub:
  case Instruction::FSub:
    if (Dep0 == Dependency::STRIDED && Dep1 == Dependency::UNIFORM)
      updateConstStride(BO, Op0);
    else if (Dep1 == Dependency::STRIDED && Dep0 == Dependency::UNIFORM)
      updateConstStride(BO, Op1, /* Negate */ true);
    return AddConversion[Dep0][Dep1];
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::Shl:
    return MulConversion[Dep0][Dep1];
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

// Sequential IDs instruction example (%c):
// %a = insertelement <8 x i64> undef, i64 stride_val, i32 0
// %b = shufflevector <8 x i64> %a, <8 x i64> undef, <8 x i32> zeroinitializer
// %c = add <8 x i64> %b, <i64 0, i8 1, ...>
bool LoopWIInfo::isSequentialVector(Instruction *I) {
  assert(I && I->getOpcode() == Instruction::Add && "expected add inst");
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
  if (!SVI || !isBroadcast(SVI))
    return false;

  // We further demand that the shuffle is broadcast of strided value with the
  // same width of the vector.
  unsigned MaskVal = SVI->getMaskValue(0);
  auto *IEI = dyn_cast<InsertElementInst>(SVI->getOperand(0));
  if (!IEI || !IEI->hasOneUse())
    return false;
  auto *C = dyn_cast<ConstantInt>(IEI->getOperand(2));
  if (!C || !C->equalsInt(MaskVal))
    return false;
  Value *TID = IEI->getOperand(1);
  auto It = ConstStrides.find(TID);
  if (It == ConstStrides.end())
    return false;
  auto *Stride = dyn_cast<ConstantInt>(It->second);
  if (!Stride || !Stride->equalsInt(VTy->getNumElements()))
    return false;

  // Fill data structure.
  LLVM_DEBUG(dbgs() << "Found sequential IDs inst: " << *I
                    << ", Stride: " << *Stride << "\n");
  StrideIntermediate.insert(IEI);
  StrideIntermediate.insert(SVI);
  ConstStrides[I] = Stride;
  ConstStrides[IEI] = Stride;
  ConstStrides[SVI] = Stride;

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

  for (unsigned I = 0, E = VTy->getNumElements(); I != E; ++I) {
    auto *C = dyn_cast<ConstantInt>(CV->getAggregateElement(I));
    // If this is not sequence 0,1,2,... return false.
    if (!C->equalsInt(I))
      return false;
  }

  return true;
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

INITIALIZE_PASS_BEGIN(LoopWIAnalysisLegacy, DEBUG_TYPE,
                      "provides work item dependency info for loops", false,
                      true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(LoopWIAnalysisLegacy, DEBUG_TYPE,
                    "provides work item dependency info for loops", false, true)

char LoopWIAnalysisLegacy::ID = 0;

LoopWIAnalysisLegacy::LoopWIAnalysisLegacy() : LoopPass(ID) {
  initializeLoopWIAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

void LoopWIAnalysisLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.setPreservesAll();
}

bool LoopWIAnalysisLegacy::runOnLoop(Loop *L, LPPassManager &LPM) {
  DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  WIInfo.run(L, DT, LI);
  return false;
}

LoopPass *llvm::createLoopWIAnalysisLegacyPass() {
  return new LoopWIAnalysisLegacy();
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
