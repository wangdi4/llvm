/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef OCL_PASS_SUPPORT_H
#define OCL_PASS_SUPPORT_H

#include "llvm/PassSupport.h"

#define OCL_INITIALIZE_PASS(passName, arg, name, cfg, analysis)                  \
  static void *initialize##passName##PassOnce(llvm::PassRegistry &Registry) {    \
    llvm::PassInfo *PI = new llvm::PassInfo(                                     \
        name, arg, &passName::ID,                                                \
        llvm::PassInfo::NormalCtor_t(callDefaultCtor<passName>), cfg, analysis); \
    Registry.registerPass(*PI, true);                                            \
    return PI;                                                                   \
  }                                                                              \
  static llvm::once_flag Initialize##passName##PassFlag;                         \
  void initialize##passName##Pass(llvm::PassRegistry &Registry) {                \
    llvm::call_once(Initialize##passName##PassFlag,                              \
                    initialize##passName##PassOnce, std::ref(Registry));         \
  }                                                                             

#define OCL_INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis)            \
INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis) 

#define OCL_INITIALIZE_PASS_DEPENDENCY(depName) INITIALIZE_PASS_DEPENDENCY(depName) 

#define OCL_INITIALIZE_PASS_END(passName, arg, name, cfg, analysis)              \
  llvm::PassInfo *PI = new llvm::PassInfo(                                       \
      name, arg, &passName::ID,                                                  \
      llvm::PassInfo::NormalCtor_t(callDefaultCtor<passName>), cfg, analysis);   \
  Registry.registerPass(*PI, true);                                              \
  return PI;                                                                     \
  }                                                                              \
  static llvm::once_flag Initialize##passName##PassFlag;                         \
  void initialize##passName##Pass(llvm::PassRegistry &Registry) {                \
    llvm::call_once(Initialize##passName##PassFlag,                              \
                    initialize##passName##PassOnce, std::ref(Registry));         \
  }                                                                             

#endif
