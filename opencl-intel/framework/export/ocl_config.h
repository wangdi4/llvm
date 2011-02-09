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

#pragma once

#include "cl_config.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**************************************************************************************************
* Configuration keys
**************************************************************************************************/

#define	CL_CONFIG_LOG_FILE				"CL_CONFIG_LOG_FILE"			// string
#define	CL_CONFIG_USE_LOGGER			"CL_CONFIG_USE_LOGGER"			// bool
#define	CL_CONFIG_DEVICES				"CL_CONFIG_DEVICES"				// string (use tokenize to get substrings)
#define CL_CONFIG_DEFAULT_DEVICE		"CL_CONFIG_DEFAULT_DEVICE"		// string
#define	CL_CONFIG_FE_COMPILERS			"CL_CONFIG_FE_COMPILERS"		// string (use tokenize to get substrings)
#define CL_CONFIG_DEFAULT_FE_COMPILER	"CL_CONFIG_DEFAULT_FE_COMPILER"	// string
#define	CL_CONFIG_USE_TASKALYZER		"CL_CONFIG_USE_TASKALYZER"		// bool

	
	/**********************************************************************************************
	* Class name:	OCLConfig
	*
	* Description:	represents an OCLConfig object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class OCLConfig
	{
	public:

		OCLConfig();
		~OCLConfig();

		cl_err_code    Initialize(string file_name);
		void           Release();

		string		   GetLogFile() const { return m_pConfigFile->Read<string>(CL_CONFIG_LOG_FILE, "C:\\cl.log"); }
		bool		   UseLogger() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_LOGGER, false); }
		
		vector<string> GetDevices(string& default_device);
		string         GetDefaultDevice() const { return m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_DEVICE, "cpu_device.dll"); }
		vector<string> GetFeCompilers(string& default_compiler);
		string         GetDefaultFeCompiler() const { return m_pConfigFile->Read<string>(CL_CONFIG_DEFAULT_FE_COMPILER, "clang_compiler.dll"); }
		bool			UseTaskalyzer() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_TASKALYZER, false); }

	private:

		Intel::OpenCL::Utils::ConfigFile * m_pConfigFile;
	};

}}}

