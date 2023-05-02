//===- IntelVPSOAAnalysis.cpp ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the VPSOAAnalysis class.
//===---------------------------------------------------------------------===//

#include "IntelVPSOAAnalysis.h"
#include "IntelVPlanUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SaveAndRestore.h"
#include <queue>

#define DEBUG_TYPE "vpsoa-analysis"

using namespace llvm;
using namespace llvm::vpo;

// Flag to enable printing of SOA-analysis information.
static cl::opt<bool, true> VPlanDisplaySOAAnalysisInformationOpt(
    "vplan-dump-soa-info", cl::Hidden,
    cl::location(VPlanDisplaySOAAnalysisInformation),
    cl::desc("Display information about SOA Analysis on loop-entities."));

// Flag to bailout on any phi-node.
static cl::opt<bool> VPlanAllowSOAPhis(
    "vplan-enable-soa-phis", cl::init(true), cl::Hidden,
    cl::desc("Allow phi nodes for SOA pointers. I.e. if it's disabled the only "
             "accesses like a[i] for SOA arrays are allowed, any constructions "
             "like *a++ will bailout of SOA transformation."));

namespace llvm {
namespace vpo {
bool VPlanDisplaySOAAnalysisInformation = false;

static bool checkInstructionInLoop(const VPValue *V, const VPLoop &Loop) {
  // Check for nullptr and VPInstruction here to avoid these checks on the
  // caller-side.
  return V == nullptr || !isa<VPInstruction>(V) ||
         Loop.contains(cast<VPInstruction>(V)->getParent());
}

static bool areTypeSizesEqual(const DataLayout *DL, Type *Ty1, Type *Ty2) {
  return DL->getTypeSizeInBits(Ty1) == DL->getTypeSizeInBits(Ty2);
}

// Public interface for SOA-analysis for all loop-privates. \p SOAVars is the
// output argument that return the variables marked for SOA-layout.
void VPSOAAnalysis::doSOAAnalysis(SmallPtrSetImpl<VPInstruction *> &SOAVars) {
  if (!Plan.isSOAAnalysisEnabled())
    return;
  assert(Plan.getVPlanDA() && "Expect DA to be run and the DA object be set "
                              "before running of SOAAnalysis.");

  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  SmallVector<VPAllocatePrivate *, 4> AllocaList;
  // Iterate through all the instructions in the loop-preheader, and for
  // VPAllocatePrivate instruction check if the allocated private is safe to
  // convert it to SOA format.
  for (VPInstruction &VInst : *Preheader) {
    if (VPAllocatePrivate *AllocaPriv = dyn_cast<VPAllocatePrivate>(&VInst)) {
      LLVM_DEBUG(dbgs() << "Analyzing "; AllocaPriv->printAsOperand(dbgs());
                 dbgs() << "\n");
      AllocaList.push_back(AllocaPriv);
      if (!memoryEscapes(AllocaPriv)) {
        AllocaPriv->setSOASafe(true);
      }
      LLVM_DEBUG(dbgs() << (AllocaPriv->isSOASafe() ? "Safe\n" : "Unsafe\n"););
    }
  }
  // During previous analysis we assume all VPAllocatePrivate encountered
  // as phi operands are safe. Now, we need to propagate the non-safety
  // through phis which use unsafe allocate private.
  // E.g. suppose we have the following.
  //
  //  %p1 = allocate_private i32
  //  %p2 = allocate_private i32
  //
  //  ...
  //  ; safe  uses of %p1
  //  ...
  //  ; unsafe use of %p2
  //
  //  %phi1 = phi i32* [%p1, %b1], [%p2, b2]
  //
  // At the moment when we analyze %p1 we don't know that %p2 is unsafe
  // and mark %p1 as safe. Thus we need to propagate unsafety of %p2
  // to %p1. For that we put such phis in the AllocaToPhiMap and in the
  // PhiToAllocaMap.
  //

  std::queue<VPAllocatePrivate *> UnsafeAllocas;
  for (VPAllocatePrivate *AP : AllocaList) {
    // Register as unsafe if so.
    if (!AP->isSOASafe())
      UnsafeAllocas.push(AP);

    // Simple scalars are always safe, no need to update maps.
    if (isScalarTy(AP->getAllocatedType()))
      continue;

    // Update maps for propagation.
    for (auto It = df_begin<VPUser *>(AP), End = df_end<VPUser *>(AP);
         It != End;) {
      // Users of the loads and calls are not interesting.
      if (isa<VPLoadStoreInst>(*It) || isa<VPCallInstruction>(*It)) {
        It.skipChildren();
        continue;
      }
      if (isa<VPPHINode>(*It)) {
        auto *Inst = cast<VPInstruction>(*It);
        PhiToAllocaMap[Inst].insert(AP);
        AllocaToPhiMap[AP].insert(Inst);
      }
      ++It;
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DEBUG(
      SaveAndRestore<bool> EnableP(VPlanDisplaySOAAnalysisInformation, true);
      dbgs() << "After safety analysis\n"; dump(dbgs()););
#endif

  while (!UnsafeAllocas.empty()) {
    auto *AP = UnsafeAllocas.front();
    UnsafeAllocas.pop();
    auto PhiIter = AllocaToPhiMap.find(AP);
    if (PhiIter == AllocaToPhiMap.end())
      continue;
    // Unsafe alloca is used by a phi: propagate the unsafety to
    // other allocas if they are used by the same phis.
    for (const VPInstruction *I : PhiIter->second) {
      auto AllocaIter = PhiToAllocaMap.find(I);
      assert(AllocaIter != PhiToAllocaMap.end() &&
             "Expected non null iterator");
      for (VPAllocatePrivate *DepAlloca : AllocaIter->second)
        if (DepAlloca != AP && DepAlloca->isSOASafe()) {
          DepAlloca->setSOASafe(false);
          UnsafeAllocas.push(DepAlloca);
        }
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DEBUG(
      SaveAndRestore<bool> EnableP(VPlanDisplaySOAAnalysisInformation, true);
      dbgs() << "After propagation\n"; dump(dbgs()););
#endif

  auto PropagateProfitability = [this,
                                 &SOAVars](VPAllocatePrivate *AP) -> void {
    auto PhiIter = AllocaToPhiMap.find(AP);
    if (PhiIter == AllocaToPhiMap.end())
      return;
    for (const VPInstruction *I : PhiIter->second) {
      auto AllocaIter = PhiToAllocaMap.find(I);
      assert(AllocaIter != PhiToAllocaMap.end() &&
             "Expected non null iterator");
      for (VPAllocatePrivate *DepAlloca : AllocaIter->second)
        if (DepAlloca != AP) {
          assert(DepAlloca->isSOASafe() && "Expected SOASafe linked private");
          DepAlloca->setSOAProfitable();
          SOAVars.insert(DepAlloca);
        }
    }
  };

  for (VPAllocatePrivate *AllocaPriv : AllocaList) {
    if (!AllocaPriv->isSOASafe())
      continue;
    if (!AllocaPriv->isSOAProfitable()) {
      // If not propagated already, calculate profitability
      if (isSOAProfitable(AllocaPriv)) {
        AllocaPriv->setSOAProfitable();
        SOAVars.insert(AllocaPriv);
      }
    }
    if (AllocaPriv->isSOAProfitable())
      PropagateProfitability(AllocaPriv);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dump(dbgs());
#endif
}

// Returns true if the pointee type of the alloca-inst is a scalar or an array
// of scalars.
bool VPSOAAnalysis::isSOASupportedTy(Type *Ty) {

  return isScalarTy(Ty) ||
         isa<ArrayType>(Ty) &&
             isScalarTy(cast<ArrayType>(Ty)->getElementType());
}

// Returns true if UseInst is any function call, that we know is safe to pass a
// private-pointer to and does not change the data-layout.
bool VPSOAAnalysis::SOASafetyChecker::isSafePointerEscapeFunction(
    const VPCallInstruction *VPCall) {

  // Only lifetime_start/end, invariant_start/end, and elementsize intrinsics
  // are considered safe.
  return VPCall->isIntrinsicFromList(
      {Intrinsic::lifetime_start, Intrinsic::lifetime_end,
       Intrinsic::invariant_start, Intrinsic::invariant_end,
       Intrinsic::intel_directive_elementsize});
}

// Returns true if any of the following instruction with specific constraints
// are encountered.
bool VPSOAAnalysis::SOASafetyChecker::isSafeLoadStore(
    const VPLoadStoreInst *LSI, const VPInstruction *CurrentI,
    Type *PrivElemSize) {
  // If this is a non-simple (e.g. volatile) load/store, treat this as
  // unsafe.
  if (!LSI->isSimple())
    return false;

  // In addition, we consider stores unsafe when the private-pointer or its
  // alias escapes via a write to external memory.
  if (LSI->getOpcode() == Instruction::Store && LSI->getOperand(0) == CurrentI)
    return false;

  // Non-scalar types are not supported yet.
  if (!isScalarTy(LSI->getValueType()))
    return false;

  // UseInst is a potentially unsafe load/store instruction, i.e.
  // the size of memory loaded/stored is different from the original private
  // alloca element type size, e.g.:
  //
  // ptr %vp1 = allocate-priv [2520 x i8]*
  // double %vp2 = load ptr %vp1
  // getTypeSizeInBits(i8) != getTypeSizeInBits(double).
  // Note, SOA transformation is legal when load/store size is
  // less than an element type. However, it is not profitable
  // as it causes gathers, it is also difficult to calculate
  // strides for load/store without the underlying type knowledge.
  return areTypeSizesEqual(Analysis.Plan.getDataLayout(), LSI->getValueType(),
                           PrivElemSize);
}

bool VPSOAAnalysis::SOASafetyChecker::isSafeGEPInst(
    const VPGEPInstruction *VPGEP, Type *AllocatedType,
    Type *PrivElemSize) const {
  Type *SourceType = VPGEP->getSourceElementType();

  // Any GEP with original type is safe.
  if (SourceType == AllocatedType)
    return true;

  if (!Analysis.isSOASupportedTy(SourceType))
    return false;

  // If the GEP is for an array type, check whether it has same type elements.
  auto *SourceArrayType = dyn_cast<ArrayType>(SourceType);
  if (SourceArrayType && SourceArrayType->getElementType() != PrivElemSize)
    return false;

  // GEP can be potentially unsafe when it computes pointers
  // using a type with different size from the original private
  // alloca element type size, e.g.:
  //
  // %arr.priv = alloca [1024 x i32]
  // %gep = getelementptr inbounds i8, ptr %down, i64 2
  // %ld = load i32, ptr %gep
  //
  // TODO: for struct aggregates, check if the type is the same as the
  // original private element type.
  return areTypeSizesEqual(Analysis.Plan.getDataLayout(),
                           VPGEP->getResultElementType(), PrivElemSize);
}

bool VPSOAAnalysis::SOASafetyChecker::isSafeVPSubscriptInst(
    const VPSubscriptInst *VPS, Type *AllocatedType, Type *PrivElemSize) const {
  if (isSelfAddressOfInst(VPS))
    return true;

  // For supported privates we should have 2-dim VPSubscript.
  if (VPS->getNumDimensions() != 2)
    return false;

  Type *Dim1Ty = VPS->dim(1).DimElementType;
  Type *Dim0Ty = VPS->dim(0).DimElementType;

  // Non-scalar types are not supported yet.
  if (!isScalarTy(Dim0Ty))
    return false;

  // The type of the 0 dimension should have the same width,
  // and the type of 1 should be equal to Allocated type.
  return (Dim1Ty == AllocatedType) &&
         areTypeSizesEqual(Analysis.Plan.getDataLayout(), Dim0Ty, PrivElemSize);
}

// Check whether \p V is a chain of VPInstructions started with
// VPAllocatePrivate(s) - in case of phi/select we check for all operands be
// derived from VPAllocatePrivate.
//
// The "true" case (considering %merge):
//   %p1 = allocate_private
//   %p2 = allocate_private
//   ...
//   %p1_cast = getelementptr %p1...
//   ...
//   %merge = phi ptr [%p1_cast, %some_block], [%p2, %other_block]
//
// The "false" case (considering %merge):
//   %p1 = allocate_private
//   ...
//   %p1_cast = getelementptr %p1...
//   ...
//   %p2_cast = getelementptr %global_ptr...
//
//   %merge = phi ptr [%p1_cast, %some_block], [%p2_cast, %other_block]
//
bool VPSOAAnalysis::SOASafetyChecker::isDerivedFromAllocatePriv(
    const VPValue *V) {
  // Early quick-check to see if this is a VPAllocatePrivate.
  if (isa<VPAllocatePrivate>(V))
    return true;

  SmallVector<const VPValue *, 20> WL;
  SmallPtrSet<const VPValue *, 20> Visited;
  WL.push_back(V);

  while (!WL.empty()) {
    const VPValue *CurrentI = WL.pop_back_val();

    // If this instruction/value has been encountered before, continue.
    if (!Visited.insert(CurrentI).second)
      continue;

    // Skip casts, geps, and subscripts.
    auto SkipPointerTransforms = [](const VPValue *Val) -> const VPValue * {
      auto *VPI = dyn_cast<VPInstruction>(Val);
      if (!VPI)
        return Val;
      while (VPI->isCast() || isa<VPGEPInstruction>(VPI) ||
             isa<VPSubscriptInst>(VPI)) {
        auto Op = VPI->getOperand(0);
        if (isa<VPAllocatePrivate>(Op))
          return Op;
        if (!(VPI = dyn_cast<VPInstruction>(Op)))
          return Op;
      }
      return VPI;
    };

    CurrentI = SkipPointerTransforms(CurrentI);

    // If we encounter VPAllocatePrivate just skip.
    if (isa<VPAllocatePrivate>(CurrentI)) {
      continue;
    }

    auto ProcessOperand =
        [&WL, &SkipPointerTransforms](const VPValue *InVal) -> void {
      InVal = SkipPointerTransforms(InVal);
      WL.push_back(InVal);
    };

    if (auto *PHI = dyn_cast<VPPHINode>(CurrentI)) {
      for (auto *InVal : PHI->incoming_values())
        ProcessOperand(InVal);
      continue;
    }

    if (auto VPI = dyn_cast<VPInstruction>(CurrentI))
      if (VPI->getOpcode() == Instruction::Select) {
        for (unsigned I = 1; I < VPI->getNumOperands(); I++)
          ProcessOperand(VPI->getOperand(I));
        continue;
      }

    // Other cases, like ptrtoint or load, are not supported
    return false;
  }
  // Nothing on stack means all the ways lead to a VPAllocatePrivate.
  return true;
}

bool VPSOAAnalysis::SOASafetyChecker::isMergeSafeForSOA(
    const VPInstruction *Inst) {
  assert((Inst->getOpcode() == Instruction::Select ||
          Inst->getOpcode() == Instruction::PHI) &&
         "Expected phi or select");

  unsigned Start = Inst->getOpcode() == Instruction::Select ? 1 : 0;
  for (unsigned I = Start; I < Inst->getNumOperands(); I++) {
    auto *Operand = Inst->getOperand(I);
    if (Operand == Private) {
      continue;
    }
    if (!isDerivedFromAllocatePriv(Operand))
      return false;
  }

  return true;
}

// An umbrella function to determine the safety of an operation.
// Return false when we want to report that the memory escapes.
bool VPSOAAnalysis::SOASafetyChecker::isSafeUse(const VPInstruction *UseInst,
                                                const VPInstruction *CurrentI,
                                                Type *AllocatedType) {
  Type *PrivElemSize = cast<ArrayType>(AllocatedType)->getElementType();

  switch (UseInst->getOpcode()) {
  case Instruction::GetElementPtr:
    return isSafeGEPInst(cast<VPGEPInstruction>(UseInst),
                         AllocatedType, PrivElemSize);
  case VPInstruction::Subscript:
    return isSafeVPSubscriptInst(cast<VPSubscriptInst>(UseInst),
                                 AllocatedType, PrivElemSize);
  case Instruction::Call:
    return isSafePointerEscapeFunction(cast<VPCallInstruction>(UseInst));

  case VPInstruction::ExpandLoad:
  case VPInstruction::ExpandLoadNonu:
  case VPInstruction::CompressStore:
  case VPInstruction::CompressStoreNonu:
    // compress/expand is not supported for SOA layout
    return false;

  case Instruction::Load:
  case Instruction::Store:
    return isSafeLoadStore(cast<VPLoadStoreInst>(UseInst),
                           CurrentI, PrivElemSize);

  case Instruction::PHI:
  case Instruction::Select:
    return VPlanAllowSOAPhis && isMergeSafeForSOA(UseInst);

  case VPInstruction::ReductionInitArr:
  case VPInstruction::ReductionFinalArr:
    // SOA support is available for array reductions in CG.
    return true;

  case VPInstruction::InductionInit:
    // Intentionally use isa<> to make assert non-trivial,
    // so LLVM_FALLTHROUGH can be used without compiler warning
    // for LLVM_FALLTHROUGH being used in unreachable code.
    assert(!isa<VPInductionInit>(UseInst) &&
           "VPInductionInit is not supported yet");
    LLVM_FALLTHROUGH;
  default:
    return isTrivialPointerAliasingInst<VPInstruction>(UseInst);
  }
}

// Return true if the instruction is either in the loop-preheader or the
// loop-body.
bool VPSOAAnalysis::isInstructionInRelevantScope(const VPInstruction *UseInst) {
  if (!UseInst)
    return false;
  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  return ((UseInst->getParent() == Preheader) ||
          (checkInstructionInLoop(UseInst, Loop)));
}

bool VPSOAAnalysis::memoryEscapes(VPAllocatePrivate *Alloca) {
  // Non-array aggregate types are currently not supported. Conservatively, just
  // return 'true', i.e., the memory escapes.
  // TODO: add support for non-scalar types.
  if (!isSOASupportedTy(Alloca->getAllocatedType()))
    return true;

  SOASafetyChecker Chk(*this, Alloca);
  return !Chk.isSafeForSOA();
}

bool VPSOAAnalysis::SOASafetyChecker::isSafeForSOA() {

  // Clear the WorkList and AnalyzedInsts of contents of the earlier run.
  WL.clear();
  AnalyzedInsts.clear();

  // If this is a scalar-private, just return. The real memory layout for simple
  // scalars is identical for both SOA and AOS, it's just vector of elements.
  Type *AllocatedType = Private->getAllocatedType();
  if (isScalarTy(AllocatedType))
    return true;

  // Initialize the WorkList with the memory-pointer.
  WL.insert(Private);

  // Get all the Uses of these instructions. Consider the use as safe under
  // the following conditions,
  //
  // 1) If it is load/store, where the size of the loaded/stored memory is
  // the same as the size of the original element type in the alloca.
  //
  // 2) If it is a store and the value-operand, a pointer we are checking, is
  // not written to external memory (e.g., output argument of a function).
  //
  // 3) Call to a function that is known to not read/write memory passed by
  // pointer and not storing the pointer to memory.

  // Every other instruction is considered unsafe or potentially-unsafe.

  // Instructions that create pointer aliases, checked via
  // isTrivialPointerAliasingInst(), are added to the worklist for further
  // analysis.

  while (!WL.empty()) {
    const VPInstruction *CurrentI = WL.pop_back_val();

    // Analyze the users of the current-instruction.
    for (VPValue *User : CurrentI->users()) {
      const VPInstruction *UseInst = cast<VPInstruction>(User);

      // We are only interested in pointer or its alias which is either in the
      // Loop-preheader of within the loop itself.
      // TODO: this seems fragile, it is better to use an explicit check,
      // in case VPlan evolves in the future.
      if (!Analysis.isInstructionInRelevantScope(UseInst))
        continue;

      // Determine if the instruction is potentially unsafe.
      LLVM_DEBUG(dbgs() << "Checking use "; UseInst->printAsOperand(dbgs()););
      if (!isSafeUse(UseInst, CurrentI, AllocatedType)) {
        LLVM_DEBUG(dbgs() << " unsafe\n";);
        return false;
      }
      LLVM_DEBUG(dbgs() << " safe\n";);
      // If we already pushed the users of the UseInst then skip them.
      if (!AnalyzedInsts.insert(UseInst).second)
        continue;

      if (isTrivialPointerAliasingInst<VPInstruction>(UseInst))
        // If this is one of the aliasing instructions add it to the
        // worklist, we'd like to look through them.
        WL.insert(UseInst);
    }
  }
  // All encountered uses to the pointer or its aliases are safe instructions.
  // We can say that it is SOASafe.
  return true;
}

// Return true if the given memory-access would be profitable under
// SOA-layout.
// TODO: Make this function iterative (non-recursive).
bool VPSOAAnalysis::isProfitableForSOA(const VPInstruction *I) {

  // Memory access-type can be reasonably inferred from GEP. For other
  // instruction-types, we try to further analyze the source operand. If it
  // originates from a GEP, or another aliasing instruction, try and analyze
  // that instruction further.

  // Checked or cached Profitable memory accesses.
  if (AccessProfitabilityInfo.count(I))
    return AccessProfitabilityInfo[I];

  auto OpRangeIsOk = [this](VPUser::const_operand_range Operands) {
    return all_of(Operands, [this](const VPValue *V) {
      assert(V->getType()->isPointerTy() && "Expected ptr");
      if (auto *VI = dyn_cast<VPInstruction>(V))
        return isProfitableForSOA(VI);
      return false;
    });
  };

  // A pointer-op/alias running into a PHI denotes a pointer-induction of some
  // sort. For now, treat this as profitable if it's uniform.
  if (I->getOpcode() == Instruction::PHI) {
    AccessProfitabilityInfo[I] = true; // avoid cyclic dependencies
    return AccessProfitabilityInfo[I] =
               OpRangeIsOk(make_range(I->op_begin(), I->op_end()));
  }
  if (I->getOpcode() == Instruction::Select) {
    bool IsProfitable = OpRangeIsOk(make_range(I->op_begin() + 1, I->op_end()));
    return AccessProfitabilityInfo[I] = IsProfitable;
  }

  // For bitcast and addrspacecast instruction, recur on the source-operand.
  if (I->getOpcode() == Instruction::BitCast ||
      I->getOpcode() == Instruction::AddrSpaceCast) {
    bool IsProfitable =
        isProfitableForSOA(cast<VPInstruction>(I->getOperand(0)));
    return AccessProfitabilityInfo[I] = IsProfitable;
  }

  // If we reached the original alloca, then access does not depend
  // on any indices.
  if (auto *VPAlloca = dyn_cast<VPAllocatePrivate>(I))
    return AccessProfitabilityInfo[I] = true;

  // We should have a GEP/Subscript at this point. The safety-analysis rules out
  // other instructions which were deemed unsafe for SOA-layout. This includes
  // unsafe aliasing instuctions or function-calls. lifetime.start/end and
  // invariant.start/end intrinsics should not appear here.

  VPValue *PtrOp = nullptr;
  if (auto *MemRef = dyn_cast<VPGEPInstruction>(I))
    PtrOp = MemRef->getPointerOperand();
  else if (auto *MemRef = dyn_cast<VPSubscriptInst>(I))
    PtrOp = MemRef->getPointerOperand();
  else {
    // The assert is triggered in DEBUG compiler and we return false in the
    // release compiler.
    assert(false &&
           "Expect a GEP/Subscript instruction at this point in the code.");
    return false;
  }

  if (!isProfitableForSOA(cast<VPInstruction>(PtrOp)))
    return AccessProfitabilityInfo[I] = false;

  auto *DA = Plan.getVPlanDA();
  // Uniform-access.
  // If all the indices are uniform, then the access under SOA-layout would be
  // unit-stride.
  if (all_of(make_range(I->op_begin() + 1, I->op_end()),
             [DA](const VPValue *V) { return !DA->isDivergent(*V); }))
    return AccessProfitabilityInfo[I] = true;

  // TODO: Unit-stride access.
  // Unit-stride accesses on array-privates are 'divergent' and shape is
  // 'random'. We analyze the GEP further, looking at the last-idx's shape. If
  // it is 'unit-stride' and the GEP is directly used we return true.
  // There is a scenario where we might have a chain of uniform GEPs following
  // this unit-stride GEP, and the load at the end is using it. We do not
  // capture this scenario.

  return AccessProfitabilityInfo[I] = false;
}

// Determine if SOA-layout is profitable for the given alloca (loop-entity).
void VPSOAAnalysis::collectLoadStores(const VPAllocatePrivate *Alloca,
                                      DenseSet<VPLoadStoreInst *> &LoadStores) {

  // Use a WL-based approach to collect loads and stores on a particular alloca.
  std::queue<const VPValue *> WL;

  // Set of visited instructions to avoid infinite-loop arising out of cyclic
  // use-def chains.
  DenseSet<const VPValue *> Visited;

  WL.push(Alloca);

  while (!WL.empty()) {

    auto *CurrentI = WL.front();
    WL.pop();

    //If the instruction has been already visited, continue.
    if (!Visited.insert(CurrentI).second)
      continue;

    // If this instruction is not in the loop, continue.
    if (isa<VPInstruction>(CurrentI) &&
        !isInstructionInRelevantScope(cast<VPInstruction>(CurrentI)))
      continue;

    // Analyze the users of the current Instruction.
    for (auto *User : CurrentI->users())
      if (auto *LoadStore = dyn_cast<VPLoadStoreInst>(User))
        LoadStores.insert(LoadStore);
      else
        WL.push(User);
  }
}

// Determine if SOA-layout is profitable for the given alloca (loop-entity).
bool VPSOAAnalysis::isSOAProfitable(VPAllocatePrivate *Alloca) {

  // Clear the access-profitability map.
  AccessProfitabilityInfo.clear();

  // If this is a scalar-private, it is always profitable.
  if (isScalarTy(Alloca->getAllocatedType()))
    return true;

  // Algorithm: Collect all the loads and stores for the given alloca. Analyze
  // the pointer op of the load/store, recursively if needed, and determine if
  // that memory access is an uniform memory access.

  // Collect the Loads and Stores on the alloca.
  DenseSet<VPLoadStoreInst *> LoadStores;
  collectLoadStores(Alloca, LoadStores);

  for (auto *LSI : LoadStores) {
    VPValue *PtrOp = LSI->getPointerOperand();
    // Return true if there is a single load/store instruction which has a
    // profitable memory access.
    if (isProfitableForSOA(cast<VPInstruction>(PtrOp)))
      return true;
  }

  // Couldn't positively identify the Alloca having any profitable memory
  // access.
  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPSOAAnalysis::dump(raw_ostream &OS) const {
  if (!VPlanDisplaySOAAnalysisInformation)
    return;

  VPBasicBlock *Preheader = Loop.getLoopPreheader();
  OS << "SOA profitability:\n";
  for (VPInstruction &VPInst : *Preheader) {
    if (VPAllocatePrivate *VPAllocaPriv =
            dyn_cast<VPAllocatePrivate>(&VPInst)) {

      StringRef OrigName = VPAllocaPriv->getOrigName();
      if (VPAllocaPriv->isSOASafe()) {
        OS << "SOASafe = ";
        VPAllocaPriv->printAsOperandNoType(OS);
        if (!OrigName.empty())
          OS << " (" << OrigName << ")";
        OS << " Profitable = " << VPAllocaPriv->isSOAProfitable() << "\n";
      } else {
        OS << "SOAUnsafe = ";
        VPAllocaPriv->printAsOperandNoType(OS);
        if (!OrigName.empty())
          OS << " (" << OrigName << ")";
        OS << "\n";
      }
    }
  }
}

// For use in debug sessions, dump the list of SOA-profitable instructions
// encountered till the given point in Analysis.
void VPSOAAnalysis::dumpSOAProfitable(raw_ostream &OS) const {
  for (const VPInstruction *I : profitabilityAnalyzedAccessInsts()) {
    OS << *I << '\n';
  }
}

// For use in debug sessions, dump the list of SOA-unprofitable instructions
// encountered till the given point in Analysis.
void VPSOAAnalysis::dumpSOAUnprofitable(raw_ostream &OS) const {
  for (const VPInstruction *I : profitabilityAnalyzedAccessInsts<false>()) {
    OS << *I << '\n';
  }
}
#endif // NDEBUG
} // namespace vpo.
} // namespace llvm.
