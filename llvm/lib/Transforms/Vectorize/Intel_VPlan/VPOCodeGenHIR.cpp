//===----- VPOCodeGenHIR.cpp --------------------------------------------===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the HIR vector Code generation for VPlan.
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

#include "VPOCodeGenHIR.h"

#define DEBUG_TYPE "VPOCGHIR"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::vpo;

STATISTIC(LoopsVectorized, "Number of HIR loops vectorized");

static cl::opt<bool>
    DisableStressTest("disable-vplan-stress-test", cl::init(false), cl::Hidden,
                      cl::desc("Disable VPO Vectorizer Stress Testing"));

/// Don't vectorize loops with a known constant trip count below this number if
/// set to a non zero value.
static cl::opt<unsigned> TinyTripCountThreshold(
    "vplan-vectorizer-min-trip-count", cl::init(0), cl::Hidden,
    cl::desc("Don't vectorize loops with a constant "
             "trip count that is smaller than this value."));

namespace llvm {

static RegDDRef *getConstantSplatDDRef(DDRefUtils &DDRU, Constant *ConstVal,
                                       unsigned VF) {
  Constant *ConstVec = ConstantVector::getSplat(VF, ConstVal);
  if (isa<ConstantDataVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantDataVector>(ConstVec));
  if (isa<ConstantAggregateZero>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantAggregateZero>(ConstVec));
  if (isa<ConstantVector>(ConstVec))
    return DDRU.createConstDDRef(cast<ConstantVector>(ConstVec));
  llvm_unreachable("Unhandled vector type");
}

bool VPOCodeGenHIR::isConstStrideRef(const RegDDRef *Ref, unsigned NestingLevel,
                                     int64_t *CoeffPtr) {
  if (!Ref->isMemRef())
    return false;

  // Return false for cases where the lowest dimension has trailing struct
  // field offsets.
  if (Ref->hasTrailingStructOffsets(1))
    return false;

  const CanonExpr *FirstCE = *(Ref->canon_begin());

  // Consider a[(i1 + 1) & 3], this is changed to a[zext.i2.i64(i1 + 1)] - we
  // do not want to treat this reference as unit stride.
  if (FirstCE->isSExt() || FirstCE->isZExt()) {
    auto SrcTy = FirstCE->getSrcType();
    auto DestTy = FirstCE->getDestType();

    // Do not go conservative for the common case.
    bool OK = false;
    if (SrcTy->isIntegerTy(32) && DestTy->isIntegerTy(64)) {
      // Check for simple sum of IVs
      OK = true;
      for (auto IV = FirstCE->iv_begin(), IVE = FirstCE->iv_end(); IV != IVE;
           ++IV) {
        int64_t Coeff;
        unsigned BlobCoeff;
        FirstCE->getIVCoeff(IV, &BlobCoeff, &Coeff);

        if (Coeff == 0)
          continue;

        if (BlobCoeff != 0 || Coeff != 1) {
          OK = false;
          break;
        }
      }
    }

    if (!OK)
      return false;
  }

  int64_t ConstStride;
  if (!Ref->getConstStrideAtLevel(NestingLevel, &ConstStride))
    return false;

  if (!ConstStride)
    return false;

  // Compute stride in terms of number of elements
  auto DL = Ref->getDDRefUtils().getDataLayout();
  auto RefSizeInBytes = DL.getTypeSizeInBits(Ref->getDestType()) >> 3;
  ConstStride /= RefSizeInBytes;
  if (CoeffPtr)
    *CoeffPtr = ConstStride;

  return true;
}

namespace {
class HandledCheck final : public HLNodeVisitorBase {
private:
  bool IsHandled;
  const HLLoop *OrigLoop;
  TargetLibraryInfo *TLI;
  unsigned VF;
  bool UnitStrideRefSeen;
  bool MemRefSeen;
  unsigned LoopLevel;

  void visitRegDDRef(RegDDRef *RegDD);
  void visitCanonExpr(CanonExpr *CExpr);

public:
  HandledCheck(const HLLoop *OrigLoop, TargetLibraryInfo *TLI, int VF)
      : IsHandled(true), OrigLoop(OrigLoop), TLI(TLI), VF(VF),
        UnitStrideRefSeen(false), MemRefSeen(false) {
    LoopLevel = OrigLoop->getNestingLevel();
  }

  void visit(HLDDNode *Node);

  void visit(HLNode *Node) {
    DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - unsupported HLNode\n");
    IsHandled = false;
  }

  void postVisit(HLNode *Node) {}

  bool isDone() const { return (!IsHandled); }
  bool isHandled() { return IsHandled; }
  bool getUnitStrideRefSeen() { return UnitStrideRefSeen; }
  bool getMemRefSeen() { return MemRefSeen; }
};
} // End anonymous namespace

void HandledCheck::visit(HLDDNode *Node) {
  if (!isa<HLInst>(Node) && !isa<HLIf>(Node)) {
    DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - only HLInst/HLIf are "
                    "supported\n");
    IsHandled = false;
    return;
  }

  // Calls supported are masked/non-masked svml and non-masked intrinsics.
  if (HLInst *Inst = dyn_cast<HLInst>(Node)) {
    auto LLInst = Inst->getLLVMInstruction();

    if (LLInst->mayThrow()) {
      DEBUG(Inst->dump());
      DEBUG(dbgs()
            << "VPLAN_OPTREPORT: Loop not handled - instruction may throw\n");
      IsHandled = false;
      return;
    }

    auto Opcode = LLInst->getOpcode();
    if ((Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
         Opcode == Instruction::URem || Opcode == Instruction::SRem) &&
        (Inst->getParent() != OrigLoop)) {
      DEBUG(Inst->dump());
      DEBUG(dbgs()
            << "VPLAN_OPTREPORT: Loop not handled - DIV/REM instruction\n");
      IsHandled = false;
      return;
    }

    auto TLval = Inst->getLvalDDRef();

    if (TLval && TLval->isTerminalRef() &&
        OrigLoop->isLiveOut(TLval->getSymbase()) &&
        Inst->getParent() != OrigLoop) {
      DEBUG(Inst->dump());
      DEBUG(dbgs() << "VPLAN_OPTREPORT: Liveout conditional scalar assign "
                      "not handled\n");
      IsHandled = false;
      return;
    }

    if (Inst->isCallInst()) {
      const CallInst *Call = cast<CallInst>(Inst->getLLVMInstruction());
      StringRef CalledFunc = Call->getCalledFunction()->getName();

      if (Inst->getParent() != OrigLoop &&
          (VF > 1 && !TLI->isFunctionVectorizable(CalledFunc, VF))) {
        // Masked svml calls are supported, but masked intrinsics are not at
        // the moment.
        DEBUG(Inst->dump());
        DEBUG(
            dbgs() << "VPLAN_OPTREPORT: Loop not handled - masked intrinsic\n");
        IsHandled = false;
        return;
      }

      // Quick hack to avoid loops containing fabs in 447.dealII from becoming
      // vectorized due to bug in unrolling. The problem involves loop index
      // variable that spans outside the array range, resulting in segfault.
      // floor calls are also temporarily disabled until FeatureOutlining is
      // fixed (CQ410864)
      if (CalledFunc == "fabs" || CalledFunc == "floor") {
        DEBUG(Inst->dump());
        DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - fabs/floor call "
                        "disabled\n");
        IsHandled = false;
        return;
      }

      Intrinsic::ID ID = getVectorIntrinsicIDForCall(Call, TLI);
      if ((VF > 1 && !TLI->isFunctionVectorizable(CalledFunc, VF)) && !ID) {
        DEBUG(dbgs()
              << "VPLAN_OPTREPORT: Loop not handled - call not vectorizable\n");
        IsHandled = false;
        return;
      }
    }
  }

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    visitRegDDRef(*Iter);
  }
}

// visitRegDDRef - Visits RegDDRef to visit the Canon Exprs
// present inside it.
void HandledCheck::visitRegDDRef(RegDDRef *RegDD) {
  int64_t IVConstCoeff;

  if (!VectorType::isValidElementType(RegDD->getSrcType()) ||
      !VectorType::isValidElementType(RegDD->getDestType())) {
    DEBUG(RegDD->getSrcType()->dump());
    DEBUG(RegDD->getDestType()->dump());
    DEBUG(
        dbgs() << "VPLAN_OPTREPORT: Loop not handled - invalid element type\n");
    IsHandled = false;
    return;
  }

  if (VPOCodeGenHIR::isConstStrideRef(RegDD, LoopLevel, &IVConstCoeff) &&
      IVConstCoeff == 1)
    UnitStrideRefSeen = true;

  // Visit CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    visitCanonExpr(*Iter);
  }

  // Visit GEP Base
  if (RegDD->hasGEPInfo()) {
    MemRefSeen = true;

    auto BaseCE = RegDD->getBaseCE();

    if (!BaseCE->isInvariantAtLevel(LoopLevel)) {
      DEBUG(dbgs()
            << "VPLAN_OPTREPORT: Loop not handled - BaseCE not invariant\n");
      IsHandled = false;
      return;
    }
  }
}

// Checks Canon Expr to see if we support it. Currently, we do not
// support blob IV coefficients
void HandledCheck::visitCanonExpr(CanonExpr *CExpr) {
  if (CExpr->hasIVBlobCoeff(LoopLevel)) {
    DEBUG(dbgs()
          << "VPLAN_OPTREPORT: Loop not handled - IV with blob coefficient\n");
    IsHandled = false;
    return;
  }

  SmallVector<unsigned, 8> BlobIndices;
  CExpr->collectBlobIndices(BlobIndices, false);

  for (auto &BI : BlobIndices) {
    auto TopBlob = CExpr->getBlobUtils().getBlob(BI);

    if (CExpr->getBlobUtils().isNestedBlob(TopBlob)) {
      IsHandled = false;
      return;
    }
  }
}

// Return true if Loop is currently handled by HIR vector code generation.
bool VPOCodeGenHIR::loopIsHandled(HLLoop *Loop, unsigned int VF) {

  // Only handle normalized loops
  if (!Loop->isNormalized()) {
    DEBUG(
        dbgs()
        << "VPLAN_OPTREPORT: Loop not handled - loop not in normalized form\n");
    return false;
  }

  // We are working with normalized loops, trip count is loop UpperBound + 1.
  auto UBRef = Loop->getUpperDDRef();
  int64_t UBConst;

  if (UBRef->isIntConstant(&UBConst)) {
    auto ConstTripCount = UBConst + 1;

    // Check for minimum trip count threshold
    if (TinyTripCountThreshold && ConstTripCount <= TinyTripCountThreshold) {
      DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - loop with small "
                      "trip count\n");
      return false;
    }

    // Check that main vector loop will have at least one iteration
    if (ConstTripCount < VF) {
      DEBUG(
          dbgs()
          << "VPLAN_OPTREPORT: Loop not handled - zero iteration main loop\n");
      return false;
    }

    // Set constant trip count
    setTripCount((uint64_t)ConstTripCount);
  }

  HandledCheck NodeCheck(Loop, TLI, VF);
  HLNodeUtils::visitRange(NodeCheck, Loop->child_begin(), Loop->child_end());
  if (!NodeCheck.isHandled())
    return false;

  // If we are not in stress testing mode, only vectorize when some
  // unit stride refs are seen. Still vectorize the case when no mem refs
  // are seen. Remove this check once vectorizer cost model is fully
  // implemented.
  if (DisableStressTest && NodeCheck.getMemRefSeen() &&
      !NodeCheck.getUnitStrideRefSeen()) {
    DEBUG(dbgs() << "VPLAN_OPTREPORT: Loop not handled - all mem refs non "
                    "unit-stride\n");
    return false;
  }

  return true;
}

void VPOCodeGenHIR::initializeVectorLoop(unsigned int VF) {
  assert(VF > 1);
  setVF(VF);

  DEBUG(dbgs() << "VPLAN_OPTREPORT: VPlan handled loop, VF = " << VF << " "
               << Fn.getName() << "\n");
  DEBUG(dbgs() << "Handled loop before vec codegen: \n");
  DEBUG(OrigLoop->dump());

  LoopsVectorized++;
  SRA->computeSafeReductionChains(OrigLoop);
  eraseLoopIntrins();

  // Setup main and remainder loops
  bool NeedRemainderLoop = false;
  auto MainLoop = HIRTransformUtils::setupMainAndRemainderLoops(
      OrigLoop, VF, NeedRemainderLoop, true /* VecMode */);

  MainLoop->extractZtt();
  setNeedRemainderLoop(NeedRemainderLoop);
  setMainLoop(MainLoop);

  // Disable further vectorization attempts on main and remainder loops
  MainLoop->markDoNotVectorize();
  if (NeedRemainderLoop) {
    OrigLoop->markDoNotVectorize();
  }
}

void VPOCodeGenHIR::finalizeVectorLoop(void) {
  DEBUG(dbgs() << "\n\n\nHandled loop after: \n");
  DEBUG(MainLoop->dump());
  if (NeedRemainderLoop)
    DEBUG(OrigLoop->dump());

  if (!MainLoop->hasChildren()) {
    DEBUG(dbgs() << "\n\n\nRemoving empty loop\n");
    HLNodeUtils::remove(MainLoop);
  } else {
    // Prevent LLVM from possibly unrolling vectorized loops with non-constant
    // trip counts. See loop in function fxpAutoCorrelation() that is part of
    // telecom/autcor00data_1 (opt_base_st_64_hsw). Inner loop has max trip
    // count estimate of 16, VPO vectorizer chooses VF=4, and LLVM unrolls by 4.
    // However, the inner loop does not always have a constant 16 trip count,
    // leading to a performance degradation caused by entering the scalar code
    // path.
    if (!MainLoop->isConstTripLoop()) {
      const Loop *Lp = MainLoop->getLLVMLoop();
      LLVMContext &Context = Lp->getHeader()->getContext();
      SmallVector<Metadata *, 4> MDs;
      MDs.push_back(nullptr);
      SmallVector<Metadata *, 1> DisableOperands;
      DisableOperands.push_back(
          MDString::get(Context, "llvm.loop.unroll.disable"));
      MDNode *DisableUnroll = MDNode::get(Context, DisableOperands);
      MDs.push_back(DisableUnroll);
      MDNode *NewLoopID = MDNode::get(Context, MDs);
      NewLoopID->replaceOperandWith(0, NewLoopID);
      MainLoop->setLoopMetadata(NewLoopID);
    }
  }

  // If a remainder loop is not needed get rid of the OrigLoop at this point.
  // Replace calls in remainderloop for FP consistency
  if (NeedRemainderLoop) {
    HIRLoopVisitor LV(OrigLoop, this);
    LV.replaceCalls();
  } else {
    HLNodeUtils::remove(OrigLoop);
  }
}

void VPOCodeGenHIR::eraseLoopIntrinsImpl(bool BeginDir) {
  HLContainerTy::iterator StartIter;
  HLContainerTy::iterator EndIter;
  if (BeginDir) {
    auto BeginNode = WVecNode->getEntryHLNode();
    assert(BeginNode && "Unexpected null entry node in WRNVecLoopNode");
    StartIter = BeginNode->getIterator();
    EndIter = OrigLoop->getIterator();
  } else {
    auto ExitNode = WVecNode->getExitHLNode();
    assert(ExitNode && "Unexpected null exit node in WRNVecLoopNode");
    StartIter = ExitNode->getIterator();

    auto LastNode =
        HLNodeUtils::getLastLexicalChild(OrigLoop->getParent(), OrigLoop);
    EndIter = std::next(LastNode->getIterator());
  }

  int BeginOrEndDirID = BeginDir ? DIR_OMP_SIMD : DIR_OMP_END_SIMD;
  for (auto Iter = StartIter; Iter != EndIter;) {
    auto HInst = dyn_cast<HLInst>(&*Iter);

    if (!HInst) {
      break;
    }

    // Move to the next iterator now as HInst may get removed below
    ++Iter;

    Intrinsic::ID IntrinID;
    if (HInst->isIntrinCall(IntrinID)) {
      if (vpo::VPOAnalysisUtils::isIntelClause(IntrinID)) {
        HLNodeUtils::remove(HInst);
        continue;
      }

      if (vpo::VPOAnalysisUtils::isIntelDirective(IntrinID)) {
        auto Inst = cast<IntrinsicInst>(HInst->getLLVMInstruction());
        StringRef DirStr = vpo::VPOAnalysisUtils::getDirectiveMetadataString(
            const_cast<IntrinsicInst *>(Inst));

        int DirID = vpo::VPOAnalysisUtils::getDirectiveID(DirStr);

        if (DirID == BeginOrEndDirID) {
          HLNodeUtils::remove(HInst);
        } else if (VPOAnalysisUtils::isListEndDirective(DirID)) {
          HLNodeUtils::remove(HInst);
          return;
        }
      }
    }
  }

  assert(false && "Missing SIMD Begin/End directive");
}

void VPOCodeGenHIR::eraseLoopIntrins() {
  eraseLoopIntrinsImpl(true /* Intrinsics before loop */);
  eraseLoopIntrinsImpl(false /* Intrinsics before loop */);
}

// This function replaces scalar math lib calls in the remainder loop with
// the svml version used in the main vector loop in order to maintain
// consistency of precision. See the example below:
//
// Original remainder loop:
//
// <14>  + DO i1 = 128, 130, 1   <DO_LOOP>
// <5>   |   %call = @sinf((%b)[i1]);
// <7>   |   (%a)[i1] = %call;
// <14>  + END LOOP
//
// Transformed remainder loop:
//
// <14>  + DO i1 = 128, 130, 1   <DO_LOOP>
// <15>  |   %load = (%b)[i1];
// <16>  |   %__svml_sinf48 = @__svml_sinf4(%load);
// <17>  |   %call = extractelement %__svml_sinf48,  0;
// <7>   |   (%a)[i1] = %call;
// <14>  + END LOOP
//
// Detailed HIR:
//
// <14>  + DO i64 i1 = 128, 130, 1   <DO_LOOP>
// <15>  |   %load = (%b)[i1];
// <15>  |   <LVAL-REG> NON-LINEAR float %load {sb:15}
// <15>  |   <RVAL-REG> {al:4}(LINEAR float* %b)[LINEAR i64 i1] !tbaa !5 {sb:12}
// <15>  |      <BLOB> LINEAR float* %b {sb:6}
// <15>  |
// <16>  |   %__svml_sinf48 = @__svml_sinf4(%load);
// <16>  |   <LVAL-REG> NON-LINEAR <4 x float> %__svml_sinf48 {sb:16}
// <16>  |   <RVAL-REG> NON-LINEAR bitcast.float.<4 x float>(%load) {sb:15}
// <16>  |      <BLOB> NON-LINEAR float %load {sb:15}
// <16>  |
// <17>  |   %call = extractelement %__svml_sinf48,  0;
// <17>  |   <LVAL-REG> NON-LINEAR float %call {sb:7}
// <17>  |   <RVAL-REG> NON-LINEAR <4 x float> %__svml_sinf48 {sb:16}
// <17>  |
// <7>   |   (%a)[i1] = %call;
// <7>   |   <LVAL-REG> {al:4}(LINEAR float* %a)[LINEAR i64 i1] !tbaa !5 {sb:13}
// <7>   |      <BLOB> LINEAR float* %a {sb:9}
// <7>   |   <RVAL-REG> NON-LINEAR float %call {sb:7}
// <7>   |
// <14>  + END LOOP

void VPOCodeGenHIR::replaceLibCallsInRemainderLoop(HLInst *HInst) {

  // Used to remove the original math calls after iterating over them.
  SmallVector<HLInst *, 1> InstsToRemove;

  const CallInst *Call = cast<CallInst>(HInst->getLLVMInstruction());
  Function *F = Call->getCalledFunction();
  StringRef FnName = F->getName();

  // Check to see if the call was vectorized in the main loop.
  if (TLI->isFunctionVectorizable(FnName, VF)) {
    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type *, 1> ArgTys;

    // For each call argument, insert a scalar load of the element,
    // broadcast it to a vector.
    for (auto It = HInst->rval_op_ddref_begin(),
              ItEnd = HInst->rval_op_ddref_end();
         It != ItEnd; ++It) {
      // TODO: it is assumed that call arguments need to become vector.
      // In the future, some vectorizable calls may contain scalar
      // arguments. Additional checking is needed for these cases.

      // The DDRef of the original scalar call instruction.
      RegDDRef *Ref = *It;

      // The resulting type of the widened ref/broadcast.
      auto VecDestTy = VectorType::get(Ref->getDestType(), VF);

      RegDDRef *WideRef = nullptr;
      HLInst *LoadInst = nullptr;

      // Create the scalar load of the call argument. This is done so that
      // we can clone the new LvalDDRef and change its type to force the
      // broadcast. See %load in the example above. Essentially, the original
      // scalar %load becomes bitcast.float.<4 x float>, which is how HIRCG
      // knows to do the broadcast.
      if (Ref->isMemRef()) {
        // Ref is a memory reference: %t = sinf(a[i]);
        LoadInst = HInst->getHLNodeUtils().createLoad(Ref->clone(), "load");
      } else {
        // Ref in this case is a temp from a previous load: %r = sinf(%t).
        // Create a new temp and broadcast it for the call argument.
        LoadInst = HInst->getHLNodeUtils().createCopyInst(Ref->clone(), "copy");
      }

      // Construct the new RegDDRef for the call argument. Set the dest
      // type to the vector type required to do a broadcast. So, for
      // example, source type is float, and dest type becomes <4 x float>.
      // This causes the RegDDRef to obtain a bitcast. Because of this,
      // the ref is no longer a self blob and we must copy the BlobDDRef
      // from the original reference to this one. This is what the call
      // to makeConsistent() does.
      //
      // e.g., %load is a self blob, bitcast.float.<4 x float>(%load) is
      // no longer a self blob due to the existence of the bitcast. So,
      // copy BlobDDRef from %load to bitcast.float.<4 x float>(%load).
      HLNodeUtils::insertBefore(HInst, LoadInst);
      WideRef = LoadInst->getLvalDDRef()->clone();
      auto CE = WideRef->getSingleCanonExpr();
      CE->setDestType(VecDestTy);
      const SmallVector<const RegDDRef *, 1> AuxRefs = {
          LoadInst->getLvalDDRef()};
      WideRef->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel());

      // Collect call arguments and types so that the function declaration
      // and call instruction can be generated.
      CallArgs.push_back(WideRef);
      ArgTys.push_back(VecDestTy);
    }

    // Using the newly created vector call arguments, generate the vector
    // call instruction and extract the low element.
    Function *VectorF = getOrInsertVectorFunction(
        Call, VF, ArgTys, TLI, Intrinsic::not_intrinsic,
        nullptr /*simd function*/, false /*non-masked*/);
    assert(VectorF && "Can't create vector function.");

    HLInst *WideCall = HInst->getHLNodeUtils().createCall(
        VectorF, CallArgs, VectorF->getName(), nullptr);
    HLNodeUtils::insertBefore(HInst, WideCall);

    // TODO: Matt can you look into the following code review comment
    // from Pankaj?
    // Call instructions can have one fake memref for each AddressOf
    // operand that can be read from/written into. This is necessary to
    // preserve data dependence semantics. I would recommend that vectorizer
    // handle those as well. You can access them using
    // HInst->fake_ddref_begin(). Keep in mind that they may contain
    // 'undef' in the index. This is used to obtain DV of *.
    if (FnName.find("sincos") != StringRef::npos) {
      // Since we're in the remainder loop and scalarizing for now,
      // then set the call argument strides for the sin/cos results
      // to indirect to force scalarization in MapIntrinToIml. Later,
      // when we support remainder loop vectorization, swap out the
      // following loop with the call to analyzeCallArgMemoryReferences().
      Instruction *WideInst =
          const_cast<Instruction *>(WideCall->getLLVMInstruction());
      CallInst *VecCall = cast<CallInst>(WideInst);
      for (unsigned I = 1; I < 3; I++) {
        AttrBuilder AttrList;
        AttrList.addAttribute("stride", "indirect");
        VecCall->setAttributes(VecCall->getAttributes().addAttributes(
            VecCall->getContext(), I + 1, AttrList));
      }
      // analyzeCallArgMemoryReferences(HInst, WideCall, CallArgs);
    }

    InstsToRemove.push_back(HInst);

    if (auto LvalDDRef = HInst->getLvalDDRef()) {
      HLInst *ExtractInst = HInst->getHLNodeUtils().createExtractElementInst(
          WideCall->getLvalDDRef()->clone(), 0, "elem", LvalDDRef->clone());
      HLNodeUtils::insertAfter(WideCall, ExtractInst);
    }
  }

  // Remove the original scalar call(s) to clean up the IR.
  for (unsigned Idx = 0; Idx < InstsToRemove.size(); Idx++) {
    HLInst *Inst = InstsToRemove[Idx];
    HLNodeUtils::remove(Inst);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::replaceCalls() {
  for (unsigned i = 0; i < CallInsts.size(); i++) {
    CG->replaceLibCallsInRemainderLoop(CallInsts[i]);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitInst(HLInst *I) {
  // Check for function calls.
  if (I->isCallInst()) {
    CallInsts.push_back(I);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitIf(HLIf *If) {
  for (auto ThenIt = If->then_begin(), ThenEnd = If->then_end();
       ThenIt != ThenEnd; ++ThenIt) {
    visit(*ThenIt);
  }
  for (auto ElseIt = If->else_begin(), ElseEnd = If->else_end();
       ElseIt != ElseEnd; ++ElseIt) {
    visit(*ElseIt);
  }
}

void VPOCodeGenHIR::HIRLoopVisitor::visitLoop(HLLoop *L) {
  for (auto Iter = L->child_begin(), EndItr = L->child_end(); Iter != EndItr;
       ++Iter) {
    visit(*Iter);
  }
}

bool VPOCodeGenHIR::isReductionRef(const RegDDRef *Ref, unsigned &Opcode) {
  // When widening decomposed nested blobs, we create temp Refs without
  // an associated DDNode.
  if (!Ref->getHLDDNode())
    return false;

  return SRA->isReductionRef(Ref, Opcode);
}

RegDDRef *VPOCodeGenHIR::widenRef(const RegDDRef *Ref) {
  RegDDRef *WideRef;
  int64_t IVConstCoeff;
  auto RefDestTy = Ref->getDestType();
  auto VecRefDestTy = VectorType::get(RefDestTy, VF);
  auto RefSrcTy = Ref->getSrcType();
  auto VecRefSrcTy = VectorType::get(RefSrcTy, VF);

  // If the DDREF has a widened counterpart, return the same after setting
  // SrcType/DestType appropriately.
  if (Ref->isTerminalRef()) {
    unsigned RedOpCode;

    if (WidenMap.find(Ref->getSymbase()) != WidenMap.end()) {
      auto WInst = WidenMap[Ref->getSymbase()];
      WideRef = WInst->getLvalDDRef()->clone();

      auto CE = WideRef->getSingleCanonExpr();
      CE->setDestType(VecRefDestTy);
      CE->setSrcType(VecRefSrcTy);
      CE->setExtType(Ref->getSingleCanonExpr()->isSExt());

      return WideRef;
    }

    // Check if Ref is a reduction - we create widened DDREF for a
    // reduction ref the first time it is encountered and use this to replace
    // all occurrences of Ref. The widened ref is added to the WidenMap
    // here to accomplish this.
    if (isReductionRef(Ref, RedOpCode)) {

      auto Identity = HLInst::getRecurrenceIdentity(RedOpCode, RefDestTy);
      auto RedOpVecInst = insertReductionInitializer(Identity);

      // Add to WidenMap and handle generating code for building reduction tail
      addToMapAndHandleLiveOut(Ref, RedOpVecInst);

      // LVAL ref of the initialization instruction is the widened reduction
      // ref.
      return RedOpVecInst->getLvalDDRef()->clone();
    }

    // Lval terminal refs get the widened ref duing the widened HLInst creation
    // later - simply return NULL.
    if (Ref->getHLDDNode() && Ref->isLval())
      return nullptr;
  }

  WideRef = Ref->clone();

  // Set VectorType on WideRef base pointer - BaseDestType is set to pointer
  // type of VF-wide vector of Ref's DestType. For addressof DDRef, desttype
  // is set to vector of pointers(scalar desttype).
  if (WideRef->hasGEPInfo()) {
    PointerType *PtrType = cast<PointerType>(Ref->getBaseDestType());
    auto AddressSpace = PtrType->getAddressSpace();

    // Omit the range metadata as is done in loop vectorize which does
    // not propagate the same. We get a compile time error otherwise about
    // type mismatch for range values.
    WideRef->setMetadata(LLVMContext::MD_range, nullptr);

    if (WideRef->isAddressOf()) {
      WideRef->setBaseDestType(VecRefDestTy);

      auto StructElemTy =
          dyn_cast<StructType>(PtrType->getPointerElementType());

      // There is nothing more to do for opaque types as they can only occur in
      // this form: &p[0].
      if (StructElemTy && StructElemTy->isOpaque()) {
        return WideRef;
      }
    } else {
      WideRef->setBaseDestType(PointerType::get(VecRefDestTy, AddressSpace));
    }
  }

  unsigned NestingLevel = OrigLoop->getNestingLevel();
  // For unit stride ref, nothing else to do
  if (isConstStrideRef(Ref, NestingLevel, &IVConstCoeff) && IVConstCoeff == 1)
    return WideRef;

  SmallVector<const RegDDRef *, 4> AuxRefs;
  // For cases other than unit stride refs, we need to widen the induction
  // variable and replace blobs in Canon Expr with widened equivalents.
  for (auto I = WideRef->canon_begin(), E = WideRef->canon_end(); I != E; ++I) {
    auto CE = *I;
    bool AnyChange = true;

    if (CE->hasIV(NestingLevel)) {
      SmallVector<Constant *, 4> CA;
      Type *Int64Ty = CE->getSrcType();

      CE->getIVCoeff(NestingLevel, nullptr, &IVConstCoeff);

      for (unsigned i = 0; i < VF; ++i) {
        CA.push_back(ConstantInt::getSigned(Int64Ty, IVConstCoeff * i));
      }
      ArrayRef<Constant *> AR(CA);
      auto CV = ConstantVector::get(AR);

      unsigned Idx = 0;
      CE->getBlobUtils().createBlob(CV, true, &Idx);
      CE->addBlob(Idx, 1);
      AnyChange = true;
    }

    SmallVector<unsigned, 8> BlobIndices;
    CE->collectBlobIndices(BlobIndices, false);

    for (auto &BI : BlobIndices) {
      auto TopBlob = CE->getBlobUtils().getBlob(BI);

      // We do not need to widen invariant blobs - check for blob invariance
      // by comparing maxbloblevel against the loop's nesting level.
      if (WideRef->findMaxBlobLevel(BI) < NestingLevel)
        continue;

      if (CE->getBlobUtils().isNestedBlob(TopBlob)) {
        assert(false && "Nested blob support TBD");
        continue;
      }

      assert(CE->getBlobUtils().isTempBlob(TopBlob) &&
             "Only temp blobs expected");

      auto OldSymbase = CE->getBlobUtils().getTempBlobSymbase(BI);

      if (WidenMap.find(OldSymbase) != WidenMap.end()) {
        auto WInst1 = WidenMap[OldSymbase];
        auto WRef = WInst1->getLvalDDRef();
        AuxRefs.push_back(WRef);
        CE->replaceBlob(BI, WRef->getSingleCanonExpr()->getSingleBlobIndex());
        AnyChange = true;
      }
    }

    if (AnyChange) {
      auto VecCEDestTy = VectorType::get(CE->getDestType(), VF);
      auto VecCESrcTy = VectorType::get(CE->getSrcType(), VF);

      CE->setDestType(VecCEDestTy);
      CE->setSrcType(VecCESrcTy);
    }
  }

  // The blobs in the scalar ref have been replaced by widened refs, call
  // the utility to update the widened Ref consistent.
  WideRef->makeConsistent(&AuxRefs, NestingLevel);
  return WideRef;
}

/// \brief Return result of combining horizontal vector binary operation with
/// initial value. Horizontal binary operation splits VecRef recursively
/// into 2 parts until the VF becomes 2. Then we extract elements from the
/// vector and perform scalar operation, the result of which is then
/// combined with the initial value and assigned to ResultRef. The created
/// instructions are added to the InstContainer initially and are added
/// after Loop at the end after generating the combined result.
static HLInst *buildReductionTail(HLContainerTy &InstContainer,
                                  unsigned BOpcode, const RegDDRef *VecRef,
                                  const RegDDRef *InitValRef, HLLoop *Loop,
                                  const RegDDRef *ResultRef) {

  // Take Vector Length from the WideRedInst type
  Type *VecTy = VecRef->getDestType();

  // For Sub/FSub operation, we need to use Add/FAdd for the horizontal
  // vector and combine operations.
  if (BOpcode == Instruction::Sub)
    BOpcode = Instruction::Add;
  else if (BOpcode == Instruction::FSub)
    BOpcode = Instruction::FAdd;

  unsigned VF = cast<VectorType>(VecTy)->getNumElements();
  if (VF == 2) {
    HLInst *Lo = Loop->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), 0, "Lo");
    HLInst *Hi = Loop->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), 1, "Hi");

    HLInst *Combine = Loop->getHLNodeUtils().createBinaryHLInst(
        BOpcode, Lo->getLvalDDRef()->clone(), Hi->getLvalDDRef()->clone(),
        "reduced");
    InstContainer.push_back(*Lo);
    InstContainer.push_back(*Hi);
    InstContainer.push_back(*Combine);

    RegDDRef *ScalarValue = Combine->getLvalDDRef();

    // Combine with initial value
    auto FinalInst = Loop->getHLNodeUtils().createBinaryHLInst(
        BOpcode, ScalarValue->clone(), InitValRef->clone(), "final" /* Name */,
        ResultRef->clone());
    InstContainer.push_back(*FinalInst);
    return FinalInst;
  }
  SmallVector<uint32_t, 16> LoMask, HiMask;
  for (unsigned i = 0; i < VF / 2; ++i)
    LoMask.push_back(i);
  for (unsigned i = VF / 2; i < VF; ++i)
    HiMask.push_back(i);
  HLInst *Lo = Loop->getHLNodeUtils().createShuffleVectorInst(
      VecRef->clone(), VecRef->clone(), LoMask, "Lo");
  HLInst *Hi = Loop->getHLNodeUtils().createShuffleVectorInst(
      VecRef->clone(), VecRef->clone(), HiMask, "Hi");
  HLInst *Result = Loop->getHLNodeUtils().createBinaryHLInst(
      BOpcode, Lo->getLvalDDRef()->clone(), Hi->getLvalDDRef()->clone(),
      "reduce");
  InstContainer.push_back(*Lo);
  InstContainer.push_back(*Hi);
  InstContainer.push_back(*Result);
  return buildReductionTail(InstContainer, BOpcode, Result->getLvalDDRef(),
                            InitValRef, Loop, ResultRef);
}

void VPOCodeGenHIR::analyzeCallArgMemoryReferences(
    const HLInst *OrigCall, HLInst *WideCall,
    SmallVectorImpl<RegDDRef *> &Args) {

  Instruction *Inst = const_cast<Instruction *>(WideCall->getLLVMInstruction());

  CallInst *VecCall = cast<CallInst>(Inst);

  HLLoop *L = cast<HLLoop>(OrigCall->getParentLoop());
  unsigned LoopLevel = L->getNestingLevel();

  // Analyze memory references for the arguments used to store sin/cos
  // results. This information will later be used to generate appropriate
  // store instructions.

  for (unsigned I = 0; I < Args.size(); I++) {

    // Only consider call arguments that involve address computations.
    // For example, this is limited at the moment to call arguments like:
    // sincos(..., &a[i], &b[i], ...). In order to extend to other memory
    // references, the type derivations below will need to change. Some
    // assumptions are made for addressOf references.
    if (Args[I]->isAddressOf()) {
      AttrBuilder AttrList;
      int64_t ByteStride;
      CanonExpr *CE = Args[I]->getStrideAtLevel(LoopLevel);

      // TODO - Matt, please look into using
      //    Args[I]->getConstStrideAtLevel(LoopLevel, &ByteStride)
      // to avoid creation of a canon expression.
      if (CE->isLinearAtLevel() && CE->isIntConstant(&ByteStride)) {
        // Type of the argument will be something like <4 x double*>
        // The following code will yield a type of double. This type is used
        // to determine the stride in elements.
        Type *ArgTy = Args[I]->getDestType();
        PointerType *PtrTy = cast<PointerType>(ArgTy);
        VectorType *VecTy = cast<VectorType>(PtrTy->getElementType());
        PointerType *ElemPtrTy = cast<PointerType>(VecTy->getElementType());
        Type *ElemTy = ElemPtrTy->getElementType();
        unsigned ElemSize = ElemTy->getPrimitiveSizeInBits() / 8;
        unsigned ElemStride = ByteStride / ElemSize;
        AttrList.addAttribute("stride",
                              APInt(32, ElemStride).toString(10, false));
      } else {
        AttrList.addAttribute("stride", "indirect");
      }

      if (AttrList.hasAttributes()) {
        VecCall->setAttributes(VecCall->getAttributes().addAttributes(
            VecCall->getContext(), I + 1, AttrList));
      }
    }
  }
}

HLInst *VPOCodeGenHIR::widenIfPred(const HLIf *HIf, RegDDRef *Mask) {
  RegDDRef *WOp0, *WOp1;

  if (!Mask)
    Mask = CurMaskValue;

  // TODO - supports only single if predicate for now
  auto FirstPred = HIf->pred_begin();
  auto Op0 = HIf->getOperandDDRef(0);
  auto Op1 = HIf->getOperandDDRef(1);
  WOp0 = widenRef(Op0);
  WOp1 = widenRef(Op1);

  auto WideInst =
      HIf->getHLNodeUtils().createCmp(*FirstPred, WOp0, WOp1, "wide.cmp.");
  addInst(WideInst, Mask);
  return WideInst;
}

HLInst *VPOCodeGenHIR::widenNode(const HLInst *INode, RegDDRef *Mask) {
  const HLInst *Node = INode;
  auto CurInst = INode->getLLVMInstruction();
  SmallVector<RegDDRef *, 6> WideOps;

  if (!Mask)
    Mask = CurMaskValue;

  HLInst *WideInst = nullptr;

  DEBUG(INode->dump(true));
  bool InsertInMap = true;

  // Widen instruction operands
  for (auto Iter = INode->op_ddref_begin(), End = INode->op_ddref_end();
       Iter != End; ++Iter) {
    RegDDRef *WideRef, *Ref;

    Ref = *Iter;

    WideRef = widenRef(Ref);
    WideOps.push_back(WideRef);
  }

  // Generate the widened instruction using widened operands
  if (auto BOp = dyn_cast<BinaryOperator>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createBinaryHLInst(
        BOp->getOpcode(), WideOps[1], WideOps[2], CurInst->getName() + ".vec",
        WideOps[0], BOp);
  } else if (isa<LoadInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createLoad(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<StoreInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createStore(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
    InsertInMap = false;
  } else if (isa<CastInst>(CurInst)) {
    assert(WideOps.size() == 2 && "invalid cast");

    WideInst = Node->getHLNodeUtils().createCastHLInst(
        VectorType::get(CurInst->getType(), VF), CurInst->getOpcode(),
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<SelectInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createSelect(
        INode->getPredicate(), WideOps[1], WideOps[2], WideOps[3], WideOps[4],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<CmpInst>(CurInst)) {
    WideInst = Node->getHLNodeUtils().createCmp(
        INode->getPredicate(), WideOps[1], WideOps[2],
        CurInst->getName() + ".vec", WideOps[0]);
  } else if (isa<GetElementPtrInst>(CurInst)) {
    // Gep Instructions in LLVM may have any number of operands but the HIR
    // representation for them is always a single rhs ddref - copy rval to
    // lval.
    WideInst = Node->getHLNodeUtils().createCopyInst(
        WideOps[1], CurInst->getName() + ".vec", WideOps[0]);
  } else if (const CallInst *Call = dyn_cast<CallInst>(CurInst)) {

    Function *Fn = Call->getCalledFunction();
    StringRef FnName = Fn->getName();

    // Default to svml. If svml is not available, try the intrinsic.
    Intrinsic::ID ID = Intrinsic::not_intrinsic;
    if (!TLI->isFunctionVectorizable(FnName, VF)) {
      ID = getVectorIntrinsicIDForCall(Call, TLI);
      if (ID && (ID == Intrinsic::assume || ID == Intrinsic::lifetime_end ||
                 ID == Intrinsic::lifetime_start)) {
        return const_cast<HLInst *>(INode);
      }
    }

    unsigned ArgOffset = 0;
    if (!Fn->getReturnType()->isVoidTy()) {
      ArgOffset = 1;
    }
    SmallVector<RegDDRef *, 1> CallArgs;
    SmallVector<Type *, 1> ArgTys;
    for (unsigned i = ArgOffset; i < WideOps.size(); i++) {
      CallArgs.push_back(WideOps[i]);
      ArgTys.push_back(WideOps[i]->getDestType());
    }

    bool Masked = false;
    if (Mask) {
      auto CE = Mask->getSingleCanonExpr();
      ArgTys.push_back(CE->getDestType());
      CallArgs.push_back(Mask->clone());
      Masked = true;
    }

    Function *VectorF =
        getOrInsertVectorFunction(Call, VF, ArgTys, TLI, ID, nullptr, Masked);
    assert(VectorF && "Can't create vector function.");

    WideInst = Node->getHLNodeUtils().createCall(
        VectorF, CallArgs, VectorF->getName(), WideOps[0]);
    Instruction *Inst =
        const_cast<Instruction *>(WideInst->getLLVMInstruction());

    if (isa<FPMathOperator>(Inst)) {
      Inst->copyFastMathFlags(Call);
    }

    if (FnName.find("sincos") != StringRef::npos) {
      analyzeCallArgMemoryReferences(INode, WideInst, CallArgs);
    }

    if (ArgOffset) {
      // If this is a void function, there will be no LVal DDRef for it, so
      // don't try to insert it in the map. i.e., there are no users of an
      // LVal for a void function.
      InsertInMap = true;
    } else {
      InsertInMap = false;
    }
  } else {
    llvm_unreachable("Unimplemented widening for inst");
  }

  // Add to WidenMap and handle generating code for any liveouts
  if (InsertInMap) {
    addToMapAndHandleLiveOut(INode->getLvalDDRef(), WideInst);
    if (WideInst->getLvalDDRef()->isTerminalRef())
      WideInst->getLvalDDRef()->makeSelfBlob();
  }

  addInst(WideInst, Mask);
  return WideInst;
}

HLInst *VPOCodeGenHIR::insertReductionInitializer(Constant *Iden) {
  auto IdentityVec = getConstantSplatDDRef(MainLoop->getDDRefUtils(), Iden, VF);
  HLInst *RedOpVecInst =
      MainLoop->getHLNodeUtils().createCopyInst(IdentityVec, "RedOp");
  HLNodeUtils::insertBefore(MainLoop, RedOpVecInst);

  auto LvalSymbase = RedOpVecInst->getLvalDDRef()->getSymbase();
  MainLoop->addLiveInTemp(LvalSymbase);
  return RedOpVecInst;
}

void VPOCodeGenHIR::addToMapAndHandleLiveOut(const RegDDRef *ScalRef,
                                             HLInst *WideInst) {
  auto ScalSymbase = ScalRef->getSymbase();

  // If already in WidenMap, nothing further to do
  if (WidenMap.count(ScalSymbase))
    return;

  // Insert in WidenMap
  WidenMap[ScalSymbase] = WideInst;

  // Generate any necessary code to handle loop liveout/reduction
  if (!MainLoop->isLiveOut(ScalSymbase))
    return;

  auto VecRef = WideInst->getLvalDDRef();

  MainLoop->addLiveOutTemp(VecRef->getSymbase());

  unsigned OpCode;

  if (isReductionRef(ScalRef, OpCode)) {
    HLContainerTy Tail;

    buildReductionTail(Tail, OpCode, VecRef, ScalRef, MainLoop, ScalRef);
    HLNodeUtils::insertAfter(MainLoop, &Tail);
  } else {
    auto Extr = WideInst->getHLNodeUtils().createExtractElementInst(
        VecRef->clone(), VF - 1, "Last", ScalRef->clone());
    auto Lval = Extr->getLvalDDRef();

    // Convert to selfblob if Lval has IV at Loop level since last value
    // extract instruction is added after the Loop.
    if (Lval->getSingleCanonExpr()->hasIV(MainLoop->getNestingLevel()))
      Lval->makeSelfBlob();

    HLNodeUtils::insertAfter(MainLoop, Extr);
  }
}
} // end namespace llvm
