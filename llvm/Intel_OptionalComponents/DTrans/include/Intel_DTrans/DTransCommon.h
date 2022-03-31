//===------------------ DTransCommon.h - Shared DTrans code ---------------===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares functions that are common to all DTrans passes.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransCommon.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_DTRANS_COMMON_H
#define INTEL_DTRANS_DTRANS_COMMON_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace legacy {
class PassManagerBase;
} // namespace legacy

void initializeDTransPasses(PassRegistry&);

// Analysis passes
void initializeDTransAnalysisWrapperPass(PassRegistry&);
void initializeDTransFieldModRefAnalysisWrapperPass(PassRegistry&);
void initializeDTransFieldModRefOPAnalysisWrapperPass(PassRegistry&);
void initializeDTransFieldModRefResultWrapperPass(PassRegistry&);
void initializeDTransImmutableAnalysisWrapperPass(PassRegistry&);
void initializeDTransSafetyAnalyzerWrapperPass(PassRegistry&);

// Transform passes
void initializeDTransAOSToSOAWrapperPass(PassRegistry&);
void initializeDTransAOSToSOAOPWrapperPass(PassRegistry&);
void initializeDTransDeleteFieldWrapperPass(PassRegistry&);
void initializeDTransDeleteFieldOPWrapperPass(PassRegistry&);
void initializeDTransPaddedMallocWrapperPass(PassRegistry&);
void initializeDTransPaddedMallocOPWrapperPass(PassRegistry&);
void initializeDTransReorderFieldsWrapperPass(PassRegistry&);
void initializeDTransResolveTypesWrapperPass(PassRegistry&);
// Pass for elimination of unreachable access to field which is only read.
void initializeDTransEliminateROFieldAccessWrapperPass(PassRegistry&);
void initializeDTransEliminateROFieldAccessOPWrapperPass(PassRegistry&);
void initializePaddedPtrPropWrapperPass(PassRegistry&);
void initializePaddedPtrPropOPWrapperPass(PassRegistry&);
void initializeDTransDynCloneWrapperPass(PassRegistry&);
void initializeDTransDynCloneOPWrapperPass(PassRegistry&);
void initializeDTransSOAToAOSWrapperPass(PassRegistry&);
void initializeDTransSOAToAOSOPWrapperPass(PassRegistry&);
void initializeDTransSOAToAOSPrepareWrapperPass(PassRegistry&);
void initializeDTransSOAToAOSOPPrepareWrapperPass(PassRegistry&);
void initializeDTransAnnotatorCleanerWrapperPass(PassRegistry&);
void initializeDTransWeakAlignWrapperPass(PassRegistry&);
void initializeDTransMemInitTrimDownWrapperPass(PassRegistry&);
void initializeDTransMemInitTrimDownOPWrapperPass(PassRegistry&);
void initializeDTransMemManageTransWrapperPass(PassRegistry&);
void initializeDTransCodeAlignWrapperPass(PassRegistry&);
void initializeDTransTransposeWrapperPass(PassRegistry&);
void initializeDTransCommuteCondWrapperPass(PassRegistry&);
void initializeDTransCommuteCondOPWrapperPass(PassRegistry&);
void initializeRemoveAllDTransTypeMetadataWrapperPass(PassRegistry &);
void initializeRemoveDeadDTransTypeMetadataWrapperPass(PassRegistry &);
void initializeDTransForceInlineWrapperPass(PassRegistry &);
void initializeDTransForceInlineOPWrapperPass(PassRegistry &);
void initializeDTransNormalizeOPWrapperPass(PassRegistry &);

#if !INTEL_PRODUCT_RELEASE
void initializeDTransOPOptBaseTestWrapperPass(PassRegistry&);
void initializeDTransOptBaseTestWrapperPass(PassRegistry&);
void initializeDTransTypeMetadataReaderTestWrapperPass(PassRegistry&);
void initializeDTransPtrTypeAnalyzerTestWrapperPass(PassRegistry&);
#endif // !INTEL_PRODUCT_RELEASE

// This is used by ForcePassLinking.
void createDTransPasses();

void addDTransPasses(ModulePassManager &MPM);
void addDTransLegacyPasses(legacy::PassManagerBase &PM);
void addLateDTransPasses(ModulePassManager &MPM);
void addLateDTransLegacyPasses(legacy::PassManagerBase &PM);

} // namespace llvm


#endif // INTEL_DTRANS_DTRANS_COMMON_H
