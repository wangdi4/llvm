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

#include "cpu_config.h"
#include "ICLDevBackendOptions.h"
#include "ocl_supported_extensions.h"

#include <cl_cpu_detect.h>

#ifndef INTEL_PRODUCT_RELEASE
#include <stdlib.h>
#endif // INTEL_PRODUCT_RELEASE

#include <algorithm>
#include <string>
#include <sstream>

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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

int CPUDeviceConfig::GetNumDevices() const
{
    if (FPGA_EMU_DEVICE == this->GetDeviceMode())
    {
        return m_pConfigFile->
          Read<int>(CL_CONFIG_CPU_EMULATE_DEVICES, 1);
    }
    return 1;
}

cl_ulong CPUDeviceConfig::GetForcedGlobalMemSize() const
{
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize,
                                 CL_CONFIG_CPU_FORCE_GLOBAL_MEM_SIZE))
    {
        return 0;
    }

    return ParseStringToSize(strForcedSize);
}

cl_ulong CPUDeviceConfig::GetForcedMaxMemAllocSize() const
{
    std::string strForcedSize;
    if (!m_pConfigFile->ReadInto(strForcedSize,
                                 CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE))
    {
        return 0;
    }

    return ParseStringToSize(strForcedSize);
}

cl_int CPUDeviceConfig::GetVectorizerMode() const
{
    using namespace Intel::OpenCL::DeviceBackend;
    return m_pConfigFile->Read(CL_CONFIG_CPU_VECTORIZER_MODE,
                               static_cast<uint32_t>(TRANSPOSE_SIZE_NOT_SET));
}

VectorizerType CPUDeviceConfig::GetVectorizerType() const
{
    std::string VType = m_pConfigFile->Read<string>(
        CL_CONFIG_CPU_VECTORIZER_TYPE, "default");
    std::transform(VType.begin(), VType.end(), VType.begin(), ::tolower);

    if ("vpo" == VType) {
        return VPO_VECTORIZER;
    } else if ("volcano" == VType) {
        return VOLCANO_VECTORIZER;
    }
    return DEFAULT_VECTORIZER;
}

bool CPUDeviceConfig::GetUseNativeSubgroups() const {
  return m_pConfigFile->Read<bool>(
      CL_CONFIG_CPU_ENABLE_NATIVE_SUBGROUPS, false);
}

bool CPUDeviceConfig::IsSpirSupported() const
{
    return true;
}

bool CPUDeviceConfig::IsDoubleSupported() const
{
    if (EYEQ_EMU_DEVICE == GetDeviceMode())
    {
        return false;
    }
#if WIN32
    if(CPUDetect::GetInstance()->isBroxton())
        return true;
#endif

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
    bool isAVXSupported =
        CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX10);
    if (isAVXSupported)
    {
        return true;
    }

    // enabled on Westmere
    if (CPUDetect::GetInstance()->isWestmere())
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
        if (FPGA_EMU_DEVICE == GetDeviceMode())
        {
            m_extensions =  OCL_EXT_KHR_ICD " ";
            m_extensions += OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE " ";
            m_extensions += OCL_EXT_INTEL_FPGA_HOST_PIPE " ";
            m_extensions += OCL_EXT_ES_KHR_INT64 " ";
            m_extensions += OCL_EXT_KHR_IL_PROGRAM " ";
            m_extensions += OCL_EXT_KHR_GLOBAL_BASE_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_LOCAL_BASE_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS " ";

            return m_extensions.c_str();
        }

        if (EYEQ_EMU_DEVICE == GetDeviceMode())
        {
            m_extensions =  OCL_EXT_KHR_ICD " ";
            m_extensions += OCL_EXT_KHR_GLOBAL_BASE_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_LOCAL_BASE_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS " ";
            m_extensions += OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE " ";

            return m_extensions.c_str();
        }

        // build the extensions list dynamically
        // common KHR extensions
        m_extensions =  OCL_EXT_KHR_ICD " ";
        m_extensions += OCL_EXT_KHR_GLOBAL_BASE_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_LOCAL_BASE_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_INT64_BASE_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_INT64_EXTENDED_ATOMICS " ";
        m_extensions += OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE " ";

        // KHR CPU execlusive extensions
        m_extensions += OCL_EXT_KHR_DEPTH_IMAGES " ";
        m_extensions += OCL_EXT_KHR_3D_IMAGE_WRITES " ";
        m_extensions += OCL_EXT_KHR_IL_PROGRAM " ";

        // common Intel extensions
        m_extensions += OCL_EXT_INTEL_UNIFIED_SHARED_MEMORY " ";
        // TODO: The switch is required until subgroup implementation passes
        // the conformance test fully (meaning that masked kernel is integrated).
        if (GetUseNativeSubgroups()) {
            m_extensions += OCL_EXT_INTEL_SUBGROUPS " ";
            m_extensions += OCL_EXT_INTEL_SPIRV_SUBGROUPS " ";
            m_extensions += OCL_EXT_INTEL_SUBGROUPS_REQD_SIZE " ";
        }

        // INTEL CPU execlusive extensions
        m_extensions += OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD " ";
        m_extensions += OCL_EXT_INTEL_VEC_LEN_HINT " ";
        #ifndef _WIN32
            m_extensions += OCL_EXT_INTEL_DEVICE_PARTITION_BY_NAMES " ";
        #endif
        // SPIR extension
        if (IsSpirSupported())
        {
            m_extensions += OCL_EXT_KHR_SPIR " ";
        }

        // double floating point extension
        if (IsDoubleSupported())
        {
            m_extensions += OCL_EXT_KHR_FP64 " ";
        }

        // OpenCL 2.0 extensions
        if (OPENCL_VERSION_2_0 == GetOpenCLVersion() ||
            OPENCL_VERSION_2_1 == GetOpenCLVersion())
        {
            m_extensions += OCL_EXT_KHR_IMAGE2D_FROM_BUFFER " ";
        }
    }

    return m_extensions.c_str();
}
