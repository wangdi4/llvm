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

#include "cl_logger.h"
#include "log_message.h"
#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace Utils {

	/**********************************************************************************************
	* Class name:	LogHandler
	*
	* Description:	each logging output processor is being represented by a logHandler. for
	*				example, file and console are log handlers. Logger dispatches logging requests
	*				to all log handlers registered to it.
	*				log handlers will output only messages with level >= m_logLevel in order to
	*				turn off a log handler, its m_logLevel need to be set to LEVEL_OFF
	*				framework's moduls
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class LogHandler
	{
	public:

		/******************************************************************************************
		* Function: 	LogHandler
		* Description:	The LogHandler class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		LogHandler() : m_handle(nullptr)
        {};

		/******************************************************************************************
		* Function: 	~LogHandler
		* Description:	The LogHandler class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~LogHandler()
        {
			assert(m_handle==nullptr && "Base handle expected to be released");
        };

		/******************************************************************************************
		* Function: 	Log
		* Description:	logs message to log handler
		*				log handlers, such as file, console and others need to implement this
		*				function separately. or example FileLogger with write message to file while
		*				console with send it to output stream (i.e. stdout).
		* Arguments:	message [in] -	message to be logged
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual void Log(LogMessage& message) = 0;

		/******************************************************************************************
		* Function: 	Init
		* Description:	intializes log handler
		*				each log handler queries the Logger instance for its configuration and
		*				initializes its self accordingly
		* Arguments:	level [in] -	log level
		*				fileName [in] -	file name
		*				title [in]		- title to be used for logger, NULL for default title
		*
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code Init(ELogLevel level, const char* fileName, const char* title, FILE* fileDesc = stderr) =0;

		/******************************************************************************************
		* Function: 	Flush
		* Description:	Make sure all log messages are dumped to their destination upon return
		*				from this function
		* Arguments:
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual void Flush() = 0;

	protected:

		char*           m_handle;         // unique string handle representation
		ELogLevel		m_logLevel;       // log handler log level (ignore levels < m_logLevel)
	};

	/**********************************************************************************************
	* Class name:	FileLogHandler
	*
	* Inharit:		LogHandler
	* Description:	simple file logger. dumps all log messages to file 'm_fileName'
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class FileDescriptorLogHandler : public LogHandler
	{

	public:

		/******************************************************************************************
		* Function: 	FileLogHandler
		* Description:	The FileLogHandler class constructor
		* Arguments:	handle [in] -	unique string handle
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		FileDescriptorLogHandler(const char* handle);

		/******************************************************************************************
		* Function: 	~FileLogHandler
		* Description:	The FileLogHandler class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		~FileDescriptorLogHandler();

		/******************************************************************************************
		* Function: 	Init
		* Description:	initialize log handler. retrieve configuration from Logger by calling
		*				Logger::GetLogHandlerParams
		* Arguments:	level [in] -	log level
		*				fileName [in] -	file name
		*				title [in]		- title to be used for logger, NULL for default title
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code Init(ELogLevel level, const char* fileName, const char* title, FILE* fileDesc = stderr);

		/******************************************************************************************
		* Function: 	Log
		* Description:	log message
		* Arguments:	logMessage [in] -	wrappes all message info
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Log(LogMessage& logMessage);

		/******************************************************************************************
		* Function: 	Flush
		* Description:	dump data to file
		* Arguments:
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Flush();

	protected:
		FILE*   m_fileHandler;          // file handle of the logging file

	private:
		int     m_dupStderr;             // duplicate file descriptor of stderr. (stderr redirect to m_fileName in order to get log messages from MIC device)

	};


	/**********************************************************************************************
	* Class name:	ConsoleLogHandler
	*
	* Inharit:		LogHandler
	* Description:	simple file logger. dumps all log messages to stderr (as default) or other file descriptor (i.e.: stdout / file)
	**********************************************************************************************/
	class FileLogHandler : public FileDescriptorLogHandler
	{

	public:

		/******************************************************************************************
		* Function: 	ConsoleLogHandler
		* Description:	The ConsoleLogHandler class constructor
		* Arguments:	handle [in] -	unique string handle
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		FileLogHandler(const char* handle);

		/******************************************************************************************
		* Function: 	~ConsoleLogHandler
		* Description:	The ConsoleLogHandler class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		~FileLogHandler();

		/******************************************************************************************
		* Function: 	Init
		* Description:	initialize log handler. retrieve configuration from Logger by calling
		*				Logger::GetLogHandlerParams
		* Arguments:	level [in] -	log level
		*				fileName [in] -	file name
		* Return value:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code Init(ELogLevel level, const char* fileName, const char* title = nullptr, FILE* fileDesc = stderr);

	private:
        FileLogHandler& operator=(const FileLogHandler&);
        FileLogHandler(const FileLogHandler&);

		char*	m_fileName;             // filename of the logging file

	};

}}}
