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
using namespace Intel::OpenCL::Framework;

/////////////////////////////////////////////////////////////////////////////////////////
// Logger Ctor
/////////////////////////////////////////////////////////////////////////////////////////
Logger::Logger()
{
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
Logger& Logger::GetInstance()
{
    static Logger loggerInstance;
    return loggerInstance;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::AddLogHandler
/////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Logger::AddLogHandler(LogHandler* logHandler)
{

    CAutoThreadSync CS(m_CS); // Lock the function
    int	i;
    for (i = 0; i < MAX_LOG_HANDLERS; i++)
    {
        if (m_logHandlers[i] == logHandler)
        {
            return CL_INT_LOGGER_FAILED;
        }
        if (m_logHandlers[i] == NULL)
        {
            m_logHandlers[i] = logHandler;
			return CL_SUCCESS;
        }
    }    
    return CL_INT_LOGGER_FAILED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Logger::Log
/////////////////////////////////////////////////////////////////////////////////////////
void Logger::Log(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, va_list va)
{        
    LogMessage	logMessage(level, sourceFile, functionName, sourceLine, message, va);    
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
void LoggerClient::Log(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, ...)
{         
    if (m_logLevel > level)
    {
        return;
    }    
    va_list va;
    va_start(va, message);

    Logger::GetInstance().Log(level, sourceFile, functionName,  sourceLine, message, va);

    va_end( va );      
}


