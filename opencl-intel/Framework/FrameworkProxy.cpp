
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "FrameworkProxy.h"
#include "Logger.h"

using namespace Intel::OpenCL::Framework;

FrameworkProxy * FrameworkProxy::m_pInstance = NULL;

FrameworkProxy::FrameworkProxy()
{
	m_PlatformModule = NULL;
	m_ContextModule = NULL;
	m_ExecutionModule = NULL;
	m_pFileLogHandler = NULL;
	Initialize();
}
FrameworkProxy::~FrameworkProxy()
{
	Release();
}

void FrameworkProxy::Initialize()
{
	m_pFileLogHandler = new FileLogHandler(L"cl_framework");
	m_pFileLogHandler->Init(LL_DEBUG, L"C:\\cl.log");
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