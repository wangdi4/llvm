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
 *  cl_logger.h                                         
 *  Created on: 10-Dec-2008 11:42:24 AM                      
 *  Implementation of the logger       
 *  Original author: ulevy                     
 ****************************************************/


#include <cl_types.h>
#include <stdio.h>  

namespace Intel { namespace OpenCL { namespace Utils {

#define USE_LOGGER

/** 
* define MAX_LOG_HANDLERS
*
* define the maximal number of LogHandlers
* this number represents how many loghandlers be registered to the Logger in a single run.
**/
#define MAX_LOG_HANDLERS 128

	/** 
	* typedef Message level
	*
	* the enumeration has a heirarchical structure
	*  CRITICAL - 
	*  INFO     - 
	*  DEBUG    - 
	**/
	enum ELogLevel
	{   	
		LL_DEBUG     = 100,
		LL_INFO      = 200,
		LL_ERROR     = 300,
		LL_CRITICAL  = 400,	
		LL_STATISTIC	= 500,
		LL_OFF       = 1000
	};

	enum ELogConfigField
	{
		LCF_NONE                    = 0x0000,   //!< Empty configuration
		LCF_LINE_PID				= 0x0001,   //!< Include process id to log message prefix
		LCF_LINE_TID                = 0x0002,   //!< Include thread id to log message prefix
		LCF_LINE_DATE               = 0x0004,   //!< Include date to log message prefix
		LCF_LINE_TIME               = 0x0008,   //!< Include time to log message prefix
		LCF_LINE_CLIENT_NAME        = 0x0010,   //!< Include client name to log message prefix
		LCF_LINE_LOG_LEVEL          = 0x0020,   //!< Include log level to log message prefix
		LCF_LINE_ALL                = 0x003F,   //!< Include all line options to log message prefix
	};


/** 
 * define logging helper macros
 *
 * for every loglevel there is a specified macro for customizing the print message
 */

#define	DECLARE_LOGGER_CLIENT			\
	Intel::OpenCL::Utils::LoggerClient * m_pLoggerClient;

#define SET_LOGGER_CLIENT(CLIENT)		\
	m_pLoggerClient = CLIENT;

#define GET_LOGGER_CLIENT	m_pLoggerClient

#define INIT_LOGGER_CLIENT(NAME, LEVEL)											\
	m_pLoggerClient = NULL;														\
	if (Logger::GetInstance().IsActive()){										\
		m_pLoggerClient = new Intel::OpenCL::Utils::LoggerClient(NAME, LEVEL);	\
	}
	
#define RELEASE_LOGGER_CLIENT			\
	if (NULL != m_pLoggerClient){		\
		delete m_pLoggerClient;			\
		m_pLoggerClient = NULL;			\
	}

#ifdef _DEBUG
#define LogDebugW(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->LogW(LL_DEBUG		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define LogDebugW(DBG_PRINT, ...)
#endif
#define LogInfoW(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->LogW(LL_INFO		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogErrorW(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->LogW(LL_ERROR		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogCriticalW(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->LogW(LL_CRITICAL	, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogStatisticW(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->LogW(LL_STATISTIC	, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);

#ifdef _DEBUG
#define LogDebugA(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->Log(LL_DEBUG		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define LogDebugA(DBG_PRINT, ...)
#endif
#define LogInfoA(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->Log(LL_INFO		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogErrorA(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->Log(LL_ERROR		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogCriticalA(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->Log(LL_CRITICAL	, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define LogStatisticA(DBG_PRINT, ...)		\
	if (m_pLoggerClient) (m_pLoggerClient)->Log(LL_STATISTIC	, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);

#ifdef _DEBUG
#define DbgLogW(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) (CLIENT)->LogW(LL_DEBUG		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define DbgLogW(DBG_PRINT, ...)
#endif
#define InfoLogW(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) CLIENT->LogW(LL_INFO		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define ErrLogW(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) CLIENT->LogW(LL_ERROR		, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);
#define CriticLogW(CLIENT, DBG_PRINT, ...)		\
	if (CLIENT) CLIENT->LogW(LL_CRITICAL	, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT,  __VA_ARGS__);

#ifdef _DEBUG
#define DbgLogA(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) (CLIENT)->Log(LL_DEBUG		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#else
#define DbgLogA(DBG_PRINT, ...)
#endif
#define InfoLogA(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) CLIENT->Log(LL_INFO		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define ErrLogA(CLIENT, DBG_PRINT, ...)			\
	if (CLIENT) CLIENT->Log(LL_ERROR		, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);
#define CriticLogA(CLIENT, DBG_PRINT, ...)		\
	if (CLIENT) CLIENT->Log(LL_CRITICAL	, __FILE__, __FUNCTION__, __LINE__, DBG_PRINT,  __VA_ARGS__);


#ifdef _UNICODE

#define LOG_DEBUG		LogDebugW
#define	LOG_INFO		LogInfoW
#define	LOG_ERROR		LogErrorW
#define LOG_CRITICAL	LogCriticalW
#define LOG_STATISTIC	LogStatisticW

#define DbgLog		DbgLogW
#define InfoLog		InfoLogW
#define ErrLog		ErrLogW
#define CriticLog	CriticLogW

#else

#define LOG_DEBUG		LogDebugA
#define	LOG_INFO		LogInfoA
#define	LOG_ERROR		LogErrorA
#define LOG_CRITICAL	LogCriticalA
#define LOG_STATISTIC	LogStatisticA

#define DbgLog		DbgLogA
#define InfoLog		InfoLogA
#define ErrLog		ErrLogA
#define CriticLog	CriticLogA

#endif

}}};