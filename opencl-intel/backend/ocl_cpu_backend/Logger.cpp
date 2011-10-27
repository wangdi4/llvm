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

File Name:  Logger.cpp

\*****************************************************************************/

#include "Logger.h"

#include <stdarg.h>
#include <string>


using namespace Intel::OpenCL::DeviceBackend::Utils;

Logger::Logger(const wchar_t * name, Logger::LogLevel level) : m_pLogDescriptor(0),m_iLogHandle(0)
{
#ifdef _DEBUG
	//// TODO : fix m_uiCpuId
	//cl_int m_uiCpuId = 0;
	//m_pLogDescriptor->clLogCreateClient(m_uiCpuId, name, &m_iLogHandle);

	filestr.open ("OclCpuBackEnd.txt", std::fstream::out | std::fstream::app);
#endif
}

Logger::~Logger(void) 
{
#ifdef _DEBUG
	//if ( NULL != m_pLogDescriptor )
	//{
	//	m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	//}

	filestr.close();
#endif

}

void Logger::Log(Logger::LogLevel level, const wchar_t * message, ...)
{	

#ifdef _DEBUG
	//if (m_pLogDescriptor && m_iLogHandle) 
	//{
	//	// TODO : fix WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__
	//	// m_pLogDescriptor->clLogAddLine(m_iLogHandle, cl_int(level),WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, message,  __VA_ARGS__);
	//}
	std::wstring levelStr;
	switch (level) 
	{
	case DEBUG_LEVEL:
		levelStr = L"Debug";
		break;
	case INFO_LEVEL:
		levelStr = L"Info";
		break;
	case ERROR_LEVEL:
		levelStr = L"Error";
		break;
	}
	std::wstring messageStr(message);
	filestr << levelStr << L": " << messageStr << std::endl;

#endif

}
