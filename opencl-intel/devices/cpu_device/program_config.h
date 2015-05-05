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
* File program_config.h
* declares the program configuration object 
*
*/
#pragma once

#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "cl_user_logger.h"
#include "cpu_logger.h"
#include <string>
#include <algorithm>

using Intel::OpenCL::Utils::g_pUserLogger;

namespace Intel { namespace OpenCL { namespace CPUDevice {

    using namespace Intel::OpenCL::DeviceBackend;
    class CPUDeviceConfig;

    /**
     * Program options used upon program creation
     */
    class ProgramConfig: public ICLDevBackendOptions
    {
    public:

        ProgramConfig() : m_vectorizerMode(TRANSPOSE_SIZE_AUTO)
        {}

        void InitFromCpuConfig(const CPUDeviceConfig& cpuConfig);

        bool GetBooleanValue(int optionId, bool defaultValue) const
        {
            return (CL_DEV_BACKEND_OPTION_USE_VTUNE == optionId) ? m_useVTune : defaultValue;
        }

        virtual int GetIntValue( int optionId, int defaultValue) const
        {
            switch(optionId )
            {
              case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
              {
                // The transpoze size is applicable only then
                // CL_CONFIG_USE_VECTORIZER is false.
                return m_useVectorizer ? m_vectorizerMode : TRANSPOSE_SIZE_1;
              }
              case CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR:
              {
                return std::max(1, std::min(16, m_rtLoopUnrollFactor));
              }
              default:
                return defaultValue;
            }
        }

        virtual const char* GetStringValue(int optionId, const char* defaultValue)const
        {
            return defaultValue;
        }

        virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
        {
            return false;
        }

    private:
        bool m_useVectorizer;
        int  m_vectorizerMode;
        int  m_rtLoopUnrollFactor;
        bool m_useVTune;

    };

    /**
     * Options used during program code container dump
     */
    class ProgramDumpConfig: public ICLDevBackendOptions
    {
    public:
        ProgramDumpConfig(const char* options)
        { 
            InitFromString(options); 
        }

        void InitFromString(const char* options);

        bool GetBooleanValue(int optionId, bool defaultValue) const
        {
            return defaultValue;
        }

        virtual int GetIntValue( int optionId, int defaultValue) const
        {
            return defaultValue;
        }

        virtual const char* GetStringValue(int optionId, const char* defaultValue)const
        {
            if( CL_DEV_BACKEND_OPTION_DUMPFILE != optionId )
            {
                return defaultValue;
            }

            return m_fileName.c_str();
        }

        virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
        {
            return false;
        }

    private:
        std::string m_fileName;
    };


}}}
