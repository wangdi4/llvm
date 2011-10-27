//===-- MICCodeGenerationEngine.cpp - Drive MIC JIT code gen ----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation for driving JIT code generation of full
// module. Used for MIC.
//
//===----------------------------------------------------------------------===//

#include <assert.h>
#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/SubtargetFeature.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/MICJITEngine/MICCodeGenerationEngine.h"
#include "llvm/MICJITEngine/ModuleJITHolder.h"
#include "stdlib.h"

using namespace llvm;

/// selectTarget - Pick a target either via -march or by guessing the native
/// arch.  Add any CPU features specified via -mcpu or -mattr.
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

MICCodeGenerationEngine::MICCodeGenerationEngine(TargetMachine &tm, 
                                                 CodeGenOpt::Level optlvl) :
    TM(tm), optLevel(optlvl) {}

MICCodeGenerationEngine::~MICCodeGenerationEngine() {}

size_t MICCodeGenerationEngine::sizeOf(const Type* t) const{
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

  // Override default to generate verbose assembly.
  TM.setAsmVerbosityDefault(true);

  TargetMachine::CodeGenFileType FileType = TargetMachine::CGFT_AssemblyFile;
  
  {
    // Open the file.
    std::string error;
    tool_output_file *Out = new tool_output_file("outf.s",
                                                   error, 0);
    if (!error.empty()) {
      errs() << error << '\n';
      delete Out;
      return 0;
    }
    formatted_raw_ostream FOS(Out->os());

    CodeGenOpt::Level OLvl = CodeGenOpt::Default;

    // Ask the target to add backend passes as necessary.
    if (TM.addPassesToEmitFile(PM, FOS, FileType, OLvl, false)) {
      errs() << "target does not support generation of this file type!\n";
      return 0;
    }

    PM.run(mod);

    delete Out;
  }

  const ModuleJITHolder *MJH = ModuleJITStore::instance()->retrieveModule(&mod);
  return MJH;

}
