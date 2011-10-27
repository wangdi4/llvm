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

#include "llvm/Support/InstIterator.h"
#include "llvm/Instructions.h"
#include "llvm/System/DynamicLibrary.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char UndefExternalFunctions::ID = 0;

  ModulePass* createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions) {
    return new UndefExternalFunctions(undefinedExternalFunctions);
  }

  bool UndefExternalFunctions::runOnModule(Module &M) {

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration () ) {
        // Function is not defined inside module
        continue;
      }
      runOnFunction(pFunc);
    }

    return false;
  }

  void UndefExternalFunctions::runOnFunction(Function *pFunc) {

    // Go through function instructions and search calls
    for ( inst_iterator ii = inst_begin(pFunc), ie = inst_end(pFunc); ii != ie; ++ii ) {

      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      // Call instruction

      std::string calledFuncName = pCall->getCalledFunction()->getNameStr();

      //TODO: rewrite this check!
      // Check call for not inlined functions/ kernels
      Function *pCallee = pCall->getCalledFunction();
      // check for external functions, and make sure they exist
      if ( ( NULL != pCallee) && ( pCallee->isDeclaration() ) && pCallee->getNameStr().compare(0, 4, "llvm") ) {
        void *Ptr = llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(pCallee->getNameStr());
        if ( NULL == Ptr ) {
          // report error
          m_pUndefinedExternalFunctions->push_back(pCallee->getNameStr() + " in function " + pFunc->getNameStr());
        }
      }
    }
  }


}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {