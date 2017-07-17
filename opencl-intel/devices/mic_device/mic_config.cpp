// Copyright (c) 2006-2007 Intel Corporation
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
//  mic_config.cpp
///////////////////////////////////////////////////////////

#include "mic_config.h"

using namespace Intel::OpenCL::MICDevice;

bool MICDeviceConfig::config_already_printed = false;

MICDeviceConfig::MICDeviceConfig() 
#ifdef __INCLUDE_MKL__
    : m_bUseMKL(false)
#endif
{
	m_pConfigFile = nullptr;
}
MICDeviceConfig::~MICDeviceConfig()
{
	Release();
}
cl_err_code MICDeviceConfig::Initialize(std::string filename)
{
	m_pConfigFile = new ConfigFile(filename);
    
    if (!config_already_printed)
    {
        config_already_printed = true;
        if ((nullptr != m_pConfigFile) && Device_PrintConfig())
        {
            PrintConfiguration();
        }

    }
	return CL_SUCCESS;
}
void MICDeviceConfig::Release()
{
	if (nullptr != m_pConfigFile)
	{
		delete m_pConfigFile;
		m_pConfigFile = nullptr;
	}
}
