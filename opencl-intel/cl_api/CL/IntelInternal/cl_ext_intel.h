/*******************************************************************************
Copyright (c) 2014 Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to
the source code ("Material") are owned by Intel Corporation or its suppliers
or licensors. Title to the Material remains with Intel Corporation or its
suppliers and licensors. The Material contains trade secrets and proprietary
and confidential information of Intel or its suppliers and licensors. The
Material is protected by worldwide copyright and trade secret laws and
treaty provisions. No part of the Material may be used, copied, reproduced,
modified, published, uploaded, posted, transmitted, distributed, or
disclosed in any way without Intel's prior express written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel or
otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

File Name: cl_ext_intel.h

Abstract: Contains Intel OpenCL extensions for internal use only.

Notes: This file should never be released publicly.
 ******************************************************************************/

#ifndef __CL_EXT_INTEL_H
#define __CL_EXT_INTEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/*********************************
* cl_intel_accelerator extension *
*********************************/
#define cl_intel_image_transform 1
#define cl_intel_feature_extraction 1

typedef cl_uint                           cl_transform_flags_intel;
typedef cl_uint                           cl_feature_extraction_flags_intel;
    
typedef struct _cl_transform_2d_desc_intel
    {
    cl_transform_flags_intel        flags;
    cl_uint                         origin[2];
    cl_uint                         extent[2];
    void*                           param;
} cl_transform_2d_desc_intel;

typedef struct _cl_feature_extraction_desc_intel {
    cl_feature_extraction_flags_intel   flags;
} cl_feature_extraction_desc_intel;

/* cl_accelerator_type_intel */
#define CL_ACCELERATOR_TYPE_ERODE_INTEL                         0x1
#define CL_ACCELERATOR_TYPE_DILATE_INTEL                        0x2
#define CL_ACCELERATOR_TYPE_MINMAX_FILTER_INTEL                 0x3
#define CL_ACCELERATOR_TYPE_CONVOLVE_2D_INTEL                   0x4
#define CL_ACCELERATOR_TYPE_MINMAX_INTEL                        0x5
#define CL_ACCELERATOR_TYPE_CENTROID_INTEL                      0x6
#define CL_ACCELERATOR_TYPE_BOOL_CENTROID_INTEL                 0x7
#define CL_ACCELERATOR_TYPE_BOOL_SUM_INTEL                      0x8

/* cl_device_info */
#define CL_DEVICE_TRANSFORM_MASK_MAX_WIDTH_INTEL        0x409C
#define CL_DEVICE_TRANSFORM_MASK_MAX_HEIGHT_INTEL       0x409D
#define CL_DEVICE_TRANSFORM_FILTER_MAX_WIDTH_INTEL      0x409E
#define CL_DEVICE_TRANSFORM_FILTER_MAX_HEIGHT_INTEL     0x409F

/* cl_feature_extraction_desc_intel flags */
#define CL_FEATURE_EXTRACTION_FLAG_NONE_INTEL           0x00000000

/* cl_transform_2d_desc_intel flags */
#define CL_TRANSFORM_FLAG_NONE_INTEL                    0x00000000
#define CL_TRANSFORM_BORDER_CLAMP_INTEL                 0x00000001
#define CL_TRANSFORM_BORDER_MIRROR_REPEAT_INTEL         0x00000002

/***************************************
* cl_intel_debug_info extension *
****************************************/
#define cl_intel_debug_info               1

// New queries for clGetProgramInfo:
#define CL_PROGRAM_DEBUG_INFO_INTEL       0x4100
#define CL_PROGRAM_DEBUG_INFO_SIZES_INTEL 0x4101

// New queries for clGetKernelInfo:
#define CL_KERNEL_BINARIES_INTEL          0x4102
#define CL_KERNEL_BINARY_SIZES_INTEL      0x4103

/***************************************
* cl_intel_performance_hints extension *
****************************************/
#define CL_CONTEXT_SHOW_PERFORMANCE_HINTS_INTEL         0x1234

#define CL_CONTEXT_HINT_LEVEL_ALL                       ( 0xff )
#define CL_CONTEXT_HINT_LEVEL_GOOD                      ( 1 )
#define CL_CONTEXT_HINT_LEVEL_WRONG                     ( 1 << 1 )
#define CL_CONTEXT_HINT_LEVEL_NEUTRAL                   ( 1 << 2 )

/***************************************
* Internal only queue properties *
****************************************/
#define CL_QUEUE_VME_THREADS_INTEL                  0x10000
#define CL_QUEUE_SPECIAL_INTEL                      0x10001

// TODO: Intel evaluation now. Remove it after approval for public release
#if ( _DEBUG || _RELEASE_INTERNAL )
#define CL_DEVICE_CORE_CURRENT_CLOCK_FREQUENCY_INTEL    0x10002
#endif

#ifdef __cplusplus
}
#endif


#endif /* __CL_EXT_INTEL_H */
