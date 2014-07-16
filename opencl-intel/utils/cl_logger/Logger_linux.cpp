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
#include <sstream>              // required by: owstringstream
#include <assert.h>
#include <malloc.h>

#include "cl_secure_string_linux.h"
#include "cl_shutdown.h"

using namespace Intel::OpenCL::Utils;

#ifndef DEVICE_NATIVE
	// use only global handler - do not use local one
	USE_SHUTDOWN_HANDLER(NULL); 
#endif

void Logger::RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    Intel::OpenCL::Utils::RegisterGlobalAtExitNotification(fn);
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
struct LoggerSingletonHandler
{
    LoggerSingletonHandler()
    {
        pLogger = new Logger;
    }

    ~LoggerSingletonHandler()
    {
        delete(pLogger);
    }

    // Pointer to a singleton object
    static Logger*    pLogger;
};

Logger* LoggerSingletonHandler::pLogger = NULL;

LoggerSingletonHandler    logger;

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
    int    i;
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
void Logger::Log(ELogLevel level, ELogConfigField config, const char* psClientName, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, va_list va)
{
    LogMessage    logMessage(level, config, psClientName, sourceFile, functionName, sourceLine, message, va);
    for (int i = 0; i < MAX_LOG_HANDLERS && m_logHandlers[i]; i++)
    {
        if (m_logHandlers[i] != NULL)
        {
            m_logHandlers[i]->Log(logMessage);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::GetLogHandlerParams
/////////////////////////////////////////////////////////////////////////////////////////
const char* Logger::GetLogHandlerParams(const char* logHandler)
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
    m_eLogConfig =    (ELogConfigField)(LCF_LINE_TID | LCF_LINE_TIME |
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
