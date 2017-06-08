//===-- Nios2Subtarget.cpp - Nios2 Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Nios2 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Nios2Subtarget.h"

#include "Nios2MachineFunction.h"
#include "Nios2.h"
#include "Nios2RegisterInfo.h"

#include "Nios2TargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "nios2-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Nios2GenSubtargetInfo.inc"

static cl::opt<bool> EnableOverflowOpt
                ("nios2-enable-overflow", cl::Hidden, cl::init(false),
                 cl::desc("Use trigger overflow instructions add and sub \
                 instead of non-overflow instructions addu and subu"));

static cl::opt<bool> UseSmallSectionOpt
                ("nios2-use-small-section", cl::Hidden, cl::init(false),
                 cl::desc("Use small section. Only work when -relocation-model="
                 "static. pic always not use small section."));

static cl::opt<bool> ReserveGPOpt
                ("nios2-reserve-gp", cl::Hidden, cl::init(false),
                 cl::desc("Never allocate $gp to variable"));

static cl::opt<bool> NoCploadOpt
                ("nios2-no-cpload", cl::Hidden, cl::init(false),
                 cl::desc("No issue .cpload"));

bool Nios2ReserveGP;
bool Nios2NoCpload;

extern bool FixGlobalBaseReg;

void Nios2Subtarget::anchor() { }

Nios2Subtarget::Nios2Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, bool little, 
                             const Nios2TargetMachine &_TM) :
  // Nios2GenSubtargetInfo will display features by llc -march=nios2 -mcpu=help
  Nios2GenSubtargetInfo(TT, CPU, FS),
  IsLittle(little), TM(_TM), TargetTriple(TT), TSInfo(),
      InstrInfo(
          Nios2InstrInfo::create(initializeSubtargetDependencies(CPU, FS, TM))),
      FrameLowering(Nios2FrameLowering::create(*this)),
      TLInfo(Nios2TargetLowering::create(TM, *this)), GISel() {

  EnableOverflow = EnableOverflowOpt;
  // Set UseSmallSection.
  UseSmallSection = UseSmallSectionOpt;
  Nios2ReserveGP = ReserveGPOpt;
  Nios2NoCpload = NoCploadOpt;
#ifdef ENABLE_GPRESTORE
  if (!TM.isPositionIndependent() && !UseSmallSection && !Nios2ReserveGP)
    FixGlobalBaseReg = false;
  else
#endif
    FixGlobalBaseReg = true;
}

const CallLowering *Nios2Subtarget::getCallLowering() const {
  assert(GISel && "Access to GlobalISel APIs not set");
  return GISel->getCallLowering();
}

const InstructionSelector *Nios2Subtarget::getInstructionSelector() const {
  assert(GISel && "Access to GlobalISel APIs not set");
  return GISel->getInstructionSelector();
}

const LegalizerInfo *Nios2Subtarget::getLegalizerInfo() const {
  assert(GISel && "Access to GlobalISel APIs not set");
  return GISel->getLegalizerInfo();
}

const RegisterBankInfo *Nios2Subtarget::getRegBankInfo() const {
  assert(GISel && "Access to GlobalISel APIs not set");
  return GISel->getRegBankInfo();
}

bool Nios2Subtarget::isPositionIndependent() const {
  return TM.isPositionIndependent();
}

Nios2Subtarget &
Nios2Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                               const TargetMachine &TM) {
  if (TargetTriple.getArch() == Triple::nios2) {
    if (CPU.empty() || CPU == "generic") {
      CPU = "nios2r1";
    }
    else if (CPU == "help") {
      CPU = "";
      return *this;
    }
    else if (CPU != "nios2r2" && CPU != "nios2r1") {
      CPU = "nios2r1";
    }
  }
  else {
    errs() << "!!!Error, TargetTriple.getArch() = " << TargetTriple.getArch()
           <<  "CPU = " << CPU << "\n";
    exit(0);
  }
  
  if (CPU == "nios2r1")
    Nios2ArchVersion = Nios2r1;
  else if (CPU == "nios2r2")
    Nios2ArchVersion = Nios2r2;

  // Parse features string.
  ParseSubtargetFeatures(CPU, FS);
  // Initialize scheduling itinerary for the specified CPU.
  InstrItins = getInstrItineraryForCPU(CPU);

  return *this;
}

bool Nios2Subtarget::abiUsesSoftFloat() const {
//  return TM->Options.UseSoftFloat;
  return true;
}

const Nios2ABIInfo &Nios2Subtarget::getABI() const { return TM.getABI(); }
