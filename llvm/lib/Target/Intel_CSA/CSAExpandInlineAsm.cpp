//===- CSAExpandInlineAsm.cpp - CSA Inline Assembly Expansion --*- C++ -*--===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements an interesting feature of the CSA Target: expansion of
// INLINEASM MachineInstrs into functional MachineInstrs.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetMachine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "csa-expand-asm"
#define PASS_NAME "CSA: Inline Assembly Expansion"

static cl::opt<bool> EnableExpandInlineAsm(
  "csa-expand-inline-asm", cl::Hidden,
  cl::desc("CSA Specific: Parse and expand inline assembly into MachineInstrs"),
  cl::init(true));

STATISTIC(NumInlineAsmExpansions, "Number of asm()s expanded into MIs");
STATISTIC(NumInlineAsmInstrs,
          "Number of machine instructions resulting from asm()s");

namespace {

/* This is a private MCStreamer for purposes of recovering parsed MCInsts. It
 * started off life as a copy of MCNullStreamer. */
class MCInstStreamer : public MCStreamer {
private:
  std::vector<MCInst> parsedInsts;

public:
  MCInstStreamer(MCContext &Context) : MCStreamer(Context) {}

  /// @name MCStreamer Interface
  /// @{

  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override {
    return true;
  }

  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        unsigned ByteAlignment) override {}
  void emitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, unsigned ByteAlignment = 0,
                    SMLoc Loc = SMLoc()) override {}
  void emitGPRel32Value(const MCExpr *Value) override {}

  void emitInstruction(const MCInst &Inst,
                       const MCSubtargetInfo &STI) override {
    // Just stash the MCInst for retrieval later. The list of parsedInsts
    // should correspond to only a single INLINEASM MI.
    parsedInsts.push_back(Inst);
  }

  std::vector<MCInst> getMCInsts(void) { return parsedInsts; }
  unsigned numMCInsts(void) { return parsedInsts.size(); }
};

class CSAExpandInlineAsm : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &MF) override;
  const TargetMachine *TM;
  const CSASubtarget *STI;
  const MCInstrInfo *MII;
  const CSAInstrInfo *TII;

public:
  static char ID;
  CSAExpandInlineAsm() : MachineFunctionPass(ID) {
    initializeCSAExpandInlineAsmPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override {
    return PASS_NAME;
  }

private:
  bool expandInlineAsm(MachineInstr *MI);
  // Some helper functions used by expandInlineAsm:
  std::unique_ptr<MemoryBuffer> getAsmStringBuffer(const MachineInstr *MI);
  MachineOperand &getDollarAsmOperand(MachineInstr *MI, unsigned dollarNum);
  void buildMachineInstrFromMCInst(MachineInstr *MI, MCInst *parsedInst);
};
} // namespace

char CSAExpandInlineAsm::ID = 0;

INITIALIZE_PASS(CSAExpandInlineAsm, DEBUG_TYPE, PASS_NAME, false, false)

MachineFunctionPass *llvm::createCSAExpandInlineAsmPass() {
  return new CSAExpandInlineAsm();
}

bool CSAExpandInlineAsm::runOnMachineFunction(MachineFunction &MF) {

  STI = &MF.getSubtarget<CSASubtarget>();
  TM  = &MF.getTarget();
  MII = TM->getTarget().createMCInstrInfo();
  TII = static_cast<const CSAInstrInfo *>(MF.getSubtarget().getInstrInfo());

  std::vector<MachineInstr *> toRemove;

  // Loop through the basic blocks. We'll be looking for INLINEASM pseudo MIs.
  for (MachineBasicBlock &MBB : make_range(MF.rbegin(), MF.rend())) {
    for (MachineInstr &MI : MBB) {
      if (MI.isInlineAsm()) {
        if (EnableExpandInlineAsm && expandInlineAsm(&MI))
          toRemove.push_back(&MI);
        else
          LLVM_DEBUG(
            errs() << "Found inline assembly, but expansion is disabled.\n");
      }
    }
  }

  // We didn't find anything that we could expand.
  if (toRemove.size() == 0)
    return false;

  // Remove the expanded INLINEASM instructions.
  for (MachineInstr *asmMI : toRemove)
    asmMI->eraseFromParent();

  return true;
}

/* Convenience function to fetch the assembly text as a MemoryBuffer from an
 * inlineAsm MachineInstr. */
std::unique_ptr<MemoryBuffer>
CSAExpandInlineAsm::getAsmStringBuffer(const MachineInstr *MI) {
  assert(MI->isInlineAsm() && "Expected an inline asm MachineInstr");

  const MachineOperand asmOp = MI->getOperand(InlineAsm::MIOp_AsmString);
  assert(asmOp.isSymbol() && "Couldn't find MI asmstring");

  const char *AsmStr = asmOp.getSymbolName();
  StringRef Str(AsmStr);
  return MemoryBuffer::getMemBufferCopy(Str, "<CSA inline asm>");
}

/* Attempt to parse and create one or more instructions from an inline assembly
 * MachineInstr. */
bool CSAExpandInlineAsm::expandInlineAsm(MachineInstr *MI) {
  assert(MI != nullptr);
  assert(MI->isInlineAsm());
  assert(MI->getParent() != nullptr);
  assert(MI->getParent()->getParent() != nullptr);

  MachineFunction *MF = MI->getParent()->getParent();

  // Retrieve the assembly string as a MemoryBuffer.
  std::unique_ptr<MemoryBuffer> Buffer = getAsmStringBuffer(MI);

  // Much of this code for firing up the Parser is taken from
  // AsmPrinterInlineAsm, since normally the parsing happens all the way
  // back in the AsmPrinter.

  // Tell SrcMgr about this buffer. It takes ownership of the buffer.
  SourceMgr SrcMgr;
  SrcMgr.AddNewSourceBuffer(std::move(Buffer), SMLoc());

  // Create a special "InstStreamer" MCStreamer for use with the parsing
  // infrastructure. Instead of emitting instructions to an assembly text
  // stream or object, InstStreamer allows us to recover the intermediate
  // MCInsts, which will be used to reconstruct MachineInstrs.
  MCContext &OutContext = MF->getContext();
  MCInstStreamer InstStreamer(OutContext);
  InstStreamer.InitSections(false);

  // Create a parser, which knows generally how to parse assembly and send
  // results to an MCStreamer.
  const MCAsmInfo *MAI = TM->getMCAsmInfo();
  std::unique_ptr<MCAsmParser> Parser(
    createMCAsmParser(SrcMgr, OutContext, InstStreamer, *MAI));

  // Create an AsmParser, which teaches the parser to understand our
  // particular assembly instructions. Hook it up to the parser.
  std::unique_ptr<MCTargetAsmParser> TAP(TM->getTarget().createMCAsmParser(
    *STI, *Parser, *MII, TM->Options.MCOptions));
  assert(TAP && "An MCTargetAsmParser is required for parsing MachineInstrs!");
  Parser->setAssemblerDialect(MI->getInlineAsmDialect());
  Parser->setTargetParser(*TAP.get());

  LLVM_DEBUG(errs() << "Attempting to parse INLINEASM MI:\n" << *MI);

  // We have a parser!
  bool Res = Parser->Run(/*NoInitialTextSection*/ true,
                         /*NoFinalize*/ true);

  // Reading some code, it looks like Run() returns true if there was an error.
  // Complain loudly about this.
  if (Res) {
    errs() << "INLINEASM early parsing failed: passing through.\n";
    return false;
  }

  NumInlineAsmExpansions++;
  LLVM_DEBUG(errs() << "\t-> parsing resulted in " << InstStreamer.numMCInsts()
             << " MCInsts.\n");

  // Finally, get the parsed instructions from the fake streamer and use them
  // to create some new MachineInstrs.
  for (MCInst theInst : InstStreamer.getMCInsts()) {
    NumInlineAsmInstrs++;
    buildMachineInstrFromMCInst(MI, &theInst);
  }

  return true;
}

/* Find the MachineOperand which corresponds to a "$n" attached to an INLINEASM
 * pseudo-MachineInstr resulting from extended-style inline assembly. Much of
 * this is lifted from AsmPrinterInlineAsm, and should be kept in sync with the
 * strategy used for recovering these operands there. */
MachineOperand &CSAExpandInlineAsm::getDollarAsmOperand(MachineInstr *MI,
                                                        unsigned dollarNum) {
  unsigned OpNo = InlineAsm::MIOp_FirstOperand;

  for (; dollarNum; --dollarNum) {
    if (OpNo >= MI->getNumOperands())
      break;
    unsigned OpFlags = MI->getOperand(OpNo).getImm();
    OpNo += InlineAsm::getNumOperandRegisters(OpFlags) + 1;
  }

  assert(OpNo < MI->getNumOperands() &&
         "Failed to find $ operand for INLINEASM.");
  assert(!MI->getOperand(OpNo).isMetadata() &&
         "Unexpected metadata in INLINEASM MI.");

  // We've arrived at a consecutive pair of operands: the first one is an
  // immediate "flags", and the second one is the operand we want.
  assert(MI->getOperand(OpNo).isImm() &&
         "Didn't find expected $n flags operand");
  unsigned OpFlags = MI->getOperand(OpNo).getImm();
  ++OpNo; // Skip over the ID number.

  unsigned defOp;
  if (InlineAsm::isUseOperandTiedToDef(OpFlags, defOp)) {
    LLVM_DEBUG(errs() << "NOTE: $" << dollarNum << " is a use tied to op " <<
               defOp << "\n");
  }

  // OpNo is now the operand number of the INLINEASM MachineInstr which we
  // want to use in our expanded MachineInstr.
  return MI->getOperand(OpNo);
}

/* Given an INLINEASM MachineInst and a parsed MCInst resulting from its
 * assemly text, build a corresponding real MachineInstr. The new MachineInstr
 * is inserted into the same MachineBasicBlock that the INLINEASM is in. Note
 * that the INLINEASM isn't deleted here; runOnMachineFunction deletes them all
 * later. */
void CSAExpandInlineAsm::buildMachineInstrFromMCInst(MachineInstr *MI,
                                                     MCInst *parsedInst) {
  const MCInstrDesc &desc = MII->get(parsedInst->getOpcode());
  // Sanity check: does our description match the parsed operands?
  assert(desc.getNumOperands() == parsedInst->getNumOperands());

  // Start building an MI.
  MachineInstrBuilder builder = BuildMI(*MI->getParent(), MI, DebugLoc(), desc);

  // Loop through the parsed operands, adding a new MachineOperand to the
  // builder for every parsed MCOperand.
  for (unsigned i = 0; i < parsedInst->getNumOperands(); i++) {
    MCOperand op = parsedInst->getOperand(i);
    if (op.isReg()) {
      unsigned regNo = op.getReg();
      if (regNo >= CSA::NUM_TARGET_REGS) {
        // This is not a real register, but a reference to an operand attached
        // to the INLINEASM psuedo-instruction. Go find it and steal it.
        unsigned dollarNum  = regNo - CSA::NUM_TARGET_REGS;
        MachineOperand &mOp = getDollarAsmOperand(MI, dollarNum);
        builder.add(mOp);
      } else {
        // This is a regular register. This actually seems to be the slightly
        // trickier case, since we have to build our own MachineOperand.
        // It seems that defs are always the leading operands. E.g., if there
        // are 5 operands and NumDefs==2, then operands 0 and 1 are defs, while
        // operands 2, 3, and 4 are uses. (See the comment above
        // MCInstrDesc::getNumDefs().)
        builder.addReg(op.getReg(),
                       i < desc.getNumDefs() ? RegState::Define : 0);
      }
    } else if (op.isImm()) {
      builder.addImm(op.getImm());
    } else {
      errs() << "Don't know how to convert " << *parsedInst
             << " to a MachineInst.\n";
      assert(false && "Unknown parsed unstruction operand!");
    }

    // Also take care of adding memory operands when appropriate
    const MCOperandInfo &opinfo = desc.OpInfo[i];
    if (opinfo.OperandType == MCOI::OPERAND_MEMORY) {
      errs() << "WARNING: Found a memory instruction in an inline asm.\n";
      errs()
        << "These aren't currently ordered; please write it in C instead.\n\n";
    }
  }
}
