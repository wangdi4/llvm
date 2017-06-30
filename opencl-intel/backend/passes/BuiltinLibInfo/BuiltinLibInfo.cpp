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
    llvm::Pass* createBuiltinLibInfoPass(llvm::SmallVector<llvm::Module*, 2> builtinsList, std::string type) {
    intel::BuiltinLibInfo::RuntimeServicesTypes rtType = intel::BuiltinLibInfo::RTS_OCL;
    if(type == "ocl") {
      rtType = intel::BuiltinLibInfo::RTS_OCL;
    }
    else if(type == "dx") {
      rtType = intel::BuiltinLibInfo::RTS_DX;
    }
    else if(type == "rs") {
      rtType = intel::BuiltinLibInfo::RTS_RS;
    } else {
      assert(type == "" && "Unknown runtime service type");
    }
    return new intel::BuiltinLibInfo(builtinsList, rtType);
  }

  intel::RuntimeServices* createVolcanoOpenclRuntimeSupport(SmallVector<Module*, 2> runtimeModuleList);
  intel::RuntimeServices* createDXRuntimeSupport(SmallVector<Module*, 2> runtimeModuleList);
  intel::RuntimeServices* createRenderscriptRuntimeSupport(SmallVector<Module*, 2> runtimeModuleList);
}

namespace intel{

  char BuiltinLibInfo::ID = 0;

  OCL_INITIALIZE_PASS(BuiltinLibInfo, "builtin-lib-info", "Builtin Library Info", false, false)

  BuiltinLibInfo::BuiltinLibInfo(SmallVector<Module*, 2> builtinsList, RuntimeServicesTypes type) :
    ImmutablePass(ID) {

    m_BIModuleList = builtinsList;

    initializeBuiltinLibInfoPass(*PassRegistry::getPassRegistry());

    // Generate runtimeSupport object, to be used as input for vectorizer
    switch(type) {
    case RTS_OCL:
      m_pRuntimeServices = createVolcanoOpenclRuntimeSupport(m_BIModuleList);
      break;
    case RTS_DX:
      m_pRuntimeServices = createDXRuntimeSupport(m_BIModuleList);
      break;
    case RTS_RS:
      m_pRuntimeServices = createRenderscriptRuntimeSupport(m_BIModuleList);
      break;
    default:
      assert(false && "Unknown runtime services type.");
      m_pRuntimeServices = NULL;
    }
  }
} // namespace intel
