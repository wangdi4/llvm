/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "GroupBuiltinPass.h"
#include "OCLPassSupport.h"
#include "CompilationUtils.h"

#include "llvm/Instructions.h"
#include "llvm/Function.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

  char GroupBuiltin::ID = 0;

  OCL_INITIALIZE_PASS(GroupBuiltin, "B-GroupBuiltins", "Barrier Pass - Handle async copy instructions called from functions", false, true)

  GroupBuiltin::GroupBuiltin() : ModulePass(ID) {}

  bool GroupBuiltin::runOnModule(Module &M) {
    //Initialize barrier utils class with current module
    m_util.init(&M);

    // handle async copy built-ins
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = &*fi;
      if( !pFunc->isDeclaration() ) {
        //Built-in functions assumed to be declarations at this point.
        continue;
      }
      std::string funcName = pFunc->getName().str();
      if(CompilationUtils::isAsyncWorkGroupCopy(funcName) || CompilationUtils::isAsyncWorkGroupStridedCopy(funcName)) {
        //Module contains declaration of an async copy built-in, fix its usages.
        Function::use_iterator ui = pFunc->use_begin();
        Function::use_iterator ue = pFunc->use_end();
        for ( ; ui != ue; ++ui ) {
          CallInst *pCallInst = dyn_cast<CallInst>(*ui);
          if( !pCallInst ) {
            assert(false && "usage of async built-in is not a call instruction!");
            continue;
          }
          //Found a call instruction to async copy built-in, handle it.

          // Add Barrier before function call instruction
          m_util.createBarrier(pCallInst);

          // Add dummyBarrier after function call instruction
          Instruction *pDummyBarrierCall = m_util.createDummyBarrier();
          pDummyBarrierCall->insertAfter(pCallInst);
        }
      // } else if(CompilationUtils::isWaitGroupEvents(funcName)) {
        //Nothing to do for wait-group-events instruction.
        //Its implementation would be simply empty
      } else {
        // Not async instruction, just continue
        continue;
      }
    }

    return true;
  }


} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createGroupBuiltinPass() {
    return new intel::GroupBuiltin();
  }
}
