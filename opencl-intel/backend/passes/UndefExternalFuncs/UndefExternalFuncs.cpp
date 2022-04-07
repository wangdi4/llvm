// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "UndefExternalFuncs.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/DynamicLibrary.h"

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(UndefExternalFuncs, "UndefExternalFuncs",
                          "Find undefined external functions", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_END(UndefExternalFuncs, "UndefExternalFuncs",
                        "Find undefined external functions", false, true)

char UndefExternalFuncs::ID = 0;

UndefExternalFuncs::UndefExternalFuncs() : ModulePass(ID) {
  initializeUndefExternalFuncsPass(*PassRegistry::getPassRegistry());
}

UndefExternalFuncs::UndefExternalFuncs(
    std::vector<std::string> &undefinedExternalFunctions)
    : UndefExternalFuncs() {
  m_pUndefinedExternalFunctions = &undefinedExternalFunctions;
  initializeUndefExternalFuncsPass(*PassRegistry::getPassRegistry());
}

  bool UndefExternalFuncs::runOnModule(Module &M) {
    ArrayRef<Module *> BuiltinModules = getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult().getBuiltinModules();
    m_RuntimeModules.assign(BuiltinModules.begin(), BuiltinModules.end());
    assert(!BuiltinModules.empty() && "No builtin module");

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc ) {
        continue;
      }
      if( pFunc->getNumUses() > 0 && pFunc->isDeclaration () ) {
        // Function is not defined inside module
        if ( pFunc->isIntrinsic() ) continue;
        bool found = SearchForFunction(std::string(pFunc->getName()));
        if( !found ) {
          // The external function not found in any of the runtime libraries
          // Report an error
          m_pUndefinedExternalFunctions->push_back((std::string)pFunc->getName() + " is undefined ");
        }
      }
    }

    return false;
  }

  bool UndefExternalFuncs::SearchForFunction(const std::string& name) {
    for (auto *RTLModule : m_RuntimeModules) {
      // look for the required function in all the runtime modules
      Function *pFunc = RTLModule->getFunction(name);

      // Note that due to the lazy parsing of built-in modules the function
      // body might not be materialized yet and is reported
      // as declaration in that case. (This behaviour was fixed in LLVM 3.6)
      // We report a function as found iff it is already materialized or is materialazible.
      if(pFunc != nullptr &&
        (pFunc->isMaterializable() || !pFunc->isDeclaration())) {
        return true;
      }
    }

    // TODO: Remove this and replace it with a module which represents the images library
    const unsigned int NumImageFunctions = 33;

    static const std::string ImageFunctions[NumImageFunctions] = {
      "_Z12read_imageui9image2d_tjDv2_i",
      "_Z12read_imageui9image3d_tjDv4_i",
      "_Z12read_imageui9image2d_tjDv2_f",
      "_Z12read_imageui9image3d_tjDv4_f",
      "_Z13write_imageui9image2d_tDv2_iDv4_j",
      "_Z11read_imagei9image2d_tjDv2_i",
      "_Z11read_imagei9image3d_tjDv4_i",
      "_Z11read_imagei9image2d_tjDv2_f",
      "_Z11read_imagei9image3d_tjDv4_f",
      "_Z12write_imagef9image2d_tDv2_iDv4_f",
      "_Z12write_imagei9image2d_tDv2_iDv4_i",
      "_Z11read_imagef9image2d_tjDv2_f",
      "_Z11read_imagef9image2d_tjDv2_i",
      "_Z11read_imagef9image3d_tjDv4_i",
      "_Z11read_imagef9image3d_tjDv4_i",
      "_Z11read_imagef9image3d_tjDv4_f",
// images 1.2 functions
      "_Z11read_imagefP15image2d_array_tjDv4_f",
      "_Z11read_imageiP15image2d_array_tjDv4_i",
      "_Z11read_imageiP15image2d_array_tjDv4_f",
      "_Z12read_imageuiP15image2d_array_tjDv4_i",
      "_Z12read_imageuiP15image2d_array_tjDv4_f",
      "_Z11read_imagef9image1d_tji",
      "_Z11read_imagef9image1d_tjf",
      "_Z11read_imagei9image1d_tji",
      "_Z11read_imagei9image1d_tjf",
      "_Z12read_imageui9image1d_tji",
      "_Z12read_imageui9image1d_tjf",
      "_Z11read_imagef15image1d_array_tjDv2_i",
      "_Z11read_imagef15image1d_array_tjDv2_f",
      "_Z11read_imagei15image1d_array_tjDv2_i",
      "_Z11read_imagei15image1d_array_tjDv2_f",
      "_Z12read_imageui15image1d_array_tjDv2_i",
      "_Z12read_imageui15image1d_array_tjDv2_f"
//TODO: add sampler-less and writing

    };

    for( unsigned int i=0; i < NumImageFunctions ; i++ ) {
      if ( !name.compare(ImageFunctions[i]) ) {
        return true;
      }
    }

    // Some builtin functions supplied by the backend itself, need to check if the LLVM
    // knows the implementation of the required function
    void* ptr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(name);
    return (ptr != nullptr);
  }


} // namespace intel {

extern "C"{
  ModulePass* createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions) {
    return new intel::UndefExternalFuncs(undefinedExternalFunctions);
  }
}
