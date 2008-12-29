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
 *  LogMessage.cpp
 *  Created on: 10-Dec-2008 11:42:24 AM                      
 *  Implementation of the log message class
 *  Original author: ulevy                     
 ****************************************************/

#include "log_message.h"
#include <stdarg.h>
#include <sstream>              // required by: owstringstream
#include <assert.h>
#include <time.h>
#include <windows.h>
using namespace Intel::OpenCL::Framework;

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
LogMessage::LogMessage(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, va_list va)
{
    m_logLevel      = level;
    m_message       = message;
    m_sourceFile    = sourceFile;
    m_sourceLine    = sourceLine;
    m_functionName  = functionName;
    m_va            = va;   
    m_formattedMsg  = NULL;

    CreateFormattedMessage();        
}

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Dtor
/////////////////////////////////////////////////////////////////////////////////////////

inline LogMessage::~LogMessage()
{
    if (m_formattedMsg)
	{
        delete m_formattedMsg;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CreateFormattedMessage
/////////////////////////////////////////////////////////////////////////////////////////
void LogMessage::CreateFormattedMessage()
{        
    
	wchar_t szLine[MAX_LOG_STRING_LENGTH] = {0};
	wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"\n" );

    //std::wostringstream tmpFormatMessage;
    //tmpFormatMessage << L"\n////////////////////////////////////////////////////////////////\n";
	
	// Message format:
	// <LEVEL>|<TAB>|<DATE>|<TAB>|<TIME>|<TAB>|<PID>|<TAB>|<TID>|<TAB>|<FILE>(<LINE#>)|<TAB>|<FUNC>|<TAB>|<MSG>
	
	// write log level
	switch (m_logLevel)
	{
	case LL_DEBUG:
		wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"DEBUG\t" );
		break;
	case LL_INFO:
		wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"INFO\t" );
		break;
	case LL_ERROR:
		wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"ERROR\t" );
		break;
	case LL_CRITICAL:
		wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"CRITICAL\t" );
		break;
	case LL_STATISTIC:
		wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"STATISTIC\t" );
		break;
	}

	// get time and date
	time_t tNow =0;
    tm tmNow;
	tNow = time( NULL );
    gmtime_s( &tmNow, &tNow );

	// date
	wcsftime( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%x\t", &tmNow );
	// time
    wcsftime( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%X\t", &tmNow );	

	// write process ID
	swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%d\t", GetCurrentProcessId());

	// write thread ID
	swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%d\t", GetCurrentThreadId());

	// write source file name
	swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), m_sourceFile);

	// write line number
	swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"(%d)\t", m_sourceLine);

	// write function name
	if (wcslen(m_functionName) > 0)	
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws\t", m_functionName);
	}	 

	// write message
	int len = _vscwprintf( m_message, m_va ) + 1;
	wchar_t* tmpBuffer;
    tmpBuffer = new wchar_t[len * sizeof(wchar_t)];
    if (tmpBuffer)
    {
        vswprintf( tmpBuffer, len, m_message, m_va );
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws", tmpBuffer);
        delete  tmpBuffer;    
    } 

	m_formattedMsg = new wchar_t[MAX_LOG_STRING_LENGTH];
	if (m_formattedMsg)
	{
		wcscpy_s(m_formattedMsg, MAX_LOG_STRING_LENGTH, szLine);
	}
  
}