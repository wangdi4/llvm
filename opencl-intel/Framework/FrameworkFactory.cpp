
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "FrameworkFactory.h"

using namespace Intel::OpenCL::Framework;

FrameworkFactory * FrameworkFactory::m_pInstance = NULL;

FrameworkFactory::FrameworkFactory() : 
m_PlatformModule(NULL),
m_ContextModule(NULL),
m_ExecutionModule(NULL)
{
	Initialize();
}
FrameworkFactory::~FrameworkFactory()
{
	Release();
}

void FrameworkFactory::Initialize()
{
	m_PlatformModule = new PlatformModule();
	m_PlatformModule->Initialize();
}

void FrameworkFactory::Release()
{
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
FrameworkFactory* FrameworkFactory::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new FrameworkFactory();
	}
	return m_pInstance;
}

/**
* GetContextModule
* get the context module
*/
ContextModule* FrameworkFactory::GetContextModule(){

	return  NULL;
}


/**
* GetExecutionModule
* get the execution model
*/
ExecutionModule* FrameworkFactory::GetExecutionModule(){

	return  NULL;
}


/**
* GetPlatformModule
* get the platform module
*/
PlatformModule* FrameworkFactory::GetPlatformModule(){

	return  m_PlatformModule;
}