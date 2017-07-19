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

/****************************************************
 *  Logger.cpp
 *  Created on: 10-Dec-2008 11:42:24 AM
 *  Implementation of the logger class
 *  Original author: ulevy
 ****************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <sstream>              // required by: owstringstream
#include <assert.h>

#include "Logger.h"
#include <windows.h>            // required by: GetCurrentThreadId()

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

