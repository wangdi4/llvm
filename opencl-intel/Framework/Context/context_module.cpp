///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#include "context_module.h"
#include "context.h"
#include "program.h"
#include <platform_module.h>
#include <device.h>
#include <cl_objects_map.h>
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

	m_pPrograms = new OCLObjectsMap();
	if (NULL == m_pPrograms)
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
	if (NULL != m_pContexts)
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

	if (NULL != m_pPrograms)
	{
		m_pPrograms->Clear();
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
		return CL_INVALID_HANDLE;
	}
	if (NULL == devices)
	{
		ErrLog(m_pLoggerClient, L"devices==NULL; return CL_INVALID_VALUE", properties);
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}
	if (0 == num_devices)
	{
		ErrLog(m_pLoggerClient, L"num_devices==0; return CL_INVALID_VALUE", properties);
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
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
		return CL_INVALID_HANDLE;
	}
	cl_err_code clErrRet = CheckDevices(num_devices, devices, ppDevices);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != errcode_ret)
		{	
			*errcode_ret = CL_INVALID_DEVICE;
		}
		return CL_INVALID_HANDLE;
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
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithSource
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithSource(cl_context     clContext,
												  cl_uint        uiCount, 
												  const char **  ppcStrings, 
												  const size_t * szLengths, 
												  cl_int *       pErrcodeRet)
{
	InfoLog(m_pLoggerClient, L"CreateProgramWithSource enter. clContext=%d, uiCount=%d, ppcStrings=%d, szLengths=%d, pErrcodeRet=%d", 
		clContext, uiCount, ppcStrings, szLengths, pErrcodeRet);

	cl_err_code clErrRet = CL_SUCCESS;
	// get the context from the contexts map list
	Context * pContext = NULL;
	if (NULL == m_pContexts || NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL || NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_INITILIZATION_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	clErrRet = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", clContext, &pContext, clErrRet);
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
			return CL_INVALID_HANDLE;
		}
	}
	Program *pProgram = NULL;
	clErrRet = pContext->CreateProgramWithSource(uiCount, ppcStrings, szLengths, &pProgram);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
			return CL_INVALID_HANDLE;
		}
	}
	cl_int iProgramId = m_pPrograms->AddObject((OCLObject*)pProgram);
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_SUCCESS;
	}
	return (cl_program)iProgramId;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBinary
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBinary(cl_context           clContext,
												  cl_uint              uiNumDevices,
												  const cl_device_id * pclDeviceList,
												  const size_t *       pszLengths,
												  const void **        ppBinaries,
												  cl_int *             piBinaryStatus,
												  cl_int *             pErrRet)
{
	InfoLog(m_pLoggerClient, L"CreateProgramWithBinary enter. clContext=%d, uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d", 
		clContext, uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus);
	if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
	{
		// invalid value
		ErrLog(m_pLoggerClient, L"NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries");
		if (NULL != pErrRet)
		{
			*pErrRet = CL_INVALID_VALUE;
			return CL_INVALID_HANDLE;
		}
	}
	// get the context from the contexts map list
	Context * pContext = NULL;
	if (NULL == m_pContexts || NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL || NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		if (NULL != pErrRet)
		{
			*pErrRet = CL_ERR_INITILIZATION_FAILED;
			return CL_INVALID_HANDLE;
		}
	}
	// get the context object
	cl_err_code clErrRet = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", clContext, &pContext, clErrRet);
		if (NULL != pErrRet)
		{
			*pErrRet = CL_INVALID_CONTEXT;
			return CL_INVALID_HANDLE;
		}
	}
	Program *pProgram = NULL;
	clErrRet = pContext->CreateProgramWithBinary(uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, &pProgram);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrRet)
		{
			*pErrRet = clErrRet;
			return CL_INVALID_HANDLE;
		}
	}
	cl_int iProgramId = m_pPrograms->AddObject((OCLObject*)pProgram);
	if (NULL != pErrRet)
	{
		*pErrRet = CL_SUCCESS;
	}
	return (cl_program)iProgramId;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code	ContextModule::RetainProgram(cl_program clProgram)
{
	InfoLog(m_pLoggerClient, L"RetainProgram enter. clProgram=%d", clProgram);
	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"program %d is invalid program", clProgram);
		return CL_INVALID_PROGRAM;
	}
	return pProgram->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseProgram(cl_program clProgram)
{
	InfoLog(m_pLoggerClient, L"ReleaseProgram enter. clProgram=%d", clProgram);
	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"program %d is invalid program", clProgram);
		return CL_INVALID_PROGRAM;
	}
	return pProgram->Release();
}
cl_int ContextModule::BuildProgram(cl_program clProgram, 
								   cl_uint uiNumDevices, 
								   const cl_device_id * pclDeviceList, 
								   const char * pcOptions, 
								   void (*pfn_notify)(cl_program program, void * user_data), 
								   void * pUserData)
{
	InfoLog(m_pLoggerClient, L"BuildProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d", 
		clProgram, uiNumDevices, pclDeviceList, pcOptions, pUserData);

	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPrograms");
		return CL_ERR_NOT_IMPLEMENTED;
	}
	// get program from programs map list
	Program * pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
			ErrLog(m_pLoggerClient, L"program %d isn't valid program", clProgram);
			return CL_INVALID_PROGRAM;
	}

	clErrRet = pProgram->Build(uiNumDevices, pclDeviceList, pcOptions, pfn_notify, pUserData);
	return clErrRet;
}