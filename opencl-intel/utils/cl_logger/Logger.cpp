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

/****************************************************
 *  Logger.cpp
 *  Created on: 10-Dec-2008 11:42:24 AM                      
 *  Implementation of the logger class
 *  Original author: ulevy                     
 ****************************************************/
#include "Logger.h"

#include <stdio.h>
#include <stdarg.h>         
#include <windows.h>            // required by: GetCurrentThreadId()
#include <sstream>              // required by: owstringstream
#include <assert.h>
using namespace Intel::OpenCL::Utils;

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
const wchar_t g_szMemoryNameTemplate[]=L"LoggerSharedMemory(%06d)";
const wchar_t g_szMutexNameTemplate[]=L"LoggerMutex(%06d)";

struct LoggerSingletonHandler
{
	LoggerSingletonHandler()
	{
		wchar_t szName[sizeof(g_szMemoryNameTemplate)/sizeof(wchar_t)+6];

		// Create process unique name
		swprintf_s(szName, sizeof(szName)/sizeof(wchar_t), g_szMemoryNameTemplate, GetCurrentProcessId());

		// Open shared memory, we are looking for previously allocated executor
		hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security 
			PAGE_READWRITE,          // read/write access
			0,                       // max. object size 
			sizeof(void*),           // buffer size  
			szName);         // name of mapping object
		if (hMapFile == NULL) 
		{ 
			return;
		}

		// Get pointer to shared memory
		pSharedBuf = MapViewOfFile(hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,                   
			0,                   
			sizeof(void*));           
		if (pSharedBuf == NULL) 
		{ 
			CloseHandle(hMapFile);
			return;
		}

		// Test for singleton existence
		swprintf_s(szName, sizeof(szName)/sizeof(wchar_t), g_szMutexNameTemplate, GetCurrentProcessId());
		hMutex = CreateMutex(NULL, TRUE, szName);
		if ( NULL == hMutex)
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
			return;
		}

		// The mutex was created, we need allocate task executor and share it.
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

Logger* LoggerSingletonHandler::pLogger = NULL;

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
        if (m_logHandlers[i] == NULL)
        {
            m_logHandlers[i] = logHandler;
			return CL_SUCCESS;
        }
    }    
    return CL_ERR_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::Log
/////////////////////////////////////////////////////////////////////////////////////////
void Logger::Log(ELogLevel level, ELogConfigField config, char* psClientName, char* sourceFile, char* functionName, __int32 sourceLine, char* message, va_list va)
{
    LogMessage	logMessage(level, config, psClientName, sourceFile, functionName, sourceLine, message, va);    
    for (int i = 0; i < MAX_LOG_HANDLERS && m_logHandlers[i]; i++)
    {
        if (m_logHandlers[i] != NULL)
        {
            m_logHandlers[i]->Log(logMessage);            
        }
    }
}

void Logger::LogW(ELogLevel level, ELogConfigField config, wchar_t* psClientName, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, va_list va)
{        
    LogMessage	logMessage(level, config, psClientName, sourceFile, functionName, sourceLine, message, va);    
    for (int i = 0; i < MAX_LOG_HANDLERS && m_logHandlers[i]; i++)
    {
        if (m_logHandlers[i] != NULL)
        {
            m_logHandlers[i]->LogW(logMessage);            
        }
    }    
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetLogHandlerParams
/////////////////////////////////////////////////////////////////////////////////////////
wchar_t*  Logger::GetLogHandlerParams(wchar_t* logHandler)
{    
    // not implemented yet
    assert(false);
    return L"";
}

/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient Ctor
/////////////////////////////////////////////////////////////////////////////////////////
LoggerClient::LoggerClient(wchar_t* clientHandle, ELogLevel loglevel)
{   
    m_logLevel = loglevel;	
	m_eLogConfig =	(ELogConfigField)(LCF_LINE_TID | LCF_LINE_TIME |
										LCF_LINE_CLIENT_NAME | LCF_LINE_LOG_LEVEL);
	m_handle = NULL;
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
void LoggerClient::Log(ELogLevel level, char* sourceFile, char* functionName, __int32 sourceLine, char* message, ...)
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
void LoggerClient::LogW(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, ...)
{
    if (m_logLevel > level)
    {
        return;
    }    
    va_list va;
    va_start(va, message);

    Logger::GetInstance().LogW(level, m_eLogConfig, m_handle, sourceFile, functionName,  sourceLine, message, va);

    va_end( va );
}
/////////////////////////////////////////////////////////////////////////////////////////
// LoggerClient::LogArgList
/////////////////////////////////////////////////////////////////////////////////////////
void LoggerClient::LogArgList(ELogLevel level, char* sourceFile, char* functionName, __int32 sourceLine, char* message, va_list va)
{
	if (m_logLevel > level)
    {
        return;
    } 
	Logger::GetInstance().Log(level, m_eLogConfig, "", sourceFile, functionName,  sourceLine, message, va);
}
void LoggerClient::LogArgListW(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, va_list va)
{
	if (m_logLevel > level)
    {
        return;
    } 
	Logger::GetInstance().LogW(level, m_eLogConfig, m_handle, sourceFile, functionName,  sourceLine, message, va);
}


