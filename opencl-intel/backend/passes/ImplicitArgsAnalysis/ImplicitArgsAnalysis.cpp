//===-- ImplicitArgsAnalysis.cpp - Device BE <--> PCG  --------------------==//
//
//
//===----------------------------------------------------------------------===//

#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "ImplicitArgsAnalysis.h"
namespace intel{
char ImplicitArgsAnalysis::ID = 0;

// Handle the Pass registration stuff necessary to use ImplicitArgsAnalysis's.

// Register comminucation channel between code generation driver and executor...
OCL_INITIALIZE_PASS(ImplicitArgsAnalysis, "implicit-args-analysis", "Implicit Args Analysis", false, true)

}
extern "C" {
  llvm::ImmutablePass * createImplicitArgsAnalysisPass(llvm::LLVMContext* C) {
    return new intel::ImplicitArgsAnalysis(C);
  }
}

