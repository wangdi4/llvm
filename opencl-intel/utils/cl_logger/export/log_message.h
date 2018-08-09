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

#pragma once

#include <stdio.h>
#include <cstdarg>
#include "cl_logger.h"

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
					const char *			psClientName,
					const char *			psSourceFile,
					const char *			psFunctionName,
					__int32			        i32SourceLine,
					const char *			psMessage,
					va_list			va );

		virtual	~LogMessage();

		bool IsUnicode() { return m_bUnicodeMessage; }

		// return message body
		const char * GetLogMessage() { return m_psMessage; }

		// GetFunctionName
		// return message function name
		const char* GetFunctionName() { return m_psFunctionName; }

		// GetSourceFile
		// return message source filename
		const char* GetSourceFile() { return m_psSourceFile; }

		// GetsourceLine
		// return message source line number
		__int32 GetsourceLine() { return m_i32SourceLine; }

		// GetLogLevel
		// return message log level
		ELogLevel GetLogLevel() { return m_eLogLevel; }

		// GetLogArgs
		// return message arguments
//		va_list GetLogArgs() { return m_va; }

		// GetFormattedMessage
		// returns a full formatted message for logging
		// returned message includes all message properties (FileName, LineNumber, FunctionName,
		// ThreadID, Body) in a single string.
		char* GetFormattedMessage() { return m_psFormattedMsg; }

    private:

		// CreateFormattedMessage
		// construct formatted message from its fields and stores the result into m_formattedMsg
		void CreateFormattedMessage();
		void CreateFormattedMessageW();
		LogMessage(const LogMessage&);
		LogMessage& operator=(const LogMessage& other);

		bool			m_bUnicodeMessage;

		ELogLevel		m_eLogLevel;			// message log level
		ELogConfigField	m_eLogConfig;			// configuration flag
		__int32			m_i32SourceLine;		// message line number at sourceFile
		va_list			m_va;					// message argument list

		const char*		m_psMessage;			// ptr to message body
		const char*		m_psSourceFile;			// message source filename
		const char*		m_psFunctionName;		// message function name
		char*		    m_psFormattedMsg;		// formatted message including header
		const char*		m_psClientName;			// client name
	};

}}}
