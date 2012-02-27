/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  UndefinedExternalFunctions.cpp

\*****************************************************************************/

#include "UndefinedExternalFunctions.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Instructions.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char UndefExternalFunctions::ID = 0;

  ModulePass* createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions,
    const std::vector<llvm::Module*>& runtimeModules) {
    return new UndefExternalFunctions(undefinedExternalFunctions, runtimeModules);
  }

  bool UndefExternalFunctions::runOnModule(Module &M) {

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc ) {
        continue;
      }
      if( pFunc->getNumUses() > 0 && pFunc->isDeclaration () ) {
        // Function is not defined inside module
        if ( pFunc->isIntrinsic() ) continue;
        bool found = SearchForFunction(pFunc->getNameStr());
        if( !found ) {
          // The extenral function not found in any of the runtime libraries
          // Report an error
          m_pUndefinedExternalFunctions->push_back(pFunc->getNameStr() + " is undefined ");
        }
      }
    }

    return false;
  }

  bool UndefExternalFunctions::SearchForFunction(const std::string& name) {
    for(std::vector<llvm::Module*>::iterator it = m_RuntimeModules.begin()
        ,fi = m_RuntimeModules.end(); it != fi; ++it) {
      // look for the required function in all the runtime modules
      Function *pFunc = (*it)->getFunction(name);
      if(pFunc && !pFunc->isDeclaration()){
        return true;
      }
    }

    // TODO: Remove this and replace it with a module which represents the images library
    const unsigned int NumImageFunctions = 33;

    static const std::string ImageFunctions[NumImageFunctions] = {
      "_Z12read_imageuiP10_image2d_tjDv2_i",
      "_Z12read_imageuiP10_image3d_tjDv4_i",
      "_Z12read_imageuiP10_image2d_tjDv2_f",
      "_Z12read_imageuiP10_image3d_tjDv4_f",
      "_Z13write_imageuiP10_image2d_tDv2_iDv4_j",
      "_Z11read_imageiP10_image2d_tjDv2_i",
      "_Z11read_imageiP10_image3d_tjDv4_i",
      "_Z11read_imageiP10_image2d_tjDv2_f",
      "_Z11read_imageiP10_image3d_tjDv4_f",
      "_Z12write_imagefP10_image2d_tDv2_iDv4_f",
      "_Z12write_imageiP10_image2d_tDv2_iDv4_i",
      "_Z11read_imagefP10_image2d_tjDv2_f",
      "_Z11read_imagefP10_image2d_tjDv2_i",
      "_Z11read_imagefP10_image3d_tjDv4_i",
      "_Z11read_imagefP10_image3d_tjDv4_i",
      "_Z11read_imagefP10_image3d_tjDv4_f",
// images 1.2 functions
      "_Z11read_imagefP16_image2d_array_tjDv4_f",
      "_Z11read_imageiP16_image2d_array_tjDv4_i",
      "_Z11read_imageiP16_image2d_array_tjDv4_f",
      "_Z12read_imageuiP16_image2d_array_tjDv4_i",
      "_Z12read_imageuiP16_image2d_array_tjDv4_f",
      "_Z11read_imagefP10_image1d_tji",
      "_Z11read_imagefP10_image1d_tjf",
      "_Z11read_imageiP10_image1d_tji",
      "_Z11read_imageiP10_image1d_tjf",
      "_Z12read_imageuiP10_image1d_tji",
      "_Z12read_imageuiP10_image1d_tjf",
      "_Z11read_imagefP16_image1d_array_tjDv2_i",
      "_Z11read_imagefP16_image1d_array_tjDv2_f",
      "_Z11read_imageiP16_image1d_array_tjDv2_i",
      "_Z11read_imageiP16_image1d_array_tjDv2_f",
      "_Z12read_imageuiP16_image1d_array_tjDv2_i",
      "_Z12read_imageuiP16_image1d_array_tjDv2_f"
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
    return (ptr != NULL);
  }


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {