/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
// Common default action.
#ifndef VPLAN_OPTRPT_HANDLE_GROUP_BEGIN
#define VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)
#endif

#ifndef VPLAN_OPTRPT_HANDLE
#define VPLAN_OPTRPT_HANDLE(ID, NAME)
#endif

#ifndef VPLAN_OPTRPT_GS
#define VPLAN_OPTRPT_GS(Gathers, Scatters)
#endif

#ifndef VPLAN_OPTRPT_VEC_HANDLE
#define VPLAN_OPTRPT_VEC_HANDLE(VEC)
#endif

#ifndef VPLAN_OPTRPT_ORIGIN_VEC_HANDLE
#define VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(VEC)
#endif

#ifndef VPLAN_OPTRPT_HANDLE_GROUP_END
#define VPLAN_OPTRPT_HANDLE_GROUP_END(ID)
#endif

// For each group, default to common action.
#ifndef VPLAN_OPTRPT_HANDLE_FUNC_GROUP_BEGIN
#define VPLAN_OPTRPT_HANDLE_FUNC_GROUP_BEGIN(ID) VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)
#endif
#ifndef VPLAN_OPTRPT_HANDLE_FUNC
#define VPLAN_OPTRPT_HANDLE_FUNC(ID, NAME) VPLAN_OPTRPT_HANDLE(ID, NAME)
#endif
#ifndef VPLAN_OPTRPT_HANDLE_FUNC_GROUP_END
#define VPLAN_OPTRPT_HANDLE_FUNC_GROUP_END(ID) VPLAN_OPTRPT_HANDLE_GROUP_END(ID)
#endif

#ifndef VPLAN_OPTRPT_HANDLE_MEM_GROUP_BEGIN
#define VPLAN_OPTRPT_HANDLE_MEM_GROUP_BEGIN(ID) VPLAN_OPTRPT_HANDLE_GROUP_BEGIN(ID)
#endif
#ifndef VPLAN_OPTRPT_HANDLE_MEM
#define VPLAN_OPTRPT_HANDLE_MEM(ID, NAME) VPLAN_OPTRPT_HANDLE(ID, NAME)
#endif
#ifndef VPLAN_OPTRPT_HANDLE_MEM_GROUP_END
#define VPLAN_OPTRPT_HANDLE_MEM_GROUP_END(ID) VPLAN_OPTRPT_HANDLE_GROUP_END(ID)
#endif

// NOTE: Order of remarks is controlled by the order in which remark vectors and remark
// groups are added below. New remarks vectors should be added after GeneralRemarks vector.

// Origin remarks
VPLAN_OPTRPT_ORIGIN_VEC_HANDLE(OriginRemarks)
// General purpose remarks
VPLAN_OPTRPT_VEC_HANDLE(GeneralRemarks)

VPLAN_OPTRPT_HANDLE_FUNC_GROUP_BEGIN(OptRemarkID::BeginVectorLoopCostSummary)
// Remarks related to CM
VPLAN_OPTRPT_VEC_HANDLE(CostModelRemarks)
// Basically everything vectorizable through TargetLibraryInfo.
VPLAN_OPTRPT_HANDLE_FUNC(OptRemarkID::VectorizedMathLibCalls, VectorMathCalls)
// Intrinsics converted to vectorized intrinsics.
VPLAN_OPTRPT_HANDLE_FUNC(OptRemarkID::VectorizedIntrinsics, VectorIntrinsics)
// Vectorizable due to vector-variants attribute.
VPLAN_OPTRPT_HANDLE_FUNC(OptRemarkID::VectorFunctionCalls, VectorVariantCalls)
// Calls that get serialized.
VPLAN_OPTRPT_HANDLE_FUNC(OptRemarkID::SerializedFunctionCalls, SerializedCalls)
// Serialized code and scalar instructions reasons
VPLAN_OPTRPT_VEC_HANDLE(SerializedInstRemarks)
VPLAN_OPTRPT_HANDLE_FUNC_GROUP_END(OptRemarkID::EndVectorLoopCostSummary)

VPLAN_OPTRPT_HANDLE_MEM_GROUP_BEGIN(OptRemarkID::BeginVectorLoopMemRefSummary)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedAlignedUnitStrideLoads,
                        UnmaskedAlignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedAlignedUnitStrideStores,
                        UnmaskedAlignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedUnalignedUnitStrideLoads,
                        UnmaskedUnalignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedUnalignedUnitStrideStores,
                        UnmaskedUnalignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedAlignedUnitStrideLoads,
                        MaskedAlignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedAlignedUnitStrideStores,
                        MaskedAlignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedUnalignedUnitStrideLoads,
                        MaskedUnalignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedUnalignedUnitStrideStores,
                        MaskedUnalignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedGathers, MaskedGathers)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedScatters, MaskedScatters)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedGathers, UnmaskedGathers)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedScatters, UnmaskedScatters)
// Remarks about gathers/scatters generating reasons
VPLAN_OPTRPT_GS(Gathers, Scatters)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedVLSLoads, UnmaskedVLSLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedVLSLoads, MaskedVLSLoads)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::UnmaskedVLSStores, UnmaskedVLSStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::MaskedVLSStores, MaskedVLSStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::VectorCompressStores, CompressStores)
VPLAN_OPTRPT_HANDLE_MEM(OptRemarkID::VectorExpandLoads, ExpandLoads)

// TODO: No entry for serialized atomic/volatile loads/stores.
// TODO: No entry for loads from uniform address (O0/kernel vectorization).
VPLAN_OPTRPT_HANDLE_MEM_GROUP_END(OptRemarkID::EndVectorLoopMemRefSummary)

// Remarks related to reduction reasons
VPLAN_OPTRPT_VEC_HANDLE(ReductionInstRemarks)

#undef VPLAN_OPTRPT_HANDLE_GROUP_BEGIN
#undef VPLAN_OPTRPT_HANDLE
#undef VPLAN_OPTRPT_GS
#undef VPLAN_OPTRPT_VEC_HANDLE
#undef VPLAN_OPTRPT_ORIGIN_VEC_HANDLE
#undef VPLAN_OPTRPT_HANDLE_GROUP_END
#undef VPLAN_OPTRPT_HANDLE_FUNC_GROUP_BEGIN
#undef VPLAN_OPTRPT_HANDLE_FUNC
#undef VPLAN_OPTRPT_HANDLE_FUNC_GROUP_END
#undef VPLAN_OPTRPT_HANDLE_MEM_GROUP_BEGIN
#undef VPLAN_OPTRPT_HANDLE_MEM
#undef VPLAN_OPTRPT_HANDLE_MEM_GROUP_END
