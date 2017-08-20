/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#pragma once
#include "mic_sys_info.h"
#include "buildversion.h"

#ifdef KNC_CARD
	#define CL_COI_ISA_MIC COI_ISA_KNC
#endif

//                                          info_id array_count    type_size    si_value_type               const_value          func_value          info_id_name
#define SCAL_VALUE( id,  type,  value )      { id,     1,          sizeof(type), MICSysInfo::VALUE_SCALAR,  (size_t)(value),     nullptr,               #id }
#define ARRY_VALUE( id,  type,  st_ar_name ) { id,     ARRAY_ELEMENTS(st_ar_name),  \
                                                                   sizeof(type), MICSysInfo::VALUE_SCALAR,  (size_t)st_ar_name,  nullptr,               #id }
#define STRG_VALUE( id,  value )             { id,     1,          0,            MICSysInfo::VALUE_STRING,  (size_t)(value),     nullptr,               #id }
#define FUNC_VALUE( id,  value )             { id,     1,          0,            MICSysInfo::VALUE_FUNCTION, 0,                  &MICSysInfo::value, #id }


#define MIC_STRING                      "Intel(R) Many Integrated Core Acceleration Card"
#define VENDOR_STRING                   "Intel(R) Corporation"
#define VENDOR_ID                       0x8086
#define MIC_DRIVER_VERSION_STRING       "1.2"
#define MIC_DEVICE_PROFILE_STRING       "FULL_PROFILE"
#define MIC_DEVICE_VERSION_STRING       "OpenCL 1.2 " BUILDVERSIONSTR
#define MIC_DEVICE_OPENCL_C_VERSION     "OpenCL C 1.2 "

// initialization functions
namespace Intel { namespace OpenCL { namespace MICDevice {
    void add_mic_info( void );

	const char* get_mic_cpu_arch();
}}}

