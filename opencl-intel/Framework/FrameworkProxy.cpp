
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "FrameworkProxy.h"
#include "Logger.h"

using namespace Intel::OpenCL::Framework;

char clFRAMEWORK_CFG_PATH[MAX_PATH];

FrameworkProxy * FrameworkProxy::m_pInstance = NULL;

FrameworkProxy::FrameworkProxy()
{
	m_PlatformModule = NULL;
	m_ContextModule = NULL;
	m_ExecutionModule = NULL;
	m_pFileLogHandler = NULL;
	m_pConfigFile = NULL;
	Initialize();
}
FrameworkProxy::~FrameworkProxy()
{
	Release();
}

void FrameworkProxy::Initialize()
{
	// initialize configuration file
	m_pConfigFile = new ConfigFile(clFRAMEWORK_CFG_PATH);
	string str = m_pConfigFile->Read<string>(CL_CONFIG_LOG_FILE, "C:\\cl.cfg");
	//null-call to get the size
	size_t needed = ::mbstowcs(NULL,&str[0],str.length());
	// allocate
	std::wstring wstr;
	wstr.resize(needed);
	// real call
	::mbstowcs(&wstr[0],&str[0],str.length());
	const wchar_t *pout = wstr.c_str();

	// initialise logger
	m_pFileLogHandler = new FileLogHandler(L"cl_framework");
	m_pFileLogHandler->Init(LL_DEBUG, pout);
	Logger::GetInstance().AddLogHandler(m_pFileLogHandler);

	m_PlatformModule = new PlatformModule();
	m_PlatformModule->Initialize();
}

void FrameworkProxy::Release()
{
	m_pFileLogHandler->Flush();
	delete m_pFileLogHandler;

	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}
/**
* Instance
* Get the instance of the framework factory module
* 
* CFrameworkFactory* - referense to the instance of the framework factory class
* 
*/
FrameworkProxy* FrameworkProxy::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new FrameworkProxy();
	}
	return m_pInstance;
}

/**
* GetContextModule
* get the context module
*/
ContextModule* FrameworkProxy::GetContextModule(){

	return  NULL;
}


/**
* GetExecutionModule
* get the execution model
*/
ExecutionModule* FrameworkProxy::GetExecutionModule(){

	return  NULL;
}


/**
* GetPlatformModule
* get the platform module
*/
PlatformModule* FrameworkProxy::GetPlatformModule(){

	return  m_PlatformModule;
}