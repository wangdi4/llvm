//===-- CSAStreamingMemoryConversion.cpp - Streaming memory operations ----===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass that converts memory operations to streaming
// memory loads and stores where applicable.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAInstBuilder.h"
#include "CSAInstrInfo.h"
#include "CSAMachineFunctionInfo.h"
#include "CSAMatcher.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "csa-streamem"
#define PASS_NAME "CSA: Streaming memory conversion pass."

static cl::opt<bool> DisableMemoryConversion(
  "csa-disable-streammem", cl::Hidden,
  cl::desc("CSA Specific: Disable streaming memory conversion"));

namespace llvm {
class CSAStreamingMemoryConversionPass : public MachineFunctionPass {
public:
  static char ID;
  CSAStreamingMemoryConversionPass();

  StringRef getPassName() const override {
    return PASS_NAME;
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<MachineOptimizationRemarkEmitterPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  MachineFunction *MF;
  MachineOptimizationRemarkEmitter *ORE;
  MachineRegisterInfo *MRI;
  CSAMachineFunctionInfo *LMFI;
  const CSAInstrInfo *TII;
  std::vector<MachineInstr *> to_delete;

  MachineInstr *makeStreamMemOp(MachineInstr *MI);
  void formWideOps(SmallVectorImpl<MachineInstr *> &insts);
  MachineInstr *getDefinition(const MachineOperand &MO) const;
  void getUses(const MachineOperand &MO,
               SmallVectorImpl<MachineInstr *> &uses) const;
  MachineInstr *getSingleUse(const MachineOperand &MO) const;
  bool isZero(const MachineOperand &MO) const;
  MachineOp getLength(const MachineOperand &start, const MachineOperand &end,
                      bool isEqual, int64_t stride, bool isOneTrip,
                      MachineInstr *buildPoint) const;
};

void initializeCSAStreamingMemoryConversionPassPass(PassRegistry &);
} // namespace llvm

char CSAStreamingMemoryConversionPass::ID = 0;

INITIALIZE_PASS_BEGIN(CSAStreamingMemoryConversionPass, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(MachineOptimizationRemarkEmitterPass)
INITIALIZE_PASS_END(CSAStreamingMemoryConversionPass, DEBUG_TYPE, PASS_NAME,
                    false, false)

CSAStreamingMemoryConversionPass::CSAStreamingMemoryConversionPass()
    : MachineFunctionPass(ID) {
  initializeCSAStreamingMemoryConversionPassPass(
      *PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSAStreamingMemoryConversionPass() {
  return new CSAStreamingMemoryConversionPass();
}

bool CSAStreamingMemoryConversionPass::runOnMachineFunction(
  MachineFunction &MF) {
  if (DisableMemoryConversion)
    return false;

  this->MF = &MF;
  MRI      = &MF.getRegInfo();
  LMFI     = MF.getInfo<CSAMachineFunctionInfo>();
  TII      = static_cast<const CSAInstrInfo *>(
    MF.getSubtarget<CSASubtarget>().getInstrInfo());
  ORE = &getAnalysis<MachineOptimizationRemarkEmitterPass>().getORE();

  SmallVector<MachineInstr *, 8> opsForCoalescing;

  // Go through the code, generating streaming memory operands for acceptable
  // loads and stores. The deleted instructions have to wait until the end of
  // the program to be cleared (to avoid iteration issues).
  bool changed = false;
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      MachineInstr *newInst = makeStreamMemOp(&MI);
      changed |= newInst != nullptr;
      if (newInst)
        opsForCoalescing.push_back(newInst);
    }
  }
  for (auto MI : to_delete)
    MI->eraseFromParent();
  to_delete.clear();

  // Try to coalesce streaming loads into wide streaming loads.
  formWideOps(opsForCoalescing);

  this->MF = nullptr;
  return changed;
}

MachineInstr *CSAStreamingMemoryConversionPass::getDefinition(
  const MachineOperand &MO) const {
  // We might end up searching on a global variable.
  return MO.isReg() ? MRI->getUniqueVRegDef(MO.getReg()) : nullptr;
}

void CSAStreamingMemoryConversionPass::getUses(
  const MachineOperand &MO, SmallVectorImpl<MachineInstr *> &uses) const {
  assert(MO.isReg() && "LICs to search for can only be registers");
  for (auto &use : MRI->use_instructions(MO.getReg())) {
    uses.push_back(&use);
  }
}

MachineInstr *
CSAStreamingMemoryConversionPass::getSingleUse(const MachineOperand &MO) const {
  SmallVector<MachineInstr *, 4> uses;
  getUses(MO, uses);
  return uses.size() == 1 ? uses[0] : nullptr;
}

bool isImm(const MachineOperand &MO, int64_t immValue) {
  return MO.isImm() && MO.getImm() == immValue;
}

bool CSAStreamingMemoryConversionPass::isZero(const MachineOperand &MO) const {
  if (MO.isReg()) {
    const MachineInstr *def = getDefinition(MO);
    if (def->getOpcode() == CSA::MOV64 && isImm(def->getOperand(1), 0))
      return true;
  }
  return isImm(MO, 0);
}

MachineOp CSAStreamingMemoryConversionPass::getLength(
  const MachineOperand &start, const MachineOperand &end, bool isEqual,
  int64_t stride, bool isOneTrip, MachineInstr *MI) const {
  CSAInstBuilder builder(*TII);
  builder.setInsertionPoint(MI);

  // In the one trip count, we need to account for the possibility that the
  // pattern is executed only once. Test the condition for the starting values
  // of start and end, and the length is (loop cond ? length : 1).
  if (isOneTrip) {
    MachineOp executed = getLength(start, end, isEqual, stride, false, MI);
    if (!executed)
      return nullptr;
    CSA::Generic cmpOpcode;
    switch (TII->getGenericOpcode(MI->getOpcode())) {
      case CSA::Generic::SEQOTNE: cmpOpcode = CSA::Generic::CMPNE; break;
      case CSA::Generic::SEQOTLE: cmpOpcode = CSA::Generic::CMPLE; break;
      case CSA::Generic::SEQOTLT: cmpOpcode = CSA::Generic::CMPLT; break;
      case CSA::Generic::SEQOTGE: cmpOpcode = CSA::Generic::CMPGE; break;
      case CSA::Generic::SEQOTGT: cmpOpcode = CSA::Generic::CMPGT; break;
      default: llvm_unreachable("Bad opcode");
    }
    unsigned licSize = TII->getLicSize(MI->getOpcode());

    // We need to execute the loop exactly once if the loop condition turns out
    // to be false. So do the comparison of the first iteration to see if it is
    // false. In this circumstance, we would return exactly 1 iteration instead
    // of our calculated count. Try to reuse older instructions if they exist.
    unsigned compare = TII->adjustOpcode(MI->getOpcode(), cmpOpcode);
    MachineOp loopCondition(nullptr);
    if (start.isReg()) {
      for (auto &use : MRI->use_instructions(start.getReg())) {
        if (use.getOpcode() == compare &&
            start.isIdenticalTo(use.getOperand(1)) &&
            end.isIdenticalTo(use.getOperand(2))) {
          loopCondition = OpReg(use.getOperand(0).getReg());
          break;
        }
      }
    }
    if (!loopCondition) {
      loopCondition = builder.makeOrConstantFold(*LMFI,
        TII->adjustOpcode(MI->getOpcode(), cmpOpcode), start, end);
    }

    // Check for the merge instruction already existing.
    unsigned mergeOpcode = TII->makeOpcode(CSA::Generic::MERGE, licSize);
    if (loopCondition.isReg()) {
      for (auto &use : MRI->use_instructions(loopCondition.getReg())) {
        if (use.getOpcode() == mergeOpcode &&
            use.getOperand(1) == loopCondition &&
            use.getOperand(2).isIdenticalTo(MachineOperand::CreateImm(1)) &&
            use.getOperand(3) == executed)
          return OpUse(use.getOperand(0));
      }
    }
    // Doesn't exist, make a new one instead.
    unsigned newLic = LMFI->allocateLIC(TII->getLicClassForSize(licSize));
    if (loopCondition.isReg())
      LMFI->setLICGroup(newLic, LMFI->getLICGroup(loopCondition.getReg()));
    builder.makeInstruction(mergeOpcode,
        OpRegDef(newLic),
        loopCondition,
        OpImm(1),
        executed);
    return OpReg(newLic);
  }

  if (stride < 0)
    return getLength(end, start, isEqual, -stride, isOneTrip, MI);
  if (stride != 1) {
    if (!isPowerOf2_64(stride)) {
      LLVM_DEBUG(dbgs() << "Stride is not a power of 2, bailing.\n");
      return nullptr;
    }

    // Trip count = (end + isEqual - start + stride - 1) / stride
    return builder.makeOrConstantFold(
      *LMFI, CSA::SRL64,
      builder.makeOrConstantFold(
        *LMFI, CSA::SUB64,
        builder.makeOrConstantFold(*LMFI, CSA::ADD64, end,
                                   OpImm(stride - 1 + isEqual)),
        start),
      OpImm(countTrailingZeros((unsigned)stride)));
  }
  if (isZero(start) && !isEqual) {
    return end;
  }

  MachineOperand effectiveStart = start;
  if (isEqual && start.isImm()) {
    effectiveStart = MachineOperand::CreateImm(start.getImm() - 1);
  } else if (isEqual) {
    LLVM_DEBUG(dbgs() << "<= bounds not handled for non-immediate starts\n");
    return nullptr;
  }

  // In the case where multiple loads/stores originate from this stream, we'll
  // find the sub we want just above us.
  MachineInstr *possible = MI->getPrevNode();
  if (possible && possible->getOpcode() == CSA::SUB64) {
    if (possible->getOperand(1).isIdenticalTo(end) &&
        possible->getOperand(2).isIdenticalTo(effectiveStart))
      return OpUse(possible->getOperand(0));
  }

  // Compute the length as end - start.
  return builder.makeOrConstantFold(*LMFI, CSA::SUB64, end, effectiveStart);
}

// This takes a load or a store controlled by a sequence operator and
// converts it into a streaming load and store. The requirements for legality
// are as follows:
// 1. The address is calculated as a strided offset, with base and stride
//    known. The stride may be limited to 1 for CSA v1. (TODO: implementation
//    not yet considered).
// 2. The length of the stride must be constant, at least in a SCEV-style
//    sense.
// 3. The input and output memory orders must consume/produce a single memory
//    order for the entire loop and not be used otherwise. This is effectively
//    saying that the input is a repeat guarded by a loop stream and the
//    output is a switch where all but the last value are ignored, but it's
//    possible that earlier optimizations do aggregation on a different level.
//
// The biggest constraint on the valid operations is the second one. For now,
// we accept only sequence operators, since calculating length is easy:
// * SEQOTNE64 0, %lic, 1  => length = %lic
// * SEQOTNE64 %lic, 0, -1 => length = %lic
// * SEQOTLTS64 0, %lic, 1 => length = %lic
// * SEQOTLTU64 0, %lic, 1 => length = %lic
// * SEQOT{NE,LTS,LTU}64 %base, %lic, 1 => length = %lic - %base
// Note that the pred output here is the %stream we consider.
//
// The source of the address computations is more complicated. The following
// patterns should be okay:
// * LD (STRIDE %stream, %base, %stride) => base = %base, stride = %stride
// * LDD (STRIDE %stream, %base, %stride), imm => base = %base + imm
// * LD{X,D} (REPEATO %stream, %base), (SEQOT**64_index 0, %N, %stride)
// * LD{X,D} (REPEATO %stream, %base), (SEQOT**64_index %start, %end, %stride)

MIRMATCHER_REGS(RESULT, REPEATED, SEQ_VAL, SEQ_PRED, SEQ_FIRST, SEQ_LAST, CTL);
using namespace CSAMatch;
constexpr auto repeated_pat = mirmatch::graph(
  RESULT = repeato_N(CTL, REPEATED), CTL = not1(SEQ_LAST),
  (SEQ_VAL, SEQ_PRED, SEQ_FIRST, SEQ_LAST) =
    seqot(mirmatch::AnyOperand, mirmatch::AnyOperand, mirmatch::AnyOperand));

MIRMATCHER_REGS(INMEM, OUTMEM1, OUTMEM2, VAL1, VAL2, BASE1, BASE2, LEN);
constexpr auto match2 = mirmatch::LiteralMatcher<uint64_t, 2>{};
constexpr auto wide_pat = mirmatch::graph(
  (VAL1, OUTMEM1) = sld_N(BASE1, LEN, match2, mirmatch::AnyOperand, INMEM),
  (VAL2, OUTMEM2) = sld_N(BASE2, LEN, match2, mirmatch::AnyOperand, INMEM),
  BASE2 = add64(BASE1, mirmatch::AnyLiteral));

MachineInstr *CSAStreamingMemoryConversionPass::makeStreamMemOp(MachineInstr *MI) {
  auto reportFailure = [=](const char *message) {
    MachineOptimizationRemarkMissed R(DEBUG_TYPE, "StreamingMemory",
        MI->getDebugLoc(), MI->getParent());
    ORE->emit(R << "streaming memory conversion failed: " << message);
  };

  const MachineOperand *base, *value;
  int64_t stride;
  const MachineOperand *inOrder, *outOrder, *memOrder;
  MachineInstr *stream;
  bool baseUsesStream = false;

  auto matchesStridePattern = [&](const MachineOperand &baseOp) -> bool {
    MachineInstr *memAddr = getDefinition(baseOp);
    if (!memAddr || memAddr->getOpcode() != CSA::STRIDE64) {
      return false;
    }
    base                           = &memAddr->getOperand(2);
    const MachineOperand &strideOp = memAddr->getOperand(3);
    unsigned opcodeSize            = TII->getLicSize(MI->getOpcode()) / 8;
    if (!strideOp.isImm()) {
      reportFailure("stride is not constant 1");
      LLVM_DEBUG(dbgs() <<
                 "Stride is not an immediate, cannot compute stride\n");
      return false;
    } else if (strideOp.getImm() % opcodeSize) {
      reportFailure("stride is not constant 1");
      LLVM_DEBUG(dbgs() << "Stride " << strideOp.getImm()
                 << " is not a multiple of opcode size\n");
      return false;
    }
    stride = strideOp.getImm() / opcodeSize;

    // The STRIDE's stream parameter defines the stream.
    // TODO: assert that we use the seq predecessor output.
    stream = getDefinition(memAddr->getOperand(1));
    return true;
  };

  auto genericOpcode  = TII->getGenericOpcode(MI->getOpcode());
  switch (genericOpcode) {
  case CSA::Generic::LD:
  case CSA::Generic::ST: {
    // The address here must be a STRIDE.
    bool isLoad           = MI->mayLoad();
    if (!matchesStridePattern(MI->getOperand(isLoad ? 2 : 1)))
      return nullptr;
    memOrder = &MI->getOperand(3);
    inOrder  = &MI->getOperand(4);
    outOrder = &MI->getOperand(isLoad ? 1 : 0);
    value    = &MI->getOperand(isLoad ? 0 : 2);
    break;
  }
  case CSA::Generic::LDD:
  case CSA::Generic::STD: {
    // LDD instructions are a bit of a mixed bag: they can act like a LD
    // instruction with a fixed displacement, or they can act like an LDX with
    // a pre-multiplied stride.
    bool isLoad   = MI->mayLoad();
    auto &baseOp  = MI->getOperand(isLoad ? 2 : 1);
    auto &indexOp = MI->getOperand(isLoad ? 3 : 2);
    if (indexOp.isImm() && baseOp.isReg() && matchesStridePattern(baseOp)) {
      // This is a LDD (STRIDE), imm_displacement. We need to adjust the base
      // computed by matchesStridePattern to include the displacement. Check to
      // see if there is an add we can undo.
      const MachineInstr *baseDef = getDefinition(*base);
      if (baseDef && baseDef->getOpcode() == CSA::ADD64 &&
          baseDef->getOperand(2).isImm() &&
          baseDef->getOperand(2).getImm() == -indexOp.getImm()) {
        base = &baseDef->getOperand(1);
      } else {
        MachineInstrBuilder builder = BuildMI(*MI->getParent(), MI,
          MI->getDebugLoc(), TII->get(CSA::ADD64),
          LMFI->allocateLIC(&CSA::CI64RegClass));
        builder.setMIFlag(MachineInstr::NonSequential);
        builder.add(*base);
        builder.add(indexOp);
        base = &builder->getOperand(0);
      }
      memOrder = &MI->getOperand(4);
      inOrder  = &MI->getOperand(5);
      outOrder = &MI->getOperand(isLoad ? 1 : 0);
      value    = &MI->getOperand(isLoad ? 0 : 3);
      break;
    }

    // Fall through to handling this like a LDX.
    LLVM_FALLTHROUGH;
  }
  case CSA::Generic::LDX:
  case CSA::Generic::STX: {
    bool isLoad   = MI->mayLoad();
    auto &baseOp  = MI->getOperand(isLoad ? 2 : 1);
    auto &indexOp = MI->getOperand(isLoad ? 3 : 2);
    if (baseOp.isImm() || indexOp.isImm())
      return nullptr;

    // The base address needs to be repeated.
    MachineInstr *memBase  = getDefinition(baseOp);
    MachineInstr *memIndex = getDefinition(indexOp);
    if (!memBase)
      return nullptr;
    auto repeat_result = mirmatch::match(repeated_pat, memBase);
    if (!repeat_result) {
      return nullptr;
    }

    // The stream controls the base REPEAT--they should be the same
    // instruction.
    stream = MRI->getVRegDef(repeat_result.reg(SEQ_LAST));
    if (stream != memIndex) {
      return nullptr;
    }

    switch (memIndex->getOpcode()) {
    case CSA::SEQOTNE64:
    case CSA::SEQOTLTS64:
    case CSA::SEQOTLTU64:
    case CSA::SEQOTLES64:
    case CSA::SEQOTLEU64:
      break; // These are the valid ones.
    default:
      LLVM_DEBUG(dbgs() <<
                 "Candidate indexed memory store failed to have valid "
                 "stream parameter. It may yet be valid.\n");
      LLVM_DEBUG(MI->print(dbgs()));
      LLVM_DEBUG(dbgs() << "Failed operator: ");
      LLVM_DEBUG(memIndex->print(dbgs()));
      return nullptr;
    }

    base                           = &memBase->getOperand(2);
    baseUsesStream                 = true;
    const MachineOperand &strideOp = memIndex->getOperand(6);
    if (!strideOp.isImm()) {
      reportFailure("stride is not constant 1");
      LLVM_DEBUG(dbgs() << "Candidate instruction has non-constant stride.\n");
      return nullptr;
    }
    stride = strideOp.getImm();
    if (genericOpcode != CSA::Generic::LDX &&
        genericOpcode != CSA::Generic::STX) {
      unsigned opcodeSize = TII->getLicSize(MI->getOpcode()) / 8;
      if (stride % opcodeSize) {
        reportFailure("stride is not constant 1");
        LLVM_DEBUG(dbgs() << "Candidate instruction has improper stride.\n");
        return nullptr;
      }
      stride /= opcodeSize;
    }
    memOrder = &MI->getOperand(4);
    inOrder  = &MI->getOperand(5);
    outOrder = &MI->getOperand(isLoad ? 1 : 0);
    value    = &MI->getOperand(isLoad ? 0 : 3);
    break;
  }
  default:
    return nullptr;
  }

  LLVM_DEBUG(dbgs() <<
             "Identified candidate for streaming memory conversion: ");
  LLVM_DEBUG(MI->print(dbgs()));
  LLVM_DEBUG(dbgs() << "Base: " << *base << "; stride: " << stride
             << "; controlling stream: ");
  LLVM_DEBUG(stream->print(dbgs()));
  CSAInstBuilder builder(*TII);
  builder.setInsertionPoint(MI);

  // Verify that the memory orders are properly constrained by the stream.
  const bool inIsIgn = inOrder->isReg() && inOrder->getReg() == CSA::IGN;
  const MachineOperand *realInSource = nullptr;
  if (!inIsIgn) {
    MachineInstr *const inSource = getDefinition(*inOrder);
    if (!inSource) {
      reportFailure("memory ordering tokens are not loop-invariant");
      LLVM_DEBUG(dbgs() << "Conversion failed due to bad in memory order.\n");
      return nullptr;
    }
    auto mem_result = mirmatch::match(repeated_pat, inSource);
    if (!mem_result || MRI->getVRegDef(mem_result.reg(SEQ_LAST)) != stream) {
      reportFailure("memory ordering tokens are not loop-invariant");
      LLVM_DEBUG(dbgs() << "Conversion failed due to bad in memory order.\n");
      return nullptr;
    }
    realInSource = &inSource->getOperand(2);
  } else {
    realInSource = inOrder;
  }

  const bool outIsIgn   = outOrder->isReg() && outOrder->getReg() == CSA::IGN;
  MachineInstr *outSink = nullptr;
  const MachineOperand *realOutSink = nullptr;
  if (!outIsIgn) {
    outSink = getSingleUse(*outOrder);
    if (!outSink ||
        TII->getGenericOpcode(outSink->getOpcode()) != CSA::Generic::FILTER) {
      reportFailure("memory ordering tokens are not loop-invariant");
      LLVM_DEBUG(dbgs()
            << "Conversion failed because out memory order is not a switch.\n");
      return nullptr;
    }

    // The output memory order should be a switch that ignores the signal unless
    // it's the last iteration of the stream.
    MachineInstr *sinkControl = getDefinition(outSink->getOperand(1));
    if (!sinkControl) {
      reportFailure("memory ordering tokens are not loop-invariant");
      LLVM_DEBUG(dbgs() << "Cannot found the definition of the output order switch");
      return nullptr;
    }

    // TODO: check that we are using the last output of the stream.
    realOutSink = &outSink->getOperand(0);
  } else {
    realOutSink = outOrder;
  }

  // Compute the length of the stream from the stream parameter.
  const MachineOperand &seqStart = stream->getOperand(4);
  const MachineOperand &seqEnd   = stream->getOperand(5);
  const MachineOperand &seqStep  = stream->getOperand(6);
  if (!seqStep.isImm()) {
    LLVM_DEBUG(dbgs() << "Sequence step is not an immediate\n");
    return nullptr;
  }

  bool isEqual = false;
  switch (TII->getGenericOpcode(stream->getOpcode())) {
  case CSA::Generic::SEQOTNE:
  case CSA::Generic::SEQOTLT:
    isEqual = false;
    break;
  case CSA::Generic::SEQOTLE:
    isEqual = true;
    break;
  default:
    LLVM_DEBUG(dbgs() << "Stream operand is of unknown form.\n");
    return nullptr;
  }
  const MachineOp length =
    getLength(seqStart, seqEnd, isEqual, seqStep.getImm(), true, stream);
  if (!length) {
    LLVM_DEBUG(dbgs() << "Stream operand is of unknown form.\n");
    return nullptr;
  }

  if (baseUsesStream) {
    if (seqStep.getImm() < 0) {
      LLVM_DEBUG(dbgs() <<
                 "Base using stream needs to have an incrementing step\n");
      return nullptr;
    }
    if (!isZero(seqStart)) {
      unsigned loadBase  = LMFI->allocateLIC(&CSA::CI64RegClass);
      if (seqStart.isReg()) {
        LMFI->setLICGroup(loadBase, LMFI->getLICGroup(seqStart.getReg()));
      }
      auto baseForStream = builder.makeInstruction(
        CSA::SLADD64, OpRegDef(loadBase), seqStart,
        OpImm(countTrailingZeros(TII->getLicSize(MI->getOpcode()) / 8)), *base);
      base = &baseForStream->getOperand(0);
    }
  }

  MachineOptimizationRemark R(DEBUG_TYPE, "StreamingMemory",
      MI->getDebugLoc(), MI->getParent());
  ORE->emit(R << "converted to streaming memory reference");

  LLVM_DEBUG(dbgs() <<
             "No reason to disqualify the memory operation found, "
             "converting\n");

  // Actually build the new instruction now.
  unsigned opcode = TII->adjustOpcode(
    MI->getOpcode(), MI->mayLoad() ? CSA::Generic::SLD : CSA::Generic::SST);
  MachineInstr *newInst = builder.makeInstruction(
    opcode, OpIf(MI->mayLoad(), OpDef(*value)), // Value (for load)
    *realOutSink,                               // Output memory order
    OpUse(*base),                               // Address
    length,                                     // Length
    OpImm(stride),                              // Stride
    OpIf(!MI->mayLoad(), OpUse(*value)),        // Value (for store)
    *memOrder,                                  // Memory ordering
    *realInSource);                             // Input memory order

  if (MI->getFlag(MachineInstr::RasReplayable)) {
      newInst->setFlag(MachineInstr::RasReplayable);
  }

  // Delete the old instruction. Also delete the old output switch if needed,
  // since we added a second definition of its input. Dead instruction
  // elimination should handle the rest.
  to_delete.push_back(MI);
  if (!outIsIgn)
    to_delete.push_back(outSink);

  return newInst;
}

void CSAStreamingMemoryConversionPass::formWideOps(
    SmallVectorImpl<MachineInstr *> &insts) {
  unsigned count = insts.size();
  for (unsigned i = 0; i < count; i++) {
    auto op1 = insts[i];
    if (!op1) continue;
    // At the moment, we can only coalesce SLD operations into SLDX2.
    if (TII->getGenericOpcode(op1->getOpcode()) != CSA::Generic::SLD)
      continue;

    for (unsigned j = i + 1; j < count; j++) {
      auto op2 = insts[j];
      if (!op2 || op1->getOpcode() != op2->getOpcode())
        continue;

      // The length, stride, input, and memlevel operands must be the same.
      bool legal = true;
      for (unsigned op = 3; op <= 6; op++) {
        if (!op1->getOperand(op).isIdenticalTo(op2->getOperand(op)))
          legal = false;
      }
      if (!legal)
        continue;

      // The stride must additionally be 2 for both.
      const MachineOperand &strideOp = op1->getOperand(4);
      if (!strideOp.isImm() || strideOp.getImm() != 2)
        continue;

      // The bases must be offset by sizeof(T).
      auto isBaseOffsetBySize = [=](const MachineInstr *first,
          const MachineInstr *second) {
        const MachineInstr *base = getDefinition(second->getOperand(2));
        return base && base->getOpcode() == CSA::ADD64 &&
          base->getOperand(1).isIdenticalTo(first->getOperand(2)) &&
          base->getOperand(2).isImm() &&
          base->getOperand(2).getImm() == TII->getLicSize(first->getOpcode()) / 8;
      };
      MachineInstr *first, *second;
      if (isBaseOffsetBySize(op1, op2)) {
        first = op1; second = op2;
      } else if (isBaseOffsetBySize(op2, op1)) {
        first = op2; second = op1;
      } else {
        continue;
      }

      // Now check to see that the output memory operands go to the same ALL
      // chain or are both %ign.
      auto getTargetUse = [&](const MachineOperand *MO) {
        const MachineInstr *use;
        while ((use = getSingleUse(*MO)) && use->getOpcode() == CSA::ALL0)
          MO = &use->getOperand(0);
        return MO->getParent();
      };
      if (!first->getOperand(1).isReg() || !second->getOperand(1).isReg())
        continue;
      const bool bothOutordsIgn = first->getOperand(1).getReg() == CSA::IGN &&
                                  second->getOperand(1).getReg() == CSA::IGN;
      if (!bothOutordsIgn && getTargetUse(&first->getOperand(1)) !=
                               getTargetUse(&second->getOperand(1)))
        continue;

      // Now we know that we can combine these two streaming loads into a single
      // wide streaming load.
      MachineOptimizationRemark R(DEBUG_TYPE, "StreamingMemory",
        first->getDebugLoc(), first->getParent());
      ORE->emit(R << "converted to wide streaming memory reference");

      unsigned newOpcode = TII->adjustOpcode(first->getOpcode(),
          CSA::Generic::SLDX2);
      unsigned newLenReg = LMFI->allocateLIC(&CSA::CI64RegClass);
      MachineInstrBuilder newLen = BuildMI(*first->getParent(), first,
        first->getDebugLoc(), TII->get(CSA::ADD64), newLenReg);
      newLen.setMIFlag(MachineInstr::NonSequential);
      newLen.add(first->getOperand(3));
      newLen.add(second->getOperand(3));

      MachineInstrBuilder builder = BuildMI(*first->getParent(), first,
        first->getDebugLoc(), TII->get(newOpcode));
      builder.setMIFlag(MachineInstr::NonSequential);
      builder.add(first->getOperand(0)); // Value 1
      builder.add(second->getOperand(0)); // Value 2
      builder.add(first->getOperand(1)); // Out memory order
      builder.add(first->getOperand(2)); // Base (comes specifically from first)
      builder.addReg(newLenReg); // Length
      builder.addImm(1); // Stride
      builder.add(first->getOperand(5)); // Memory level
      builder.add(first->getOperand(6)); // In memory order.

      // Replace the uses of all of the old second memory operands with the
      // first one if they are not already both %ign.
      if (!bothOutordsIgn)
        MRI->replaceRegWith(second->getOperand(1).getReg(),
                            first->getOperand(1).getReg());

      // Delete the old streaming loads.
      first->eraseFromParent();
      second->eraseFromParent();
      op1 = insts[i] = nullptr;
      op2 = insts[j] = nullptr;

      // Stop trying to merge with different instructions
      break;
    }
  }
}
