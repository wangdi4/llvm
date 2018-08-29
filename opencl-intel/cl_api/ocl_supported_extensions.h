// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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
#pragma once

// define all supported extensions names
// each device may build it's supported extension list from these defines

#define OCL_EXT_KHR_ICD                         "cl_khr_icd"
#define OCL_INTEL_VEC_LEN_HINT                  "cl_intel_vec_len_hint"
#define OCL_EXT_KHR_GLOBAL_BASE_ATOMICS         "cl_khr_global_int32_base_atomics"
#define OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS     "cl_khr_global_int32_extended_atomics"
#define OCL_EXT_KHR_LOCAL_BASE_ATOMICS          "cl_khr_local_int32_base_atomics"
#define OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS      "cl_khr_local_int32_extended_atomics"
#define OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE      "cl_khr_byte_addressable_store"

#define OCL_EXT_KHR_FP64                        "cl_khr_fp64"

#define OCL_EXT_KHR_DX9_MEDIA_SHARING           "cl_khr_dx9_media_sharing"
#define OCL_EXT_INTEL_DX9_MEDIA_SHARING         "cl_intel_dx9_media_sharing"
#define OCL_EXT_KHR_D3D11_SHARING               "cl_khr_d3d11_sharing"
#define OCL_EXT_KHR_GL_SHARING                  "cl_khr_gl_sharing"

#define OCL_EXT_KHR_DEPTH_IMAGES                "cl_khr_depth_images"
#define OCL_EXT_KHR_3D_IMAGE_WRITES             "cl_khr_3d_image_writes"
#define OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD      "cl_intel_exec_by_local_thread"
#define OCL_EXT_INTEL_DEVICE_PARTITION_BY_NAMES "cl_intel_device_partition_by_names"

#define OCL_EXT_KHR_SPIR                        "cl_khr_spir"

#define OCL_EXT_KHR_IMAGE2D_FROM_BUFFER         "cl_khr_image2d_from_buffer"

#define OCL_EXT_INTEL_CHANNELS                  "cl_intel_channels"
#define OCL_EXT_ALTERA_CHANNELS                 "cl_altera_channels"
#define OCL_EXT_INTEL_FPGA_HOST_PIPE            "cl_intel_fpga_host_pipe"

#define OCL_EXT_ES_KHR_INT64                    "cles_khr_int64"
