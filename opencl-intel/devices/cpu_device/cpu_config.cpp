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

#include <string>
#include <sstream>

using namespace Intel::OpenCL::CPUDevice;

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

