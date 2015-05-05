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
//  program_config.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"
#include "program_config.h"
#include "cpu_config.h"

using namespace Intel::OpenCL::CPUDevice;

void ProgramConfig::InitFromCpuConfig(const CPUDeviceConfig& cpuConfig)
{
    m_useVectorizer  = cpuConfig.UseVectorizer();
    m_vectorizerMode = cpuConfig.GetVectorizerMode();
    m_rtLoopUnrollFactor   = cpuConfig.GetRTLoopUnrollFactor();
    m_useVTune       = cpuConfig.UseVTune();
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
