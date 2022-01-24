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

#include "cl_env.h"
#define DEBUG_TYPE "Vectorizer"

#include "ChooseVectorizationDimension.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "LoopUtils/LoopUtils.h"
#include "OCLAddressSpace.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WorkItemAnalysis.h"

using namespace Intel::OpenCL::DeviceBackend;
using namespace intel;

/// Support for dynamic loading of modules under Linux
char ChooseVectorizationDimension::ID = 0;

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(ChooseVectorizationDimension,
                          "ChooseVectorizationDimension",
                          "Choosing Vectorization Dimension", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_DEPENDENCY(WorkItemAnalysisLegacy)
OCL_INITIALIZE_PASS_END(ChooseVectorizationDimension,
                        "ChooseVectorizationDimension",
                        "Choosing Vectorization Dimension", false, true)

} // namespace intel

ChooseVectorizationDimension::ChooseVectorizationDimension()
    : FunctionPass(ID) {
  initializeChooseVectorizationDimensionPass(*PassRegistry::getPassRegistry());
}

void ChooseVectorizationDimension::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  AU.addRequired<WorkItemAnalysisLegacy>();
}

bool ChooseVectorizationDimension::runOnFunction(Function &F) {
  const auto *RTService = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                              .getResult()
                              .getRuntimeService();
  if (!VDInfo.preCheckDimZero(F, RTService)) {
    WorkItemInfo &WIInfo = getAnalysis<WorkItemAnalysisLegacy>().getResult();
    VDInfo.compute(F, WIInfo);
  }
  return false;
}

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" ChooseVectorizationDimension *createChooseVectorizationDimension() {
  return new ChooseVectorizationDimension();
}
