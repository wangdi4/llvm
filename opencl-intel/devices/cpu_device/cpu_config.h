// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/*
*
* File cpu_config.h
*
*/
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

        bool            UseVectorizer() const  { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VECTORIZER, true ); }
        bool            UseVTune()      const  { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VTUNE,      false); }
        int             GetRTLoopUnrollFactor() const { return m_pConfigFile->Read<int>(CL_CONFIG_CPU_RT_LOOP_UNROLL_FACTOR, 2); }

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
