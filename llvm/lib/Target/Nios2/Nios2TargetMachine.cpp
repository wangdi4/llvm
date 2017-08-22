//===-- Nios2TargetMachine.cpp - Define TargetMachine for Nios2 -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Nios2 target spec.
//
//===----------------------------------------------------------------------===//

#include "Nios2TargetMachine.h"
#include "Nios2.h"

#include "Nios2SEISelDAGToDAG.h"
#include "Nios2Subtarget.h"
#include "Nios2TargetObjectFile.h"

#include "Nios2RegisterBankInfo.h"
#include "Nios2CallLowering.h"
#include "Nios2InstructionSelector.h"
#include "Nios2LegalizerInfo.h"
#include "Nios2RegisterBankInfo.h"

#include "llvm/CodeGen/GlobalISel/GISelAccessor.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "nios2"

extern "C" void LLVMInitializeNios2Target() {
  // Register the target.
  RegisterTargetMachine<Nios2elTargetMachine> X(getTheNios2Target());
  auto PR = PassRegistry::getPassRegistry();
  initializeNios2LowerAllocaPass(*PR);
  initializeGlobalISel(*PR);
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options,
                                     bool isLittle) {
  std::string Ret = "";
  // There are both little and big endian nios2.
  if (isLittle)
    Ret += "e";
  else
    Ret += "E";

  Ret += "-p:32:32:32-i8:8:32-i16:16:32-n32";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(CodeModel::Model CM,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue() || CM == CodeModel::JITDefault)
    return Reloc::Static;
  return *RM;
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
Nios2TargetMachine::Nios2TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     CodeModel::Model CM, CodeGenOpt::Level OL,
                                     bool isLittle)
  //- Default is big endian
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options, isLittle), TT,
                        CPU, FS, Options, getEffectiveRelocModel(CM, RM), CM,
                        OL),
      isLittle(isLittle), TLOF(make_unique<Nios2TargetObjectFile>()),
      ABI(Nios2ABIInfo::computeTargetABI()),
      DefaultSubtarget(TT, CPU, FS, isLittle, *this) {
  // initAsmInfo will display features by llc -march=nios2 -mcpu=help on 3.7 but
  // not on 3.6
  initAsmInfo();
}

Nios2TargetMachine::~Nios2TargetMachine() {}

void Nios2elTargetMachine::anchor() { }

namespace {

struct Nios2GISelActualAccessor : public GISelAccessor {
  std::unique_ptr<CallLowering> CallLoweringInfo;
  std::unique_ptr<InstructionSelector> InstSelector;
  std::unique_ptr<LegalizerInfo> Legalizer;
  std::unique_ptr<RegisterBankInfo> RegBankInfo;

  const CallLowering *getCallLowering() const override {
    return CallLoweringInfo.get();
  }

  const InstructionSelector *getInstructionSelector() const override {
    return InstSelector.get();
  }

  const LegalizerInfo *getLegalizerInfo() const override {
    return Legalizer.get();
  }

  const RegisterBankInfo *getRegBankInfo() const override {
    return RegBankInfo.get();
  }
};
} // end anonymous namespace

Nios2elTargetMachine::Nios2elTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
    : Nios2TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}

const Nios2Subtarget *
Nios2TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = llvm::make_unique<Nios2Subtarget>(TargetTriple, CPU, FS, isLittle,
                                         *this);
    Nios2GISelActualAccessor *GISel =
        new Nios2GISelActualAccessor();
    GISel->CallLoweringInfo.reset(
        new Nios2CallLowering(*I->getTargetLowering()));
    GISel->Legalizer.reset(new Nios2LegalizerInfo());

    auto *RBI = new Nios2RegisterBankInfo(*I->getRegisterInfo());

    // FIXME: At this point, we can't rely on Subtarget having RBI.
    // It's awkward to mix passing RBI and the Subtarget; should we pass
    // TII/TRI as well?
    GISel->InstSelector.reset(new Nios2InstructionSelector(*this, *I, *RBI));

    GISel->RegBankInfo.reset(RBI);
    I->setGISelAccessor(*GISel);
  }
  return I.get();
}

namespace {
//@Nios2PassConfig {
/// Nios2 Code Generator Pass Configuration Options.
class Nios2PassConfig : public TargetPassConfig {
public:
  Nios2PassConfig(Nios2TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  Nios2TargetMachine &getNios2TargetMachine() const {
    return getTM<Nios2TargetMachine>();
  }

  const Nios2Subtarget &getNios2Subtarget() const {
    return *getNios2TargetMachine().getSubtargetImpl();
  }
  void addCodeGenPrepare() override;
  void addIRPasses() override;
  bool addInstSelector() override;
  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
  void addPreEmitPass() override;
#ifdef ENABLE_GPRESTORE
  void addPreRegAlloc() override;
#endif
};
} // namespace

TargetPassConfig *Nios2TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new Nios2PassConfig(this, PM);
}

void Nios2PassConfig::addCodeGenPrepare() {
  TargetPassConfig::addCodeGenPrepare();
  // Required to lower select to branches:
  addPass(createCodeGenPreparePass(TM));
}

void Nios2PassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
  addPass(createNios2LowerAllocaPass(&getNios2TargetMachine()));
  addPass(createAtomicExpandPass(&getNios2TargetMachine()));
}

// Install an instruction selector pass using
// the ISelDag to gen Nios2 code.
bool Nios2PassConfig::addInstSelector() {
  addPass(createNios2SEISelDag(getNios2TargetMachine(), getOptLevel()));
  return false;
}

bool Nios2PassConfig::addIRTranslator() {
  addPass(new IRTranslator());
  return false;
}

bool Nios2PassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

bool Nios2PassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool Nios2PassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

#ifdef ENABLE_GPRESTORE
void Nios2PassConfig::addPreRegAlloc() {
  if (!Nios2ReserveGP) {
    // $gp is a caller-saved register.
    addPass(createNios2EmitGPRestorePass(getNios2TargetMachine()));
  }
  return;
}
#endif

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
void Nios2PassConfig::addPreEmitPass() {
  Nios2TargetMachine &TM = getNios2TargetMachine();
  addPass(createNios2DelJmpPass(TM));
  addPass(createNios2DelaySlotFillerPass(TM));
  return;
}
