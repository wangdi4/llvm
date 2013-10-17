/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"
#include "llvm/Module.h"

using namespace llvm;

extern "C" {
  llvm::Pass* createBuiltinLibInfoPass(Module *Builtins, std::string type) {
    intel::BuiltinLibInfo::RuntimeServicesTypes rtType = intel::BuiltinLibInfo::RTS_OCL;
    if(type == "ocl") {
      rtType = intel::BuiltinLibInfo::RTS_OCL;
    }
    else if(type == "apple") {
      rtType = intel::BuiltinLibInfo::RTS_OSX;
    }
    else if(type == "dx") {
      rtType = intel::BuiltinLibInfo::RTS_DX;
    } else {
      assert(type == "" && "Unknown runtime service type");
#ifdef __APPLE__
      rtType = intel::BuiltinLibInfo::RTS_OSX;
#endif
    }
    return new intel::BuiltinLibInfo(Builtins, rtType);
  }

  intel::RuntimeServices* createVolcanoOpenclRuntimeSupport(const Module *runtimeModule);
  intel::RuntimeServices* createDXRuntimeSupport(const Module *runtimeModule);
  intel::RuntimeServices* createAppleOpenclRuntimeSupport(const Module *runtimeModule);
}

namespace intel{

  char BuiltinLibInfo::ID = 0;

  OCL_INITIALIZE_PASS(BuiltinLibInfo, "builtin-lib-info", "Builtin Library Info", false, false)

  BuiltinLibInfo::BuiltinLibInfo(Module *builtins, RuntimeServicesTypes type) :
    ImmutablePass(ID), m_pBIModule(builtins) {

    initializeBuiltinLibInfoPass(*PassRegistry::getPassRegistry());

    // Generate runtimeSupport object, to be used as input for vectorizer
#ifdef __APPLE__
    m_pRuntimeServices = createAppleOpenclRuntimeSupport(m_pBIModule);
#else
    switch(type) {
    case RTS_OCL:
      m_pRuntimeServices = createVolcanoOpenclRuntimeSupport(m_pBIModule);
      break;
    case RTS_OSX:
      m_pRuntimeServices = createAppleOpenclRuntimeSupport(m_pBIModule);
      break;
    case RTS_DX:
      m_pRuntimeServices = createDXRuntimeSupport(m_pBIModule);
      break;
    default:
      assert(false && "Unknown runtime services type.");
      m_pRuntimeServices = NULL;
    }
#endif
  }
} // namespace intel
