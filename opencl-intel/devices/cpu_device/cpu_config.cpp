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
//  cpu_config.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"
#include "cpu_config.h"

using namespace Intel::OpenCL::CPUDevice;

CPUDeviceConfig::CPUDeviceConfig()
{
	m_pConfigFile = NULL;
}
CPUDeviceConfig::~CPUDeviceConfig()
{
	Release();
}
cl_err_code CPUDeviceConfig::Initialize(std::string filename)
{
	m_pConfigFile = new ConfigFile(filename);
	return CL_SUCCESS;
}
void CPUDeviceConfig::Release()
{
	if (NULL != m_pConfigFile)
	{
		delete m_pConfigFile;
		m_pConfigFile = NULL;
	}
}
