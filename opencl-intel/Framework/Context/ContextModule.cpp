///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#include "ContextModule.h"
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//////////////////////////////////////////////////////////////////////////
// ContextModule C'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::ContextModule(PlatformModule *pPlatformModule)
{
	m_pLoggerClient = new LoggerClient(L"Context Module Logger Client",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"ContextModule constructor enter");

	m_pPlatformModule = pPlatformModule;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule D'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::~ContextModule()
{
	InfoLog(m_pLoggerClient, L"ContextModule destructor enter");

	delete m_pLoggerClient;
	m_pLoggerClient = NULL;

}

//////////////////////////////////////////////////////////////////////////
// ContextModule::Initializ
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Initialize()
{
	InfoLog(m_pLoggerClient, L"ContextModule::Initialize enter");
	m_pContexts = new OCLObjectsMap();
	if (NULL == m_pContexts)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::Release
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Release()
{
	InfoLog(m_pLoggerClient, L"ContextModule::Release enter");
	cl_err_code clErrRet = CL_SUCCESS;
	Context *pContext = NULL;
	if (NULL == m_pContexts)
	{
		for(cl_uint ui=0; ui<m_pContexts->Count(); ++ui)
		{
			clErrRet = m_pContexts->GetObjectByIndex(ui, (OCLObject**)&pContext);
			if (CL_SUCCEEDED(clErrRet))
			{
				pContext->Release();
				delete pContext;
			}
		}
		m_pContexts->Clear();
	}
	return clErrRet;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateContext
//////////////////////////////////////////////////////////////////////////
cl_context	ContextModule::CreateContext(cl_context_properties properties,
										   cl_uint num_devices,
										   const cl_device_id *devices,
										   logging_fn pfn_notify,
										   void *user_data,
										   cl_err_code *errcode_ret)
{
	InfoLog(m_pLoggerClient, L"ContextModule::clCreateContext enter");
	InfoLog(m_pLoggerClient, L"cl_context_properties=%d, num_devices=%d, devices=%d", properties, num_devices, devices);
	if (NULL != errcode_ret)
	{
		*errcode_ret = CL_SUCCESS;
	}
	
	if (0 != properties)
	{
		ErrLog(m_pLoggerClient, L"properties==%d (!=0); return CL_INVALID_VALUE", properties);
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_VALUE;
		}
		return 0;
	}
	if (NULL == devices)
	{
		ErrLog(m_pLoggerClient, L"devices==NULL; return CL_INVALID_VALUE", properties);
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_VALUE;
		}
		return 0;
	}
	if (0 == num_devices)
	{
		ErrLog(m_pLoggerClient, L"num_devices==0; return CL_INVALID_VALUE", properties);
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_VALUE;
		}
		return 0;
	}

	InfoLog(m_pLoggerClient, L"Device ** ppDevices = new (Device*)[%d]", num_devices);
	Device ** ppDevices = new Device*[num_devices];
	if (NULL == ppDevices)
	{
		ErrLog(m_pLoggerClient, L"ppDevices==NULL; return CL_ERR_INITILIZATION_FAILED");
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_ERR_INITILIZATION_FAILED;
		}		
		return 0;
	}
	cl_err_code clErrRet = CheckDevices(num_devices, devices, ppDevices);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_DEVICE;
		}
		return 0;
	}

	Context *pContext = new Context(properties, num_devices, ppDevices, pfn_notify, user_data);
	pContext->Retain();
	return (cl_context)m_pContexts->AddObject(pContext);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CheckDevices
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::CheckDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices)
{
	InfoLog(m_pLoggerClient, L"ContextModule::CheckDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d", uiNumDevices, pclDeviceIds, ppDevices);
	cl_err_code clErrRet = CL_SUCCESS;

	// check input parameters
	if (NULL == ppDevices)
	{
		ErrLog(m_pLoggerClient, L"ppDevices==NULL; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}
	if (NULL == m_pPlatformModule)
	{
		ErrLog(m_pLoggerClient, L"m_pPlatformModule==NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	
	// go through device ids and get the device from the platform module
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pPlatformModule->GetDevice(pclDeviceIds[ui], &pDevice);
		if (CL_FAILED(clErrRet))
		{
			ErrLog(m_pLoggerClient, L"m_pPlatformModule->GetDevice(%d, %d) = %d", pclDeviceIds[ui], &pDevice, clErrRet);
			return clErrRet;
		}
		ppDevices[ui] = pDevice;
	}
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::RetainContext(cl_context context)
{
	InfoLog(m_pLoggerClient, L"ContextModule::RetainContext enter. context=%d", context);
	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	if (NULL == m_pContexts)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	clErrRet = m_pContexts->GetOCLObject((cl_int)context, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", context, &pContext, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pContext->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseContext(cl_context context)
{
	InfoLog(m_pLoggerClient, L"ContextModule::ReleaseContext enter. context=%d", context);
	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	if (NULL == m_pContexts)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	clErrRet = m_pContexts->GetOCLObject((cl_int)context, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", context, &pContext, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pContext->Release();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetContextInfo
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::GetContextInfo(cl_context      context,
										  cl_context_info param_name,
										  size_t          param_value_size,
										  void *          param_value,
										  size_t *        param_value_size_ret)
{
	InfoLog(m_pLoggerClient, L"ContextModule::GetContextInfo enter. context=%d, param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", 
		context, param_name, param_value_size, param_value, param_value_size_ret);
	
	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	if (NULL == m_pContexts)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get context from the contexts map list
	clErrRet = m_pContexts->GetOCLObject((cl_int)context, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", context, &pContext, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pContext->GetInfo((cl_int)param_name, param_value_size, param_value, param_value_size_ret);
}