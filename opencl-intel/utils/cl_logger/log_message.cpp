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
using namespace Intel::OpenCL::Utils;

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Ctor Implementation
/////////////////////////////////////////////////////////////////////////////////////////
LogMessage::LogMessage(ELogLevel eLevel, 
					   ELogConfigField eConfig, 
					   wchar_t * pwsClientName,
					   wchar_t * pwsSourceFile, 
					   wchar_t * pwsFunctionName, 
					   __int32 i32SourceLine, 
					   wchar_t * pwsMessage, 
					   va_list va)
{
	m_bUnicodeMessage = true;

    m_eLogLevel        = eLevel;
	m_eLogConfig	   = eConfig;
    m_pwsMessage       = pwsMessage;
    m_pwsSourceFile    = pwsSourceFile;
    m_i32SourceLine    = i32SourceLine;
    m_pwsFunctionName  = pwsFunctionName;
    m_va               = va;  
	m_pwsClientName	   = pwsClientName;
    m_pwsFormattedMsg  = NULL;

    CreateFormattedMessageW();        
}

LogMessage::LogMessage(ELogLevel eLevel, 
					   ELogConfigField eConfig, 
					   char * psClientName,
					   char * psSourceFile, 
					   char * psFunctionName, 
					   __int32 i32SourceLine, 
					   char * psMessage, 
					   va_list va)
{
	m_bUnicodeMessage = false;

    m_eLogLevel        = eLevel;
	m_eLogConfig	   = eConfig;
    m_psMessage        = psMessage;
    m_psSourceFile     = psSourceFile;
    m_i32SourceLine    = i32SourceLine;
    m_psFunctionName   = psFunctionName;
    m_va               = va;   
	m_psClientName	   = psClientName;
	m_psFormattedMsg   = NULL;

    CreateFormattedMessage();        
}

/////////////////////////////////////////////////////////////////////////////////////////
// LogMessage Dtor
/////////////////////////////////////////////////////////////////////////////////////////

inline LogMessage::~LogMessage()
{
	if (m_bUnicodeMessage && m_pwsFormattedMsg)
	{
        delete[] m_pwsFormattedMsg;
	}
	else if (m_psFormattedMsg)
	{
		delete[] m_psFormattedMsg;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CreateFormattedMessage
/////////////////////////////////////////////////////////////////////////////////////////
void LogMessage::CreateFormattedMessageW()
{        
    
	wchar_t szLine[MAX_LOG_STRING_LENGTH] = {0};
	wcscat_s( szLine, MAX_LOG_STRING_LENGTH, L"\n" );

    //std::wostringstream tmpFormatMessage;
    //tmpFormatMessage << L"\n////////////////////////////////////////////////////////////////\n";
	
	// Message format:
	// <LEVEL>|<TAB>|<DATE>|<TAB>|<TIME>|<TAB>|<PID>|<TAB>|<TID>|<TAB>|<FILE>(<LINE#>)|<TAB>|<FUNC>|<TAB>|<MSG>
	
	// write log level
	switch (m_eLogLevel)
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

	// write client name
	if ( (m_eLogConfig & LCF_LINE_CLIENT_NAME) && m_pwsClientName != NULL && wcslen(m_pwsClientName) > 0)
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws\t", m_pwsClientName);
	}

	// get time and date
	time_t tNow =0;
    tm tmNow;
	tNow = time( NULL );
    gmtime_s( &tmNow, &tNow );

	// date
	if (m_eLogConfig & LCF_LINE_DATE)
	{
		wcsftime( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%x\t", &tmNow );
	}

	// time
	if (m_eLogConfig & LCF_LINE_TIME)
	{
		wcsftime( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%X\t", &tmNow );
	}

	// write process ID
	if (m_eLogConfig & LCF_LINE_PID)
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%d\t", GetCurrentProcessId());
	}

	// write thread ID
	if (m_eLogConfig & LCF_LINE_TID)
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%d\t", GetCurrentThreadId());
	}

	// write source file name
	if (NULL != m_pwsSourceFile && wcslen(m_pwsSourceFile) > 0)
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws\t", m_pwsSourceFile);
	}

	// write line number
	if (m_i32SourceLine >= 0)
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"(%d)\t", m_i32SourceLine);
	}

	// write function name
	if (NULL != m_pwsFunctionName && wcslen(m_pwsFunctionName) > 0)	
	{
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws\t", m_pwsFunctionName);
	}	 

	// write message
	int len = _vscwprintf( m_pwsMessage, m_va ) + 1;
	wchar_t* tmpBuffer;
    tmpBuffer = new wchar_t[len * sizeof(wchar_t)];
    if (tmpBuffer)
    {
        vswprintf( tmpBuffer, len, m_pwsMessage, m_va );
		swprintf_s( &szLine[wcslen( szLine )], MAX_LOG_STRING_LENGTH - wcslen( szLine ), L"%ws", tmpBuffer);
        delete[]  tmpBuffer;    
    } 

	m_pwsFormattedMsg = new wchar_t[MAX_LOG_STRING_LENGTH];
	if (m_pwsFormattedMsg)
	{
		wcscpy_s(m_pwsFormattedMsg, MAX_LOG_STRING_LENGTH, szLine);
	}
  
}
void LogMessage::CreateFormattedMessage()
{        
    
	char szLine[MAX_LOG_STRING_LENGTH] = {0};
	strcat_s(szLine, MAX_LOG_STRING_LENGTH, "\n");

    //std::wostringstream tmpFormatMessage;
    //tmpFormatMessage << L"\n////////////////////////////////////////////////////////////////\n";
	
	// Message format:
	// <LEVEL>|<TAB>|<DATE>|<TAB>|<TIME>|<TAB>|<PID>|<TAB>|<TID>|<TAB>|<FILE>(<LINE#>)|<TAB>|<FUNC>|<TAB>|<MSG>
	
	// write log level
	switch (m_eLogLevel)
	{
	case LL_DEBUG:
		strcat_s( szLine, MAX_LOG_STRING_LENGTH, "DEBUG\t" );
		break;
	case LL_INFO:
		strcat_s( szLine, MAX_LOG_STRING_LENGTH, "INFO\t" );
		break;
	case LL_ERROR:
		strcat_s( szLine, MAX_LOG_STRING_LENGTH, "ERROR\t" );
		break;
	case LL_CRITICAL:
		strcat_s( szLine, MAX_LOG_STRING_LENGTH, "CRITICAL\t" );
		break;
	case LL_STATISTIC:
		strcat_s( szLine, MAX_LOG_STRING_LENGTH, "STATISTIC\t" );
		break;
	}

	// write client name
	if ( (m_eLogConfig & LCF_LINE_CLIENT_NAME) && m_psClientName != NULL && strlen(m_psClientName) > 0)
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%s\t", m_psClientName);
	}

	// get time and date
	time_t tNow =0;
    tm tmNow;
	tNow = time( NULL );
    gmtime_s( &tmNow, &tNow );

	// date
	if (m_eLogConfig & LCF_LINE_DATE)
	{
		
		strftime( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%x\t", &tmNow );
	}

	// time
	if (m_eLogConfig & LCF_LINE_TIME)
	{
		strftime( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%X\t", &tmNow );
	}

	// write process ID
	if (m_eLogConfig & LCF_LINE_PID)
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%d\t", GetCurrentProcessId());
	}

	// write thread ID
	if (m_eLogConfig & LCF_LINE_TID)
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%d\t", GetCurrentThreadId());
	}

	// write source file name
	if (NULL != m_psSourceFile && strlen(m_psSourceFile) > 0)
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%s\t", m_psSourceFile);
	}

	// write line number
	if (m_i32SourceLine >= 0)
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "(%d)\t", m_i32SourceLine);
	}

	// write function name
	if (NULL != m_psFunctionName && strlen(m_psFunctionName) > 0)	
	{
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%s\t", m_psFunctionName);
	}	 

	// write message
	int len = _vscprintf( m_psMessage, m_va ) + 1;
	char* tmpBuffer;
    tmpBuffer = new char[len * sizeof(char)];
    if (tmpBuffer)
    {
        vsprintf_s( tmpBuffer, len, m_psMessage, m_va );
		sprintf_s( &szLine[strlen( szLine )], MAX_LOG_STRING_LENGTH - strlen( szLine ), "%s", tmpBuffer);
        delete[]  tmpBuffer;    
    } 

	m_psFormattedMsg = new char[MAX_LOG_STRING_LENGTH];
	if (m_psFormattedMsg)
	{
		strcpy_s(m_psFormattedMsg, MAX_LOG_STRING_LENGTH, szLine);
	}
  
}