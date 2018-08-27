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

#include <string>

#include "cl_config.h"

/**************************************************************************************************
* Configuration keys
**************************************************************************************************/

// CPU specific:
#define CL_CONFIG_CPU_VECTORIZER_MODE           "CL_CONFIG_CPU_VECTORIZER_MODE"             // cl_int
#define CL_CONFIG_CPU_FORCE_GLOBAL_MEM_SIZE     "CL_CONFIG_CPU_FORCE_GLOBAL_MEM_SIZE"       // cl_ulong
#define CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE  "CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE"    // cl_ulong
#define CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR     "CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR"       // int
#define CL_CONFIG_USE_VECTORIZER                "CL_CONFIG_USE_VECTORIZER"                  // bool
#define CL_CONFIG_USE_VTUNE                     "CL_CONFIG_USE_VTUNE"                       // bool
#define CL_CONFIG_USE_TRAPPING                  "CL_CONFIG_USE_TRAPPING"                    // bool
#define CL_CONFIG_GL_DIRECTX_INTEROP            "CL_CONFIG_GL_DIRECTX_INTEROP"              // bool
#define CL_CONFIG_CPU_EMULATE_DEVICES           "CL_CONFIG_CPU_EMULATE_DEVICES"             // int

namespace Intel { namespace OpenCL { namespace CPUDevice {


    extern const char* CPU_STRING;

    class CPUDeviceConfig : public Intel::OpenCL::Utils::BasicCLConfigWrapper
    {
    public:

        CPUDeviceConfig();
        ~CPUDeviceConfig();

        cl_ulong        GetForcedGlobalMemSize() const;
        cl_ulong        GetForcedMaxMemAllocSize() const;
        cl_int          GetVectorizerMode() const;
        int             GetNumDevices() const;

        bool            UseVectorizer() const  { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VECTORIZER, true ); }
        bool            UseVTune()      const  { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VTUNE,      false); }
        int             GetRTLoopUnrollFactor() const { return m_pConfigFile->Read<int>(CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR, 1); }
        bool            IsSpirSupported() const;

        bool            IsGLDirectXSupported() const;
        bool            IsDoubleSupported() const;
        const char*     GetExtensions() const;

#ifdef __HARD_TRAPPING__
        bool            UseTrapping()   const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_TRAPPING,    false); }
#endif
    private:
        CPUDeviceConfig(const CPUDeviceConfig&);
        CPUDeviceConfig& operator=(const CPUDeviceConfig&);

        static std::string  m_extensions;
    };

}}}
