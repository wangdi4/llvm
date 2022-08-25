//===- IntelVPlan.cpp - Vectorizer Plan -----------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the LLVM vectorization plan. It represents a candidate for
// vectorization, allowing to plan and optimize how to vectorize a given loop
// before generating LLVM-IR.
// The vectorizer uses vectorization plans to estimate the costs of potential
// candidates and if profitable to execute the desired plan, generating vector
// LLVM-IR code.
//
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
#include "IntelVPlan.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPOCodeGen.h"
#include "IntelVPSOAAnalysis.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanClone.h"
#include "IntelVPlanDivergenceAnalysis.h"
#include "IntelVPlanDominatorTree.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanScalarEvolution.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPOCodeGenHIR.h"
#else
#include "VPlan.h"
#endif // INTEL_CUSTOMIZATION

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#if INTEL_CUSTOMIZATION
#define DEBUG_TYPE "intel-vplan"
#else
#define DEBUG_TYPE "vplan"
#endif

using namespace llvm;
using namespace llvm::vpo;

std::atomic<unsigned> VPlanUtils::NextOrdinal{1};
#if INTEL_CUSTOMIZATION
// Replace dot print output with plain print.
static cl::opt<bool>
    DumpPlainVPlanIR("vplan-plain-dump", cl::init(false), cl::Hidden,
                       cl::desc("Print plain VPlan IR"));

static cl::opt<bool>
    EnableNames("vplan-enable-names", cl::init(false), cl::Hidden,
                cl::desc("Print VP Operands using VPValue's Name member."));

static cl::opt<bool> DumpExternalDefsHIR("vplan-dump-external-defs-hir",
                                         cl::init(true), cl::Hidden,
                                         cl::desc("Print HIR VPExternalDefs."));

static cl::opt<bool>
    VPlanDumpDetails("vplan-dump-details", cl::init(false), cl::Hidden,
                     cl::desc("Print VPlan instructions' details like "
                              "underlying attributes/metadata."));

static cl::opt<bool> VPlanDumpDebugLoc(
    "vplan-dump-debug-loc", cl::init(false), cl::Hidden,
    cl::desc("Print VPlan instructions' debug location information."));

static cl::opt<bool> VPlanDumpInductionInitDetails(
  "vplan-dump-induction-init-details", cl::init(false), cl::Hidden,
  cl::desc("Print induction value range information."));

static cl::opt<bool> UseGetType(
  "vplan-cost-model-use-gettype", cl::init(true), cl::Hidden,
  cl::desc("Use getType() instead of getCMType() if true. "
           "The knob is temporal and should be removed once every "
           "getCMType() is replaced with getType()."));

static cl::opt<bool> VPlanDumpSubscriptDetails(
    "vplan-dump-subscript-details", cl::init(false), cl::Hidden,
    cl::desc("Print details for subscript instructions like lower, stride and "
             "struct offsets."));

static cl::opt<bool>
    EnableScalVecAnalysis("vplan-enable-scalvec-analysis", cl::init(true),
                          cl::Hidden,
                          cl::desc("Enable analysis to compute scalar/vector "
                                   "nature of instructions in VPlan."));
static cl::opt<bool> DumpVPlanLiveInsLiveOuts(
    "vplan-dump-live-inout", cl::init(false), cl::Hidden,
    cl::desc("Print live-ins and live-outs of main loop"));

static cl::opt<bool>
    VPGEPPrintSrcElemType("vpgep-print-src-elem-type", cl::init(false),
                          cl::Hidden,
                          cl::desc("Print VPGEPInstruction's SourceElementType "
                                   "even for non-opaque pointers."));

static cl::opt<bool>
    VPlanDumpDAShapes("vplan-dump-da-shapes", cl::init(false), cl::Hidden,
                      cl::desc("Print VPlan instructions' DA shape "
                               "instead of simple Uni/Div."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &llvm::vpo::operator<<(raw_ostream &OS, const VPValue &V) {
  V.print(OS);
  return OS;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif

// When a VPBasicBlock is added in VPlan, the parent pointer needs to be
// updated.
void ilist_traits<VPBasicBlock>::addNodeToList(VPBasicBlock *VPBB) {
  assert(!VPBB->getParent() && "VPBasicBlock already in VPlan");
  VPBB->setParent(getListOwner<VPlan, VPBasicBlock>(this));
}

// When we remove a VPBasicBlock from VPlan, we need to update its parent
// pointer.
void ilist_traits<VPBasicBlock>::removeNodeFromList(VPBasicBlock *VPBB) {
  assert(VPBB->getParent() && "VPBasicBlock not in VPlan");
  VPBB->setParent(nullptr);
}

void ilist_traits<VPBasicBlock>::deleteNode(VPBasicBlock *VPBB) {
  assert(!VPBB->getParent() && "VPBasicBlock is still in a VPlan!");
  delete VPBB;
}

void ilist_traits<VPBasicBlock>::transferNodesFromList(ilist_traits &FromList,
                                                       instr_iterator First,
                                                       instr_iterator Last) {
  // If it's within the same list, there's nothing to do.
  if (this == &FromList)
    return;

  VPlan *CurP = getListOwner<VPlan, VPBasicBlock>(this);
  VPlan *FromP = getListOwner<VPlan, VPBasicBlock>(&FromList);
  assert(CurP != FromP && "Two lists have the same parent?");
  (void)CurP;
  (void)FromP;

  // If splicing between two VPlans then update the parent pointers.
  for (; First != Last; ++First)
    First->setParent(getListOwner<VPlan, VPBasicBlock>(this));
}

void VPInstruction::moveBefore(VPInstruction *MovePos) {
  moveBefore(*MovePos->getParent(), MovePos->getIterator());
}

void VPInstruction::moveAfter(VPInstruction *MovePos) {
  moveBefore(*MovePos->getParent(), ++MovePos->getIterator());
}

void VPInstruction::moveBefore(VPBasicBlock &BB, VPBasicBlock::iterator I) {
  assert((I == BB.end() || I->getParent() == &BB) &&
         "Iterator is out of basic block");
  BB.getInstructions().splice(I, getParent()->getInstructions(), getIterator());
}

void VPInstruction::generateInstruction(VPTransformState &State,
                                        unsigned Part) {
#if INTEL_CUSTOMIZATION
  State.ILV->processInstruction(this);
  return;
#endif
  IRBuilder<> &Builder = State.Builder;

  switch (getOpcode()) {
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::SRem:
  case Instruction::URem:
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {
    Value *A = State.get(getOperand(0), Part);
    Value *B = State.get(getOperand(1), Part);
    Value *V = Builder.CreateBinOp((Instruction::BinaryOps)getOpcode(), A, B);
    State.set(this, V, Part);
    break;
  }
  case VPInstruction::Not: {
    Value *A = State.get(getOperand(0), Part);
    Value *V = Builder.CreateNot(A);
    State.set(this, V, Part);
    break;
  }
  default:
    llvm_unreachable("Unsupported opcode for instruction");
  }
}

#if INTEL_CUSTOMIZATION
void VPInstruction::executeHIR(VPOCodeGenHIR *CG) {
  // TODO: For the reuse/invalidation of the underlying HIR to be working
  // properly, we need to do an independent traversal of all the VPInstructions
  // in the CFG and invalidate their HIR when:
  //  1) there are nested blobs that need to be widened,
  //  2) the decomposed VPInstructions don't precede their master VPInstruction
  //     (rule to be refined), and
  //  3) the decomposed VPInstructions have more than one use.
  CG->widenNode(this, nullptr);
  // Propagate debug location for the generated HIR construct.
  CG->propagateDebugLocation(this);
}

Type *VPInstruction::getCMType() const {
  if (UseGetType)
    return getType();

  if (getUnderlyingValue())
    return getUnderlyingValue()->getType();

  if (!HIR().isMaster())
    return nullptr;

  const loopopt::HLNode *Node = HIR().getUnderlyingNode();
  const loopopt::HLInst *Inst = dyn_cast_or_null<loopopt::HLInst>(Node);

  if (!Inst)
    return nullptr;

  const Instruction *LLVMInst = Inst->getLLVMInstruction();
  if (!LLVMInst)
    return nullptr;

  return LLVMInst->getType();
}
#endif // INTEL_CUSTOMIZATION

void VPInstruction::execute(VPTransformState &State) {
  assert(!State.Instance && "VPInstruction executing an Instance");
  for (unsigned Part = 0; Part < State.UF; ++Part)
    generateInstruction(State, Part);
}

bool VPInstruction::mayHaveSideEffects() const {
  if (auto *Instr = getInstruction()) {
    if (Instr->mayHaveSideEffects())
      return true;

    // mayHaveSideEffects does not consider malloc/alloca to have side effects.
    // Do more checks.
    if (isa<AllocaInst>(Instr))
      return true;

    // FIXME: malloc-like routines.

    return false;
  }

  // TODO: Probably should be unified with llvm::Instruction's opcode
  // handling. Harder to do without INTEL_CUSTOMIZATION before VPlan
  // upstreaming.
  unsigned Opcode = getOpcode();
  if (Instruction::isCast(Opcode) || Instruction::isShift(Opcode) ||
      Instruction::isBitwiseLogicOp(Opcode) ||
      Instruction::isBinaryOp(Opcode) || Instruction::isUnaryOp(Opcode) ||
      Opcode == Instruction::ExtractElement ||
      Opcode == Instruction::InsertElement ||
      Opcode == Instruction::ShuffleVector || Opcode == Instruction::Select ||
      Opcode == Instruction::GetElementPtr ||
      Opcode == VPInstruction::Subscript || Opcode == Instruction::PHI ||
      Opcode == Instruction::ICmp || Opcode == Instruction::FCmp ||
      Opcode == VPInstruction::Not || Opcode == VPInstruction::Abs ||
      Opcode == VPInstruction::AllZeroCheck ||
      Opcode == VPInstruction::InductionInit || Opcode == Instruction::Br ||
      Opcode == VPInstruction::HIRCopy || Opcode == VPInstruction::ActiveLane ||
      Opcode == VPInstruction::ActiveLaneExtract ||
      Opcode == VPInstruction::ConstStepVector)
    return false;

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *VPInstruction::getOpcodeName(unsigned Opcode) {
  switch (Opcode) {
  case VPInstruction::Not:
    return "not";
#if INTEL_CUSTOMIZATION
  case VPInstruction::Abs:
    return "abs";
  case VPInstruction::AllZeroCheck:
    return "all-zero-check";
  case VPInstruction::Pred:
    return "block-predicate";
  case VPInstruction::SMax:
    return "smax";
  case VPInstruction::UMax:
    return "umax";
  case VPInstruction::SMin:
    return "smin";
  case VPInstruction::UMin:
    return "umin";
  case VPInstruction::UMinSeq:
    return "umin-seq";
  case VPInstruction::FMax:
    return "fmax";
  case VPInstruction::FMin:
    return "fmin";
  case VPInstruction::InductionInit:
    return "induction-init";
  case VPInstruction::InductionInitStep:
    return "induction-init-step";
  case VPInstruction::InductionFinal:
    return "induction-final";
  case VPInstruction::ReductionInit:
    return "reduction-init";
  case VPInstruction::ReductionFinal:
    return "reduction-final";
  case VPInstruction::ReductionFinalUdr:
    return "reduction-final-udr";
  case VPInstruction::ReductionFinalInscan:
    return "reduction-final-inscan";
  case VPInstruction::AllocatePrivate:
    return "allocate-priv";
  case VPInstruction::Subscript:
    return "subscript";
  case VPInstruction::Blend:
    return "blend";
  case VPInstruction::HIRCopy:
    return "hir-copy";
  case VPInstruction::OrigTripCountCalculation:
    return "orig-trip-count";
  case VPInstruction::VectorTripCountCalculation:
    return "vector-trip-count";
  case VPInstruction::ActiveLane:
    return "active-lane";
  case VPInstruction::ActiveLaneExtract:
    return "lane-extract";
  case VPInstruction::PeelOrigLiveOut:
  case VPInstruction::RemOrigLiveOut:
    return "orig-live-out";
  case VPInstruction::PeelOrigLiveOutHIR:
  case VPInstruction::RemOrigLiveOutHIR:
    return "orig-live-out-hir";
  case VPInstruction::PushVF:
    return "pushvf";
  case VPInstruction::PopVF:
    return "popvf";
  case VPInstruction::ScalarPeel:
    return "scalar-peel";
  case VPInstruction::ScalarPeelHIR:
    return "scalar-peel-hir";
  case VPInstruction::ScalarRemainder:
    return "scalar-remainder";
  case VPInstruction::ScalarRemainderHIR:
    return "scalar-remainder-hir";
  case VPInstruction::PlanAdapter:
    return "vplan-adapter";
  case VPInstruction::PlanPeelAdapter:
    return "vplan-peel-adapter";
  case VPInstruction::PrivateFinalUncond:
    return "private-final-uc";
  case VPInstruction::PrivateFinalUncondMem:
    return "private-final-uc-mem";
  case VPInstruction::PrivateFinalMasked:
    return "private-final-masked";
  case VPInstruction::PrivateFinalMaskedMem:
    return "private-final-masked-mem";
  case VPInstruction::VLSLoad:
    return "vls-load";
  case VPInstruction::VLSStore:
    return "vls-store";
  case VPInstruction::VLSExtract:
    return "vls-extract";
  case VPInstruction::VLSInsert:
    return "vls-insert";
  case VPInstruction::InvSCEVWrapper:
    return "inv-scev-wrapper";
  case VPInstruction::PrivateFinalCond:
    return "private-final-c";
  case VPInstruction::PrivateFinalCondMem:
    return "private-final-c-mem";
  case VPInstruction::PrivateFinalArray:
    return "private-final-array";
  case VPInstruction::PrivateFinalArrayMasked:
    return "private-final-array-masked";
  case VPInstruction::GeneralMemOptConflict:
    return "vp-general-mem-opt-conflict";
  case VPInstruction::TreeConflict:
    return "tree-conflict";
  case VPInstruction::Permute:
    return "permute";
  case VPInstruction::ConflictInsn:
    return "vpconflict-insn";
  case VPInstruction::PrivateLastValueNonPOD:
    return "private-last-value-nonpod";
  case VPInstruction::PrivateLastValueNonPODMasked:
    return "private-last-value-nonpod-masked";
  case VPInstruction::CvtMaskToInt:
    return "convert-mask-to-int";
  case VPInstruction::ExpandLoad:
    return "expand-load";
  case VPInstruction::ExpandLoadNonu:
    return "expand-load-nonu";
  case VPInstruction::CompressStore:
    return "compress-store";
  case VPInstruction::CompressStoreNonu:
    return "compress-store-nonu";
  case VPInstruction::CompressExpandIndexInit:
    return "compress-expand-index-init";
  case VPInstruction::CompressExpandIndexFinal:
    return "compress-expand-index-final";
  case VPInstruction::CompressExpandIndex:
    return "compress-expand-index";
  case VPInstruction::CompressExpandIndexInc:
    return "compress-expand-index-inc";
  case VPInstruction::RunningInclusiveReduction:
    return "running-inclusive-reduction";
  case VPInstruction::RunningExclusiveReduction:
    return "running-exclusive-reduction";
  case VPInstruction::ExtractLastVectorLane:
    return "extract-last-vector-lane";
  case VPInstruction::TransformLibraryCall:
    return "transform-lib-call";
  case VPInstruction::SOAExtractValue:
    return "soa-extract-value";
#endif
  default:
    return Instruction::getOpcodeName(Opcode);
  }
}

void VPInstruction::print(raw_ostream &O) const {
  const VPBasicBlock *VPBB = getParent();
  if (!VPBB) {
    printWithoutAnalyses(O);
    return;
  }
  const VPlan *Plan = VPBB->getParent();
  if (!Plan) {
    printWithoutAnalyses(O);
    return;
  }
  const VPlanDivergenceAnalysisBase *DA = Plan->getVPlanDA();
  VPlanScalVecAnalysisBase *SVA = nullptr;
  if (auto *VecVPlan = dyn_cast<VPlanVector>(Plan))
    SVA = VecVPlan->getVPlanSVA();

  if (DA || SVA)
    O << "[";
  // Print DA information.
  if (DA) {
    O << "DA: ";
    if (VPlanDumpDAShapes)
      DA->getVectorShape(*this).print(O);
    else if (DA->isDivergent(*this))
      O << "Div";
    else
      O << "Uni";
  }
  // Print SVA information
  if (SVA) {
    // Demarker if DA info was already printed.
    if (DA)
      O << ", ";

    O << "SVA: ";
    SVA->printSVAKindForInst(O, this);
  }
  if (DA || SVA)
    O << "] ";

  printWithoutAnalyses(O);

  // Print list of operand SVA bits.
  if (SVA) {
    O << " (SVAOpBits ";
    for (unsigned OpIdx = 0; OpIdx < getNumOperands(); ++OpIdx) {
      O << OpIdx << "->";
      SVA->printSVAKindForOperand(O, this, OpIdx);
      O << " ";
    }
    O << ")";
  }

  if (VPlanDumpDetails || VPlanDumpDebugLoc) {
    O << "\n";
    // TODO: How to get Indent here?
    O << "    DbgLoc: ";
    getDebugLocation().print(O);
    O << "\n";
  }
  if (VPlanDumpDetails) {
    O << "    OperatorFlags -\n";
    O << "      FMF: " << hasFastMathFlags() << ", NSW: " << hasNoSignedWrap()
      << ", NUW: " << hasNoUnsignedWrap() << ", Exact: " << isExact() << "\n";
    if (auto *LSI = dyn_cast<VPLoadStoreInst>(this))
      LSI->printDetails(O);
    if (auto *Br = dyn_cast<VPBranchInst>(this)) {
      if (auto *LpID = Br->getLoopIDMetadata()) {
        O << "    LoopID: ";
        LpID->print(O);
        O << "\n";
      }
    }
    // Print other attributes here when imported.
    O << "    end of details\n";
  }
}

void VPInstruction::printWithoutAnalyses(raw_ostream &O) const {
  if (!getType()->isVoidTy()) {
    printAsOperand(O);
    O << " = ";
  }

  auto PrintOpcodeWithInBounds = [&](auto MemAddrInst) {
    O << getOpcodeName(getOpcode());
    if (MemAddrInst->isInBounds())
      O << " inbounds";
  };

  switch (getOpcode()) {
  case Instruction::Br:
    cast<VPBranchInst>(this)->printImpl(O);
    break;
  case VPInstruction::Blend: {
    cast<VPBlendInst>(this)->printImpl(O);
    break;
  }
  case Instruction::Call:
  case VPInstruction::TransformLibraryCall: {
    cast<VPCallInstruction>(this)->printImpl(O);
    break;
  }
  case VPInstruction::ConstStepVector: {
    cast<VPConstStepVector>(this)->printImpl(O);
    break;
  }
  case Instruction::GetElementPtr: {
    auto *GEP = cast<const VPGEPInstruction>(this);
    PrintOpcodeWithInBounds(GEP);
    if (GEP->isOpaque() || VPGEPPrintSrcElemType) {
      // We only print it for opaque so that we won't have to update all the
      // tests twice - right now and when all the GEPs will have to be
      // transitioned to be opaque.
      O << " ";
      GEP->getSourceElementType()->print(O, false /*IsForDebug*/,
                                         true /*NoDetails*/);
      O << ",";
    }
    break;
  }
  case VPInstruction::Subscript:
    PrintOpcodeWithInBounds(cast<const VPSubscriptInst>(this));
    break;
  case VPInstruction::ReductionInit: {
    O << getOpcodeName(getOpcode());
    if (cast<const VPReductionInit>(this)->isScalar())
      O << "-scalar";
    break;
  }
  case VPInstruction::InductionInit: {
    O << getOpcodeName(getOpcode()) << "{"
      << getOpcodeName(cast<const VPInductionInit>(this)->getBinOpcode());
    if (VPlanDumpInductionInitDetails)
      cast<const VPInductionInit>(this)->printDetails(O);
    O << "}";
    break;
  }
  case VPInstruction::InductionInitStep:
    O << getOpcodeName(getOpcode()) << "{"
      << getOpcodeName(cast<const VPInductionInitStep>(this)->getBinOpcode())
      << "}";
    break;
  case VPInstruction::InductionFinal: {
    O << getOpcodeName(getOpcode());
    const VPInductionFinal *Ind = cast<const VPInductionFinal>(this);
    if (Ind->getBinOpcode() != Instruction::BinaryOpsEnd)
      O << "{" << getOpcodeName(Ind->getBinOpcode()) << "}";
    break;
  }
  case VPInstruction::ReductionFinal: {
    O << getOpcodeName(getOpcode()) << "{";
    Type *Ty = getType();
    if (Ty->isIntegerTy()) {
      O << (cast<const VPReductionFinal>(this)->isSigned() ? "s_" : "u_");
    }
    O << getOpcodeName(cast<const VPReductionFinal>(this)->getBinOpcode())
      << "}";
    break;
  }
  case Instruction::ICmp:
  case Instruction::FCmp: {
    O << getOpcodeName(getOpcode()) << ' '
      << CmpInst::getPredicateName(cast<VPCmpInst>(this)->getPredicate());
    break;
  }
  case VPInstruction::PrivateLastValueNonPOD: {
    O << getOpcodeName(getOpcode()) << ' ';
    auto *CopyAssignFn =
        cast<VPPrivateLastValueNonPODInst>(this)->getCopyAssign();
    O << "CopyAssign: " << CopyAssignFn->getName();
    break;
  }
  default:
    O << getOpcodeName(getOpcode());
  }

  if (auto *ScalarLp = dyn_cast<VPScalarPeel>(this)) {
    ScalarLp->printImpl(O);
    return;
  }

  if (auto *ScalarLp = dyn_cast<VPScalarRemainder>(this)) {
    ScalarLp->printImpl(O);
    return;
  }

  if (auto *ScalarLpHIR = dyn_cast<VPScalarPeelHIR>(this)) {
    ScalarLpHIR->printImpl(O);
    return;
  }

  if (auto *ScalarLpHIR = dyn_cast<VPScalarRemainderHIR>(this)) {
    ScalarLpHIR->printImpl(O);
    return;
  }

  if (auto *SCEVWrapper = dyn_cast<VPInvSCEVWrapper>(this)) {
    SCEVWrapper->printImpl(O);
    return;
  }

  if (auto *LiveOut = dyn_cast<VPPeelOrigLiveOut>(this)) {
    LiveOut->printImpl(O);
    return;
  }

  if (auto *LiveOut = dyn_cast<VPRemainderOrigLiveOut>(this)) {
    LiveOut->printImpl(O);
    return;
  }

  if (auto *LiveOut = dyn_cast<VPPeelOrigLiveOutHIR>(this)) {
    LiveOut->printImpl(O);
    return;
  }

  if (auto *LiveOut = dyn_cast<VPRemainderOrigLiveOutHIR>(this)) {
    LiveOut->printImpl(O);
    return;
  }

  if (auto *PushVF = dyn_cast<VPPushVF>(this)) {
    PushVF->printImpl(O);
    return;
  }

  if (getOpcode() == VPInstruction::OrigTripCountCalculation) {
    auto *Self = cast<VPOrigTripCountCalculation>(this);
    O << " for original loop " << Self->getOrigLoop()->getName();
  }

  if (auto *Adapter = dyn_cast<VPlanAdapter>(this))
    Adapter->printImpl(O);

  // TODO: print type when this information will be available.
  // So far don't print anything, because PHI may not have Instruction
  if (auto *Phi = dyn_cast<const VPPHINode>(this)) {
    if (Phi->getMergeId() != VPExternalUse::UndefMergeId)
      O << "-merge";
    auto PrintValueWithBB = [&](const unsigned i) {
      O << " ";
      O << " [ ";
      Phi->getIncomingValue(i)->printAsOperand(O);
      O << ", ";
      O << Phi->getIncomingBlock(i)->getName();
      O << " ]";
    };
    const unsigned size = Phi->getNumIncomingValues();
    for (unsigned i = 0; i < size; ++i) {
      if (i > 0)
        O << ",";
      PrintValueWithBB(i);
    }
  } else if (auto *Subscript = dyn_cast<VPSubscriptInst>(this)) {
    O << " ";
    Subscript->getPointerOperand()->printAsOperand(O);
    auto PrintStructOffsets = [&](ArrayRef<unsigned> Offsets) {
      if (!Offsets.empty()) {
        O << " (";
        for (unsigned I : Offsets)
          O << I << " ";
        O << ")";
      }
    };
    // Dump operands per dimension (with details if needed).
    for (int Dim = Subscript->getNumDimensions() - 1; Dim >= 0; --Dim) {
      auto DimInfo = Subscript->dim(Dim);
      auto *Idx = DimInfo.Index;
      ArrayRef<unsigned> DimStructOffsets = DimInfo.StructOffsets;
      if (!VPlanDumpSubscriptDetails) {
        O << " ";
        Idx->printAsOperand(O);
        PrintStructOffsets(DimStructOffsets);
      } else {
        O << " {";
        DimInfo.LowerBound->printAsOperand(O);
        O << " : ";
        Idx->printAsOperand(O);
        O << " : ";
        DimInfo.StrideInBytes->printAsOperand(O);
        O << " : ";
        DimInfo.DimType->print(O);
        O << "(";
        DimInfo.DimElementType->print(O, true /*IsForDebug*/,
                                      true /*NoDetails*/);
        O << ")";
        PrintStructOffsets(DimStructOffsets);
        O << "}";
      }
    }
  } else if (isa<VPBranchInst>(this) || isa<VPBlendInst>(this) ||
             isa<VPCallInstruction>(this)) {
    // Nothing to print, operands are already printed for these instructions.
  } else {
    if (getOpcode() == VPInstruction::AllocatePrivate) {
      O << " ";
      getType()->print(O);
      O << ", OrigAlign = "
        << cast<VPAllocatePrivate>(this)->getOrigAlignment().value();
    }
    for (const VPValue *Operand : operands()) {
      O << " ";
      Operand->printAsOperand(O);
    }
    switch (getOpcode()) {
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToUI:
    case Instruction::FPToSI:
    case Instruction::FPExt:
    case Instruction::PtrToInt:
    case Instruction::IntToPtr:
    case Instruction::SIToFP:
    case Instruction::UIToFP:
    case Instruction::Trunc:
    case Instruction::FPTrunc: {
      O << " to ";
      getType()->print(O);
      break;
    }
    case VPInstruction::HIRCopy:
      O << " , OriginPhiId: " << cast<VPHIRCopyInst>(this)->getOriginPhiId();
      break;
    case VPInstruction::VectorTripCountCalculation: {
      auto *Self = cast<VPVectorTripCountCalculation>(this);
      O << ", UF = " << Self->getUF();
      break;
    }
    case VPInstruction::InductionFinal: {
      auto *Self = cast<VPInductionFinal>(this);
      if (Self->isLastValPreIncrement())
        O << ", LastValPreInc = 1";
      break;
    }
    case VPInstruction::ReductionFinalUdr: {
      auto *Self = cast<VPReductionFinalUDR>(this);
      O << ", Combiner: " << Self->getCombiner()->getName();
      break;
    }
    case VPInstruction::VLSLoad: {
      auto *VLSLoad = cast<VPVLSLoad>(this);
      O << ", group_size=" << VLSLoad->getGroupSize()
        << ", align=" << VLSLoad->getAlignment().value();
      break;
    }
    case VPInstruction::VLSStore: {
      auto *VLSStore = cast<VPVLSStore>(this);
      O << ", group_size=" << VLSStore->getGroupSize()
        << ", align=" << VLSStore->getAlignment().value();
      break;
    }
    case VPInstruction::VLSExtract: {
      auto *Extract = cast<VPVLSExtract>(this);
      O << ", group_size=" << Extract->getGroupSize()
        << ", offset=" << Extract->getOffset();
      break;
    }
    case VPInstruction::VLSInsert: {
      auto *Insert = cast<VPVLSInsert>(this);
      O << ", group_size=" << Insert->getGroupSize()
        << ", offset=" << Insert->getOffset();
      break;
    }
    case VPInstruction::GeneralMemOptConflict: {
      auto *Conflict = cast<VPGeneralMemOptConflict>(this);
      O << " -> VConflictRegion {\n";
      O << "    value : none \n";
      O << "    mask : none \n";
      Conflict->getRegion()->dump(O);
      O << "   }";
      break;
    }
    case VPInstruction::TreeConflict: {
      auto *TreeConf = cast<VPTreeConflict>(this);
      O << " { Redux Opcode: ";
      O << getOpcodeName(TreeConf->getRednOpcode());
      O << " }";
      break;
    }
    }
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

unsigned VPInstruction::getNumSuccessors() const {
  return cast<VPBranchInst>(this)->getNumSuccessors();
}

#if INTEL_CUSTOMIZATION
VPlanVector::VPlanVector(VPlanKind K, VPExternalValues &E,
                         VPUnlinkedInstructions &UVPI)
    : VPlan(K, E, UVPI) {}
VPlanMasked::VPlanMasked(VPExternalValues &E, VPUnlinkedInstructions &UVPI)
    : VPlanVector(VPlanKind::Masked, E, UVPI) {}
VPlanNonMasked::VPlanNonMasked(VPExternalValues &E,
                               VPUnlinkedInstructions &UVPI)
    : VPlanVector(VPlanKind::NonMasked, E, UVPI) {}

VPlan::~VPlan() {
  // After this it is safe to delete instructions.
  for (auto &BB : *this)
    BB.dropAllReferences();
  // Drop all operands of VPLiveOutValues that belong to current Plan.
  for (auto *LOV : liveOutValues())
    if (LOV)
      LOV->dropAllReferences();
}

void VPlanVector::computeDT(void) {
  if (!PlanDT)
    PlanDT.reset(new VPDominatorTree);
  PlanDT->recalculate(*this);
}

void VPlanVector::computePDT(void) {
  if (!PlanPDT)
    PlanPDT.reset(new VPPostDominatorTree);
  PlanPDT->recalculate(*this);
}

#endif // INTEL_CUSTOMIZATION

void VPlanVector::runSVA(unsigned VF) {
  if (!EnableScalVecAnalysis)
    return;
  if (VF == 1)
    VPlanSVA = std::make_unique<VPlanScalVecAnalysisScalar>();
  else
    VPlanSVA = std::make_unique<VPlanScalVecAnalysis>();
  VPlanSVA->compute(this);
}

void VPlanVector::clearSVA() {
  // Reset CallVecDecisions results that were recorded for this VPlan.
  VPlanCallVecDecisions CVDA(*this);
  CVDA.reset();
  VPlanSVA.reset();
}

void VPlanVector::invalidateAnalyses(ArrayRef<VPAnalysisID> Analyses) {
  for (auto ID : Analyses) {
    switch (ID) {
    case VPAnalysisID::SVA:
      clearSVA();
      break;
    case VPAnalysisID::DA:
    case VPAnalysisID::VLS:
      llvm_unreachable("Add invalidation support for analysis.");
    }
  }
}

// Generate the code inside the body of the vectorized loop. Assumes a single
// LoopVectorBody basic block was created for this; introduces additional
// basic blocks as needed, and fills them all.
void VPlanVector::execute(VPTransformState *State) {
  VPLoop *VLoop = getMainLoop(false);
  State->ILV->setVPlan(this);

  IRBuilder<>::InsertPointGuard Guard(State->Builder);

  State->CFG.InsertBefore = State->ILV->getOrigLoop()->getExitBlock();
  State->CFG.PrevBB = State->ILV->getOrigLoop()->getLoopPreheader();
  // Find first VPBB that is on the path to vector loop. We can't
  // place vector instructions before that block, this might lead
  // to unnecessary vector code executed when the vector path is
  // not taken (e.g. after TC check).
  // Supposing that CFG is built like below
  //
  // pred.block:
  //   %vectorTC = vector-trip-count %op
  //   %c = icmp eq i64 %vectorTC, 0
  //   br i1 %c, label vector.ph, label %scalar.ph
  // vector.ph:
  //   ...
  // vector.body:
  //   ...
  // Here vector.ph is the needed block. We go from loop preheader
  // back by single predecessor until find the block with more than
  // one succesor.
  // TODO. That might need correction if we will insert some
  // if-then-else initilization sequences before VPLoop preheader.
  VPBasicBlock *BB;
  for (BB = VLoop->getLoopPreheader();
       BB && BB->getSinglePredecessor() &&
       BB->getSinglePredecessor()->getNumSuccessors() == 1;
       BB = BB->getSinglePredecessor()) {
    if (any_of(*BB, [](VPInstruction &Inst) {
          return isa<VPVectorTripCountCalculation>(Inst);
        }))
      break;
  }
  assert(BB && "Can't find first executable VPlan block");
  State->CFG.FirstExecutableVPBB = BB;
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(&getEntryBlock());
  for (VPBasicBlock *BB : RPOT) {
    LLVM_DEBUG(dbgs() << "LV: VPBlock in RPO " << BB->getName() << '\n');
    BB->execute(State);
  }
}

#if INTEL_CUSTOMIZATION
void VPlanVector::executeHIR(VPOCodeGenHIR *CG) {
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(&getEntryBlock());

  for (VPBasicBlock *VPBB : RPOT) {
    LLVM_DEBUG(dbgs() << "HIRV: VPBlock in RPO " << VPBB->getName() << '\n');
    VPBB->executeHIR(CG);
  }
}
#endif

void VPlanVector::setVPSE(std::unique_ptr<VPlanScalarEvolution> A) {
  VPSE = std::move(A);
  for (auto &VPBB : VPBasicBlocks)
    for (auto &VPInst : VPBB)
      if (auto *Memref = dyn_cast<VPLoadStoreInst>(&VPInst))
        Memref->setAddressSCEV(VPSE->computeAddressSCEV(*Memref));
}

void VPlanVector::updateDominatorTree(DominatorTree *DT,
                                      BasicBlock *LoopPreHeaderBB,
                                      BasicBlock *LoopLatchBB) {
  BasicBlock *LoopHeaderBB = LoopPreHeaderBB->getSingleSuccessor();
  assert(LoopHeaderBB && "Loop preheader does not have a single successor.");
  DT->addNewBlock(LoopHeaderBB, LoopPreHeaderBB);
  // The vector body may be more than a single basic block by this point.
  // Update the dominator tree information inside the vector body by
  // propagating
  // it from header to latch, expecting only triangular control-flow, if any.
  BasicBlock *PostDomSucc = nullptr;
  for (auto *BB = LoopHeaderBB; BB != LoopLatchBB; BB = PostDomSucc) {
    // Get the list of successors of this block.
    std::vector<BasicBlock *> Succs(succ_begin(BB), succ_end(BB));
    assert(Succs.size() <= 2 &&
           "Basic block in vector loop has more than 2 successors.");
    PostDomSucc = Succs[0];
    if (Succs.size() == 1) {
      assert(PostDomSucc->getSinglePredecessor() &&
             "PostDom successor has more than one predecessor.");
      DT->addNewBlock(PostDomSucc, BB);
      continue;
    }
    BasicBlock *InterimSucc = Succs[1];
    if (PostDomSucc->getSingleSuccessor() == InterimSucc) {
      PostDomSucc = Succs[1];
      InterimSucc = Succs[0];
    }
    assert(InterimSucc->getSingleSuccessor() == PostDomSucc &&
           "One successor of a basic block does not lead to the other.");
    assert(InterimSucc->getSinglePredecessor() &&
           "Interim successor has more than one predecessor.");
    assert(std::distance(pred_begin(PostDomSucc), pred_end(PostDomSucc)) == 2 &&
           "PostDom successor has more than two predecessors.");
    DT->addNewBlock(InterimSucc, BB);
    DT->addNewBlock(PostDomSucc, BB);
  }
}

VPlanAdapter::VPlanAdapter(VPlan &P)
    : VPlanAdapter(VPInstruction::PlanAdapter, P) {}

VPlanAdapter::VPlanAdapter(unsigned Opcode, VPlan &P)
    : VPInstruction(Opcode, Type::getTokenTy(*P.getLLVMContext()), {}),
      Plan(P) {}

VPValue *VPlanPeelAdapter::getPeelLoop() const {
  VPInstruction *LoopI = cast<VPlanScalar>(Plan).getScalarLoopInst();
  assert((isa<VPScalarPeel>(LoopI) || isa<VPScalarPeelHIR>(LoopI)) &&
         "Scalar loop instruction expected for peel adapter VPlan.");
  return LoopI;
}

// Visit all orig-liveout-hir instructions and update original loop UB.
// Transform looks like -
// Before:
//  PeelBlk:
//   token %hir-loop = scalar-peel-hir NeedsCloning: 1, TempInitMap:
//     { Initialize temp %ub with %vp.peel.ub }
//   i64 %live-out-1 = orig-live-out-hir token %hir-loop, liveout: %N + -1
//   br PostPeel
// After:
//  PeelBlk:
//   token %hir-loop = scalar-peel-hir NeedsCloning: 1, TempInitMap:
//     { Initialize temp %ub with %vp.peel.ub }
//   i64 %live-out-1 = orig-live-out-hir token %hir-loop, liveout: %ub
//   br PostPeel
void VPlanPeelAdapter::updateUBInHIROrigLiveOut() {
  auto *PeelHLp = cast<VPScalarPeelHIR>(getPeelLoop());
  loopopt::DDRef *OrigUB = PeelHLp->getLoop()->getUpperDDRef();
  loopopt::DDRef *NewUB = PeelHLp->getUpperBoundTemp();

  // Iterate over users of scalar peel loop and update all VPOrigLiveOutHIR that
  // use OrigUB.
  for (auto *U : PeelHLp->users()) {
    auto *HIRLiveOut = cast<VPPeelOrigLiveOutHIR>(U);
    // TODO: Use HIR's equal interfaces instead?
    if (HIRLiveOut->getLiveOutVal() == OrigUB)
      HIRLiveOut->setClonedLiveOutVal(NewUB);
  }
}

const VPValue *VPlanPeelAdapter::getUpperBound() const {
  VPValue *PeelLp = getPeelLoop();
  if (auto *IRPeel = dyn_cast<VPScalarPeel>(PeelLp))
    return IRPeel->getUpperBound();
  return cast<VPScalarPeelHIR>(PeelLp)->getUpperBound();
}

void VPlanPeelAdapter::setUpperBound(VPValue *TC) {
  if (isa<VPlanScalarPeel>(Plan)) {
    VPValue *PeelLp = getPeelLoop();
    if (auto *IRPeel = dyn_cast<VPScalarPeel>(PeelLp))
      IRPeel->setUpperBound(TC);
    else {
      cast<VPScalarPeelHIR>(PeelLp)->setUpperBound(TC);
      updateUBInHIROrigLiveOut();
    }
    return;
  }
  assert(isa<VPlanMasked>(Plan) && "unexpected peel VPlan");

  VPLoop *TopVPLoop = *cast<VPlanMasked>(Plan).getVPLoopInfo()->begin();
  VPValue *OrigTC;
  VPCmpInst *Cond;
  std::tie(OrigTC, Cond) = TopVPLoop->getLoopUpperBound();
  VPBasicBlock *Header = TopVPLoop->getHeader();
  // Replace OrigTC in the latch condition and in the header
  // top condition.
  OrigTC->replaceUsesWithIf(TC, [Header, Cond](auto *VUse) {
    if (VUse == Cond)
      return true;
    if (auto VCmp = dyn_cast<VPCmpInst>(VUse))
      return VCmp->getParent() == Header;
    return false;
  });
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const Twine VPlanPrinter::getUID(const VPBasicBlock *BB) {
  return "N" + Twine(getOrCreateBID(BB));
}

const Twine VPlanPrinter::getOrCreateName(const VPBasicBlock *BB) {
  const Twine &Name = BB->getName();
  if (!Name.isTriviallyEmpty())
    return Name;
  return "VPB" + Twine(getOrCreateBID(BB));
}

void VPlan::print(raw_ostream &OS, unsigned Indent) const {
  ReversePostOrderTraversal<const VPBasicBlock *> RPOT(&getEntryBlock());
  SetVector<const VPBasicBlock *> Printed;
  SetVector<const VPBasicBlock *> SuccList;
  SuccList.insert(&getEntryBlock());
  std::string StrIndent = std::string(2, ' ');
  for (const VPBasicBlock *VPBB : RPOT) {
    VPBB->print(OS, Indent + SuccList.size() - 1);
    Printed.insert(VPBB);
    SuccList.remove(VPBB);
    for (auto *Succ : VPBB->getSuccessors())
      // Do not increase Indent for back edges
      if (!Printed.count(Succ))
        SuccList.insert(Succ);
  }
}

void VPlan::dump(raw_ostream &OS) const {
  if (!isPrintingEnabled())
    return;
  formatted_raw_ostream FOS(OS);
  if (!getName().empty())
    FOS << "VPlan IR for: " << getName() << "\n";
  if (DumpExternalDefsHIR)
    Externals.dumpExternalDefs(FOS);

  if (DumpVPlanLiveInsLiveOuts)
    printLiveIns(FOS);

  // Print Scalar/Vector VPlan-specific data.
  printSpecifics(FOS);

  print(FOS, 1);

  if (DumpVPlanLiveInsLiveOuts)
    printLiveOuts(FOS);

  Externals.dumpExternalUses(FOS, LiveOutValues.size() ? this : nullptr);
}

void VPlan::dump() const { dump(dbgs()); }

void VPlanVector::printVectorVPlanData() const {
  LLVM_DEBUG(dbgs() << "Dominator Tree After initial VPlan transforms\n";
             PlanDT->print(dbgs()));
  LLVM_DEBUG(dbgs() << "PostDominator Tree After initial VPlan transforms :\n";
             PlanPDT->print(dbgs()));
  LLVM_DEBUG(dbgs() << "VPLoop Info After initial VPlan transforms:\n";
             VPLInfo->print(dbgs()));
}

// Iterate over the list of entities and print relevant data.
void VPlanVector::printSpecifics(raw_ostream &OS) const {
  for (auto EIter = LoopEntities.begin(), End = LoopEntities.end();
       EIter != End; ++EIter) {
    VPLoopEntityList *E = EIter->second.get();
    E->dump(OS, EIter->first->getHeader());
  }
}

void VPlan::printLiveIns(raw_ostream &OS) const {
  if (!LiveInValues.size())
    return;
  OS << "Live-in values:\n";
  for (auto LI : liveInValues()) {
    if (!LI)
      continue;
    OS << "ID: " << LI->getMergeId() << " Value: ";
    Externals.getOriginalIncomingValue(LI->getMergeId())->printAsOperand(OS);
    OS << "\n";
  }
}

void VPlan::printLiveOuts(raw_ostream &OS) const {
  if (!LiveOutValues.size())
    return;
  OS << "Live-out values:\n";
  for (auto LO : liveOutValues()) {
    if (LO)
      LO->print(OS);
  }
}

void VPlanPrinter::dump(bool CFGOnly) {
  if (!Plan.isPrintingEnabled())
    return;
#if INTEL_CUSTOMIZATION
  if (DumpPlainVPlanIR) {
    Plan.dump(OS);
    return;
  }
#endif /* INTEL_CUSTOMIZATION */

  Depth = 1;
  bumpIndent(0);
  OS << "digraph VPlan {\n";
  OS << "graph [labelloc=t, fontsize=30; label=\"Vectorization Plan";
  if (!Plan.getName().empty())
    OS << "\\n" << DOT::EscapeString(Plan.getName());
  OS << "\"]\n";
  OS << "node [shape=rect, fontname=Courier, fontsize=30]\n";
  OS << "edge [fontname=Courier, fontsize=30]\n";
  OS << "compound=true\n";

  for (const VPBasicBlock *BB : depth_first(&Plan.getEntryBlock()))
    dumpBasicBlock(BB, CFGOnly);

  OS << "}\n";
}

void VPlanPrinter::drawEdge(const VPBasicBlock *From, const VPBasicBlock *To,
                            bool Hidden, const Twine &Label) {
  OS << Indent << getUID(From) << " -> " << getUID(To);
  OS << " [ label=\"" << Label << '\"';
  if (Hidden)
    OS << "; splines=none";
  OS << "]\n";
}

void VPlanPrinter::dumpEdges(const VPBasicBlock *BB) {
  if (BB->getNumSuccessors() == 1)
    drawEdge(BB, BB->getSuccessor(0), false, "");
  else if (BB->getNumSuccessors() == 2) {
    drawEdge(BB, BB->getSuccessor(0), false, "T");
    drawEdge(BB, BB->getSuccessor(1), false, "F");
  } else {
    unsigned SuccessorNumber = 0;
    for (auto *Successor : BB->getSuccessors())
      drawEdge(BB, Successor, false, Twine(SuccessorNumber++));
  }
}

void VPlanPrinter::dumpBasicBlock(const VPBasicBlock *BB, bool SkipInstructions) {
  std::string Br = "<BR ALIGN=\"left\"/>\n";
  OS << Indent << getUID(BB) << " [label =<" << Br;
  bumpIndent(1);
  OS << Indent << DOT::EscapeString(BB->getName().str()) << ":" << Br;
  bumpIndent(1);
  auto Print = [this, &Br](const VPValue &Val) {
    std::string Str;
    raw_string_ostream SS(Str);
    Val.print(SS);
    for (unsigned i = 0; i != Str.length(); ++i)
      switch (Str[i]) {
      case '<':
        Str.replace(i, 1, "&lt;");
        i += 3;
        break;
      case '>':
        Str.replace(i, 1, "&gt;");
        i += 3;
        break;
      }
    OS << Indent <<  DOT::EscapeString(Str) << Br;
  };
  if (!SkipInstructions)
    for (const VPInstruction &Inst : *BB)
      Print(Inst);

  const VPValue *CBV = BB->getCondBit();
  // Dump the CondBit
  if (CBV) {
    if (SkipInstructions) {
      // In order to have DA results.
      Print(*CBV);
    }
    OS << Indent << "CondBit: ";
    if (const VPInstruction *CBI = dyn_cast<VPInstruction>(CBV)) {
      CBI->printAsOperand(OS);
      OS << " (" << DOT::EscapeString(CBI->getParent()->getName().str()) << ")" << Br;
    } else {
      CBV->printAsOperand(OS);
      OS << '"';
    }
  }
  bumpIndent(-2);
  OS << Indent << ">]\n";
  dumpEdges(BB);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#if INTEL_CUSTOMIZATION

void VPlanScalar::setNeedCloneOrigLoop(bool V) {
  NeedCloneOrigLoop = V;
  if (!V)
    return;

  VPInstruction *LoopI = getScalarLoopInst();
  if (auto *IRPeel = dyn_cast<VPScalarPeel>(&*LoopI))
    IRPeel->setCloningRequired();
  else if (auto *IRRem = dyn_cast<VPScalarRemainder>(&*LoopI))
    IRRem->setCloningRequired();
  else if (auto *HIRPeel = dyn_cast<VPScalarPeelHIR>(&*LoopI))
    HIRPeel->setCloningRequired();
  else
    cast<VPScalarRemainderHIR>(*LoopI).setCloningRequired();
}

VPInstruction *VPlanScalar::getScalarLoopInst() {
  auto It = llvm::find_if(vpinstructions(this), [](VPInstruction &I) {
    return isa<VPScalarPeel>(&I) || isa<VPScalarPeelHIR>(&I) ||
           isa<VPScalarRemainder>(&I) || isa<VPScalarRemainderHIR>(&I);
  });

  if (It != vpinstructions(this).end())
    return &*It;
  llvm_unreachable("can't find scalar loop instruction");
}

void VPBlendInst::addIncoming(VPValue *IncomingVal, VPValue *BlockPred, VPlan *Plan) {
  addOperand(IncomingVal);
  if (!BlockPred && Plan) {
    BlockPred =
        Plan->getVPConstant(ConstantInt::getTrue(*Plan->getLLVMContext()));
  }
  addOperand(BlockPred);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPInvSCEVWrapper::printImpl(raw_ostream &O) const {
  if (IsSCEV) {
    O << "{ ";
    VPlanScalarEvolutionLLVM::toSCEV(Scev)->print(O);
    O << " }";
  } else {
    auto *AddRecHIR = VPlanScalarEvolutionHIR::toVPlanAddRecHIR(Scev);
    O << *AddRecHIR;
  }
}

void VPBlendInst::printImpl(raw_ostream &O) const {
  O << getOpcodeName(getOpcode());
  auto PrintValueWithBP = [&](const unsigned i) {
    O << " [ ";
    getIncomingValue(i)->printAsOperand(O);
    O << ", ";
    getIncomingPredicate(i)->printAsOperand(O);
    O << " ]";
  };
  const unsigned size = getNumIncomingValues();
  for (unsigned i = 0; i < size; ++i) {
    if (i > 0)
      O << ",";
    PrintValueWithBP(i);
  }
}

void VPBranchInst::printImpl(raw_ostream &O) const {
  if (getNumSuccessors() == 0)
    O << "br <External Block>";
  else
    O << "br ";
  if (isConditional()) {
    getCondition()->printAsOperand(O);
    O << ", ";
  }
  if (const loopopt::HLGoto *Node = getHLGoto()) {
    if (const BasicBlock *BB = Node->getTargetBBlock())
      O << BB->getName();
    else
      // FIXME: Call HGoto print.
      O << "<External Basic Block>";
  }
  else
    for (auto It = succ_begin(); It != succ_end(); It++) {
      if (It != succ_begin())
        O << ", ";
      O << (*It)->getName();
    }
}

void VPCallInstruction::printImpl(raw_ostream &O) const {
  O << getOpcodeName(getOpcode());
  for (const VPValue *Arg : arg_operands()) {
    O << " ";
    Arg->printAsOperand(O);
  }
  O << " ";
  VPValue *CalledValue = getCalledValue();
  bool IsMasked = getParent()->getPredicate() != nullptr;
  switch (VecScenario) {
  case CallVecScenarios::Undefined:
  case CallVecScenarios::DoNotWiden: {
    CalledValue->printAsOperand(O);
    break;
  }
  case CallVecScenarios::Serialization: {
    CalledValue->printAsOperand(O);
    O << " [Serial]";
    break;
  }
  // For library function based vectorization, the expected vector library
  // function name that will be used by CG is printed.
  case CallVecScenarios::LibraryFunc: {
    O << getVectorLibraryFunc();
    O << " [x " << getPumpFactor() << "]";
    if (IsMasked)
      O << " [@CurrMask]";
    break;
  }
  // For vector-varant based vectorization, the matched variant function name
  // that will be used by CG is printed.
  case CallVecScenarios::VectorVariant: {
    O << getVectorVariant()->FullName;
    O << " [x " << getPumpFactor() << "]";
    IsMasked = IsMasked || shouldUseMaskedVariantForUnmasked();
    if (IsMasked)
      O << " [@CurrMask]";
    break;
  }
  // For trivially vectorizable intrinsics, the overload version of intrinsic
  // name that will be used by CG is printed.
  case CallVecScenarios::TrivialVectorIntrinsic: {
    O << getVectorIntrinName();
    O << " [x " << getPumpFactor() << "]";
    break;
  }
  case CallVecScenarios::UnmaskedWiden: {
    if (VecProperties.MatchedVecVariant) {
      O << getVectorVariant()->FullName;
    } else {
      O << "<VecVariant for ";
      CalledValue->printAsOperand(O);
      O << ">";
    }
    O << " [UnmaskedWiden]";
    break;
  }
  }
}

void VPlanAdapter::printImpl(raw_ostream &O) const {
  O << " for VPlan {" << Plan.getName() << "}";
}

#endif // !NDEBUG || LLVM_ENABLE_DUMP

Use *VPScalarPeel::findUpperBoundUseInLatch() const {
  Loop *L = getLoop();
  BasicBlock *Latch = L->getLoopLatch();
  auto BI = dyn_cast<BranchInst>(Latch->getTerminator());
  assert((BI && BI->isConditional()) &&
         "expected conditional branch instruction");
  auto Cond = cast<CmpInst>(BI->getCondition());
  if (L->isLoopInvariant(Cond->getOperand(0)))
    return &Cond->getOperandUse(0);
  else {
    assert(L->isLoopInvariant(Cond->getOperand(1)) &&
           "unexpectd non invariant");
    return &Cond->getOperandUse(1);
  }
}

unsigned VPVLSExtract::getNumGroupEltsPerValue() const {
  const DataLayout &DL = *getParent()->getParent()->getDataLayout();
  return llvm::vpo::getNumGroupEltsPerValue(DL, getOperand(0)->getType(),
                                            getType());
}

unsigned VPVLSInsert::getNumGroupEltsPerValue() const {
  const DataLayout &DL = *getParent()->getParent()->getDataLayout();
  return llvm::vpo::getNumGroupEltsPerValue(DL, getOperand(0)->getType(),
                                            getOperand(1)->getType());
}

void VPValue::replaceUsesWithIf(
    VPValue *NewVal, llvm::function_ref<bool(VPUser *U)> ShouldReplace,
    bool InvalidateIR) {
  assert(NewVal && "Value::replaceUsesWithIf(<null>) is invalid!");
  assert(NewVal->getType() == getType() &&
         "replaceUses of value with new value of different type!");

  SmallVector<VPUser *, 2> UsersToUpdate(
      make_filter_range(users(), ShouldReplace));

  for (VPUser *U : UsersToUpdate)
    U->replaceUsesOfWith(this, NewVal, InvalidateIR);
}

void VPValue::replaceAllUsesWithInBlock(VPValue *NewVal, VPBasicBlock &VPBB,
                                        bool InvalidateIR) {
  auto ShouldReplace = [&VPBB](VPUser *U) {
    if (VPInstruction *I = dyn_cast<VPInstruction>(U))
      return I->getParent() == &VPBB;
    return false;
  };
  replaceUsesWithIf(NewVal, ShouldReplace, InvalidateIR);
}

void VPValue::replaceAllUsesWithInLoop(VPValue *NewVal, VPLoop &Loop,
                                       bool InvalidateIR) {
  auto ShouldReplace = [&Loop](VPUser *U) {
    if (VPInstruction *I = dyn_cast<VPInstruction>(U))
      return Loop.contains(I);
    return false;
  };
  replaceUsesWithIf(NewVal, ShouldReplace, InvalidateIR);
}

void VPValue::replaceAllUsesWithInRegion(VPValue *NewVal,
                                         ArrayRef<VPBasicBlock *> Region,
                                         bool InvalidateIR) {
  auto ShouldReplace = [&Region](VPUser *U) {
    if (auto *I = dyn_cast<VPInstruction>(U))
      return is_contained(Region, I->getParent());
    return false;
  };
  replaceUsesWithIf(NewVal, ShouldReplace, InvalidateIR);
}

void VPValue::replaceAllUsesWith(VPValue *NewVal, bool InvalidateIR) {
  replaceUsesWithIf(NewVal, [](VPUser *U) { return true; }, InvalidateIR);
}

bool VPValue::isUnderlyingIRValid() const {
  if (auto *VPI = dyn_cast<VPInstruction>(this))
    return VPI->isUnderlyingIRValid();
  else {
    // Non VPInstruction values can never be invalidated.
    return true;
  }
}

void VPValue::invalidateUnderlyingIR() {
  // Non VPInstruction values should not be invalidated since they represent
  // entities outside the loop being vectorized and it is illegal to modify
  // their underlying IR. Hence invalidation is strictly limited to
  // VPInstructions only.
  if (auto *VPI = dyn_cast<VPInstruction>(this)) {
    VPI->invalidateUnderlyingIR();
  }
}

StringRef VPValue::getVPNamePrefix() const {
  // FIXME: Define the VPNamePrefix in some analogue of the
  // llvm::Context just like we plan for the 'Name' field.
  static std::string VPNamePrefix = "vp.";
  if (isa<VPBasicBlock>(this))
    return "";
  return VPNamePrefix;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPValue::printAsOperandNoType(raw_ostream &OS) const {
  const unsigned long long AddrBitsUsed = 0xFFFF;

  unsigned long long Num =
      AddrBitsUsed & ((unsigned long long)this / alignof(VPValue));
  if (EnableNames && !Name.empty())
    // There is no interface to enforce uniqueness of the names, so continue
    // using the pointer-based name for the suffix.
    OS << '%' << Name << '.' << Num;
  else
    OS << "%vp" << Num;
}

void VPValue::printAsOperand(raw_ostream &OS) const {
  if (getType()->isLabelTy()) {
    OS << "label " << cast<VPBasicBlock>(this)->getName();
    return;
  }
  OS << *getType() << ' ';
  printAsOperandNoType(OS);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

using VPDomTree = DomTreeBase<VPBasicBlock>;
template void DomTreeBuilder::Calculate<VPDomTree>(VPDomTree &DT);

using VPPostDomTree = PostDomTreeBase<VPBasicBlock>;
template void DomTreeBuilder::Calculate<VPPostDomTree>(VPPostDomTree &PDT);
#endif

void VPlanVector::computeDA() {
  VPLoopInfo *VPLInfo = getVPLoopInfo();
  VPLoop *CandidateLoop = *VPLInfo->begin();
  auto *DA = getVPlanDA();
  DA->compute(this, CandidateLoop, VPLInfo, *getDT(), *getPDT(),
              false /*Not in LCSSA form*/);
  if (isSOAAnalysisEnabled()) {
    // Do SOA-analysis for loop-privates.
    // TODO: Consider moving SOA-analysis to VPAnalysesFactoryBase.
    VPSOAAnalysis VPSOAA(*this, *CandidateLoop);
    SmallPtrSet<VPInstruction *, 32> SOAVars;
    VPSOAA.doSOAAnalysis(SOAVars);
    DA->recomputeShapes(SOAVars, true /*EnableVerifyAndPrintDA*/);
  }
}

void VPlan::cloneLiveInValues(const VPlan &OrigPlan, VPValueMapper &Mapper) {
  allocateLiveInValues(OrigPlan.getLiveInValuesSize());
  for (auto OrigLI : OrigPlan.liveInValues()) {
    if (OrigLI) {
      auto ClonedLI = OrigLI->clone();
      setLiveInValue(ClonedLI, ClonedLI->getMergeId());
      Mapper.registerClone(OrigLI, ClonedLI);
    }
  }
}

void VPlan::cloneLiveOutValues(const VPlan &OrigPlan, VPValueMapper &Mapper) {
  allocateLiveOutValues(OrigPlan.getLiveOutValuesSize());
  for (auto OrigLO : OrigPlan.liveOutValues()) {
    if (OrigLO) {
      auto ClonedLO = OrigLO->clone();
      setLiveOutValue(ClonedLO, ClonedLO->getMergeId());
      Mapper.registerClone(OrigLO, ClonedLO);
    }
  }
}

// Common functionality to do a deep-copy when cloning VPlans.
void VPlanVector::copyData(VPAnalysesFactoryBase &VPAF, UpdateDA UDA,
                           VPlanVector *TargetPlan) {
  // Clone the basic blocks from the current VPlan to the new one
  VPCloneUtils::Value2ValueMapTy OrigClonedValuesMap;
  VPCloneUtils::cloneBlocksRange(&front(), &back(), OrigClonedValuesMap,
                                 nullptr, "", TargetPlan);

  // Clone live in and live out values.
  VPValueMapper Mapper(OrigClonedValuesMap);
  TargetPlan->cloneLiveOutValues(*this, Mapper);
  TargetPlan->cloneLiveInValues(*this, Mapper);

  // Update cloned instructions' operands
  for (VPBasicBlock &OrigVPBB : *this)
    Mapper.remapOperands(&OrigVPBB);

  for (auto LO : TargetPlan->liveOutValues())
    Mapper.remapInstruction(LO);

  // Update FullLinearizationForced
  if (isFullLinearizationForced())
    TargetPlan->markFullLinearizationForced();

  if (areActiveLaneInstructionsDisabled())
    TargetPlan->disableActiveLaneInstructions();

  // Enable SOA-analysis if it was enabled in the original VPlan.
  if (isSOAAnalysisEnabled())
    TargetPlan->enableSOAAnalysis();

  // Set SCEV to Cloned Plan
  TargetPlan->setVPSE(VPAF.createVPSE());

  // Set Value Tracking to Cloned Plan
  TargetPlan->setVPVT(VPAF.createVPVT(TargetPlan->getVPSE()));

  // Update dominator tree and post-dominator tree of new VPlan
  TargetPlan->computeDT();
  TargetPlan->computePDT();

  // Calclulate VPLoopInfo for the TargetPlan
  TargetPlan->setVPLoopInfo(std::make_unique<VPLoopInfo>());
  VPLoopInfo *ClonedVPLInfo = TargetPlan->getVPLoopInfo();
  ClonedVPLInfo->analyze(*TargetPlan->getDT());
  LLVM_DEBUG(ClonedVPLInfo->verify(*TargetPlan->getDT()));

  // Update HasNormalizedInduction
  VPLoop *ThisLoop = *getVPLoopInfo()->begin();
  VPLoop *ClonedLoop = *ClonedVPLInfo->begin();
  ClonedLoop->copyHasNormalizedInductionFlag(ThisLoop);

  // Update DA.
  if (UDA != UpdateDA::DoNotUpdateDA) {
    auto TargetPlanDA = std::make_unique<VPlanDivergenceAnalysis>();
    TargetPlan->setVPlanDA(std::move(TargetPlanDA));
    if (UDA == UpdateDA::RecalculateDA)
      TargetPlan->computeDA();
    else if (UDA == UpdateDA::CloneDA) {
      getVPlanDA()->cloneDAData(TargetPlan, OrigClonedValuesMap);
      TargetPlan->getVPlanDA()->disableDARecomputation();
    }
  }

  // Copy loop nesting level
  TargetPlan->setOrigLoopNestingLevel(getOrigLoopNestingLevel());

  // Copy PrintingEnabled flag
  TargetPlan->setPrintingEnabled(isPrintingEnabled());
}

VPlanVector *VPlanMasked::clone(VPAnalysesFactoryBase &VPAF, UpdateDA UDA) {
  // Create new masked VPlan
  auto *ClonedVPlan = new VPlanMasked(getExternals(), getUnlinkedVPInsts());
  ClonedVPlan->setName(getName() + ".cloned");

  copyData(VPAF, UDA, ClonedVPlan);
  return ClonedVPlan;
}

VPlanVector *VPlanNonMasked::clone(VPAnalysesFactoryBase &VPAF, UpdateDA UDA) {
  // Create new non-masked VPlan
  auto *ClonedVPlan = new VPlanNonMasked(getExternals(), getUnlinkedVPInsts());
  ClonedVPlan->setName(getName() + ".cloned");

  copyData(VPAF, UDA, ClonedVPlan);
  return ClonedVPlan;
}

VPlanMasked *VPlanNonMasked::cloneMasked(VPAnalysesFactoryBase &VPAF,
                                         UpdateDA UDA) {
  // Create new masked VPlan from a non-masked VPlan.
  auto *ClonedVPlan = new VPlanMasked(getExternals(), getUnlinkedVPInsts());
  ClonedVPlan->setName(getName() + ".cloned.masked");

  copyData(VPAF, UDA, ClonedVPlan);
  return ClonedVPlan;
}
