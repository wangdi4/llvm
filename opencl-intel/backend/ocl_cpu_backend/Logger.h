/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Logger.h

\*****************************************************************************/

#pragma once

#include "cl_device_api.h"
#include "CL/cl.h"

#include <fstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class Logger
{
public:

	enum LogLevel
	{
		DEBUG_LEVEL = 100,
		INFO_LEVEL = 200,
		ERROR_LEVEL = 300
	};

	Logger(const wchar_t * name, LogLevel level);
	~Logger(void);

	void Log(LogLevel level, const wchar_t * message, ...);

private:
	IOCLDevLogDescriptor*	m_pLogDescriptor;
	cl_int					m_iLogHandle;

	std::wfstream filestr;

};

}}}}
