// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <stdio.h>
#include <stdarg.h>
#include <sstream>              // required by: owstringstream
#include <assert.h>

#include "Logger.h"

using namespace Intel::OpenCL::Utils;

void Logger::RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    // logger is a static lib on Windows - no need in atexit() catching   
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger Ctor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger()
{
	m_bIsActive = false;
	memset(m_logHandlers, 0, sizeof(m_logHandlers));
}


/////////////////////////////////////////////////////////////////////////////////////////
// Logger Dtor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::~Logger()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetInstance
/////////////////////////////////////////////////////////////////////////////////////////
// Shared memory for singleton object storage
// We need this shared memory because we use static library and want to have singleton across DLL's
// We need assure that the name is unique for each process
const char g_szMemoryNameTemplate[]="LoggerSharedMemory(%06d)";
const char g_szMutexNameTemplate[]="LoggerMutex(%06d)";

struct LoggerSingletonHandler
{
	LoggerSingletonHandler()
	{
		char szName[sizeof(g_szMemoryNameTemplate)/sizeof(char)+6];

		// Create process unique name
		sprintf_s(szName, sizeof(szName)/sizeof(char), g_szMemoryNameTemplate, GetCurrentProcessId());

		// Open shared memory, we are looking for previously allocated executor
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			nullptr,                 // default security
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size
			sizeof(void*),           // buffer size
			szName);         // name of mapping object
		if (hMapFile == nullptr)
		{
			return;
		}

		// Get pointer to shared memory
		pSharedBuf = MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			sizeof(void*));
		if (pSharedBuf == nullptr)
		{
			CloseHandle(hMapFile);
			return;
		}

		// Test for singleton existence
		sprintf_s(szName, sizeof(szName)/sizeof(char), g_szMutexNameTemplate, GetCurrentProcessId());
		hMutex = CreateMutex(nullptr, TRUE, szName);
		if ( nullptr == hMutex)
		{
			UnmapViewOfFile(pSharedBuf);
			CloseHandle(hMapFile);
			return;
		}
		// test if we have allocated executor
		Logger*	*ppLogger = (Logger**)(pSharedBuf);
		// Check if executor already exists
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// If so, wait for completion of executor initialization
			if ( WAIT_OBJECT_0 != WaitForSingleObject(hMutex, INFINITE) )
			{
				CloseHandle(hMutex);
				UnmapViewOfFile(pSharedBuf);
				CloseHandle(hMapFile);
				return;
			}
			// The mutex exists and released, we have a pointer to task executor instance in shared buffer
			pLogger = *ppLogger;
			ReleaseMutex(hMutex);
			return;
		}

		// The mutex was created, we need allocate logger and share it.
		pLogger = new Logger;
		*ppLogger = pLogger;
		// Release Mutex
		ReleaseMutex(hMutex);
	}

	~LoggerSingletonHandler()
	{
		CloseHandle(hMutex);
		UnmapViewOfFile(pSharedBuf);
		CloseHandle(hMapFile);
	}

	// Pointer to a singleton object
	static Logger*	pLogger;
	HANDLE			hMapFile;
	LPVOID			pSharedBuf;
	HANDLE			hMutex;
};

Logger* LoggerSingletonHandler::pLogger = nullptr;

LoggerSingletonHandler	logger;

Logger& Logger::GetInstance()
{
    return *LoggerSingletonHandler::pLogger;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::AddLogHandler
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Logger::AddLogHandler(LogHandler* logHandler)
{

	OclAutoMutex CS(&m_CS); // Lock the function
    int	i;
    for (i = 0; i < MAX_LOG_HANDLERS; i++)
    {
        if (m_logHandlers[i] == logHandler)
        {
            return CL_ERR_LOGGER_FAILED;
        }
        if (m_logHandlers[i] == nullptr)
        {
            m_logHandlers[i] = logHandler;
			return CL_SUCCESS;
        }
    }
    return CL_ERR_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetLogHandlerParams
/////////////////////////////////////////////////////////////////////////////////////////
const char*  Logger::GetLogHandlerParams(const char* logHandler)
{
    // not implemented yet
    assert(false);
    return "";
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Ctor
/////////////////////////////////////////////////////////////////////////////////////////
LoggerClient::LoggerClient(const char* clientHandle, ELogLevel loglevel)
{
    m_logLevel = loglevel;
	m_eLogConfig =	(ELogConfigField)(LCF_LINE_TID | LCF_LINE_TIME |
										LCF_LINE_CLIENT_NAME | LCF_LINE_LOG_LEVEL);
	m_handle = nullptr;
}


/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Dtor
/////////////////////////////////////////////////////////////////////////////////////////
inline LoggerClient::~LoggerClient()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::Log
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::Log(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, ...)
{
    if (m_logLevel > level)
    {
        return;
    }
    va_list va;
    va_start(va, message);

    Logger::GetInstance().Log(level, m_eLogConfig, "", sourceFile, functionName,  sourceLine, message, va);

    va_end( va );
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::LogArgList
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::LogArgList(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, va_list va)
{
	if (m_logLevel > level)
    {
        return;
    }
	Logger::GetInstance().Log(level, m_eLogConfig, "", sourceFile, functionName,  sourceLine, message, va);
}

