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
* File cpu_device.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include "cl_config.h"
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* Configuration keys
**************************************************************************************************/

#define    CL_CONFIG_USE_GPA               "CL_CONFIG_USE_GPA"              // bool
#define    CL_CONFIG_USE_VECTORIZER        "CL_CONFIG_USE_VECTORIZER"       // bool
#define    CL_CONFIG_USE_VTUNE             "CL_CONFIG_USE_VTUNE"            // bool

// device setup
#define    CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD        "CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD"         // bool
#define    CL_CONFIG_MIC_DEVICE_USE_AFFINITY        "CL_CONFIG_MIC_DEVICE_USE_AFFINITY"         // bool
#define    CL_CONFIG_MIC_DEVICE_NUM_WORKERS         "CL_CONFIG_MIC_DEVICE_NUM_WORKERS"          // unsigned int
#define    CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0       "CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0"        // bool
#define    CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE    "CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE"     // bool
#define    CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_MB  "CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_MB"   // unsigned int in MB
#define    CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE      "CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE"       // unsigned int
#define    CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER       "CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER"        // unsigned int


namespace Intel { namespace OpenCL { namespace MICDevice {

    class MICDeviceConfig
    {
    public:

        MICDeviceConfig();
        ~MICDeviceConfig();

        cl_err_code    Initialize(string file_name);
        void           Release();

        bool           UseGPA() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_GPA, false); }
        bool           UseVectorizer() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VECTORIZER, true ); }
        bool           UseVTune()      const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VTUNE,      false); }

		// Device performance setup
		bool           Device_StopAtLoad()      const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD, false); }
		bool           Device_UseAffinity()     const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_USE_AFFINITY, true); }
		unsigned int   Device_NumWorkers()      const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_NUM_WORKERS, 0); }
		bool           Device_IgnoreCore0()     const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0, false); }
		bool           Device_IgnoreLastCore()  const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE, false); }
		unsigned int   Device_2MB_BufferMinSizeInMB() const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_MB, 20); }
		unsigned int   Device_TbbGrainSize()    const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE, 1); }
		bool           Device_LazyTransfer()    const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER, true); }

    private:

        ConfigFile * m_pConfigFile;
    };

}}}
