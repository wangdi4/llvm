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
