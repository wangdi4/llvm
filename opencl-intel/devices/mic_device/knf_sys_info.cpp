/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#include <assert.h>
#include <stdio.h>

#include "mic_device.h"
#include "mic_sys_info.h"
#include "mic_sys_info_internal.h"
#include "mic_common_macros.h"
#include <CL/cl_ext.h>

#define __DOUBLE_ENABLED__

using namespace Intel::OpenCL::MICDevice;

// Update also in clang_driver.cpp (Guy)
#ifdef __DOUBLE_ENABLED__
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_fp64 cl_khr_global_int32_base_atomics "\
                                                "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
                                                "cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
                                                "cl_intel_printf cl_ext_device_fission";
#else
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_global_int32_base_atomics "\
                                               "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
                                                "cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
                                                "cl_intel_printf cl_ext_device_fission";
#endif

static const size_t MIC_MAX_WORK_ITEM_SIZES[MIC_MAX_WORK_ITEM_DIMENSIONS] =
    {
        MIC_MAX_WORK_GROUP_SIZE,
        MIC_MAX_WORK_GROUP_SIZE,
        MIC_MAX_WORK_GROUP_SIZE
    };

static const cl_device_partition_property_ext MIC_SUPPORTED_FISSION_MODES[] =
    {
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT,
        CL_DEVICE_PARTITION_BY_COUNTS_EXT,
        CL_DEVICE_PARTITION_EQUALLY_EXT
    };

static const cl_device_partition_property_ext MIC_SUPPORTED_AFFINITY_DOMAINS[] =
    {
        CL_AFFINITY_DOMAIN_NUMA_EXT
    };

static MICSysInfo::SYS_INFO_ENTRY knf_info[] =
{
//
//scalar/array/func      name                              type                 value
//
    SCAL_VALUE( CL_DEVICE_TYPE,                         cl_device_type,                 CL_DEVICE_TYPE_ACCELERATOR      ),
    SCAL_VALUE( CL_DEVICE_VENDOR_ID,                    cl_uint,                        VENDOR_ID                       ),
    FUNC_VALUE( CL_DEVICE_MAX_COMPUTE_UNITS,                                            get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,     cl_uint,                        MIC_MAX_WORK_ITEM_DIMENSIONS    ),
    SCAL_VALUE( CL_DEVICE_MAX_WORK_GROUP_SIZE,          size_t,                         MIC_MAX_WORK_GROUP_SIZE         ),
    ARRY_VALUE( CL_DEVICE_MAX_WORK_ITEM_SIZES,          size_t,                         MIC_MAX_WORK_ITEM_SIZES         ),
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,  cl_uint,                        16                              ),	// TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint,                        8                               ),  // TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,   cl_uint,                        4                               ),	// TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,  cl_uint,                        2                               ),	// TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint,                        4                               ),	// TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,  cl_uint,                        0                               ),	// TODO Should take this value from BE
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,     cl_uint,                        16                              ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,    cl_uint,                        8                               ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,      cl_uint,                        4                               ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,     cl_uint,                        2                               ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,    cl_uint,                        4                               ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,     cl_uint,                        0                               ),	// TODO Should take this value from device SKU
    FUNC_VALUE( CL_DEVICE_MAX_CLOCK_FREQUENCY,                                          get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_ADDRESS_BITS,                 cl_uint,                        sizeof(uint64_t)*8              ),
    SCAL_VALUE( CL_DEVICE_MAX_READ_IMAGE_ARGS,          cl_uint,                        MIC_MAX_READ_IMAGE_ARGS         ),
    SCAL_VALUE( CL_DEVICE_MAX_WRITE_IMAGE_ARGS,         cl_uint,                        MIC_MAX_WRITE_IMAGE_ARGS        ),
	FUNC_VALUE( CL_DEVICE_MAX_MEM_ALLOC_SIZE,                                           get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_IMAGE2D_MAX_WIDTH,            size_t,                         MIC_IMAGE2D_MAX_DIM_SIZE        ),
    SCAL_VALUE( CL_DEVICE_IMAGE2D_MAX_HEIGHT,           size_t,                         MIC_IMAGE2D_MAX_DIM_SIZE        ),
    SCAL_VALUE( CL_DEVICE_IMAGE3D_MAX_WIDTH,            size_t,                         MIC_IMAGE3D_MAX_DIM_SIZE        ),
    SCAL_VALUE( CL_DEVICE_IMAGE3D_MAX_HEIGHT,           size_t,                         MIC_IMAGE3D_MAX_DIM_SIZE        ),
    SCAL_VALUE( CL_DEVICE_IMAGE3D_MAX_DEPTH,            size_t,                         MIC_IMAGE3D_MAX_DIM_SIZE        ),
//    SCAL_VALUE( CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,        size_t,                         65536   /*min*/                ),
//    SCAL_VALUE( CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,         size_t,                         2048    /*min*/                ),
    SCAL_VALUE( CL_DEVICE_IMAGE_SUPPORT,                cl_bool,                        CL_TRUE                         ),
    SCAL_VALUE( CL_DEVICE_MAX_PARAMETER_SIZE,           size_t,                         MIC_MAX_PARAMETER_SIZE          ),
    FUNC_VALUE( CL_DEVICE_MAX_SAMPLERS,                                                 get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_MEM_BASE_ADDR_ALIGN,          cl_uint,                        MIC_MEM_BASE_ADDR_ALIGN         ),
    SCAL_VALUE( CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,     cl_uint,                        MIC_DEV_MAXIMUM_ALIGN           ),
    SCAL_VALUE( CL_DEVICE_SINGLE_FP_CONFIG,             cl_device_fp_config,            CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA ),
    SCAL_VALUE( CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,        cl_device_mem_cache_type,       CL_READ_WRITE_CACHE             ),
    FUNC_VALUE( CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,                                    get_variable_info               ),	// TODO refill this function with the right info
    FUNC_VALUE( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,                                        get_variable_info               ),	// TODO refill this function with the right info
    FUNC_VALUE( CL_DEVICE_GLOBAL_MEM_SIZE,                                              get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,     cl_ulong,                       MIC_MAX_CONSTANT_BUFFER_SIZE    ),
    SCAL_VALUE( CL_DEVICE_MAX_CONSTANT_ARGS,            cl_uint,                        MIC_MAX_CONSTANT_ARGS           ),
    SCAL_VALUE( CL_DEVICE_LOCAL_MEM_TYPE,               cl_device_local_mem_type,       CL_GLOBAL                       ),
    SCAL_VALUE( CL_DEVICE_LOCAL_MEM_SIZE,               cl_ulong,                       MIC_DEV_LCL_MEM_SIZE            ),
    SCAL_VALUE( CL_DEVICE_ERROR_CORRECTION_SUPPORT,     cl_bool,                        CL_FALSE                        ),
    FUNC_VALUE( CL_DEVICE_PROFILING_TIMER_RESOLUTION,                                   get_variable_info               ),
    SCAL_VALUE( CL_DEVICE_ENDIAN_LITTLE,                cl_bool,                        CL_TRUE                         ),
    SCAL_VALUE( CL_DEVICE_AVAILABLE,                    cl_bool,                        CL_TRUE                         ),
    SCAL_VALUE( CL_DEVICE_COMPILER_AVAILABLE,           cl_bool,                        CL_TRUE                         ),
//    SCAL_VALUE( CL_DEVICE_LINKER_AVAILABLE,             cl_bool,                        CL_FALSE                        ),
    SCAL_VALUE( CL_DEVICE_EXECUTION_CAPABILITIES,       cl_device_exec_capabilities,    CL_EXEC_KERNEL                  ),
    SCAL_VALUE( CL_DEVICE_QUEUE_PROPERTIES,             cl_command_queue_properties,    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE ),
//    STRG_VALUE( CL_DEVICE_BUILT_IN_KERNELS,                                             ""                              ),
    STRG_VALUE( CL_DEVICE_NAME,                                                         MIC_STRING                      ),
    STRG_VALUE( CL_DEVICE_VENDOR,                                                       VENDOR_STRING                   ),
    STRG_VALUE( CL_DRIVER_VERSION,                                                      MIC_DRIVER_VERSION_STRING       ),
    STRG_VALUE( CL_DEVICE_PROFILE,                                                      MIC_DEVICE_PROFILE_STRING       ),
    STRG_VALUE( CL_DEVICE_VERSION,                                                      MIC_DEVICE_VERSION_STRING       ),
    STRG_VALUE( CL_DEVICE_EXTENSIONS,                                                   OCL_SUPPORTED_EXTENSIONS        ),
//    SCAL_VALUE( CL_DEVICE_PLATFORM,																					), returned by framework - platform_module
//    SCAL_VALUE( CL_DEVICE_HALF_FP_CONFIG
    SCAL_VALUE( CL_DEVICE_HOST_UNIFIED_MEMORY,          cl_bool,                        CL_FALSE                        ),
    STRG_VALUE( CL_DEVICE_OPENCL_C_VERSION,                                             MIC_DEVICE_OPENCL_C_VERSION     ),

#ifdef __DOUBLE_ENABLED__
    SCAL_VALUE( CL_DEVICE_DOUBLE_FP_CONFIG,             cl_device_fp_config,            CL_FP_INF_NAN | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_FMA | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT ), // new OpenCL 1.2
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,   cl_uint,                        2                               ),	// TODO Should take this value from device SKU
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,cl_uint,                        2                               ),	// TODO Should take this value from BE
#else
    SCAL_VALUE( CL_DEVICE_DOUBLE_FP_CONFIG,             cl_device_fp_config,            0                               ),
    SCAL_VALUE( CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,   cl_uint,                        0                               ),
    SCAL_VALUE( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,cl_uint,                        0                               ),
#endif

    STRG_VALUE( CL_DEVICE_PARTITION_TYPES_EXT,                                          MIC_SUPPORTED_FISSION_MODES     ),	// Undefined in HLD
    STRG_VALUE( CL_DEVICE_AFFINITY_DOMAINS_EXT,                                         MIC_SUPPORTED_AFFINITY_DOMAINS[0]),	// Undefined in HLD

//    SCAL_VALUE( CL_DEVICE_PRINTF_BUFFER_SIZE,           size_t,                         MIC_PRINTF_BUFFER_SIZE          ),

//    SCAL_VALUE( CL_DEVICE_PREFERRED_INTEROP_USER_SYNC,  cl_bool,                        CL_TRUE                         ), // framework should answer
//    SCAL_VALUE( CL_DEVICE_PARENT_DEVICE,                cl_device_id,                   0                               ), // framework should answer
//    SCAL_VALUE( CL_DEVICE_PARTITION_TYPE,               cl_device_partition_property,   0                               ), // framework should answer
//    SCAL_VALUE( CL_DEVICE_REFERENCE_COUNT,              cl_uint,                        0                               ), // framework should answer


};

// additional DLLs required for device
//static const char* const knf_device_dlls[] =
//    {
//    };

void Intel::OpenCL::MICDevice::add_knf_info( void )
{
    MICSysInfo::InfoKeyType sku;

    sku.full_key = 0;
    sku.fields.device_type = COI_ISA_KNF;

    MICSysInfo::DeviceSKU_InternalAttributes attribs;
    attribs.required_dlls_count = 0;    // ARRAY_ELEMENTS( knf_device_dlls );
    attribs.required_dlls_array = NULL; // knf_device_dlls;

    MICSysInfo::add_sku_info( sku.full_key, ARRAY_ELEMENTS(knf_info), knf_info, attribs );
}
