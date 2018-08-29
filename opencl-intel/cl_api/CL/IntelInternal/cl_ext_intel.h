// INTEL CONFIDENTIAL
//
// Copyright 2014-2018 Intel Corporation.
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

/*******************************************************************************
Abstract: Contains Intel OpenCL extensions for internal use only.

Notes: This file should never be released publicly.
*******************************************************************************/

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
* cl_intel_driver_diagnostics extension *
****************************************/
#define cl_intel_driver_diagnostics 1

typedef cl_uint                           cl_diagnostics_verbose_level;

#define CL_CONTEXT_SHOW_DIAGNOSTICS_INTEL                            0x4106

#define CL_CONTEXT_DIAGNOSTICS_LEVEL_ALL_INTEL                       ( 0xff )
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_GOOD_INTEL                      ( 1 )
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_BAD_INTEL                       ( 1 << 1 )
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_NEUTRAL_INTEL                   ( 1 << 2 )
#define CL_CONTEXT_DIAGNOSTICS_LEVEL_INTERNAL_INTEL                  ( 1 << 3 )

/***************************************
* Internal only queue properties *
****************************************/
#define CL_QUEUE_VME_THREADS_INTEL                  0x10000
#define CL_QUEUE_SPECIAL_INTEL                      0x20000
#define CL_QUEUE_MASTER_INTEL                       0x40000

// TODO: Intel evaluation now. Remove it after approval for public release
#define CL_DEVICE_CORE_CURRENT_CLOCK_FREQUENCY_INTEL    0x10002

/***************************************
* wgl_intel_cl_sharing *
****************************************/
#define CL_MEM_SHAREABLE_INTEL                              ( 1 << 19 )

/***************************************
* cl_intel_mirrored_repeat_101
****************************************/
/* cl_addressing_mode */
#define CL_ADDRESS_MIRRORED_REPEAT_101_INTEL                0x10135

/***************************************
* Private API for setting debug variables
****************************************/
extern CL_API_ENTRY cl_int CL_API_CALL
clSetDebugVariableINTEL ( 
    cl_device_id                 device,
    const char*                  pKey,
    cl_uint                      value ); 

#ifdef __cplusplus
}
#endif


#endif /* __CL_EXT_INTEL_H */
