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


///////////////////////////////////////////////////////////
//  program_config.cpp
///////////////////////////////////////////////////////////


#include "program_config.h"
#include "cpu_config.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::CPUDevice;

void ProgramConfig::InitFromCpuConfig(const CPUDeviceConfig& cpuConfig)
{
    m_useVectorizer  = cpuConfig.UseVectorizer();
    m_vectorizerMode = cpuConfig.GetVectorizerMode();
    m_vectorizerType = cpuConfig.GetVectorizerType();
    m_enableNativeSubgroups = cpuConfig.GetUseNativeSubgroups();
    m_rtLoopUnrollFactor   = cpuConfig.GetRTLoopUnrollFactor();
    m_useVTune       = cpuConfig.UseVTune();
    m_targetDevice   = cpuConfig.GetDeviceMode();
    m_forcedPrivateMemorySize = cpuConfig.GetForcedPrivateMemSize();
    m_channelDepthEmulationMode = cpuConfig.GetChannelDepthEmulationMode();
}

void ProgramDumpConfig::InitFromString(const char* options)
{
    std::string fname(options);
    std::string::size_type pos1 = fname.find("\"", 0);
    std::string::size_type pos2 = fname.find("\"", pos1+1);

    if((pos1 != string::npos) && (pos2 != string::npos))
    {
         m_fileName = fname.substr(pos1 + 1, pos2 - pos1 - 1);
    }
    else
        m_fileName = "";
}
