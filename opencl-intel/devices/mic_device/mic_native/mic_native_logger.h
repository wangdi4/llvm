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

///////////////////////////////////////////////////////////
//
//   NativeLogger - contains support for Logger
//
///////////////////////////////////////////////////////////
#pragma once

#include "cl_device_api.h"
#include "Logger.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {
	
class MicNativeLogDescriptor : public IOCLDevLogDescriptor
{
public:

	cl_int clLogCreateClient( cl_int IN dev_id, const char* IN client_name, cl_int* OUT client_id ) { return CL_SUCCESS; };

	cl_int clLogReleaseClient(cl_int IN client_id) { return CL_SUCCESS; };

	cl_int clLogAddLine( cl_int IN client_id, cl_int IN log_level, const char* IN source_file, const char* IN function_name, cl_int IN line_num, const char* IN message, ...);

	/* The static method create singleton object of MicNativeLogDescriptor.
	   If it already exist - do nothing.
	   It is not thread safe. */
	static bool createLogDescriptor();

	/* Delete the singleton object MicNativeLogDescriptor.
	   It is not thread safe. */
	static void releaseLogDescriptor();

	/* return the singleton MicNativeLogDescriptor object. */
	inline static IOCLDevLogDescriptor* getLoggerClient() { return m_pLogDescriptor; };

	inline static cl_int getClientId() { return m_clientId; };

private:

	MicNativeLogDescriptor();

	bool init();

	virtual ~MicNativeLogDescriptor();

	static IOCLDevLogDescriptor*        m_pLogDescriptor;
	static cl_int                       m_clientId;

	Intel::OpenCL::Utils::LogHandler*   m_pLogHandler;
	Intel::OpenCL::Utils::LoggerClient* m_pLoggerClient;
};

}}}

