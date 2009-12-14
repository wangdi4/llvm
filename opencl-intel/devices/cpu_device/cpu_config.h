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

#define	CL_CONFIG_USE_TASKALYZER		"CL_CONFIG_USE_TASKALYZER"		// bool

namespace Intel { namespace OpenCL { namespace CPUDevice {

	extern const char* CPU_STRING;
	extern const cl_image_format suportedImageFormats[];
	extern const unsigned int NUM_OF_SUPPORTED_IMAGE_FORMATS;

	class CPUDeviceConfig
	{
	public:

		CPUDeviceConfig();
		~CPUDeviceConfig();

		cl_err_code    Initialize(string file_name);
		void           Release();

		bool		   UseTaskalyzer() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_TASKALYZER, false); }

	private:

		ConfigFile * m_pConfigFile;
	};

}}}
