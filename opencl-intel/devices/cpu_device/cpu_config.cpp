// Copyright (c) 2006-2012 Intel Corporation
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

///////////////////////////////////////////////////////////
//  cpu_config.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"
#include "cpu_config.h"
#include "ICLDevBackendOptions.h"
#include "ocl_supported_extensions.h"

#include <cl_cpu_detect.h>

#include <string>
#include <sstream>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::CPUDevice;

std::string CPUDeviceConfig::m_extensions;

CPUDeviceConfig::CPUDeviceConfig()
{
    // BasicCLConfigWrapper
}

CPUDeviceConfig::~CPUDeviceConfig()
{
    // ~BasicCLConfigWrapper
}

// ParseStringToSize:
//  Parse a string that represents memory size of the format: <integer><units>
//  And convert it to single cl_ulong in bytes
//      e.g. 128MB --> 128 * 1024 * 1024 --> 134,217,728 bytes
cl_ulong ParseStringToSize(std::string& userStr)
{
    cl_ulong integer = 0;
    std::string integerStr;
    std::string units;

    // parse the first part: the integer
    std::istringstream iss(userStr);
    iss >> integer;

    if (0 == integer)
    {
        return 0;
    }

    // all the rest of userStr are the units string
    std::stringstream ss;
    ss << integer;
    ss >> integerStr;
    units = userStr.substr(integerStr.size());

    // convert to bytes
    // accepted units are: "GB", "MB", "KB", "B"
    if (units == "GB")
    {
        integer = integer << 30;
    }
    else if (units == "MB")
    {
        integer = integer << 20;
    }
    else if (units == "KB")
    {
        integer = integer << 10;
    }
    else if (units != "B")
    {
        //invalid unit
        return 0;
    }

    return integer;
}

cl_ulong CPUDeviceConfig::GetForcedGlobalMemSize() const
{
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize, CL_CONFIG_CPU_FORCE_GLOBAL_MEM_SIZE))
    {
        return 0;
    }

    return ParseStringToSize(strForcedSize);
}

cl_ulong CPUDeviceConfig::GetForcedMaxMemAllocSize() const
{
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize, CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE))
    {
        return 0;
    }

    return ParseStringToSize(strForcedSize);
}

cl_int CPUDeviceConfig::GetVectorizerMode() const
{
    using namespace Intel::OpenCL::DeviceBackend;
    return m_pConfigFile->Read(CL_CONFIG_CPU_VECTORIZER_MODE,
                               static_cast<cl_int>(TRANSPOSE_SIZE_AUTO));
}

bool CPUDeviceConfig::IsSpirSupported() const
{
    return true;
}

bool CPUDeviceConfig::IsGLDirectXSupported() const
{
    // enabled only in Windows, also may be changed via configuration
#if WIN32
    return m_pConfigFile->Read<bool>(CL_CONFIG_GL_DIRECTX_INTEROP, true);
#else
    return false;
#endif
}

bool CPUDeviceConfig::IsDoubleSupported() const
{
    // disabled on Android
#ifdef __ANDROID__
    return false;
#endif

    if(CPUDetect::GetInstance()->isBroxton())
        return true;

    // disabled on Atom
    if (BRAND_INTEL_ATOM == CPUDetect::GetInstance()->GetCPUBrandFamily())
    {
        return false;
    }

    // enabled on non-Atom brands
    if (BRAND_UNKNOWN != CPUDetect::GetInstance()->GetCPUBrandFamily())
    {
        return true;
    }

    // if we can't detect brand, fallback to AVX support check
    // enabled on CPUs with AVX support
    bool isAVXSupported = CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX10);
    if (isAVXSupported)
    {
        return true;
    }

    // enabled on Westmere
    bool isWestmere = CPUDetect::GetInstance()->IsMicroArchitecture(MA_WESTMERE);
    if (isWestmere)
    {
        return true;
    }

    // disabled in any other case
    return false;
}

const char* CPUDeviceConfig::GetExtensions() const
{
    if (m_extensions.empty())
    {
        // build the extensions list dynamically

        // common KHR extensions
        m_extensions =  OCL_EXT_KHR_ICD " ";
        m_extensions += OCL_EXT_KHR_GLOBAL_BASE_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_LOCAL_BASE_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE " ";

        // KHR CPU execlusive extensions
        m_extensions += OCL_EXT_KHR_DEPTH_IMAGES " ";
        m_extensions += OCL_EXT_KHR_3D_IMAGE_WRITES " ";

        // INTEL CPU execlusive extensions
        m_extensions += OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD " ";

        // SPIR extension
        if (IsSpirSupported())
        {
            m_extensions += OCL_EXT_KHR_SPIR " ";
        }

        // media sharing extensions
        if (IsGLDirectXSupported())
        {
            m_extensions += OCL_EXT_KHR_DX9_MEDIA_SHARING " ";
            m_extensions += OCL_EXT_INTEL_DX9_MEDIA_SHARING " ";
            m_extensions += OCL_EXT_KHR_D3D11_SHARING " ";
            m_extensions += OCL_EXT_KHR_GL_SHARING " ";
        }

        // double floating point extension
        if (IsDoubleSupported())
        {
            m_extensions += OCL_EXT_KHR_FP64 " ";
        }

        // OpenCL 2.0 extensions
        if (OPENCL_VERSION_2_0 == GetOpenCLVersion() || OPENCL_VERSION_2_1 == GetOpenCLVersion())
        {
            m_extensions += OCL_EXT_KHR_IMAGE2D_FROM_BUFFER " ";
        }
    }

    return m_extensions.c_str();
}
