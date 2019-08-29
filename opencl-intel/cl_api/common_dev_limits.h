// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __COMMON_DEV_LIMITES_H__
#define __COMMON_DEV_LIMITES_H__

/*! \def MAX_WORK_DIM
    \brief The maximum working dimension size.

    Memory Objects and Kernels should use this define to go over working dimensions.
*/
#define MAX_WORK_DIM        3
// Assuming MAX_WORK_DIM == 3
#define MAX_WI_DIM_POW_OF_2	(MAX_WORK_DIM+1)

#define DEV_MAXIMUM_ALIGN			128
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)		( ((X)+DEV_MAXIMUM_ALIGN-1) & (~(DEV_MAXIMUM_ALIGN-1)))

// OpenCL 2.0 introduced the non-uniform work-group size.
// In our implementation RT calculates two arrays of local WG sizes and passes them to a kernel.
// One array for region of work-groups with sizes equal to the spefied in clEnqueueNDRangeKernel while
// another is for region with sizes eqaul to get_global_size(dim) % get_enqueued_local_size(dim).
enum {
  UNIFORM_WG_SIZE_INDEX    = 0, // Index of WG sizes returned by get_enqueued_local_size.
  NONUNIFORM_WG_SIZE_INDEX = 1, // Index of WG sizes returned by get_local_size for tail WGs.
  WG_SIZE_NUM                   // Number of WG size arrays.
};

// List of supported devices that differs from the a given one in cl.h
enum DeviceMode {
  CPU_DEVICE      = 0,
  FPGA_EMU_DEVICE = 1,
  EYEQ_EMU_DEVICE = 2
};

// List of supported vectorizers
enum VectorizerType {
  VOLCANO_VECTORIZER, VPO_VECTORIZER, DEFAULT_VECTORIZER
};

#endif // __COMMON_DEV_LIMITES_H__
