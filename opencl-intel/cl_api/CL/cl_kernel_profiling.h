/**********************************************************************************
 * Copyright (c) 2008-2010 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 **********************************************************************************/

/* $Revision: 11708 $ on $Date: 2010-06-13 23:36:24 -0700 (Sun, 13 Jun 2010) $ */

#ifndef __OPENCL_CL_KERNEL_PROFLING_H
#define __OPENCL_CL_KERNEL_PROFLING_H

#include <CL/cl.h>
#include <CL/cl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/* profiling points count */
#define CL_MAX_TIMESTAMP_PROFILING_POINTS_INTEL				26
#define CL_MAX_TIMESTAMP_AGGREGATED_PROFILING_POINTS_INTEL 	20

/* cl_context_info */
#define CL_CONTEXT_KERNEL_PROFILING_MODES_COUNT_INTEL       0x407A
#define CL_CONTEXT_KERNEL_PROFILING_MODE_INFO_INTEL         0x407B

/* cl_kernel_info */
#define CL_KERNEL_IL_SYMBOLS_INTEL                          0x407C
#define CL_KERNEL_BINARY_PROGRAM_INTEL                      0x407D

/* performance counter */
#define CL_PROFILING_COMMAND_PERFCOUNTERS_INTEL             0x407F

/* IL symbols */
#define CL_KERNEL_PROFILING_SYMBOLS_NO_SOURCE_LOCATION      0xFFFFFFFF
#define CL_KERNEL_PROFILING_SYMBOLS_BASIC_BLOCK_BEGIN       0xFFFFFFFE
#define CL_KERNEL_PROFILING_SYMBOLS_BASIC_BLOCK_END         0xFFFFFFFD
#define CL_KERNEL_PROFILING_SYMBOLS_PROFILE_OPCODE          0xFFFFFFFC

enum cl_profiling_kernel_layout_INTEL
{
    CL_PROFILING_KERNEL_LAYOUT_DEFAULT  = 0
};

enum cl_profiling_info_layout_INTEL
{
    CL_PROFILING_INFO_LAYOUT_DEFAULT    = 0
};

enum cl_profiling_journal_layout_INTEL
{
    CL_PROFILING_JOURNAL_LAYOUT_DEFAULT    = 0
};

enum cl_profiling_symbols_layout_INTEL
{
    CL_PROFILING_SYMBOLS_LAYOUT_DEFAULT    = 0
};

enum cl_profiling_point_type_INTEL 
{
    CL_PROFILING_POINT_TIMESTAMP_INTEL          =   1 << 0, 
    CL_PROFILING_POINT_STALL_COUNTER_INTEL      =   1 << 1,
    CL_PROFILING_POINT_SAMPLER_MESSAGE_INTEL    =   1 << 2,
    CL_PROFILING_POINT_DATA_PORT_MESSAGE_INTEL  =   1 << 3,
    CL_PROFILING_POINT_WORKGROUP_ID_X_INTEL     =   1 << 4,
    CL_PROFILING_POINT_WORKGROUP_ID_Y_INTEL     =   1 << 5,
    CL_PROFILING_POINT_WORKGROUP_ID_Z_INTEL     =   1 << 6,
    CL_PROFILING_POINT_EXECUTION_MASK           =   1 << 7
};

enum cl_il_symbols_flags_INTEL 
{
    CL_SHOW_PROFILING_POINTS_INTEL              =   1 << 0, 
    CL_SHOW_BASIC_BLOCKS_INTEL                  =   1 << 1
};

enum cl_kernel_profiling_mode_INTEL
{
    CL_PROFILING_MODE_NONE_INTEL            = 0,
    CL_PROFILING_MODE_TRACING_INTEL         = 1,
    CL_PROFILING_MODE_IL_INTEL              = 2,
    CL_PROFILING_MODE_ISA_INTEL             = 3,
    CL_PROFILING_MODE_IL_AGGREGATED_INTEL   = 4,
    CL_PROFILING_MODE_ISA_AGGREGATED_INTEL  = 5,
    CL_PROFILING_MODE_ISA_COUNER_INTEL      = 6,    // internal user usage
    CL_PROFILING_MODE_GT_PIN_INTEL          = 7,    // internal user usage
    CL_PROFILING_MODE_GPGPU_TRACING_INTEL   = 8,
    CL_PROFILING_MODES_COUNT                = 9
};

struct cl_profiling_point_INTEL 
{
    cl_profiling_point_type_INTEL       profile_point_type;
    cl_uint                             offset_value;
};

struct cl_il_symbols_data_INTEL
{
    cl_profiling_symbols_layout_INTEL   layout;
    cl_uint                             kernel_id;
    cl_il_symbols_flags_INTEL           flags;
    cl_uint                             il_symbols_data_size;
    cl_uint                             il_symbols_count;
    void*                               il_symbols_data;
};
    
struct cl_il_symbol_record_INTEL 
{ 
    cl_uint                             il_instruction_number; 
    char*                               il_instruction_name;
    cl_uint                             c_instruction_line_number; 
    char*                               c_instruction_name;
    char*                               c_function_name;
};   

struct cl_profiling_info_INTEL 
{
    cl_profiling_info_layout_INTEL      info_layout;
    cl_kernel_profiling_mode_INTEL      profiling_mode_id;
    cl_uint                             description_length;
    char*                               description;
    cl_uint                             report_data_size; 
    cl_uint                             max_profile_points;
    cl_uint                             is_cores_filter_available;
    cl_uint                             partitions_count;		
    cl_uint                             cores_in_partition_count;	
    cl_uint                             hardware_threads_count;
    cl_uint                             is_gpu_timestamp_available;
    cl_uint                             profiling_point_enum_mask;
};

struct cl_profiling_kernel_configuration_INTEL
{
    cl_profiling_kernel_layout_INTEL    configuration_layout;
    const char*                         kernel_name;
    cl_kernel_profiling_mode_INTEL      profiling_mode_id;
    cl_uint                             profiling_points_count; 
    cl_profiling_point_INTEL*           profiling_points_table;
    cl_uint                             use_cores_filter;
    cl_uint                             partitions_count;
    cl_uint                             partition_cores_mask;
    cl_uint                             threads_count;
    cl_uint                             gather_gpu_timestamp;
};

struct cl_profiling_journal_configuration_INTEL
{
    cl_profiling_journal_layout_INTEL   configuration_layout;
    cl_kernel_profiling_mode_INTEL      profiling_mode_id;
    cl_uint                             max_reports_count;
    cl_uint                             max_extended_reports_count;
    cl_mem*                             journal;
    cl_mem*                             journal_counter;
};

struct cl_eu_information_gen7_INTEL
{
    cl_ushort     thread_id           : 3;
    cl_ushort     reserved1           : 5;
    cl_ushort     eu_id               : 4;
    cl_ushort     half_slice_id       : 1;
    cl_ushort     slice_id            : 1;
    cl_ushort     core_id             : 1;
    cl_ushort     reserved0           : 1;
};

struct cl_eu_information_gen8_INTEL
{
    cl_ushort     thread_id           : 3;
    cl_ushort     reserved1           : 5;
    cl_ushort     eu_id               : 4;
    cl_ushort     subslice_id         : 2;
    cl_ushort     slice_id            : 2;
};

struct cl_profiling_kernel_trace_report_INTEL
{
    cl_uint     timestamp_low_prolog;
    cl_uint     timestamp_high_prolog;

    cl_uint     simd_type           : 16;
    cl_uint     tevent_prolog       : 8;
    cl_uint     tevent_epilog       : 8;

    cl_uint     reserved2           : 4;
    cl_uint     kernel_id           : 28;

    cl_uint     timestamp_low_epilog;
    cl_uint     timestamp_high_epilog;

    cl_uint     dispatch_mask;

    union
    {
        cl_eu_information_gen7_INTEL    eu_information_gen7;
        cl_eu_information_gen8_INTEL    eu_information_gen8;
    };

    cl_ushort   priority            : 3;
    cl_ushort   reserved1           : 4;
    cl_ushort   priority_class      : 1;
    cl_ushort   fixed_function_id   : 4;
    cl_ushort   reserved0           : 4;
};

struct cl_profiling_gpgpu_kernel_trace_report_INTEL
{
    cl_uint     fixed_function_id   : 4;
    cl_uint     simd_type           : 4;
    cl_uint     kernel_id           : 24;

    cl_uint     timestamp_prolog;
    cl_uint     timestamp_epilog;

    cl_uint     workgroup_id_x;
    cl_uint     workgroup_id_y;
    cl_uint     workgroup_id_z;

    cl_uint     dispatch_mask;

    union
    {
        cl_eu_information_gen7_INTEL    eu_information_gen7;
        cl_eu_information_gen8_INTEL    eu_information_gen8;
    };

    cl_ushort   reserved;
};

struct cl_profiling_kernel_report_INTEL
{
    cl_uint     fixed_function_id   : 4;
    cl_uint     simd_type           : 4;
    cl_uint     kernel_id           : 24;

    cl_uint     timestamp_prolog;
    cl_ushort   timestamp_epilog;

    union
    {
        cl_eu_information_gen7_INTEL    eu_information_gen7;
        cl_eu_information_gen8_INTEL    eu_information_gen8;
    };

    cl_ushort   user_timestamp[CL_MAX_TIMESTAMP_PROFILING_POINTS_INTEL];
};

struct cl_profiling_kernel_aggregated_report_INTEL
{
    cl_uint     fixed_function_id     : 4;
    cl_uint     simd_type             : 4;
    cl_uint     kernel_id             : 24;

    union
    {
        cl_eu_information_gen7_INTEL    eu_information_gen7;
        cl_eu_information_gen8_INTEL    eu_information_gen8;
    };

    cl_ushort   dispatch_mask;
    cl_uint     timestamp_prolog;
    cl_uint     timestamp_epilog;
    cl_uint     user_timestamp[CL_MAX_TIMESTAMP_AGGREGATED_PROFILING_POINTS_INTEL];
};

typedef CL_API_ENTRY cl_program (CL_API_CALL *clCreateProfiledProgramWithSourceINTEL_fn)(
    cl_context                          context,
    cl_uint                             count,
    const char**                        sources,
    const size_t*                       lengths,
    const void*                         configurations,
    cl_uint                             configurations_count,
    cl_int*                             error_code ) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *clCreateKernelProfilingJournalINTEL_fn)(
    cl_context                          context,
    const void*                         configuration ) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *clCreatePerfCountersCommandQueueINTEL_fn)(
    cl_context                          context,
    cl_device_id                        device,
    cl_command_queue_properties         properties,
    cl_uint                             configuration,
    cl_int*                             error_code ) CL_API_SUFFIX__VERSION_1_0;

#ifdef __cplusplus
}
#endif

#endif  // __OPENCL_CL_KERNEL_PROFLING_H

