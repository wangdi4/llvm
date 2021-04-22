//===- DevLimits.h - DPC++ kernel device limits ---------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DEV_LIMITS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DEV_LIMITS_H

namespace llvm {

#define MAX_WORK_DIM 3

#define MAX_WI_DIM_POW_OF_2 (MAX_WORK_DIM + 1)

#define DEV_MAXIMUM_ALIGN 128
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)                                        \
  (((X) + DEV_MAXIMUM_ALIGN - 1) & (~(DEV_MAXIMUM_ALIGN - 1)))

#define MAX_RECURSION_DEPTH 1024

// OpenCL 2.0 and SYCL support non-uniform work-group size.
// In our implementation RT calculates two arrays of local WG sizes and passes
// them to a kernel. One array for region of work-groups with sizes equal to the
// spefied in clEnqueueNDRangeKernel while another is for region with sizes
// eqaul to get_global_size(dim) % get_enqueued_local_size(dim).
enum {
  UNIFORM_WG_SIZE_INDEX =
      0, // Index of WG sizes returned by get_enqueued_local_size.
  NONUNIFORM_WG_SIZE_INDEX =
      1,      // Index of WG sizes returned by get_local_size for tail WGs.
  WG_SIZE_NUM // Number of WG size arrays.
};

} // namespace llvm

#endif
