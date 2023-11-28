//===-Legality.cpp-----------------------------------------------*- C++ -*-===//
//
//   INTEL CONFIDENTIAL
//
//   Copyright (C) 2019 Intel Corporation
//
//   This software and the related documents are Intel copyrighted materials,
//   and your use of them is governed by the express license under which they
//   were provided to you ("License").  Unless the License provides otherwise,
//   you may not use, modify, copy, publish, distribute, disclose or treansmit
//   this software or the related documents without Intel's prior written
//   permission.
//
//   This software and the related documents are provided as is, with no
//   express or implied warranties, other than those that are expressly
//   stated in the License.
//
//===----------------------------------------------------------------------===//
///
///   \file Legality.cpp
///   VPlan vectorizer legality analysis.
///
//===----------------------------------------------------------------------===//

#include "Legality.h"

#define DEBUG_TYPE "VPlanLegality"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool, true>
    ForceUDSReductionVecOpt("vplan-force-uds-reduction-vectorization",
                            cl::location(ForceUDSReductionVec), cl::Hidden,
                            cl::desc("Force vectorization of UDS reduction."));

static cl::opt<bool, true>
    EnableHIRPrivateArraysOpt("vplan-enable-hir-private-arrays",
                              cl::location(EnableHIRPrivateArrays), cl::Hidden,
                              cl::desc("Enable private arrays for HIR path."));

static cl::opt<bool, true> EnableF90DVSupportOpt(
    "vplan-enable-f90-dv", cl::location(EnableF90DVSupport), cl::Hidden,
    cl::desc("Enable OMP SIMD private support for Fortran Dope Vectors."));

static cl::opt<bool, true>
    EnableHIRF90DVSupportOpt("vplan-enable-hir-f90-dv",
                             cl::location(EnableHIRF90DVSupport), cl::Hidden,
                             cl::desc("Enable OMP SIMD private support for "
                                      "Fortran Dope Vectors in HIR path."));

static cl::opt<NestedSimdStrategies, true> NestedSimdStrategyOpt(
    "vplan-nested-simd-strategy", cl::location(NestedSimdStrategy), cl::Hidden,
    cl::desc("How to vectorize nested SIMD loops"),
    cl::values(
        clEnumValN(NestedSimdStrategies::BailOut, "bailout", "Don't vectorize"),
        clEnumValN(NestedSimdStrategies::Outermost, "outermost",
                   "Vectorize outermost loop only"),
        clEnumValN(NestedSimdStrategies::Innermost, "innermost",
                   "Vectorize innermost loop only"),
        clEnumValN(NestedSimdStrategies::FromInside, "frominside",
                   "Vectorize all loops starting from the innermost one")));

namespace llvm {
namespace vpo {
bool ForceUDSReductionVec = true;
bool EnableHIRPrivateArrays = false;
bool EnableF90DVSupport = true;
bool EnableHIRF90DVSupport = false;
NestedSimdStrategies NestedSimdStrategy = NestedSimdStrategies::Outermost;
} // namespace vpo
} // namespace llvm
