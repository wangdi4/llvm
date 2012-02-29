//===-- MICCodeGenerationEngine.cpp - Drive MIC JIT code gen ----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation for driving JIT code generation of full
// module. Used for MIC.
//
//===----------------------------------------------------------------------===//

#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/PiggyBackAnalysis.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "MICJITEngine/include/MICCodeGenerationEngine.h"
#include "MICJITEngine/include/ModuleJITHolder.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <cassert>
#include <cstdio>
#include <memory>

using namespace llvm;

/// selectTarget - Pick a target either via -march or by guessing the native
/// arch.  Add any CPU features specified via -mcpu or -mattr.
TargetMachine *MICCodeGenerationEngine::selectTarget(Module *Mod,
                              StringRef MArch,
                              StringRef MCPU,
                              const SmallVectorImpl<std::string>& MAttrs,
                              Reloc::Model RM,
                              CodeModel::Model CM,
                              std::string *ErrorStr) {
#if 1

return llvm::EngineBuilder::selectTarget(Mod, MArch, MCPU, MAttrs, RM, CM, ErrorStr);    
#else
    Triple TheTriple(Mod->getTargetTriple());
  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getHostTriple());

  // Adjust the triple to match what the user requested.
  const Target *TheTarget = 0;
  if (!MArch.empty()) {
    for (TargetRegistry::iterator it = TargetRegistry::begin(),
           ie = TargetRegistry::end(); it != ie; ++it) {
      if (MArch == it->getName()) {
        TheTarget = &*it;
        break;
      }
    }

    if (!TheTarget) {
      *ErrorStr = "No available targets are compatible with this -march, "
        "see -version for the available targets.\n";
      return 0;
    }

    // Adjust the triple to match (if known), otherwise stick with the
    // module/host triple.
    Triple::ArchType Type = Triple::getArchTypeForLLVMName(MArch);
    if (Type != Triple::UnknownArch)
      TheTriple.setArch(Type);
  } else {
    std::string Error;
    TheTarget = TargetRegistry::lookupTarget(TheTriple.getTriple(), Error);
    if (TheTarget == 0) {
      if (ErrorStr)
        *ErrorStr = Error;
      return 0;
    }
  }

  if (!TheTarget->hasJIT()) {
    errs() << "WARNING: This target JIT is not designed for the host you are"
           << " running.  If bad things happen, please choose a different "
           << "-march switch.\n";
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (!MAttrs.empty()) {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  // Allocate a target...
  TargetMachine *Target = TheTarget->createTargetMachine(TheTriple.getTriple(),
                                                         MCPU, FeaturesStr,
                                                         RM, CM);
  assert(Target && "Could not allocate target machine!");
  return Target;
#endif
}
#if 0
TargetMachine *
MICCodeGenerationEngine::selectTarget(Module *Mod,
                               StringRef MTriple,
                               StringRef MArch,
                               StringRef MCPU,
                               const SmallVectorImpl<std::string>& MAttrs,
                               std::string *ErrorStr) {
//  InitializeAllTargets();
//  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // If we are supposed to override the target triple, do so now.
  if (!MTriple.empty())
    Mod->setTargetTriple(Triple::normalize(MTriple));
  Triple TheTriple(Mod->getTargetTriple());
  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(MTriple);

  // Adjust the triple to match what the user requested.
  const Target *TheTarget = 0;
  if (!MArch.empty()) {
    for (TargetRegistry::iterator it = TargetRegistry::begin(),
           ie = TargetRegistry::end(); it != ie; ++it) {
      if (MArch == it->getName()) {
        TheTarget = &*it;
        break;
      }
    }

    if (!TheTarget) {
      *ErrorStr = "No available targets are compatible with this -march, "
        "see -version for the available targets.\n";
      return 0;
    }

    // Adjust the triple to match (if known), otherwise stick with the
    // module/host triple.
    Triple::ArchType Type = Triple::getArchTypeForLLVMName(MArch);
    if (Type != Triple::UnknownArch)
      TheTriple.setArch(Type);
  } else {
    std::string Error;
    TheTarget = TargetRegistry::lookupTarget(TheTriple.getTriple(), Error);
    if (TheTarget == 0) {
      if (ErrorStr)
        *ErrorStr = Error;
      return 0;
    }
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (!MCPU.empty() || !MAttrs.empty()) {
    SubtargetFeatures Features;
    Features.setCPU(MCPU);
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  // Allocate a target...
  TargetMachine *Target = 
    TheTarget->createTargetMachine(TheTriple.getTriple(), FeaturesStr);
  assert(Target && "Could not allocate target machine!");
  Target->setCodeModel(CodeModel::Small);
  return Target;
}
#endif

MICCodeGenerationEngine::MICCodeGenerationEngine(TargetMachine &tm, 
    CodeGenOpt::Level optlvl, const IFunctionAddressResolver* resolver) :
    TM(tm), optLevel(optlvl), Resolver(resolver) {}

MICCodeGenerationEngine::~MICCodeGenerationEngine() {}

size_t MICCodeGenerationEngine::sizeOf(Type* t) const{
  assert(t && "type is null");
  return(size_t)TM.getTargetData()->getTypeSizeInBits(t);
}

const ModuleJITHolder* MICCodeGenerationEngine::getModuleHolder(
  llvm::Module& mod) const {

  PassManager PM;

  if (getenv("DUMPIR"))
    mod.dump();
  
  // Add the target data from the target machine, if it exists, or the module.
  if (const TargetData *TD = TM.getTargetData())
    PM.add(new TargetData(*TD));
  else
    PM.add(new TargetData(&mod));

  // This pointer will *magically* get updated during PM.run()
  ModuleJITHolder* MJH = 0;


  // Override default to generate verbose assembly.
  TM.setAsmVerbosityDefault(true);

  {
    // Open the file.
    std::string PrintFilename("outf.s");
    std::string error;
    OwningPtr<tool_output_file> Out(new tool_output_file(PrintFilename.data(),
                                    error, 0));
    if (!error.empty()) {
      errs() << error << '\n';
      return 0;
    }

    FILE* OutF = 0;
    if (!PrintFilename.empty()) {
      OutF = fopen(PrintFilename.data(), "w");
      if (!OutF) {
        errs() << "Unable to open file " << PrintFilename << "\n";
        return 0;
      }
    }

    // need to do switch lowering before code generation
    PM.add(createLowerSwitchPass());

    // Add the piggy-back to communicate with the LLVM->PIL converter
    //TODO: add support for printing assermbly, PIL, etc.
    PM.add(new PiggyBackAnalysis(&MJH, Resolver, OutF));

    // Ask the target to add backend passes as necessary.
    CodeGenOpt::Level OLvl = CodeGenOpt::Default;
    LLVMTargetMachine &LTM = static_cast<LLVMTargetMachine&>(TM);
    MCContext *MCC = 0;
    if (LTM.addCommonCodeGenPasses(PM, OLvl, false, MCC, true)) {
      errs() << "target does not support generation of this file type!\n";
      return 0;
    }

    PM.run(mod);

    if (OutF)
      fflush(OutF);
    if (OutF != stderr && OutF != stdout)
      fclose(OutF);
    
    // If we reached this far, keep the file
    Out->keep();

  }

  assert (MJH && "Compilation resulted with NULL Module JIT Holder");
  return MJH;

}
