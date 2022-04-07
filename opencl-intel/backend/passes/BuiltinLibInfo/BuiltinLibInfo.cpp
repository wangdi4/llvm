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

#include "BuiltinLibInfo.h"
#include "OCLPassSupport.h"

namespace llvm {
  class Module;
}

using namespace llvm;

extern "C" {
    Pass* createBuiltinLibInfoPass(ArrayRef<Module *> builtinsList, std::string type) {
    intel::BuiltinLibInfo::RuntimeServicesTypes rtType = intel::BuiltinLibInfo::RTS_OCL;
    if(type == "ocl") {
      rtType = intel::BuiltinLibInfo::RTS_OCL;
    }
    else if(type == "dx") {
      rtType = intel::BuiltinLibInfo::RTS_DX;
    } else {
      assert(type == "" && "Unknown runtime service type");
    }
    return new intel::BuiltinLibInfo(builtinsList, rtType);
  }

  intel::RuntimeServices* createVolcanoOpenclRuntimeSupport(ArrayRef<Module *> runtimeModuleList);
  intel::RuntimeServices* createDXRuntimeSupport(ArrayRef<Module *> runtimeModuleList);
}

namespace intel{

  char BuiltinLibInfo::ID = 0;

  OCL_INITIALIZE_PASS(BuiltinLibInfo, "builtin-lib-info", "Builtin Library Info", false, true)

  BuiltinLibInfo::BuiltinLibInfo(ArrayRef<Module *> builtinsList, RuntimeServicesTypes type) :
    ImmutablePass(ID) {
    m_BIModuleList.assign(builtinsList.begin(), builtinsList.end());

    initializeBuiltinLibInfoPass(*PassRegistry::getPassRegistry());

    // Generate runtimeSupport object, to be used as input for vectorizer
    switch(type) {
    case RTS_OCL:
      m_pRuntimeServices = createVolcanoOpenclRuntimeSupport(m_BIModuleList);
      break;
    case RTS_DX:
      m_pRuntimeServices = createDXRuntimeSupport(m_BIModuleList);
      break;
    default:
      assert(false && "Unknown runtime services type.");
      m_pRuntimeServices = nullptr;
    }
  }
} // namespace intel
