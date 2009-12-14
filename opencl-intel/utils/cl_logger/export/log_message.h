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

#pragma once
/****************************************************
 *  LogMessage.h                                         
 *  Created on: 10-Dec-2008 11:42:24 AM                      
 *  Implementation of the log message class       
 *  Original author: ulevy                     
 ****************************************************/


#include "cl_logger.h"
#include <stdio.h>  

namespace Intel { namespace OpenCL { namespace Utils {

#define MAX_LOG_STRING_LENGTH 512

	// Class name: LogMessage
	//
	// each message logged to the logger is translated into LogMessage structure which can be
	// propagated between the different handlers.
	class LogMessage
	{
	public:

		// LogMessage - initiliaze log message
		// level [in]		- message LogLevel
		// sourceFile [in]	- message source file name
		// sourceLine [in]	- message source line number
		// message [in]		- message to be logged
		// va [in]			- message arguments
		LogMessage(	ELogLevel		eLevel,
					ELogConfigField	eConfig,
					wchar_t *		pwsClientName,
					wchar_t *		pwsSourceFile, 
					wchar_t *		pwsFunctionName, 
					__int32			i32SourceLine,
					wchar_t *		pwsMessage, 
					va_list			va );

		LogMessage(	ELogLevel		eLevel,
					ELogConfigField	eConfig,
					char *			psClientName,
					char *			psSourceFile, 
					char *			psFunctionName, 
					__int32			i32SourceLine,
					char *			psMessage, 
					va_list			va );

		virtual	~LogMessage();
	    
		bool IsUnicode() { return m_bUnicodeMessage; }

		// return message body
		char * GetLogMessage() { return m_psMessage; }
		
		wchar_t * GetLogMessageW() { return m_pwsMessage; }
		
		// GetFunctionName
		// return message function name
		wchar_t* GetFunctionName() { return m_pwsFunctionName; }
		
		// GetSourceFile
		// return message source filename
		wchar_t* GetSourceFile() { return m_pwsSourceFile; }
		
		// GetsourceLine
		// return message source line number
		__int32 GetsourceLine() { return m_i32SourceLine; }
		
		// GetLogLevel
		// return message log level
		ELogLevel GetLogLevel() { return m_eLogLevel; }
		
		// GetLogArgs
		// return message arguments
		va_list GetLogArgs() { return m_va; }
		
		// GetFormattedMessage
		// returns a full formatted message for logging
		// returned message includes all message properties (FileName, LineNumber, FunctionName, 
		// ThreadID, Body) in a single string.
		char* GetFormattedMessage() { return m_psFormattedMsg; }
		wchar_t* GetFormattedMessageW() { return m_pwsFormattedMsg; }

	private:

		// CreateFormattedMessage
		// construct formatted message from its fields and stores the result into m_formattedMsg
		void CreateFormattedMessage();
		void CreateFormattedMessageW();

		bool			m_bUnicodeMessage;

		ELogLevel		m_eLogLevel;			// message log level
		ELogConfigField	m_eLogConfig;			// configuration flag
		__int32			m_i32SourceLine;		// message line number at sourceFile
		va_list			m_va;					// message argument list

		wchar_t*		m_pwsMessage;			// ptr to wide character message body
		wchar_t*		m_pwsSourceFile;		// wide character message source filename
		wchar_t*		m_pwsFunctionName;		// wide character message function name
		wchar_t*		m_pwsFormattedMsg;		// formatted wide character message including header 
		wchar_t*		m_pwsClientName;		// wide character client name

		char*		m_psMessage;			// ptr to message body
		char*		m_psSourceFile;			// message source filename
		char*		m_psFunctionName;		// message function name
		char*		m_psFormattedMsg;		// formatted message including header 
		char*		m_psClientName;			// client name
	};

}}};