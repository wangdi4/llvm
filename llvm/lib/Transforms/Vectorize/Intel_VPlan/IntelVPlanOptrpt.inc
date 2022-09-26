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

VPLAN_OPTRPT_HANDLE_FUNC_GROUP_BEGIN(15475u)
// Remarks related to CM
VPLAN_OPTRPT_VEC_HANDLE(CostModelRemarks)
// Basically everything vectorizable through TargetLibraryInfo.
VPLAN_OPTRPT_HANDLE_FUNC(15482u, VectorMathCalls)
// Vectorizable due to vector-variants attribute.
VPLAN_OPTRPT_HANDLE_FUNC(15484u, VectorVariantCalls)
// Calls that get serialized.
VPLAN_OPTRPT_HANDLE_FUNC(15485u, SerializedCalls)
// Serialized code and scalar instructions reasons
VPLAN_OPTRPT_VEC_HANDLE(SerializedInstRemarks)
VPLAN_OPTRPT_HANDLE_FUNC_GROUP_END(15488u)

VPLAN_OPTRPT_HANDLE_MEM_GROUP_BEGIN(15447u)
VPLAN_OPTRPT_HANDLE_MEM(15450u, UnmaskedUnalignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(15451u, UnmaskedUnalignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(15456u, MaskedUnalignedUnitStrideLoads)
VPLAN_OPTRPT_HANDLE_MEM(15457u, MaskedUnalignedUnitStrideStores)
VPLAN_OPTRPT_HANDLE_MEM(15458u, MaskedGathers)
VPLAN_OPTRPT_HANDLE_MEM(15459u, MaskedScatters)
VPLAN_OPTRPT_HANDLE_MEM(15462u, UnmaskedGathers)
VPLAN_OPTRPT_HANDLE_MEM(15463u, UnmaskedScatters)
// Remarks about gathers/scatters generating reasons
VPLAN_OPTRPT_GS(Gathers, Scatters)
VPLAN_OPTRPT_HANDLE_MEM(15554u, UnmaskedVLSLoads)
VPLAN_OPTRPT_HANDLE_MEM(15555u, MaskedVLSLoads)
VPLAN_OPTRPT_HANDLE_MEM(15556u, UnmaskedVLSStores)
VPLAN_OPTRPT_HANDLE_MEM(15557u, MaskedVLSStores)
VPLAN_OPTRPT_HANDLE_MEM(15497u, CompressStores)
VPLAN_OPTRPT_HANDLE_MEM(15498u, ExpandLoads)

// TODO: No entry for serialized atomic/volatile loads/stores.
// TODO: No entry for loads from uniform address (O0/kernel vectorization).
VPLAN_OPTRPT_HANDLE_MEM_GROUP_END(15474u)

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
