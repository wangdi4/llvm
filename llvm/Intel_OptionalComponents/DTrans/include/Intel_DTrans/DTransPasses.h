//===------------------ DTransPasses.h - Passes for DTrans ---------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// The new pass manager's PassRegistry.def needs to see the declarations
// for each pass. This file includes them all.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransPasses.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_DTRANS_PASSES_H
#define INTEL_DTRANS_DTRANS_PASSES_H

#include "Intel_DTrans/Analysis/DTransFieldModRef.h"
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzerTest.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/Transforms/AOSToSOAOP.h"
#include "Intel_DTrans/Transforms/AnnotatorCleaner.h"
#include "Intel_DTrans/Transforms/CodeAlignOP.h"
#include "Intel_DTrans/Transforms/CommuteCond.h"
#include "Intel_DTrans/Transforms/DTransForceInlineOP.h"
#include "Intel_DTrans/Transforms/DTransNormalizeOP.h"
#include "Intel_DTrans/Transforms/DTransPaddedMalloc.h"
#include "Intel_DTrans/Transforms/DeleteFieldOP.h"
#include "Intel_DTrans/Transforms/DynClone.h"
#include "Intel_DTrans/Transforms/EliminateROFieldAccess.h"
#include "Intel_DTrans/Transforms/MemInitTrimDownOP.h"
#include "Intel_DTrans/Transforms/MemManageTransOP.h"
#include "Intel_DTrans/Transforms/PaddedPointerPropagation.h"
#include "Intel_DTrans/Transforms/RemoveDTransTypeMetadata.h"
#include "Intel_DTrans/Transforms/ReorderFieldsOP.h"
#include "Intel_DTrans/Transforms/ReuseFieldOP.h"
#include "Intel_DTrans/Transforms/SOAToAOSOP.h"
#include "Intel_DTrans/Transforms/SOAToAOSOPPrepare.h"
#include "Intel_DTrans/Transforms/SetIntelPropPass.h"
#include "Intel_DTrans/Transforms/Transpose.h"
#include "Intel_DTrans/Transforms/WeakAlign.h"

#if !INTEL_PRODUCT_RELEASE
#include "Intel_DTrans/Transforms/DTransOPOptBaseTest.h"
#endif // !INTEL_PRODUCT_RELEASE

#endif // INTEL_DTRANS_DTRANS_PASSES_H
