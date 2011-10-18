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
#define	CL_CONFIG_USE_GPA				"CL_CONFIG_USE_GPA"				// bool

#define	CL_GPA_CONFIG_ENABLE_API_TRACING		"CL_GPA_CONFIG_ENABLE_API_TRACING"	    // bool
#define CL_GPA_CONFIG_ENABLE_CONTEXT_TRACING    "CL_GPA_CONFIG_ENABLE_CONTEXT_TRACING"  // bool

// Used to Enable/Disable task state Markers in GPA Platform Analyzer
#define	CL_GPA_CONFIG_SHOW_QUEUED_MARKER		"CL_GPA_CONFIG_SHOW_QUEUED_MARKER"		    // bool
#define	CL_GPA_CONFIG_SHOW_SUBMITTED_MARKER		"CL_GPA_CONFIG_SHOW_SUBMITTED_MARKER"	    // bool
#define	CL_GPA_CONFIG_SHOW_RUNNING_MARKER		"CL_GPA_CONFIG_SHOW_RUNNING_MARKER"		    // bool
#define	CL_GPA_CONFIG_SHOW_COMPLETED_MARKER		"CL_GPA_CONFIG_SHOW_COMPLETED_MARKER"	    // bool
	
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
		
		vector<string> GetDevices(string const& default_device) const;
		string         GetDefaultDevice() const;
		bool		   UseGPA() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_GPA, false); }

		bool			EnableAPITracing() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_ENABLE_API_TRACING, false); }
		bool			EnableContextTracing() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_ENABLE_CONTEXT_TRACING, true); }
		
		bool			ShowQueuedMarker() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_SHOW_QUEUED_MARKER, true); }
		bool			ShowSubmittedMarker() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_SHOW_SUBMITTED_MARKER, false); }
		bool			ShowRunningMarker() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_SHOW_RUNNING_MARKER, false); }
		bool			ShowCompletedMarker() const { return m_pConfigFile->Read<bool>(CL_GPA_CONFIG_SHOW_COMPLETED_MARKER, true); }

	private:

		Intel::OpenCL::Utils::ConfigFile * m_pConfigFile;
	};

}}}

