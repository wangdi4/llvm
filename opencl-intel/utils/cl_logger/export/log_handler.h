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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  LogHandler.h     
//  mplementation of the log handler class 
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_logger.h"
#include "log_message.h"
#include "cl_synch_objects.h"
#include <stdio.h>  

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
		LogHandler(){};
		
		/******************************************************************************************
		* Function: 	~LogHandler
		* Description:	The LogHandler class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		virtual ~LogHandler(){};
		
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
		
		virtual void LogW(LogMessage& message) = 0;    
		
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
		virtual cl_err_code Init(ELogLevel level, const wchar_t* fileName, const wchar_t* title) =0;
		
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

		wchar_t*		m_handle;         // unique string handle representation
		ELogLevel		m_logLevel;       // log handler log level (ignore levels < m_logLevel)
		OclMutex		m_CS;             // Log function Critical Section object
	};

	/**********************************************************************************************
	* Class name:	FileLogHandler
	*
	* Inharit:		LogHandler
	* Description:	simple file logger. dumps all log messages to file 'm_fileName'
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class FileLogHandler : public LogHandler
	{

	public:

		/******************************************************************************************
		* Function: 	FileLogHandler
		* Description:	The FileLogHandler class constructor
		* Arguments:	handle [in] -	unique string handle
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		FileLogHandler(const wchar_t* handle);

		/******************************************************************************************
		* Function: 	~FileLogHandler
		* Description:	The FileLogHandler class destructor
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
		*				title [in]		- title to be used for logger, NULL for default title
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code Init(ELogLevel level, const wchar_t* fileName, const wchar_t* title);
		
		/******************************************************************************************
		* Function: 	Log    
		* Description:	log message
		* Arguments:	logMessage [in] -	wrappes all message info
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Log(LogMessage& logMessage);

		void LogW(LogMessage& logMessage);
		
		/******************************************************************************************
		* Function: 	Flush    
		* Description:	dump data to file
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Flush();

	private:
		wchar_t*	m_fileName;             // filename of the logging file
		FILE*   m_fileHandler;          // file handle of the logging file
	};


	/**********************************************************************************************
	* Class name:	ConsoleLogHandler
	*
	* Inharit:		LogHandler
	* Description:	simple file logger. dumps all log messages to stdout
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class ConsoleLogHandler : public LogHandler
	{

	public:

		/******************************************************************************************
		* Function: 	ConsoleLogHandler
		* Description:	The ConsoleLogHandler class constructor
		* Arguments:	handle [in] -	unique string handle	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ConsoleLogHandler(const wchar_t* handle);
		
		/******************************************************************************************
		* Function: 	~ConsoleLogHandler
		* Description:	The ConsoleLogHandler class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		~ConsoleLogHandler(){};
		
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
		cl_err_code Init(ELogLevel level, wchar_t* fileName = NULL);
		
		/******************************************************************************************
		* Function: 	Log    
		* Description:	log message
		* Arguments:	logMessage [in] -	wrappes all message info
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Log(LogMessage& logMessage);    

		void LogW(LogMessage& logMessage);    
		
		/******************************************************************************************
		* Function: 	Flush    
		* Description:	dump data to file
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void Flush();
	};

}}};