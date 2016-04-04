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

#pragma once

#include "cl_config.h"
#include "cl_user_logger.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**************************************************************************************************
* Configuration keys
**************************************************************************************************/
#ifdef WIN32
    #define DEFAULT_LOG_FILE_NAME "C:\\cl.log"
#else
    #define DEFAULT_LOG_FILE_NAME "~/intel_ocl.log"
#endif


// General configuration strings:
#define CL_CONFIG_LOG_FILE                      "CL_CONFIG_LOG_FILE"                    // string
#ifndef NDEBUG
#define CL_CONFIG_USE_LOGGER                    "CL_CONFIG_USE_LOGGER"                  // bool
#endif
#define CL_CONFIG_DEVICES                       "CL_CONFIG_DEVICES"                     // string (use tokenize to get substrings)

#define CL_CONFIG_USE_ITT_API                   "CL_CONFIG_USE_ITT_API"                 // bool
#define	CL_ITT_CONFIG_ENABLE_API_TRACING		"CL_ITT_CONFIG_ENABLE_API_TRACING"	    // bool
#define CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING    "CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING"  // bool

// Used to Enable/Disable task state Markers in GPA Platform Analyzer
#define	CL_ITT_CONFIG_SHOW_QUEUED_MARKER		"CL_ITT_CONFIG_SHOW_QUEUED_MARKER"		  // bool
#define	CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER		"CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER"	  // bool
#define	CL_ITT_CONFIG_SHOW_RUNNING_MARKER		"CL_ITT_CONFIG_SHOW_RUNNING_MARKER"		  // bool
#define	CL_ITT_CONFIG_SHOW_COMPLETED_MARKER		"CL_ITT_CONFIG_SHOW_COMPLETED_MARKER"	  // bool
	
	/**********************************************************************************************
	* Class name:	OCLConfig
	*
	* Description:	represents an OCLConfig object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class OCLConfig : public Intel::OpenCL::Utils::BasicCLConfigWrapper
	{
	public:
		OCLConfig();
		~OCLConfig();

		string		 GetLogFile() const { return m_pConfigFile->Read<string>(CL_CONFIG_LOG_FILE, DEFAULT_LOG_FILE_NAME); }
		bool		   UseLogger() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_LOGGER, false); }
		
		vector<string> GetDevices() const;
		string         GetDefaultDevice() const;

		bool			EnableAPITracing() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_ENABLE_API_TRACING, false); }
		bool			EnableContextTracing() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING, true); }
		
		bool			ShowQueuedMarker() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_QUEUED_MARKER, true); }
		bool			ShowSubmittedMarker() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER, false); }
		bool			ShowRunningMarker() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_RUNNING_MARKER, false); }
		bool			ShowCompletedMarker() const { return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_COMPLETED_MARKER, true); }

		bool      EnableITT() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_ITT_API, false); }

    };  

}}}
