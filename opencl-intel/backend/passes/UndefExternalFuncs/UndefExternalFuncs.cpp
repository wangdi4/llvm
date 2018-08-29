// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

namespace intel {

  char UndefExternalFuncs::ID = 0;

  bool UndefExternalFuncs::runOnModule(Module &M) {

    m_RuntimeModules.clear();
    intel::BuiltinLibInfo &BLI = getAnalysis<intel::BuiltinLibInfo>();
    SmallVector<Module*, 2> Builtins = BLI.getBuiltinModules();
    assert(!Builtins.empty() && "No builtin module");

    for (SmallVector<Module*, 2>::iterator it = Builtins.begin(); it != Builtins.end(); ++it) {
        assert(*it != nullptr && "UndefExternalFuncs::runOnModule has null ptr in Bltns");
        m_RuntimeModules.push_back(*it);
    }

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc ) {
        continue;
      }
      if( pFunc->getNumUses() > 0 && pFunc->isDeclaration () ) {
        // Function is not defined inside module
        if ( pFunc->isIntrinsic() ) continue;
        bool found = SearchForFunction(pFunc->getName());
        if( !found ) {
          // The extenral function not found in any of the runtime libraries
          // Report an error
          m_pUndefinedExternalFunctions->push_back((std::string)pFunc->getName() + " is undefined ");
        }
      }
    }

    return false;
  }

  bool UndefExternalFuncs::SearchForFunction(const std::string& name) {
    for(std::vector<llvm::Module*>::iterator it = m_RuntimeModules.begin()
        ,fi = m_RuntimeModules.end(); it != fi; ++it) {
      // look for the required function in all the runtime modules
      Function *pFunc = (*it)->getFunction(name);

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
