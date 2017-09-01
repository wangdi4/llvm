//===-- Nios2MCTargetDesc.cpp - Nios2 Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Nios2 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "Nios2MCTargetDesc.h"
#include "InstPrinter/Nios2InstPrinter.h"
#include "Nios2MCAsmInfo.h"
#include "Nios2TargetStreamer.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "Nios2GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Nios2GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Nios2GenRegisterInfo.inc"

/// Select the Nios2 Architecture Feature for the given triple and cpu name.
/// The function will be called at command 'llvm-objdump -d' for Nios2 elf input.
static StringRef selectNios2ArchFeature(const Triple &TT, StringRef CPU) {
  std::string Nios2ArchFeature;
  if (CPU.empty() || CPU == "generic") {
    if (TT.getArch() == Triple::nios2) {
      if (CPU.empty() || CPU == "nios2r2") {
        Nios2ArchFeature = "+nios2r2";
      }
      else {
        if (CPU == "nios2r1") {
          Nios2ArchFeature = "+nios2r1";
        }
      }
    }
  }
  return Nios2ArchFeature;
}

static MCInstrInfo *createNios2MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitNios2MCInstrInfo(X); // defined in Nios2GenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createNios2MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitNios2MCRegisterInfo(X, Nios2::R15); // defined in Nios2GenRegisterInfo.inc
  return X;
}

static MCSubtargetInfo *createNios2MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  std::string ArchFS = selectNios2ArchFeature(TT,CPU);
  if (!FS.empty()) {
    if (!ArchFS.empty())
      ArchFS = ArchFS + "," + FS.str();
    else
      ArchFS = FS;
  }
  return createNios2MCSubtargetInfoImpl(TT, CPU, ArchFS);
// createNios2MCSubtargetInfoImpl defined in Nios2GenSubtargetInfo.inc
}

static MCAsmInfo *createNios2MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT) {
  MCAsmInfo *MAI = new Nios2MCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(Nios2::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createNios2MCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new Nios2InstPrinter(MAI, MII, MRI);
}

namespace {

class Nios2MCInstrAnalysis : public MCInstrAnalysis {
public:
  Nios2MCInstrAnalysis(const MCInstrInfo *Info) : MCInstrAnalysis(Info) {}
};
}

static MCInstrAnalysis *createNios2MCInstrAnalysis(const MCInstrInfo *Info) {
  return new Nios2MCInstrAnalysis(Info);
}

static MCStreamer *createMCStreamer(const Triple &TT, MCContext &Context, 
                                    MCAsmBackend &MAB, raw_pwrite_stream &OS, 
                                    MCCodeEmitter *Emitter, bool RelaxAll) {
  return createELFStreamer(Context, MAB, OS, Emitter, RelaxAll);
}

static MCTargetStreamer *createNios2AsmTargetStreamer(MCStreamer &S,
                                                     formatted_raw_ostream &OS,
                                                     MCInstPrinter *InstPrint,
                                                     bool isVerboseAsm) {
  return new Nios2TargetAsmStreamer(S, OS);
}

extern "C" void LLVMInitializeNios2TargetMC() {
  for (Target *T : {&getTheNios2Target()}) {
    // Register the MC asm info.
    RegisterMCAsmInfoFn X(*T, createNios2MCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createNios2MCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createNios2MCRegisterInfo);

     // Register the elf streamer.
    TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);

    // Register the asm target streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createNios2AsmTargetStreamer);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T,
	                                        createNios2MCSubtargetInfo);
    // Register the MC instruction analyzer.
    TargetRegistry::RegisterMCInstrAnalysis(*T, createNios2MCInstrAnalysis);
    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T,
	                                      createNios2MCInstPrinter);
  }

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheNios2Target(),
                                        createNios2MCCodeEmitterEL);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(getTheNios2Target(),
                                       createNios2AsmBackendEL32);
}
