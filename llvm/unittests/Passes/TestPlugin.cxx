//===- unittests/Passes/Plugins/Plugin.cxx --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
  // Microsoft issues a warning about an exception handler present
  // in its library file <xlocale>, causing Windows build to fail.
  // This customization is a temporary workaround to supress the
  // warning.
#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable:4530)
#include "llvm/Passes/PassBuilder.h"
#pragma warning (pop)
#else
#include "llvm/Passes/PassBuilder.h"
#endif
#endif // INTEL_CUSTOMIZATION
#include "llvm/Passes/PassPlugin.h"

#include "TestPlugin.h"

using namespace llvm;

struct TestModulePass : public PassInfoMixin<TestModulePass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    return PreservedAnalyses::all();
  }
};

void registerCallbacks(PassBuilder &PB) {
  PB.registerPipelineParsingCallback(
      [](StringRef Name, ModulePassManager &PM,
         ArrayRef<PassBuilder::PipelineElement> InnerPipeline) {
        if (Name == "plugin-pass") {
          PM.addPass(TestModulePass());
          return true;
        }
        return false;
      });
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK LLVM_PLUGIN_EXPORT
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, TEST_PLUGIN_NAME, TEST_PLUGIN_VERSION,
          registerCallbacks};
}
