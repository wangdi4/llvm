// Copyright 2006-2021 Intel Corporation.
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

#define OCL_EXT_KHR_ICD                                "cl_khr_icd"
#define OCL_EXT_KHR_GLOBAL_BASE_ATOMICS                "cl_khr_global_int32_base_atomics"
#define OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS            "cl_khr_global_int32_extended_atomics"
#define OCL_EXT_KHR_LOCAL_BASE_ATOMICS                 "cl_khr_local_int32_base_atomics"
#define OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS             "cl_khr_local_int32_extended_atomics"
#define OCL_EXT_KHR_INT64_BASE_ATOMICS                 "cl_khr_int64_base_atomics"
#define OCL_EXT_KHR_INT64_EXTENDED_ATOMICS             "cl_khr_int64_extended_atomics"
#define OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE             "cl_khr_byte_addressable_store"

#define OCL_EXT_KHR_FP64                               "cl_khr_fp64"
#define OCL_EXT_KHR_FP16                               "cl_khr_fp16"

#define OCL_EXT_KHR_DEPTH_IMAGES                       "cl_khr_depth_images"
#define OCL_EXT_KHR_3D_IMAGE_WRITES                    "cl_khr_3d_image_writes"
#define OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD             "cl_intel_exec_by_local_thread"
#define OCL_EXT_INTEL_DEVICE_PARTITION_BY_NAMES        "cl_intel_device_partition_by_names"

#define OCL_EXT_KHR_SPIR                               "cl_khr_spir"
#define OCL_EXT_KHR_IL_PROGRAM                         "cl_khr_il_program"

#define OCL_EXT_KHR_IMAGE2D_FROM_BUFFER                "cl_khr_image2d_from_buffer"

#define OCL_EXT_INTEL_CHANNELS                         "cl_intel_channels"
#define OCL_EXT_INTEL_FPGA_HOST_PIPE                   "cl_intel_fpga_host_pipe"

#define OCL_EXT_INTEL_GLOBAL_VARIABLE_POINTER          "cl_intel_global_variable_pointers_preview"

#define OCL_EXT_KHR_SUBGROUP_BALLOT                    "cl_khr_subgroup_ballot"
#define OCL_EXT_KHR_SUBGROUP_SHUFFLE                   "cl_khr_subgroup_shuffle"
#define OCL_EXT_KHR_SUBGROUP_SHUFFLE_RELATIVE          "cl_khr_subgroup_shuffle_relative"
#define OCL_EXT_KHR_SUBGROUP_NON_UNIFORM_ARITHMETIC    "cl_khr_subgroup_non_uniform_arithmetic"
#define OCL_EXT_KHR_SUBGROUP_EXTENDED_TYPES            "cl_khr_subgroup_extended_types"

#define OCL_EXT_INTEL_SUBGROUPS                        "cl_intel_subgroups"
#define OCL_EXT_INTEL_SUBGROUPS_CHAR                   "cl_intel_subgroups_char"
#define OCL_EXT_INTEL_SUBGROUPS_SHORT                  "cl_intel_subgroups_short"
#define OCL_EXT_INTEL_SUBGROUPS_LONG                   "cl_intel_subgroups_long"
#define OCL_EXT_INTEL_SPIRV_SUBGROUPS                  "cl_intel_spirv_subgroups"
#define OCL_EXT_INTEL_SUBGROUPS_REQD_SIZE              "cl_intel_required_subgroup_size"

#define OCL_EXT_INTEL_UNIFIED_SHARED_MEMORY            "cl_intel_unified_shared_memory_preview"

#define OCL_EXT_INTEL_CREATE_BUFFER_WITH_PROPERTIES    "cl_intel_create_buffer_with_properties"
#define OCL_EXT_INTEL_MEM_CHANNEL_PROPERTY             "cl_intel_mem_channel_property"

#define OCL_EXT_INTEL_VEC_LEN_HINT                     "cl_intel_vec_len_hint"

#define OCL_EXT_ES_KHR_INT64                           "cles_khr_int64"

#define OCL_EXT_KHR_SPIRV_LINKONCE_ODR                 "cl_khr_spirv_linkonce_odr"

#define OCL_EXT_INTEL_DEVICE_ATTRIBUTE_QUERY           "cl_intel_device_attribute_query"
