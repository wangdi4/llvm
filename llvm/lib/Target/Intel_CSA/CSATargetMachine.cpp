//===-- CSATargetMachine.cpp - Define TargetMachine for CSA ---------------===//
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
// Top-level implementation for the CSA target.
//
//===----------------------------------------------------------------------===//

#include "CSATargetMachine.h"
#include "CSAAsmWrapOstream.h"
#include "CSAFortranIntrinsics.h"
#include "CSAIROpt.h"
#include "CSAIntrinsicCleaner.h"
#include "CSALoopIntrinsicExpander.h"
#include "CSALowerLoopIdioms.h"
#include "CSAReassocReduc.h"
#include "CSATargetTransformInfo.h"
#include "CSAUtils.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"

using namespace llvm;

static cl::opt<int> RunCSAStatistics(
  "csa-run-statistics", cl::Hidden,
  cl::desc("CSA Specific: collect statistics for DF instructions"),
  cl::init(0));

// Helper function to build a DataLayout string
static std::string computeDataLayout() { return "e-m:e-i64:64-n32:64"; }

namespace llvm {
} // namespace llvm

extern "C" void LLVMInitializeCSATarget() {
  // Register the target.
  RegisterTargetMachine<CSATargetMachine> X(getTheCSATarget());

  // The original comment in the CSA target says this optimization
  // is placed here because it is too target-specific.
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeCSAAllocUnitPassPass(PR);
  initializeCSACreateSelfContainedGraphPass(PR);
  initializeCSACvtCFDFPassPass(PR);
  initializeCSADataflowCanonicalizationPassPass(PR);
  initializeCSADataflowVerifierPass(PR);
  initializeCSADeadInstructionElimPass(PR);
  initializeCSAExpandInlineAsmPass(PR);
  initializeCSAFortranIntrinsicsPass(PR);
  initializeCSALoopPrepPass(PR);
  initializeCSAInnerLoopPrepPass(PR);
  initializeCSALowerLoopIdiomsPass(PR);
  initializeCSAMemopOrderingPasses(PR);
  initializeCSANameLICsPassPass(PR);
  initializeCSANormalizeDebugPass(PR);
  initializeCSAOptDFPassPass(PR);
  initializeCSAReassocReducPass(PR);
  initializeCSASeqotToSeqOptimizationPass(PR);
  initializeCSAStreamingMemoryPass(PR);
  initializeControlDependenceGraphPass(PR);
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

CSATargetMachine::CSATargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL,
                                   bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {

  // Although it's still not clear from a performance point of view whether or
  // not we need 'setRequiresStructuredCFG', we're enabling it because it
  // disables certain machine-level transformations in MachineBlockPlacement.
  // At The problematic transformation which prompted us to enable this again
  // was tail merging, but this disables other transformations as well.
  setRequiresStructuredCFG(true);
  initAsmInfo();
  // setAsmVerbosityDefault(true);
}

CSATargetMachine::~CSATargetMachine() {}

const CSASubtarget *
CSATargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  StringRef CPU = !CPUAttr.hasAttribute(Attribute::None)
                      ? CPUAttr.getValueAsString()
                      : (StringRef)TargetCPU;
  StringRef FS = !FSAttr.hasAttribute(Attribute::None)
                      ? FSAttr.getValueAsString()
                      : (StringRef)TargetFS;

  SmallString<512> Key;
  Key.reserve(CPU.size() + FS.size());
  Key += CPU;
  Key += FS;

  auto &I = SubtargetMap[Key];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<CSASubtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

TargetTransformInfo
CSATargetMachine::getTargetTransformInfo(const Function &F) {
  return TargetTransformInfo(CSATTIImpl(this, F));
}

namespace {
/// CSA Code Generator Pass Configuration Options.
class CSAPassConfig : public TargetPassConfig {
public:
  CSAPassConfig(CSATargetMachine &TM, legacy::PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
    // disablePass() must be run before anybody adds the pass
    // to the pass manager, so disable all the passes early.
    disablePass(&MachineLICMID);
    disablePass(&MachineCopyPropagationID);
    // TailDuplication destroys natural loop form, don't do it for CSA.
    disablePass(&EarlyTailDuplicateID);

    // These passes don't like vregs.
    disablePass(&ShrinkWrapID);
    disablePass(&PostRASchedulerID);
    disablePass(&FuncletLayoutID);
    disablePass(&StackMapLivenessID);
    disablePass(&LiveDebugValuesID);
    disablePass(&PatchableFunctionID);
    disablePass(&PostRAMachineSinkingID);
    disablePass(&PrologEpilogCodeInserterID);

    // Register coalescing causes issues with our def-after-use nature of
    // dataflow.
    disablePass(&RegisterCoalescerID);
  }

  CSATargetMachine &getCSATargetMachine() const {
    return getTM<CSATargetMachine>();
  }

  bool addInstSelector() override {

    // Install an instruction selector.
    addPass(createCSAISelDag(getCSATargetMachine(), getOptLevel()));

    // Add the pass to expand inline assembly.
    addPass(createCSAExpandInlineAsmPass(), false, true);

    return false;
  }

  bool addPreISel() override {
    // In case if lib-bc routines were not inlined till this point,
    // force their inlining by calling always inliner pass.
    addPass(createUnskippableAlwaysInlinerLegacyPass());

    // addPass(createUnifyFunctionExitNodesPass());
    addPass(createLowerSwitchPass());
    // Add a pass to generate more candidates for reduction operations
    addPass(createCSAIRReductionOptPass());
    // Add a pass to identify and prepare inner loops for pipelinling. This
    // only happens at O1+ so as to avoid requiring excessive additional
    // analyses at O0.
    if (getOptLevel() != CodeGenOpt::None) {
      addPass(createCSALoopPrepPass());
      addPass(createCSAInnerLoopPrepPass());
    }

    // Remove any remaining intrinsics which should not go through instruction
    // selection
    addPass(createCSAIntrinsicCleanerPass());

    // Add the pass to lower memset/memmove/memcpy
    addPass(createLowerLoopIdioms());

    // Clean up any redundant instructions/control flow that come from that
    // expansion.
    addPass(createInstructionCombiningPass());
    addPass(createCFGSimplificationPass());

    // Add pass to parse annotation attributes
    addPass(createCSAParseAnnotateAttributesPass());

    // Add pass to replace alloca instructions
    addPass(createPromoteMemoryToRegisterPass(true, true));
    addPass(createCSAReplaceAllocaWithMallocPass(getCSATargetMachine()));
    addPass(createUnskippableAggressiveDCEPass());
    addPass(createUnskippableInstSimplifyLegacyPass());
    addPass(createGlobalDCEPass());

    // simplify loop has to be run last, data flow converter assume natural loop
    // format, with prehdr etc...
    addPass(createLoopSimplifyPass());

    // Add ordering edges to memops.
    addPass(createCSAMemopOrderingPass(getCSATargetMachine()));

    // Lower scratchpads.
    addPass(createCSALowerScratchpadsPass());

    // Convert loads/stores to sld/sst where possible.
    if (getOptLevel() != CodeGenOpt::None)
      addPass(createCSAStreamingMemoryConversionPass());

    return false;
  }

  void addPreRegAlloc() override {
    addPass(createControlDepenceGraph(), false);

    addPass(createCSARASReplayableLoadsDetectionPass(), false);
    addPass(createCSANameLICsPass(), false);
    addPass(createCSACvtCFDFPass(), false);

    if (RunCSAStatistics) {
      addPass(createCSAStatisticsPass(), false);
    }

    addPass(createCSAOptDFPass(), false);
    // Run dead instructions elimination, because CSAOptDFPass
    // is not careful about creating new instructions.
    // This extra uses may break SEQOT->SEQ idiom recognition.
    //
    // TODO (vzakhari 9/25/2018): look for CSASeqOpt.cpp:getTripCntForSeq
    //       usage and figure out how to clean up the tripcount computation,
    //       if it is never used (e.g. DisableMultiSeq is true).
    addPass(createCSADeadInstructionElimPass(), false);
    addPass(createCSADataflowCanonicalizationPass(), false);
    addPass(createCSASeqotToSeqOptimizationPass(), false);
    addPass(createCSAMultiSeqPass(), false);
    addPass(createCSADeadInstructionElimPass(), false);
    addPass(createCSAReassocReducPass(), false);
    addPass(createCSANormalizeDebugPass(), false);
  }

  void addOptimizedRegAlloc() override {
    // If we go back to suppoting SXU code, we should add code here to enable
    // register allocation passes only on functions that have the SXU subtarget
    // feature. But for the moment, we have none of that.
  }

  void addFastRegAlloc() override {
    // See note above in addOptimizedRegAlloc.
  }

// Last call in TargetPassConfig.cpp to disable passes.
// If we don't disable here some other passes may add after disable
  void addPostRegAlloc() override {
    addPass(createCSAAllocUnitPass(), false);
  }

  void addPreEmitPass2() override {

    // TODO: This should be running right before AsmPrinter, but the procedure
    // calls pass is causing problems with it. We should be able to move it
    // later when the call lowering is improved.
    if (csa_utils::isAlwaysDataFlowLinkageSet()) {
      addPass(createCSADataflowVerifier(), false);
    }

    if (csa_utils::isAlwaysDataFlowLinkageSet()) {
      if (csa_utils::createSCG())
        addPass(createCSACreateSelfContainedGraphPass(), false);
      else
        addPass(createCSAProcCallsPass(), false);
    }
  }

  void addIRPasses() override {
    // Do any necessary atomic expansion according to Subtarget features.
    addPass(createAtomicExpandPass());

    // Pass call onto parent
    TargetPassConfig::addIRPasses();
  }

  void addAdvancedPatternMatchingOpts() override {
    // Do Fused-Multiply-Add transformations.
    addPass(createCSAGlobalFMAPass());

    // Pass call onto parent
    TargetPassConfig::addAdvancedPatternMatchingOpts();
  }

}; // class CSAPassConfig

} // namespace

TargetPassConfig *
CSATargetMachine::createPassConfig(legacy::PassManagerBase &PM) {
  CSAPassConfig *PassConfig = new CSAPassConfig(*this, PM);
  return PassConfig;
}

void CSATargetMachine::adjustPassManager(PassManagerBuilder &PMB) {
  PMB.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
                   [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {

                     // Add the pass to convert Fortran "builtin" calls
                     PM.add(createFortranIntrinsics());

                     // Add the pass to expand loop intrinsics
                     PM.add(createSROAPass());
                     PM.add(createLoopSimplifyPass());
                     PM.add(createLICMPass());
                     PM.add(createCSALoopIntrinsicExpanderPass());
                   });
}

// This function is copied from lib/CodeGen/LLVMTargetMachine.cpp because it is
// not accessible here - feel free to update this if the original changes.
// TODO (dwoodwor 6/29/2018): It would be good if we didn't have to duplicate
//       this code but it isn't clear how that would happen without modifying
//       the interface of LLVMTargetMachine. Ideally we'd just want a virtual
//       function that we could override to change how the formatted_raw_ostream
//       passed on to the MCAsmStreamer is constructed, though this is
//       admittedly a pretty niche thing to need to do. We'll also be able to
//       get rid of all of this once we have a proper binary format, so I don't
//       think it's worth getting something into common code just for this use
//       case.
static MCContext *
addPassesToGenerateCode(LLVMTargetMachine *TM, PassManagerBase &PM,
                        bool DisableVerify, bool &WillCompleteCodeGenPipeline,
                        raw_pwrite_stream &Out, MachineModuleInfo *MMI) {
  // Targets may override createPassConfig to provide a target-specific
  // subclass.
  TargetPassConfig *PassConfig = TM->createPassConfig(PM);
  // Set PassConfig options provided by TargetMachine.
  PassConfig->setDisableVerify(DisableVerify);
  WillCompleteCodeGenPipeline = PassConfig->willCompleteCodeGenPipeline();
  PM.add(PassConfig);
  if (!MMI)
    MMI = new MachineModuleInfo(TM);
  PM.add(MMI);

  if (PassConfig->addISelPasses())
    return nullptr;
  PassConfig->addMachinePasses();
  PassConfig->setInitialized();
  if (!WillCompleteCodeGenPipeline)
    PM.add(createPrintMIRPass(Out));

  return &MMI->getContext();
}

// This function is also mostly copied from lib/CodeGen/LLVMTargetMachine.cpp.
bool CSATargetMachine::addAsmPrinterWithAsmWrapping(PassManagerBase &PM,
                                                    raw_pwrite_stream &Out,
                                                    raw_pwrite_stream *DwoOut,
                                                    CodeGenFileType FileType,
                                                    MCContext &Context) {
  if (Options.MCOptions.MCSaveTempLabels)
    Context.setAllowTemporaryLabels(false);

  const MCSubtargetInfo &STI = *getMCSubtargetInfo();
  const MCAsmInfo &MAI       = *getMCAsmInfo();
  const MCRegisterInfo &MRI  = *getMCRegisterInfo();
  const MCInstrInfo &MII     = *getMCInstrInfo();

  std::unique_ptr<MCStreamer> AsmStreamer;

  switch (FileType) {
  case CGFT_AssemblyFile: {
    MCInstPrinter *InstPrinter = getTarget().createMCInstPrinter(
      getTargetTriple(), MAI.getAssemblerDialect(), MAI, MII, MRI);

    // Create a code emitter if asked to show the encoding.
    std::unique_ptr<MCCodeEmitter> MCE;
    if (Options.MCOptions.ShowMCEncoding)
      MCE.reset(getTarget().createMCCodeEmitter(MII, MRI, Context));

    std::unique_ptr<MCAsmBackend> MAB(
      getTarget().createMCAsmBackend(STI, MRI, Options.MCOptions));
    auto FOut     = wrapStreamForCSAAsmWrapping(Out);
    MCStreamer *S = getTarget().createAsmStreamer(
      Context, std::move(FOut), Options.MCOptions.AsmVerbose,
      Options.MCOptions.MCUseDwarfDirectory, InstPrinter, std::move(MCE),
      std::move(MAB), Options.MCOptions.ShowMCInst);
    AsmStreamer.reset(S);
    break;
  }
  case CGFT_ObjectFile: {
    // This isn't going to work for CSA yet - print a warning to give the user
    // a better error message while we're here.
    errs().changeColor(raw_ostream::RED, true);
    errs() << "BINARY OUTPUT ISN'T SUPPORTED FOR CSA YET; DID YOU PASS -S?";
    errs().resetColor();
    errs() << "\n";

    // Create the code emitter for the target if it exists.  If not, .o file
    // emission fails.
    MCCodeEmitter *MCE = getTarget().createMCCodeEmitter(MII, MRI, Context);
    MCAsmBackend *MAB =
      getTarget().createMCAsmBackend(STI, MRI, Options.MCOptions);
    if (!MCE || !MAB)
      return true;

    // Don't waste memory on names of temp labels.
    Context.setUseNamesOnTempLabels(false);

    Triple T(getTargetTriple().str());
    AsmStreamer.reset(getTarget().createMCObjectStreamer(
      T, Context, std::unique_ptr<MCAsmBackend>(MAB),
      DwoOut ? MAB->createDwoObjectWriter(Out, *DwoOut)
             : MAB->createObjectWriter(Out),
      std::unique_ptr<MCCodeEmitter>(MCE), STI, Options.MCOptions.MCRelaxAll,
      Options.MCOptions.MCIncrementalLinkerCompatible,
      /*DWARFMustBeAtTheEnd*/ true));
    break;
  }
  case CGFT_Null:
    // The Null output is intended for use for performance analysis and testing,
    // not real users.
    AsmStreamer.reset(getTarget().createNullStreamer(Context));
    break;
  }

  // Create the AsmPrinter, which takes ownership of AsmStreamer if successful.
  FunctionPass *Printer =
    getTarget().createAsmPrinter(*this, std::move(AsmStreamer));
  if (!Printer)
    return true;

  PM.add(Printer);
  return false;
}

// This function is also mostly copied from lib/CodeGen/LLVMTargetMachine.cpp.
bool CSATargetMachine::addPassesToEmitFile(
  PassManagerBase &PM, raw_pwrite_stream &Out, raw_pwrite_stream *DwoOut,
  CodeGenFileType FileType, bool DisableVerify, MachineModuleInfo *MMI) {
  // Add common CodeGen passes.
  bool WillCompleteCodeGenPipeline = true;
  MCContext *Context               = addPassesToGenerateCode(
    this, PM, DisableVerify, WillCompleteCodeGenPipeline, Out, MMI);
  if (!Context)
    return true;

  if (WillCompleteCodeGenPipeline &&
      addAsmPrinterWithAsmWrapping(PM, Out, DwoOut, FileType, *Context))
    return true;

  PM.add(createFreeMachineFunctionPass());
  return false;
}

bool llvm::shouldRunDataflowPass(const MachineFunction &MF) {
  return !MF.getSubtarget<CSASubtarget>().isSequential();
}
