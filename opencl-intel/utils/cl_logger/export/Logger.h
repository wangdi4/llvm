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
#include "log_handler.h"
#include "cl_synch_objects.h"
#include "cl_user_logger.h"

namespace Intel { namespace OpenCL { namespace Utils {
    class IAtExitCentralPoint;

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
		LoggerClient(const char* handle, ELogLevel loglevel);

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
		void Log(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, ...);

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
		void LogArgList(ELogLevel level, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message, va_list va);

	private:
		char*			    m_handle;           // unique string handle representation
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
    void Log(ELogLevel level, ELogConfigField config, const char* psClientName, const char* sourceFile, const char* functionName, __int32 sourceLine, const char* message,  va_list va)
    {
        LogMessage	logMessage(level, config, psClientName, sourceFile, functionName, sourceLine, message, va);
        if (nullptr != g_pUserLogger && g_pUserLogger->IsErrorLoggingEnabled() && (LL_ERROR == level || LL_CRITICAL == level))
        {
            g_pUserLogger->PrintError(logMessage.GetFormattedMessage());
        }
        
        for (int i = 0; i < MAX_LOG_HANDLERS && m_logHandlers[i]; i++)
        {
            if (m_logHandlers[i] != nullptr)
            {
                m_logHandlers[i]->Log(logMessage);
            }
        }
    }

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
		const char*  GetLogHandlerParams(const char* handle);

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

		bool IsActive() const { return m_bIsActive; }

        /******************************************************************************************
        * Function:     RegisterAtExitNotification
        * Description:  store at_exit notification callback to be called immediately when DLL receives atexit() notification
        *               from OS BEFORE any other internal action.
        * Arguments:    [in] at_exit_notification_fn. NULL if need to disable callback
        * Return value:
        * Author:       Dmitry Kaptsenel
        * Date:         
        ******************************************************************************************/
        static void RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn );

	private:

		// each logging output generator is being represented by a LogHandler. Logger will
		// propagate the LogMessages to all registered LogHandlers. Each will decide whether to
		// emit the message according to its own LogLevel and the message's LogLevel.
		LogHandler*		m_logHandlers[MAX_LOG_HANDLERS];

		bool			m_bIsActive;

		// The class critical section object.

		OclMutex	m_CS;

	};

}}}
