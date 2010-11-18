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
//  Logger.h     
//  mplementation of the logger 
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_logger.h"
#include "log_message.h"
#include "log_handler.h"
#include "cl_synch_objects.h"
#include <stdio.h>  

namespace Intel { namespace OpenCL { namespace Utils {

	/**********************************************************************************************
	* Class name:	LoggerClient
	*
	* Description:	Module programmer interface for logging
	*				Programmers willing to log messages in their module, need to create a logger 
	*				client for their module (or more than one), and log messages using it's Log 
	*				interface
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class LoggerClient
	{
	public:

		/******************************************************************************************
		* Function: 	LoggerClient
		* Description:	The LoggerClient class constructor - creater logger client
		* Arguments:	handle [in] -	client unique string handle
		*				loglevel [in] -	log level
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		LoggerClient(wchar_t* handle, ELogLevel loglevel);
		
		/******************************************************************************************
		* Function: 	~LoggerClient
		* Description:	The LoggerClient class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		virtual ~LoggerClient();    
		
		/******************************************************************************************
		* Function: 	Log(W)
		* Description:	log message to the logger client
		* Arguments:	level [in] -		message log level
		*				sourceFile [in] -	the filename where the message was generated 
		*				functionName [in] -	the functionName where the message was generated
		*				sourceLine [in] -	the sourceLine where the message was generated 
		*				message [in] -		the message body with a variable arguments list
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		void Log(ELogLevel level, char* sourceFile, char* functionName, __int32 sourceLine, char* message, ...);

		void LogW(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, ...);

		/******************************************************************************************
		* Function: 	Log    
		* Description:	log message to the logger client
		* Arguments:	level [in] -		message log level
		*				sourceFile [in] -	the filename where the message was generated 
		*				functionName [in] -	the functionName where the message was generated
		*				sourceLine [in] -	the sourceLine where the message was generated 
		*				message [in] -		the message body with a variable arguments list
		*				va [in]	-			the message variable arguments list
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		void LogArgList(ELogLevel level, char* sourceFile, char* functionName, __int32 sourceLine, char* message, va_list va);

		void LogArgListW(ELogLevel level, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message, va_list va);

	private:
		wchar_t*			m_handle;           // unique string handle representation
		ELogLevel			m_logLevel;         // client log level (ignore levels < m_logLevel)
		ELogConfigField		m_eLogConfig;		// configuration flags

	};

	/**********************************************************************************************
	* Class name:	Logger
	*
	* Description:	represents the logging interface for clients.
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class Logger
	{
	public:

		/******************************************************************************************
		* Function: 	Logger
		* Description:	The Logger class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		Logger();
		
		/******************************************************************************************
		* Function: 	~Logger
		* Description:	The Logger class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~Logger();
		
		/******************************************************************************************
		* Function: 	GetInstance    
		* Description:	returns the Logger instance
		*				The logger is implemented as a singleton. in order to get an instance of
		*				the logger module users should call this method
		* Arguments:	
		* Return value:	static Logger& - an instance of the logger
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		static Logger& GetInstance();        
		             
		/******************************************************************************************
		* Function: 	Log    
		* Description:	instruct the logger to log message. the logger will propagate the message 
		*				to all log handlers. 
		* Arguments:	level [in]			- message log level
		*				sourceFile [in]		- the filename where the message was generated   
		*				functionName [in]	- the functionName where the message was generated   
		*				sourceLine [in]		- the sourceLine where the message was generated   
		*				message [in]		- the message body
		*				va [in]				- the message variable arguments list
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		void Log(ELogLevel level, ELogConfigField config, char* psClientName, char* sourceFile, char* functionName, __int32 sourceLine, char* message,  va_list va);       

		void LogW(ELogLevel level, ELogConfigField config, wchar_t* pwsClientName, wchar_t* sourceFile, wchar_t* functionName, __int32 sourceLine, wchar_t* message,  va_list va);       
		
		/******************************************************************************************
		* Function: 	GetLogHandlerParams    
		* Description:	add new logger handler
		*				Logger dispatches log messages to all Added loghandlers. in case log 
		*				handler's level <= logMessage's level, the message will be sent to output
		* Arguments:	handle [in] - log handler unique string handle
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		wchar_t*  GetLogHandlerParams(wchar_t* handle);
		
		/******************************************************************************************
		* Function: 	AddLogHandler    
		* Description:	add new logger handler
		*				Logger dispatches log messages to all Added loghandlers. in case log 
		*				handler's level <= logMessage's level, the message will be sent to output
		* Arguments:	logHandler [in] - pointer to LogHandler instance
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code AddLogHandler(LogHandler* logHandler);

		void SetActive(const bool bActive) { m_bIsActive = bActive; }

		const bool IsActive() const { return m_bIsActive; }

	private:

		// each logging output generator is being represented by a LogHandler. Logger will 
		// propagate the LogMessages to all registered LogHandlers. Each will decide whether to 
		// emit the message according to its own LogLevel and the message's LogLevel.
		LogHandler*		m_logHandlers[MAX_LOG_HANDLERS];        
		
		bool			m_bIsActive;

		// The class critical section object.
		
		OclMutex	m_CS;             

	};


}}};