///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#include "ContextModule.h"
using namespace Intel::OpenCL::Framework;

ContextModule::ContextModule(PlatformModule *pPlatformModule)
{
	m_pLoggerClient = new LoggerClient(L"Context Module Logger Client",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"ContextModule constructor enter");

	m_pPlatformModule = pPlatformModule;
}


ContextModule::~ContextModule()
{
	InfoLog(m_pLoggerClient, L"ContextModule destructor enter");

	delete m_pLoggerClient;
	m_pLoggerClient = NULL;

}

cl_err_code ContextModule::Initialize()
{
	InfoLog(m_pLoggerClient, L"ContextModule::Initialize enter");
	return CL_ERR_NOT_IMPLEMENTED;
}

cl_err_code ContextModule::Release()
{
	InfoLog(m_pLoggerClient, L"ContextModule::Release enter");
	return CL_ERR_NOT_IMPLEMENTED;
}
