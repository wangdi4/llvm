//===--------------------------DTransOpUtils.h--------------------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
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

#if !INTEL_INCLUDE_DTRANS
#error DTransOPUtils.h included in a non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H

namespace llvm {
class LLVMContext;

namespace dtransOP {
class DTransType;

// Return 'true' if pointer types are opaque.
bool areOpaquePtrsEnabled(LLVMContext &Ctx);

// Returns 'true' if a pointer type can be reached from 'Ty', with
// the exception of a pointer that is a named structure's field member.
bool hasPointerType(DTransType *Ty);

// Remove pointer, vector, and array types to uncover the base type which
// is being used.
DTransType *unwrapDTransType(DTransType *Ty);

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSOPUTILS_H
