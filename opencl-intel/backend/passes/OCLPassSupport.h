/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef OCL_PASS_SUPPORT_H
#define OCL_PASS_SUPPORT_H

#include "llvm/PassSupport.h"

#define OCL_INITIALIZE_PASS(passName, arg, name, cfg, analysis) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    return PI; \
  } \
  LLVM_DEFINE_ONCE_FLAG(Initialize##passName##PassFlag); \
  void initialize##passName##Pass(PassRegistry &Registry) { \
    llvm::call_once(Initialize##passName##PassFlag, \
                    initialize##passName##PassOnce, std::ref(Registry)); \
  }

#define OCL_INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis) INITIALIZE_PASS_BEGIN(passName, arg, name, cfg, analysis) 

#define OCL_INITIALIZE_PASS_DEPENDENCY(depName) INITIALIZE_PASS_DEPENDENCY(depName) 

#define OCL_INITIALIZE_AG_DEPENDENCY(depName) INITIALIZE_AG_DEPENDENCY(depName) 

#define OCL_INITIALIZE_PASS_END(passName, arg, name, cfg, analysis) \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    return PI; \
  } \
  LLVM_DEFINE_ONCE_FLAG(Initialize##passName##PassFlag); \
  void initialize##passName##Pass(PassRegistry &Registry) { \
    llvm::call_once(Initialize##passName##PassFlag, \
                    initialize##passName##PassOnce, std::ref(Registry)); \
  }

#define OCL_INITIALIZE_AG_PASS(passName, agName, arg, name, cfg, analysis, def) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    if (!def) initialize##agName##AnalysisGroup(Registry); \
    PassInfo *PI = new PassInfo(name, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    \
    PassInfo *AI = new PassInfo(name, & agName :: ID); \
    Registry.registerAnalysisGroup(& agName ::ID, & passName ::ID, \
                                   *AI, def, true); \
    return AI; \
  } \
  LLVM_DEFINE_ONCE_FLAG(Initialize##passName##PassFlag); \
  void initialize##passName##Pass(PassRegistry &Registry) { \
    llvm::call_once(Initialize##passName##PassFlag, \
                    initialize##passName##PassOnce, std::ref(Registry)); \
  }


#define OCL_INITIALIZE_AG_PASS_BEGIN(passName, agName, arg, n, cfg, analysis, def) \
  static void* initialize##passName##PassOnce(PassRegistry &Registry) { \
    if (!def) initialize##agName##AnalysisGroup(Registry);

#define OCL_INITIALIZE_AG_PASS_END(passName, agName, arg, n, cfg, analysis, def) \
    PassInfo *PI = new PassInfo(n, arg, & passName ::ID, \
      PassInfo::NormalCtor_t(callDefaultCtor< passName >), cfg, analysis); \
    Registry.registerPass(*PI, true); \
    \
    PassInfo *AI = new PassInfo(n, & agName :: ID); \
    Registry.registerAnalysisGroup(& agName ::ID, & passName ::ID, \
                                   *AI, def, true); \
    return AI; \
  } \
  LLVM_DEFINE_ONCE_FLAG(Initialize##passName##PassFlag); \
  void initialize##passName##Pass(PassRegistry &Registry) { \
    llvm::call_once(Initialize##passName##PassFlag, \
                    initialize##passName##PassOnce, std::ref(Registry)); \
  }

#endif
