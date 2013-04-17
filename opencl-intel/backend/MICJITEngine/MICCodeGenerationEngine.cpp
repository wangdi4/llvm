//===-- MICCodeGenerationEngine.cpp - Drive MIC JIT code gen ----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains implementation for driving JIT code generation of full
// module. Used for MIC.
//
//===----------------------------------------------------------------------===//

#include "llvm/Type.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/PiggyBackAnalysis.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "MICJITEngine/include/MICCodeGenerationEngine.h"
#include "MICJITEngine/include/ModuleJITHolder.h"
#include "passes/OpenCLAliasAnalysisSupport/OpenCLAliasAnalysis.h"
#include <cassert>
#include <cstdio>
#include <memory>
#include <iostream>
#include <sstream>

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
  EngineBuilder EB(Mod);
  EB.setEngineKind(EngineKind::JIT);
  EB.setErrorStr(ErrorStr);
  EB.setRelocationModel(RM);
  EB.setCodeModel(CM);
  EB.create();
  return EB.selectTarget(Triple(Mod->getTargetTriple()), MArch, MCPU, MAttrs);
}

MICCodeGenerationEngine::MICCodeGenerationEngine(TargetMachine &tm,
    CodeGenOpt::Level optlvl, const IFunctionAddressResolver* resolver) :
    TM(tm), optLevel(optlvl), Resolver(resolver) {}

MICCodeGenerationEngine::~MICCodeGenerationEngine() {}

size_t MICCodeGenerationEngine::sizeOf(Type* t) const{
  assert(t && "type is null");
  return(size_t)TM.getDataLayout()->getTypeSizeInBits(t);
}

const ModuleJITHolder* MICCodeGenerationEngine::getModuleHolder(
  llvm::Module& mod, const std::string& outAsmFileName) const {

  PassManager PM;
  std::string PrintFilename(outAsmFileName);

  // if module has a neuter name, change it to a meaningful one
  // Modules that come from full blown opencl programs are usually
  // named "Program"
  if ((getenv("DUMPIR") || getenv("DUMPASM")) && PrintFilename.empty()) {
    if (0 == mod.getModuleIdentifier().compare("Program")) {
      static int moduleNameCount = 0;
      // get process name
      const char *name = getenv("_");
      // find the base name by searching for the last '/' in the name
      if (name != NULL) {
        const char *p = name;
        while (*p) {
          if (*p == '/')
            name = p+1;
          p++;
        }
      }
      // if still no meaningful name just use "Program" as module name
      if (name == NULL || *name == 0)
        name = "Program";

      // if a process generates more than one module use sequential number to
      // distinguish them.
      std::string moduleName(name);
      if (moduleNameCount == 0)
        moduleNameCount = 1;
      else {
        std::stringstream moduleNameBuilder;
        moduleNameBuilder << name << (moduleNameCount++);
        moduleName = moduleNameBuilder.str();
      }
      mod.setModuleIdentifier(moduleName);
    }

    if (getenv("DUMPASM")) {
      PrintFilename = mod.getModuleIdentifier();
      PrintFilename += ".s";
    }

    if (getenv("DUMPIR")) {
      // Create the output file.
      std::string IRFileName (mod.getModuleIdentifier());
      IRFileName += ".ll";
      std::string ErrorInfo;
      raw_fd_ostream IRFD(IRFileName.c_str(), ErrorInfo,
                  raw_fd_ostream::F_Binary);
      mod.print(IRFD, 0);
    }
  }

  // if the user asked to inject an IR file parse it and pass the new
  // module to code generation
  std::auto_ptr<Module> M;
  char *useir_name = getenv("USEIR");
  if (useir_name != NULL) {
    SMDiagnostic Err;
    M.reset(ParseIRFile(useir_name, Err, mod.getContext()));
    if (M.get() == 0) {
      errs() << "Can't open injected file defined by USERIR " << useir_name
          << "\n";
      return 0;
    }
  }

  Module& local_mod = useir_name ? *M.get() : mod;
  if (useir_name)
    printf("Injected %s\n", useir_name);

  // Add the DataLayout from the target machine, if it exists, or the module.
  if (const DataLayout *DL = TM.getDataLayout())
    PM.add(new DataLayout(*DL));
  else
    PM.add(new DataLayout(&local_mod));

  // This pointer will *magically* get updated during PM.run()
  ModuleJITHolder* MJH = 0;

  {
    // Open the file.
    FILE* OutF = 0;
    bool genAsm = !PrintFilename.empty();
    std::string error;
    OwningPtr<tool_output_file> Out(genAsm ?
        new tool_output_file(PrintFilename.data(),error, 0) : 0);
    if (genAsm && error.empty())
      OutF = fopen(PrintFilename.data(), "w");

    // Override default to generate verbose assembly.
    if (OutF)
      TM.setAsmVerbosityDefault(true);

    // need to do switch lowering before code generation
    PM.add(createLowerSwitchPass());

    // FP_CONTRACT defined in module
    // Exclude FMA instructions when FP_CONTRACT is disabled
    bool IsContractionsAllowed = local_mod.getNamedMetadata("opencl.enable.FP_CONTRACT");

    std::auto_ptr<OpenCLAliasAnalysis> openCLAA(new OpenCLAliasAnalysis());

    // Add the piggy-back to communicate with the LLVM->PIL converter
    //TODO: add support for printing assermbly, PIL, etc.
    PM.add(new PiggyBackAnalysis(&MJH, Resolver, OutF, IsContractionsAllowed, openCLAA.get()));

    // Ask the target to add backend passes as necessary.
    LLVMTargetMachine &LTM = static_cast<LLVMTargetMachine&>(TM);

    PM.add(openCLAA.release());

    formatted_raw_ostream FOS;
    if (LTM.addPassesToEmitFile(PM, FOS, TargetMachine::CGFT_AssemblyFile, false, 0, 0, true)) {
      errs() << "target does not support generation of this file type!\n";
      return 0;
    }

    PM.run(local_mod);

    if (OutF)
    {
      fflush(OutF);
      if (OutF != stderr && OutF != stdout)
        fclose(OutF);
    }

    // If we reached this far, keep the file
    if (Out)
      Out->keep();

    // if we use an injected module we need to transform the jit holder
    // so the original module functions point into the jit buffer
    if (useir_name) {
      ModuleJITHolder* NMJH = new ModuleJITHolder(MJH->getJITBufferPointer(),
          (unsigned char *)MJH->getJITCodeStartPoint(),
          (unsigned)MJH->getJITCodeSize(),
          (unsigned)MJH->getJITCodeAlignment());

      for(llvm::KernelMap::const_iterator
          it = MJH->getKernelMap().begin();
          it != MJH->getKernelMap().end();
          it++)
      {
        size_t size = it->second.size;
        size_t offset = it->second.offset;
        const Function *F0 = it->first;
        const Function *F1 = mod.getFunction(F0->getName());
        NMJH->addKernel(F1, offset, size);
      }
      MJH = NMJH;
    }
  }

  assert (MJH && "Compilation resulted with NULL Module JIT Holder");
  return MJH;

}
