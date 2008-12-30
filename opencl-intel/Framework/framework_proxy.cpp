
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "framework_proxy.h"
#include "logger.h"
#include <windows.h>

using namespace Intel::OpenCL::Framework;

char clFRAMEWORK_CFG_PATH[MAX_PATH];

FrameworkProxy * FrameworkProxy::m_pInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::FrameworkProxy()
{
	m_pPlatformModule = NULL;
	m_pContextModule = NULL;
	m_pExecutionModule = NULL;
	m_pFileLogHandler = NULL;
	m_pConfigFile = NULL;
	m_pLoggerClient = NULL;

	Initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// ~FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::~FrameworkProxy()
{
	InfoLog(m_pLoggerClient, L"FrameworkProxy destructor enter");
	Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Initialize()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Initialize()
{
	// initialize configuration file
	m_pConfigFile = new ConfigFile(clFRAMEWORK_CFG_PATH);
	// check if need to use logger
	bool bUseLogger = m_pConfigFile->Read<bool>(CL_CONFIG_USE_LOGGER, false);
	if (bUseLogger)
	{
		string str = m_pConfigFile->Read<string>(CL_CONFIG_LOG_FILE, "");
		if (str != "")
		{
			//null-call to get the size
			size_t needed;
			::mbstowcs_s(&needed, NULL, 0, &str[0], str.length());
			// allocate
			std::wstring wstr;
			wstr.resize(needed);
			// real call
			::mbstowcs_s(&needed, &wstr[0], wstr.length(), &str[0], str.length());
			const wchar_t *pout = wstr.c_str();

			// initialise logger
			m_pFileLogHandler = new FileLogHandler(L"cl_framework");
			cl_err_code clErrRet = m_pFileLogHandler->Init(LL_DEBUG, (wchar_t*)pout);
			if (CL_SUCCEEDED(clErrRet))
			{
				Logger::GetInstance().AddLogHandler(m_pFileLogHandler);
			}
		}
	}
	m_pLoggerClient = new LoggerClient(L"Framework Proxy Logger Clinet", LL_DEBUG);
	
	InfoLog(m_pLoggerClient, L"Initialize platform module: m_PlatformModule = new PlatformModule()");
	m_pPlatformModule = new PlatformModule();
	m_pPlatformModule->Initialize(m_pConfigFile);

	InfoLog(m_pLoggerClient, L"Initialize context module: m_pContextModule = new ContextModule(%d)",m_pPlatformModule);
	m_pContextModule = new ContextModule(m_pPlatformModule);
	m_pContextModule->Initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Release()
{
	InfoLog(m_pLoggerClient, L"FrameworkProxy::Release enter");
	
	m_pContextModule->Release();
	delete m_pContextModule;
	
	m_pPlatformModule->Release();
	delete m_pPlatformModule;

	m_pFileLogHandler->Flush();
	delete m_pFileLogHandler;

	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Instance()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy* FrameworkProxy::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new FrameworkProxy();
	}
	return m_pInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::GetContextModule()
///////////////////////////////////////////////////////////////////////////////////////////////////
ContextModule* FrameworkProxy::GetContextModule()
{
	InfoLog(m_pLoggerClient, L"FrameworkProxy::GetContextModule enter: m_pContextModule=%d", m_pContextModule);
	return  m_pContextModule;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::GetExecutionModule()
///////////////////////////////////////////////////////////////////////////////////////////////////
ExecutionModule* FrameworkProxy::GetExecutionModule()
{
	InfoLog(m_pLoggerClient, L"FrameworkProxy::GetExecutionModule enter: m_pExecutionModule=%d", m_pExecutionModule);
	return  m_pExecutionModule;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::GetPlatformModule()
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule* FrameworkProxy::GetPlatformModule()
{
	InfoLog(m_pLoggerClient, L"FrameworkProxy::GetPlatformModule enter: m_PlatformModule=%d", m_pPlatformModule);
	return  m_pPlatformModule;
}