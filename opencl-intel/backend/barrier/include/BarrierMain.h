// INTEL CONFIDENTIAL
//
// Copyright 2012-2022 Intel Corporation.
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

#ifndef __BARRIER_MAIN_H__
#define __BARRIER_MAIN_H__

#include "debuggingservicetype.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

namespace intel {

void addBarrierMainPasses(llvm::legacy::PassManagerBase &PM, unsigned OptLevel,
                          intel::DebuggingServiceType DebugType,
                          bool UseTLSGlobals,
                          llvm::ArrayRef<llvm::VectItem> VectInfos);

} // namespace intel

#endif // __BARRIER_MAIN_H__
