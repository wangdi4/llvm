//===--------------------------DTransOpUtils.h--------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// General utilities for DTrans opaque pointer classes.
//
// ===--------------------------------------------------------------------=== //

#if !INTEL_FEATURE_SW_DTRANS
#error DTransOPUtils.h included in a non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H

#include "Intel_DTrans/Analysis/DTrans.h"

namespace llvm {
class LLVMContext;

namespace dtransOP {
class DTransType;
class DTransStructType;

typedef std::pair<DTransStructType *, dtrans::MemfuncRegion> MFTypeRegion;
typedef SmallVector<MFTypeRegion> MFTypeRegionVec;

bool analyzePartialStructUse(const DataLayout &DL, DTransStructType *StructTy,
                             size_t FieldNum, uint64_t PrePadBytes,
                             const Value *AccessSizeVal, bool AllowRecurse,
                             MFTypeRegionVec &RegionDescVec);

// Returns 'true' if a pointer type can be reached from 'Ty', with
// the exception of a pointer that is a named structure's field member.
bool hasPointerType(DTransType *Ty);

// Remove pointer, vector, and array types to uncover the base type which
// is being used.
DTransType *unwrapDTransType(DTransType *Ty);

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H
