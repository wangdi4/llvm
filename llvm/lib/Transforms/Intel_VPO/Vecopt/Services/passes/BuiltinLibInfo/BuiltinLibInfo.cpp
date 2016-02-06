/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"
#include "llvm/IR/Module.h"

using namespace llvm;

extern "C" {
  llvm::Pass* createBuiltinLibInfoPass(Module *Builtins, std::string type) {
    intel::BuiltinLibInfo::RuntimeServicesTypes rtType = intel::BuiltinLibInfo::RTS_C;
    if(type == "C") {
      rtType = intel::BuiltinLibInfo::RTS_C;
    } else {
      assert(type == "" && "Unknown runtime service type");
    }
    return new intel::BuiltinLibInfo(Builtins, rtType);
  }

  intel::RuntimeServices* createTestRuntimeSupport(const Module *runtimeModule);

}

namespace intel{

  char BuiltinLibInfo::ID = 0;

  OCL_INITIALIZE_PASS(BuiltinLibInfo, "builtin-lib-info", "Builtin Library Info", false, false)

  BuiltinLibInfo::BuiltinLibInfo(Module *builtins, RuntimeServicesTypes type) :
    ImmutablePass(ID), m_pBIModule(builtins) {

    initializeBuiltinLibInfoPass(*PassRegistry::getPassRegistry());

    // Generate runtimeSupport object, to be used as input for vectorizer
    switch(type) {
    case RTS_C:
      m_pRuntimeServices = createTestRuntimeSupport(m_pBIModule);
      break;
    default:
      assert(false && "Unknown runtime services type.");
      m_pRuntimeServices = NULL;
    }
  }
} // namespace intel
