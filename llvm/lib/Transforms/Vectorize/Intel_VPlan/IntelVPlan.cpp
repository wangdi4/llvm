//===- IntelVPlan.cpp - Vectorizer Plan -----------------------------------===//
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
#include "IntelVPOCodeGen.h"
#include "IntelVPlanDominatorTree.h"
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
static cl::opt<bool> DumpPlainVPlanIR("vplan-plain-dump", cl::init(false),
                                      cl::Hidden,
                                      cl::desc("Print plain VPlan IR"));
static cl::opt<int>
    DumpVPlanLiveness("vplan-dump-liveness", cl::init(0), cl::Hidden,
                      cl::desc("Print VPlan instructions' liveness info"));

static cl::opt<bool>
    EnableNames("vplan-enable-names", cl::init(false), cl::Hidden,
                cl::desc("Print VP Operands using VPValue's Name member."));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
raw_ostream &llvm::vpo::operator<<(raw_ostream &OS, const VPValue &V) {
  if (const VPInstruction *I = dyn_cast<VPInstruction>(&V))
    I->dump(OS);
  else
    V.dump(OS);
  return OS;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif

void VPInstruction::generateInstruction(VPTransformState &State,
                                        unsigned Part) {
#if INTEL_CUSTOMIZATION
  State.ILV->vectorizeInstruction(this);
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

  const OVLSGroup *Group = nullptr;
  const HLInst *GrpStartInst = nullptr;
  int64_t InterleaveFactor = 0, InterleaveIndex = 0;

  // Compute group information if we have a valid master instruction
  if (HIR.isMaster() && isUnderlyingIRValid()) {
    HLNode *HNode = HIR.getUnderlyingNode();
    if (isa<HLInst>(HNode)) {
      unsigned Opcode = getOpcode();

      if (Opcode == Instruction::Load || Opcode == Instruction::Store) {
        VPlanVLSAnalysis *VLSA = CG->getVLS();
        const VPlan *Plan = CG->getPlan();
        int32_t GrpSize = 0;

        // Get OPTVLS group for current load/store instruction
        Group = VLSA->getGroupsFor(Plan, this);
        Optional<int64_t> GroupStride = Group ? Group->getConstStride() : None;

        // Only handle strided OptVLS Groups with no access gaps for now.
        if (GroupStride) {
          GrpSize = Group->size();
          APInt AccessMask = Group->computeByteAccessMask();

          // Access mask is currently 64 bits, skip groups with group stride >
          // 64 and access gaps.
          if (*GroupStride > 64 || !AccessMask.isAllOnesValue() ||
              AccessMask.getBitWidth() != *GroupStride)
            Group = nullptr;
        } else
          Group = nullptr;

        if (Group) {
          VPVLSClientMemrefHIR *FirstMemref = nullptr;
          const RegDDRef *FirstMemrefDD = nullptr;
          uint64_t RefSizeInBytes = 0;

          // Check that all members of the group have same type. Currently we
          // do not handle groups such as a[i].i, a[i].d for
          //   struct {int i; double d} a[100]
          // Also setup GrpStartInst, FirstMemref, FirstMemrefDD, and
          // RefSizeInBytes.
          bool TypesMatch = true;
          for (int64_t Index = 0; Index < Group->size(); ++Index) {
            auto *Memref = cast<VPVLSClientMemrefHIR>(Group->getMemref(Index));
            const HLInst *HInst;
            const RegDDRef *MemrefDD;

            // Members of the group can be memrefs which are not master
            // VPInstructions (due to HIR Temp cleanup). Assertions for validity
            // and to check if underlying HLInst is master VPI is not needed
            // anymore. We directly obtain the HInst from memref's RegDDRef.
            MemrefDD = Memref->getRegDDRef();
            HInst = cast<HLInst>(MemrefDD->getHLDDNode());
            if (Index == 0) {
              auto DL = MemrefDD->getDDRefUtils().getDataLayout();

              FirstMemref = Memref;
              FirstMemrefDD = MemrefDD;
              GrpStartInst = HInst;
              RefSizeInBytes =
                  DL.getTypeAllocSize(FirstMemrefDD->getDestType());
            } else if (MemrefDD->getDestType() != FirstMemrefDD->getDestType())
              TypesMatch = false;

            // Setup the interleave index of the current instruction within the
            // VLS group.
            if (Memref->getInstruction() == this) {
              auto DistOrNone = Memref->getConstDistanceFrom(*FirstMemref);
              InterleaveIndex = DistOrNone.getValueOr(0) / RefSizeInBytes;
            }
          }

          // Compute interleave factor based on the distance of the last memref
          // in the group from the first memref. This may not be the same as
          // the group size as we may see duplicate accesses like:
          //     a[2*i]
          //     a[2*i+1]
          //     a[2*i]
          auto *LastMemref =
              cast<VPVLSClientMemrefHIR>(Group->getMemref(GrpSize - 1));
          auto LastDistOrNone = LastMemref->getConstDistanceFrom(*FirstMemref);
          InterleaveFactor = LastDistOrNone.getValueOr(0) / RefSizeInBytes + 1;

          // If interleave factor is less than 2, nothing special needs to be
          // done. Similarly, we do not handle the case where all memrefs in the
          // group are not of the same type.
          if (InterleaveFactor < 2 || !TypesMatch)
            Group = nullptr;
        }
      }
    }
  }

  CG->widenNode(this, nullptr, Group, InterleaveFactor, InterleaveIndex,
                GrpStartInst);
}
#endif

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
      Opcode == Instruction::Select || Opcode == Instruction::GetElementPtr ||
      Opcode == Instruction::PHI || Opcode == Instruction::ICmp ||
      Opcode == Instruction::FCmp || Opcode == VPInstruction::Not ||
      Opcode == VPInstruction::AllZeroCheck ||
      Opcode == VPInstruction::InductionInit)
    return false;

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *VPInstruction::getOpcodeName(unsigned Opcode) {
  switch (Opcode) {
  case VPInstruction::Not:
    return "not";
#if INTEL_CUSTOMIZATION
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
  case VPInstruction::AllocatePrivate:
    return "allocate-priv";
  case VPInstruction::Subscript:
    return "subscript";
#endif
  default:
    return Instruction::getOpcodeName(Opcode);
  }
}

void VPInstruction::print(raw_ostream &O, const Twine &Indent) const {
  O << " +\n" << Indent << "\"EMIT ";
  print(O);
  O << "\\l\"";
}

#if INTEL_CUSTOMIZATION
void VPInstruction::dump(raw_ostream &O,
                         const VPlanDivergenceAnalysis *DA) const {
  print(O, DA);
  O << "\n";
}
#endif /* INTEL_CUSTOMIZATION */

void VPInstruction::print(raw_ostream &O,
                          const VPlanDivergenceAnalysis *DA) const {
#if INTEL_CUSTOMIZATION
  if (DA) {
    if (DA->isDivergent(*this))
      O << "[DA: Divergent] ";
    else
      O << "[DA: Uniform]   ";
  }

  if (getOpcode() != Instruction::Store && !isa<VPBranchInst>(this)) {
    printAsOperand(O);
    O << " = ";
  }
#else
  printAsOperand(O);
  O << " = ";
#endif /* INTEL_CUSTOMIZATION */

  switch (getOpcode()) {
#if INTEL_CUSTOMIZATION
  case Instruction::Br:
    cast<VPBranchInst>(this)->print(O);
    return;
  case Instruction::GetElementPtr:
    O << getOpcodeName(getOpcode());
    if (auto *VPGEP = dyn_cast<const VPGEPInstruction>(this)) {
      if (VPGEP->isInBounds()) {
        O << " inbounds";
      }
    }
    break;
  case VPInstruction::InductionInit:
    O << getOpcodeName(getOpcode()) << "{"
      << getOpcodeName(cast<const VPInductionInit>(this)->getBinOpcode())
      << "}";
    break;
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
#endif
  default:
    O << getOpcodeName(getOpcode());
  }

#if INTEL_CUSTOMIZATION
  // TODO: print type when this information will be available.
  // So far don't print anything, because PHI may not have Instruction
  if (auto *Phi = dyn_cast<const VPPHINode>(this)) {
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
  } else {
    if (getOpcode() == VPInstruction::AllocatePrivate) {
      O << " ";
      getType()->print(O);
    }
#endif // INTEL_CUSTOMIZATION
    for (const VPValue *Operand : operands()) {
      O << " ";
      Operand->printAsOperand(O);
    }
#if INTEL_CUSTOMIZATION
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
    case Instruction::FPTrunc:
      O << " to ";
      getType()->print(O);
    }
  }
#endif // INTEL_CUSTOMIZATION
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#if INTEL_CUSTOMIZATION
VPlan::VPlan(LLVMContext *Context, const DataLayout *DL)
    : Context(Context), DL(DL) {}

VPlan::~VPlan() {
  SmallVector<VPBasicBlock *, 8> Blocks;
  for (VPBasicBlock *BB : depth_first(getEntryBlock()))
    Blocks.push_back(BB);

  for (VPBasicBlock *BB : Blocks)
    delete BB;
}

void VPlan::recomputeSize() {
  Size = std::distance(df_iterator<const VPBasicBlock *>::begin(EntryBB),
                       df_iterator<const VPBasicBlock *>::end(ExitBB));
}

void VPlan::computeDT(void) {
  if (!PlanDT)
    PlanDT.reset(new VPDominatorTree);
  PlanDT->recalculate(*this);
}

void VPlan::computePDT(void) {
  if (!PlanPDT)
    PlanPDT.reset(new VPPostDominatorTree);
  PlanPDT->recalculate(*this);
}

#endif // INTEL_CUSTOMIZATION

// Generate the code inside the body of the vectorized loop. Assumes a single
// LoopVectorBody basic block was created for this; introduces additional
// basic blocks as needed, and fills them all.
void VPlan::execute(VPTransformState *State) {
  assert(std::distance(VPLInfo->begin(), VPLInfo->end()) == 1 &&
         "Expected single outermost loop!");
  VPLoop *VLoop = *VPLInfo->begin();
  State->ILV->setVPlan(this, getLoopEntities(VLoop));
  BasicBlock *VectorPreHeaderBB = State->CFG.PrevBB;
  BasicBlock *VectorHeaderBB = VectorPreHeaderBB->getSingleSuccessor();
  assert(VectorHeaderBB && "Loop preheader does not have a single successor.");
  BasicBlock *VectorLatchBB = VectorHeaderBB;
  auto *HTerm = VectorHeaderBB->getTerminator();
  assert(HTerm->getNumSuccessors() == 2 &&
         "Unexpected vector loop header successors");
  unsigned MidBlockSuccNum = HTerm->getSuccessor(0) == VectorHeaderBB ? 1 : 0;
  BasicBlock *MiddleBlock = HTerm->getSuccessor(MidBlockSuccNum);
  auto CurrIP = State->Builder.saveIP();

  // 1. Make room to generate basic blocks inside loop body if needed.
  VectorLatchBB = VectorHeaderBB->splitBasicBlock(
      VectorHeaderBB->getFirstInsertionPt(), "vector.body.latch");
  Loop *L = State->LI->getLoopFor(VectorHeaderBB);
  assert(L && "Unexpected null loop for Vector Header");
  L->addBasicBlockToLoop(VectorLatchBB, *State->LI);
  // Remove the edge between Header and Latch to allow other connections.
  // Temporarily terminate with unreachable until CFG is rewired.
  // Note: this asserts xform code's assumption that getFirstInsertionPt()
  // can be dereferenced into an Instruction.
  VectorHeaderBB->getTerminator()->eraseFromParent();
  State->Builder.SetInsertPoint(VectorHeaderBB);
  State->Builder.CreateUnreachable();
  // Set insertion point to vector loop PH
  State->Builder.SetInsertPoint(VectorPreHeaderBB->getTerminator());

  // 2. Generate code in loop body of vectorized version.
  State->CFG.PrevVPBB = nullptr;
  State->CFG.PrevBB = VectorPreHeaderBB;
  State->CFG.LastBB = VectorLatchBB;

  ReversePostOrderTraversal<VPBasicBlock *> RPOT(getEntryBlock());
  for (VPBasicBlock *BB : RPOT) {
    LLVM_DEBUG(dbgs() << "LV: VPBlock in RPO " << BB->getName() << '\n');
    BB->execute(State);
  }

  // 3. Fix the edges for blocks in VPBBsToFix list.
  for (auto VPBB : State->CFG.VPBBsToFix) {
    BasicBlock *BB = State->CFG.VPBB2IRBB[VPBB];
    assert(BB && "Unexpected null basic block for VPBB");

    unsigned Idx = 0;
    auto *BBTerminator = BB->getTerminator();

    for (VPBasicBlock *SuccVPBB : VPBB->getSuccessors()) {
      BBTerminator->setSuccessor(Idx, State->CFG.VPBB2IRBB[SuccVPBB]);
      ++Idx;
    }
  }

  // 4. Merge the temporary latch created with the last basic block filled.
  BasicBlock *LastBB = State->CFG.PrevBB;
  assert(isa<UnreachableInst>(LastBB->getTerminator()) &&
         "Expected VPlan CFG to terminate with unreachable");

  // LastBB will be the outermost loop exit block. Get the latch BB.
  BasicBlock *LatchBB = LastBB->getSinglePredecessor();
  assert(LatchBB && "Unexpected null latch BB");

  // Merge LatchBB with Latch.
  LatchBB->getTerminator()->eraseFromParent();
  BranchInst::Create(VectorLatchBB, LatchBB);

  bool merged = MergeBlockIntoPredecessor(VectorLatchBB, nullptr, State->LI);
  assert(merged && "Could not merge last basic block with latch.");
  (void)merged;
  VectorLatchBB = LatchBB;

  // Insert LastBB between LatchBB and MiddleBlock. TODO - currently we
  // assume MiddleBlock and LastBB do not have any PHIs. This will need
  // to be addressed if this changes.
  assert(!isa<PHINode>(MiddleBlock->begin()) &&
         "Middle block starts with a PHI");
  assert(!isa<PHINode>(LastBB->begin()) && "LastBB starts with a PHI");

  LatchBB->getTerminator()->setSuccessor(MidBlockSuccNum, LastBB);
  LastBB->getTerminator()->eraseFromParent();
  BranchInst::Create(MiddleBlock, LastBB);

  // Do no try to update dominator tree as we may be generating vector loops
  // with inner loops. Right now we are not marking any analyses as
  // preserved - so this should be ok.
  // updateDominatorTree(State->DT, VectorPreHeaderBB, VectorLatchBB);
  State->Builder.restoreIP(CurrIP);
}

#if INTEL_CUSTOMIZATION
void VPlan::executeHIR(VPOCodeGenHIR *CG) {
  CG->createAndMapLoopEntityRefs();
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(getEntryBlock());
  const VPLoop *VLoop = CG->getVPLoop();

  // Check and mark if we see any uniform control flow remaining in the loop.
  // We check for a non-latch block with two successors.
  // TODO - The code here assumes inner loop vectorization. This needs to
  // be changed for outer loop vectorization.
  for (VPBasicBlock *VPBB : RPOT) {
    if (VLoop->contains(VPBB) && !VLoop->isLoopLatch(VPBB) &&
        VPBB->getNumSuccessors() == 2) {
      CG->setUniformControlFlowSeen();
      break;
    }
  }

  for (VPBasicBlock *VPBB : RPOT) {
    LLVM_DEBUG(dbgs() << "HIRV: VPBlock in RPO " << VPBB->getName() << '\n');
    VPBB->executeHIR(CG);
  }
}
#endif

void VPlan::verifyVPConstants() const {
  SmallPtrSet<const Constant *, 16> ConstantSet;
  for (const auto &Pair : VPConstants) {
    const Constant *KeyConst = Pair.first;
    assert(KeyConst == Pair.second->getConstant() &&
           "Value key and VPConstant's underlying Constant must be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!ConstantSet.count(KeyConst) && "Repeated VPConstant!");
    ConstantSet.insert(KeyConst);
  }
}

void VPlan::verifyVPExternalDefs() const {
  SmallPtrSet<const Value *, 16> ValueSet;
  for (const auto &Def : VPExternalDefs) {
    const Value *KeyVal = Def->getUnderlyingValue();
    assert(!ValueSet.count(KeyVal) && "Repeated VPExternalDef!");
    ValueSet.insert(KeyVal);
  }
}

void VPlan::verifyVPExternalDefsHIR() const {
  SmallSet<unsigned, 16> SymbaseSet;
  SmallSet<unsigned, 16> IVLevelSet;
  for (const auto &ExtDef : VPExternalDefsHIR) {
    const VPOperandHIR *HIROperand = ExtDef.getOperandHIR();

    // Deeper verification depending on the kind of the underlying HIR operand.
    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      // For blobs we check that the symbases are unique.
      unsigned Symbase = Blob->getBlob()->getSymbase();
      assert(!SymbaseSet.count(Symbase) && "Repeated blob VPExternalDef!");
      SymbaseSet.insert(Symbase);
    } else {
      // For IVs we check that the IV levels are unique.
      const auto *IV = cast<VPIndVar>(HIROperand);
      unsigned IVLevel = IV->getIVLevel();
      assert(!IVLevelSet.count(IVLevel) && "Repeated IV VPExternalDef!");
      IVLevelSet.insert(IVLevel);
    }
  }
}

void VPlan::verifyVPMetadataAsValues() const {
  SmallPtrSet<const MetadataAsValue *, 16> MDAsValueSet;
  for (const auto &Pair : VPMetadataAsValues) {
    const MetadataAsValue *KeyMD = Pair.first;
    assert(KeyMD == Pair.second->getMetadataAsValue() &&
           "Value key and VPMetadataAsValue's underlying MetadataAsValue must "
           "be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!MDAsValueSet.count(KeyMD) && "Repeated MetadataAsValue!");
    MDAsValueSet.insert(KeyMD);
  }
}

void VPlan::updateDominatorTree(DominatorTree *DT, BasicBlock *LoopPreHeaderBB,
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const Twine VPlanPrinter::getUID(const VPBasicBlock *BB) {
  return "N" + Twine(getOrCreateBID(BB));
}

const Twine VPlanPrinter::getOrCreateName(const VPBasicBlock *BB) {
  const std::string &Name = BB->getName();
  if (!Name.empty())
    return Name;
  return "VPB" + Twine(getOrCreateBID(BB));
}

void VPlan::print(raw_ostream &OS, unsigned Indent, bool DumpDA) const {
  ReversePostOrderTraversal<const VPBasicBlock *> RPOT(getEntryBlock());
  SetVector<const VPBasicBlock *> Printed;
  SetVector<const VPBasicBlock *> SuccList;
  SuccList.insert(getEntryBlock());
  std::string StrIndent = std::string(2, ' ');
  for (const VPBasicBlock *VPBB : RPOT) {
    VPBB->print(OS, Indent + SuccList.size() - 1,
                DumpDA ? getVPlanDA() : nullptr);
    Printed.insert(VPBB);
    SuccList.remove(VPBB);
    for (auto *Succ : VPBB->getSuccessors())
      // Do not increase Indent for back edges
      if (!Printed.count(Succ))
        SuccList.insert(Succ);
  }
}

void VPlan::dump(raw_ostream &OS, bool DumpDA) const {
  formatted_raw_ostream FOS(OS);
  if (!getName().empty())
    FOS << "VPlan IR for: " << getName() << "\n";
  for (auto EIter = LoopEntities.begin(), End = LoopEntities.end();
       EIter != End; ++EIter) {
    VPLoopEntityList *E = EIter->second.get();
    E->dump(FOS, EIter->first->getHeader());
  }

  print(FOS, 1, DumpDA);

  if (!VPExternalUses.empty()) {
    FOS << "External Uses:\n";
    for (auto &ExtUse : VPExternalUses) {
      ExtUse.second->dump(FOS);
      FOS << "\n";
    }
  }
}

void VPlan::dump() const { dump(dbgs(), true); }

void VPlan::dumpLivenessInfo(raw_ostream &OS) const {
  if (DumpVPlanLiveness == 0 || !VPLInfo)
    return;
  OS << "Live-in and Live-out info:\n";
  if (!VPExternalDefs.empty()) {
    OS << "External defs:\n";
    for (auto DI = VPExternalDefs.begin(); DI != VPExternalDefs.end(); DI++)
      OS << *(*DI) << "\n";
  }
  if (!VPExternalDefsHIR.empty()) {
    OS << "External defs:\n";
    for (auto DI = VPExternalDefsHIR.begin(); DI != VPExternalDefsHIR.end();
         DI++)
      OS << *DI << "\n";
  }
  if (!VPExternalUses.empty()) {
    OS << "Used externally:\n";
    for (auto UI = VPExternalUses.begin(); UI != VPExternalUses.end(); UI++) {
      const VPValue *Op = UI->second->getOperand(0);
      Op->printAsOperand(OS);
      if (DumpVPlanLiveness > 1 && UI->second->getUnderlyingValue())
        OS << " (used by " << *(UI->second->getUnderlyingValue()) << ")\n";
      else
        OS << "\n";
    }
  }
  if (!VPExternalUsesHIR.empty()) {
    OS << "Used externally:\n";
    for (auto UI = VPExternalUsesHIR.begin(); UI != VPExternalUsesHIR.end();
         UI++) {
      const VPValue *Op = UI->getOperand(0);
      Op->printAsOperand(OS);
      if (DumpVPlanLiveness > 1 && UI->getUnderlyingValue())
        OS << " (used by " << *(UI->getUnderlyingValue()) << ")\n";
      else
        OS << "\n";
    }
  }
  std::function<void(const VPBasicBlock *)> dumpBlockLiveness =
      [&](const VPBasicBlock *BB) {
        // For each live-in or live-out instruction in the Block print the
        // corresponding comment
        const VPLoop *Loop = VPLInfo->getLoopFor(BB);
        if (DumpVPlanLiveness > 2)
          OS << "Liveness for BBlock: " << BB->getName() << "\n";
        if (Loop == nullptr) {
          if (DumpVPlanLiveness > 2)
            OS << "no loop found\n";
          return;
        }
        for (const VPInstruction &Inst : *BB) {
          const auto *VPInst = &Inst;
          if (DumpVPlanLiveness > 2)
            OS << "Instruction: " << *VPInst << "\n";
          for (const VPValue *Op : VPInst->operands()) {
            SmallVector<const VPLoop *, 4> LoopList;
            const VPLoop *ParentLoop = Loop;
            while (ParentLoop) {
              if (!ParentLoop->isLiveIn(Op))
                break;
              LoopList.push_back(ParentLoop);
              ParentLoop = ParentLoop->getParentLoop();
            }
            if (LoopList.size()) {
              Op->printAsOperand(OS);
              OS << " livein in the loops: ";
              for (const VPLoop *L : LoopList)
                OS << " " << L->getLoopPreheader()->getName();
              OS << "\n";
            }
          }
          if (Loop->isLiveOut(VPInst)) {
            VPInst->printAsOperand(OS);
            OS << " liveout in the loop: "
               << Loop->getLoopPreheader()->getName() << "\n";
          }
        }
      };

  std::function<void(const VPBasicBlock *)> dumpLiveness =
      [&](const VPBasicBlock *VPBB) {
        for (const VPBasicBlock *BB : depth_first(VPBB))
          // Print liveness information for a basic block
          dumpBlockLiveness(BB);
      };

  dumpLiveness(getEntryBlock());
  OS << "Live-in and Live-out info end\n";
}

void VPlanPrinter::dump() {
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

  for (const VPBasicBlock *BB : depth_first(Plan.getEntryBlock()))
    dumpBasicBlock(BB);

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
  auto &Successors = BB->getSuccessors();
  if (Successors.size() == 1)
    drawEdge(BB, Successors.front(), false, "");
  else if (Successors.size() == 2) {
    drawEdge(BB, Successors.front(), false, "T");
    drawEdge(BB, Successors.back(), false, "F");
  } else {
    unsigned SuccessorNumber = 0;
    for (auto *Successor : Successors)
      drawEdge(BB, Successor, false, Twine(SuccessorNumber++));
  }
}

void VPlanPrinter::dumpBasicBlock(const VPBasicBlock *BB) {
  OS << Indent << getUID(BB) << " [label =\n";
  bumpIndent(1);
  OS << Indent << "\"" << DOT::EscapeString(BB->getName()) << ":\\n\"";
  bumpIndent(1);
  for (const VPInstruction &Inst : *BB)
    Inst.print(OS, Indent);
#if INTEL_CUSTOMIZATION
  const VPValue *CBV = BB->getCondBit();
  // Dump the CondBit
  if (CBV) {
    OS << " +\n" << Indent << " \"CondBit: ";
    if (const VPInstruction *CBI = dyn_cast<VPInstruction>(CBV)) {
      CBI->printAsOperand(OS);
      OS << " (" << DOT::EscapeString(CBI->getParent()->getName()) << ")\\l\"";
    } else {
      CBV->printAsOperand(OS);
    }
  }
#endif
  bumpIndent(-2);
  OS << "\n" << Indent << "]\n";
  dumpEdges(BB);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

#if INTEL_CUSTOMIZATION
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPBranchInst::print(raw_ostream &O) const {
  O << "br ";
  const BasicBlock *BB = getTargetBlock();
  if (BB)
    O << BB->getName();
  else
    // FIXME: Call HGoto print.
    O << "<External Basic Block>";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void VPPHINode::sortIncomingBlocksForBlend(
    DenseMap<const VPBasicBlock *, int> *BlockIndexInRPOTOrNull) {
  // Sort incoming blocks according to their order in the linearized control
  // flow. After linearization, the HCFG coming to the codegen might be
  // something like this:
  //
  //   bb0:
  //     %def0 =
  //   bb1:
  //     predicate %cond0
  //     %def1 =
  //   bb2:
  //     predicate %cond1    ; %cond1 = %cond0 && %something
  //     %def2 =
  //   bb3:
  //     %blend_phi = phi [ %def1, %bb1 ], [ %def0, %bb0 ], [ %def 2, %bb2 ]
  //
  // We need to generate
  //
  //  %sel = select %cond0, %def1, %def0
  //  %blend = select %cond1 %def2, %sel
  //
  // Note, that the order of processing needs to be [ %def0, %def1, %def2 ]
  // for such CFG.

  // FIXME: Once we get rid of hierarchical CFG, we would be able to use
  // dominance as the comparator.
  DenseMap<const VPBasicBlock *, int> LocalBlockIndexInRPOT;
  DenseMap<const VPBasicBlock *, int> &BlockIndexInRPOT =
      BlockIndexInRPOTOrNull ? *BlockIndexInRPOTOrNull : LocalBlockIndexInRPOT;
  if (!BlockIndexInRPOTOrNull) {
    int CurrBlockRPOTIndex = 0;
    ReversePostOrderTraversal<VPBasicBlock *> RPOT(
        getParent()->getParent()->getEntryBlock());
    for (auto *BB : RPOT)
      BlockIndexInRPOT[BB] = CurrBlockRPOTIndex++;
  }

  unsigned NumIncoming = getNumIncomingValues();
  using PairTy = std::pair<VPValue *, VPBasicBlock *>;
  SmallVector<PairTy, 8> SortedIncomingBlocks;
  for (unsigned Idx = 0; Idx < NumIncoming; ++Idx) {
    PairTy Curr(getIncomingValue(Idx), getIncomingBlock(Idx));

    auto GetRPOTNumber = [&](VPBasicBlock *BB) -> unsigned {
      if (unsigned Idx = BlockIndexInRPOT[BB])
        return Idx;
      llvm_unreachable("RPOT index missing!");
    };

    SortedIncomingBlocks.insert(
        upper_bound(SortedIncomingBlocks, Curr,
                    [&](const PairTy &Lhs, const PairTy &Rhs) {
                      return GetRPOTNumber(Lhs.second) <
                             GetRPOTNumber(Rhs.second);
                    }),
        Curr);
  }
  LLVM_DEBUG(dbgs() << "BlendPhi: " << *this << ": ");
  for (unsigned Idx = 0; Idx < NumIncoming; ++Idx) {
    setIncomingValue(Idx, SortedIncomingBlocks[Idx].first);
    setIncomingBlock(Idx, SortedIncomingBlocks[Idx].second);
    LLVM_DEBUG(dbgs() << SortedIncomingBlocks[Idx].second->getName() << " - "
                      << BlockIndexInRPOT[SortedIncomingBlocks[Idx].second]
                      << ", ");
  }
  LLVM_DEBUG(dbgs() << "\n");
}

void VPValue::replaceAllUsesWithImpl(VPValue *NewVal, VPLoop *Loop,
                                     VPBasicBlock *VPBB, bool InvalidateIR) {
  assert(NewVal && "Can't replace uses with null value");
  assert(getType() == NewVal->getType() && "Incompatible data types");
  assert(!(Loop && VPBB) && "Cannot have both Loop and VPBB to be non-null.");
  unsigned Cnt = 0;
  while (getNumUsers() > Cnt) {
    if (Loop) {
      if (auto Instr = dyn_cast<VPInstruction>(Users[Cnt]))
        if (!Loop->contains(Instr->getParent())) {
          ++Cnt;
          continue;
        }
    } else if (VPBB) {
      if (auto Instr = dyn_cast<VPInstruction>(Users[Cnt]))
        if (Instr->getParent() != VPBB) {
          ++Cnt;
          continue;
        }
    }
    Users[Cnt]->replaceUsesOfWith(this, NewVal, InvalidateIR);
  }
}

bool VPValue::isUnderlyingIRValid() const {
  if (auto *VPI = dyn_cast<VPInstruction>(this))
    return IsUnderlyingValueValid || VPI->HIR.isValid();
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
    IsUnderlyingValueValid = false;
    // Temporary hook-up to ignore loop induction related instructions during CG
    // by not invalidating them.
    // TODO: Remove this code after VPInduction support is added to HIR CG.
    const HLNode *HNode = VPI->HIR.getUnderlyingNode();
    if (HNode && isa<HLLoop>(HNode))
      return;
    VPI->HIR.invalidate();

    // At this point, we don't have a use-case where invalidation of users of
    // instruction is needed. This is because VPInstructions mostly represent
    // r-value of a HLInst and modifying the r-value should not affect l-value
    // temp refs. For stores this was never an issue since store instructions
    // cannot have users. In case of LLVM-IR invalidating an instruction might
    // mean that the underlying bit value is different. If this is the case then
    // invalidation should be propagated to users as well.
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPValue::printAsOperand(raw_ostream &OS) const {
  if (EnableNames && !Name.empty())
    // There is no interface to enforce uniqueness of the names, so continue
    // using the pointer-based name for the suffix.
    OS << *getType() << " %" << Name << "."
       << (unsigned short)(unsigned long long)this;
  else
    OS << *getType() << " %vp" << (unsigned short)(unsigned long long)this;
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

using VPDomTree = DomTreeBase<VPBasicBlock>;
template void DomTreeBuilder::Calculate<VPDomTree>(VPDomTree &DT);

using VPPostDomTree = PostDomTreeBase<VPBasicBlock>;
template void DomTreeBuilder::Calculate<VPPostDomTree>(VPPostDomTree &PDT);
#endif
