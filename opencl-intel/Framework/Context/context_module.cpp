///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Uri Levy
///////////////////////////////////////////////////////////

#include "context_module.h"
#include "context.h"
#include "program.h"
#include "kernel.h"
#include "cl_buffer.h"
#include <platform_module.h>
#include <device.h>
#include <cl_objects_map.h>
#include <cl_utils.h>
#include <assert.h>
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

	delete m_pKernels;
	delete m_pPrograms;
	delete m_pMemObjects;
	delete m_pContexts;

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
	m_pPrograms = new OCLObjectsMap();
	m_pKernels = new OCLObjectsMap();
	m_pMemObjects = new OCLObjectsMap();

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
				clErrRet = pContext->Release();
				if (CL_FAILED(clErrRet))
				{
					return clErrRet;
				}
				delete pContext;
			}
		}
		m_pContexts->Clear();
	}

	if (NULL != m_pPrograms)
	{
		m_pPrograms->Clear();
	}

	if (NULL != m_pKernels)
	{
		m_pKernels->Clear();
	}

	return clErrRet;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateContext
//////////////////////////////////////////////////////////////////////////
cl_context	ContextModule::CreateContext(cl_context_properties clProperties,
										 cl_uint uiNumDevices,
										 const cl_device_id *pDevices,
										 logging_fn pfnNotify,
										 void *pUserData,
										 cl_err_code *pRrrcodeRet)
{
	InfoLog(m_pLoggerClient, L"Enter ContextModule::CreateContext (clProperties=%d, uiNumDevices=%d, pDevices=%d)", 
		clProperties, uiNumDevices, pDevices);
	if (NULL != pRrrcodeRet)
	{
		*pRrrcodeRet = CL_SUCCESS;
	}
	
	if (0 != clProperties)
	{
		ErrLog(m_pLoggerClient, L"clProperties==%d (!=0); return CL_INVALID_VALUE", clProperties);
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}
	if (NULL == pDevices)
	{
		ErrLog(m_pLoggerClient, L"pDevices==NULL; return CL_INVALID_VALUE");
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}
	if (0 == uiNumDevices)
	{
		ErrLog(m_pLoggerClient, L"uiNumDevices==0; return CL_INVALID_VALUE");
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}

	DbgLog(m_pLoggerClient, L"Device ** ppDevices = new (Device*)[%d]", uiNumDevices);
	Device ** ppDevices = new Device*[uiNumDevices];
	if (NULL == ppDevices)
	{
		ErrLog(m_pLoggerClient, L"ppDevices==NULL; return CL_ERR_INITILIZATION_FAILED");
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_ERR_INITILIZATION_FAILED;
		}		
		return CL_INVALID_HANDLE;
	}
	cl_err_code clErrRet = GetDevices(uiNumDevices, pDevices, ppDevices);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"CheckDevices(uiNumDevices, pDevices, ppDevices) = %ws", uiNumDevices, pDevices, ppDevices, ClErrTxt(clErrRet));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_DEVICE;
		}
		return CL_INVALID_HANDLE;
	}

	Context *pContext = new Context(clProperties, uiNumDevices, ppDevices, pfnNotify, pUserData);
	
	cl_context clContextId = (cl_context)m_pContexts->AddObject(pContext);
	DbgLog(m_pLoggerClient, L"New context created. (id = %d)", clContextId);
	return clContextId;
}
cl_context ContextModule::CreateContextFromType(cl_context_properties clProperties, 
												cl_device_type clDeviceType, 
												logging_fn pfnNotify, 
												void * pUserData, 
												cl_int * pErrcodeRet)
{
	InfoLog(m_pLoggerClient, L"Enter ContextModule::CreateContextFromType (clProperties=%d, clDeviceType=%d, pfnNotify=%d, pUserData=%d, pErrcodeRet=%d)",
		clProperties, clDeviceType, pfnNotify, pUserData, pErrcodeRet);
	
	if (NULL == m_pPlatformModule)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPlatformModule");
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}
	cl_uint uiNumDevices = 0;
	
	cl_err_code clErrRet = m_pPlatformModule->GetDeviceIDs(clDeviceType, 0, NULL, &uiNumDevices);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"GetDeviceIDs(%d, 0, NULL, %d) = %ws", clDeviceType, &uiNumDevices, ClErrTxt(clErrRet));
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		return CL_INVALID_HANDLE;
	}

	cl_device_id * pDevices = new cl_device_id[uiNumDevices];
	if (NULL == pDevices)
	{
		ErrLog(m_pLoggerClient, L"new cl_device_id[%d] = NULL", uiNumDevices);
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
		}
		return CL_INVALID_HANDLE;
	}

	clErrRet = m_pPlatformModule->GetDeviceIDs(clDeviceType, uiNumDevices, pDevices, NULL);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"GetDeviceIDs(%d, %d, %d, NULL) = %ws", clDeviceType, uiNumDevices, pDevices, ClErrTxt(clErrRet));
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		return CL_INVALID_HANDLE;
	}
	return CreateContext(clProperties, uiNumDevices, pDevices, pfnNotify, pUserData, pErrcodeRet);

}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CheckDevices
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices)
{
	InfoLog(m_pLoggerClient, L"ContextModule::CheckDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d", uiNumDevices, pclDeviceIds, ppDevices);
	cl_err_code clErrRet = CL_SUCCESS;

#ifdef _DEBUG
	assert ( NULL != m_pPlatformModule );
#endif

	// check input parameters
	if (NULL == ppDevices)
	{
		ErrLog(m_pLoggerClient, L"ppDevices==NULL; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
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
	clErrRet = pContext->Release();
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	if (pContext->GetReferenceCount() == 0)
	{
		m_pContexts->RemoveObject((cl_int)context, (OCLObject**)&pContext);
		// TODO: decide how to delete context
		//delete pContext;
	}
	return CL_SUCCESS;
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
	clErrRet = pContext->GetInfo((cl_int)param_name, param_value_size, param_value, param_value_size_ret);
	if (CL_FAILED(clErrRet))
	{
		pContext->NotifyError("clGetContextInfo failed", &clErrRet, sizeof(cl_int));
	}
	return clErrRet;
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
		}
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", clContext, &pContext, clErrRet);
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	Program *pProgram = NULL;
	clErrRet = pContext->CreateProgramWithSource(uiCount, ppcStrings, szLengths, &pProgram);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet, sizeof(cl_int));
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_pPrograms->AddObject((OCLObject*)pProgram, pProgram->GetId(), false);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet, sizeof(cl_int));
		return CL_INVALID_HANDLE;
	}
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_SUCCESS;
	}
	return (cl_program)pProgram->GetId();
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
		}
		return CL_INVALID_HANDLE;
	}
	// get the context from the contexts map list
	Context * pContext = NULL;
	if (NULL == m_pContexts || NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts == NULL || NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		if (NULL != pErrRet)
		{
			*pErrRet = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	// get the context object
	cl_err_code clErrRet = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %d", clContext, &pContext, clErrRet);
		if (NULL != pErrRet)
		{
			*pErrRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	Program *pProgram = NULL;
	clErrRet = pContext->CreateProgramWithBinary(uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, &pProgram);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrRet)
		{
			*pErrRet = clErrRet;
		}
		pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet, sizeof(cl_int));
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_pPrograms->AddObject((OCLObject*)pProgram, pProgram->GetId(), false);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrRet)
		{
			*pErrRet = clErrRet;
		}
		pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet, sizeof(cl_int));
		return CL_INVALID_HANDLE;
	}
	if (NULL != pErrRet)
	{
		*pErrRet = CL_SUCCESS;
	}
	return (cl_program)pProgram->GetId();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code	ContextModule::RetainProgram(cl_program clProgram)
{
	InfoLog(m_pLoggerClient, L"Enter RetainProgram (clProgram=%d)", clProgram);

#ifdef _DEBUG
	assert("Programs map list isn't initialized" && (NULL != m_pPrograms));
#endif
	
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
	InfoLog(m_pLoggerClient, L"Enter ReleaseProgram (clProgram=%d)", clProgram);

#ifdef _DEBUG
	assert("Programs map list isn't initialized" && (NULL != m_pPrograms));
#endif

	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"program %d is invalid program", clProgram);
		return CL_INVALID_PROGRAM;
	}
	clErrRet = pProgram->Release();
	if (CL_FAILED(clErrRet))
	{
		return CL_ERR_OUT(clErrRet);
	}
	if (pProgram->GetReferenceCount() == 0)
	{
		Context * pContext = (Context*)pProgram->GetContext();
		if (NULL == pContext)
		{
			return CL_INVALID_PROGRAM;
		}
		clErrRet = pContext->RemoveProgram((cl_program)pProgram->GetId());
		if (CL_FAILED(clErrRet))
		{
			return CL_ERR_OUT(clErrRet);
		}

		// remove program from programs list and add it to the dirty programs list
		clErrRet = m_pPrograms->RemoveObject((cl_int)pProgram->GetId(), NULL, true);
		if (CL_FAILED(clErrRet))
		{
			return CL_ERR_OUT(clErrRet);
		}
	}
	GarbageCollector();
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::BuildProgram
//////////////////////////////////////////////////////////////////////////
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
		return CL_ERR_INITILIZATION_FAILED;
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
//////////////////////////////////////////////////////////////////////////
// ContextModule::UnloadCompiler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::UnloadCompiler(void)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramInfo(cl_program clProgram, 
									 cl_program_info clParamName, 
									 size_t szParamValueSize, 
									 void * pParamValue, 
									 size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"GetProgramInfo enter. clProgram=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d", 
		clProgram, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	
	cl_err_code clErrRet = CL_SUCCESS;
	Program * pProgram = NULL;
	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"m_pPrograms == NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get program from the programs map list
	clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pPrograms->GetOCLObject(%d, %d) = %d", clProgram, &pProgram, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pProgram->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramBuildInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramBuildInfo(cl_program clProgram, 
										  cl_device_id clDevice, 
										  cl_program_info clParamName, 
										  size_t szParamValueSize, 
										  void * pParamValue, 
										  size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"GetProgramBuildInfo enter. clProgram=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d", 
		clProgram, clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	
	cl_err_code clErrRet = CL_SUCCESS;
	Program * pProgram = NULL;
	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"m_pPrograms == NULL; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get program from the programs map list
	clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pPrograms->GetOCLObject(%d, %d) = %d", clProgram, &pProgram, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pProgram->GetBuildInfo(clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernel
//////////////////////////////////////////////////////////////////////////
cl_kernel ContextModule::CreateKernel(cl_program clProgram, 
									  const char * pscKernelName, 
									  cl_int * piErr)
{
	InfoLog(m_pLoggerClient, L"CreateKernel enter. clProgram=%d, pscKernelName=%d, piErr=%d", clProgram, pscKernelName, piErr);

#ifdef _DEBUG
	assert ( (NULL != m_pPrograms) && (NULL != m_pKernels) );
#endif

	// get program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		ErrLog(m_pLoggerClient, L"clProgram is invalid program");
		if (NULL != piErr)
		{
			*piErr = CL_INVALID_PROGRAM;
		}
		return CL_INVALID_HANDLE;
	}

	// create new kernel
	Kernel * pKernel = NULL;
	clErrRet = pProgram->CreateKernel(pscKernelName, &pKernel);
	if (NULL != piErr)
	{
		*piErr = CL_ERR_OUT(clErrRet);
	}
	if (NULL != pKernel)
	{
		// add new kernel to the context module's kernels list
		m_pKernels->AddObject((OCLObject*)pKernel, pKernel->GetId(), false);
		if (NULL != piErr)
		{
			*piErr = CL_SUCCESS;
		}
		// return handle
		return (cl_kernel)pKernel->GetId();
	}
	if (NULL != piErr)
	{
		*piErr = CL_OUT_OF_HOST_MEMORY;
	}
	return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernelsInProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CreateKernelsInProgram(cl_program clProgram, 
											 cl_uint uiNumKernels, 
											 cl_kernel * pclKernels, 
											 cl_uint * puiNumKernelsRet)
{
	InfoLog(m_pLoggerClient, L"CreateKernelsInProgram enter. clProgram=%d, uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d", 
		clProgram, uiNumKernels, pclKernels, puiNumKernelsRet);

	// check invalid input
	if (NULL == m_pPrograms || NULL == m_pKernels)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPrograms || NULL == m_pKernels");
		return CL_ERR_FAILURE;
	}
	
	// get the program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgram, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		ErrLog(m_pLoggerClient, L"clProgram is invalid program");
		return CL_INVALID_PROGRAM;
	}

	// create all kernels for the program
	clErrRet = pProgram->CreateAllKernels(uiNumKernels, pclKernels, puiNumKernelsRet);
	if (CL_FAILED(clErrRet))
	{
		return CL_ERR_OUT(clErrRet);
	}

	// get kernels and add them to the context module's map list
	cl_uint uiKerenls = 0;
	clErrRet = pProgram->GetKernels(0, NULL, &uiKerenls);
	if (CL_FAILED(clErrRet))
	{
		return CL_ERR_OUT(clErrRet);
	}
	Kernel ** ppKernels = new Kernel * [uiKerenls];
	if (NULL == ppKernels)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	clErrRet = pProgram->GetKernels(uiKerenls, ppKernels, NULL);
	if (CL_FAILED(clErrRet))
	{
		delete[] ppKernels;
		return CL_ERR_OUT(clErrRet);
	}
	for (cl_uint ui=0; ui<uiKerenls; ++ui)
	{
		m_pKernels->AddObject((OCLObject*)ppKernels[ui],ppKernels[ui]->GetId(),false);
	}
	
	delete[] ppKernels;
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainKernel(cl_kernel clKernel)
{
	InfoLog(m_pLoggerClient, L"Enter RetainKernel (clKernel=%d)", clKernel);

	if (NULL == m_pKernels)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pKernels")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((cl_int)clKernel, (OCLObject**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}
	
	clErr = pKernel->Retain();

	return CL_ERR_OUT(clErr);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseKernel(cl_kernel clKernel)
{
	InfoLog(m_pLoggerClient, L"Enter ReleaseKernel (clKernel=%d)", clKernel);

	if (NULL == m_pKernels)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pKernels")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((cl_int)clKernel, (OCLObject**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	clErr = pKernel->Release();
	if (CL_FAILED(clErr))
	{
		ErrLog(m_pLoggerClient, L"pKernel->Release() returned %ws", ClErrTxt(clErr));
		return CL_ERR_OUT(clErr);
	}

	if (pKernel->GetReferenceCount() == 0)
	{
		Program * pProgram = (Program*)pKernel->GetProgram();
		if (NULL == pProgram)
		{
			return CL_INVALID_KERNEL;
		}
		clErr = pProgram->RemoveKernel((cl_kernel)pKernel->GetId());
		if (CL_FAILED(clErr))
		{
			return CL_ERR_OUT(clErr);
		}
		// remove kernel form kernels list and add it to the dirty kernels list		
		clErr = m_pKernels->RemoveObject(pKernel->GetId(), NULL, true);
		if (CL_FAILED(clErr))
		{
			return CL_ERR_OUT(clErr);
		}
	}
	GarbageCollector();
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArg
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetKernelArg(cl_kernel clKernel, 
								   cl_uint	uiArgIndex, 
								   size_t szArgSize, 
								   const void * pArgValue)
{
	InfoLog(m_pLoggerClient, L"Enter SetKernelArg (clKernel=%d, uiArgIndex=%d, szArgSize=%d, pszArgValue=%d)", 
		clKernel, uiArgIndex, szArgSize, pArgValue);

	if (NULL == m_pKernels)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pKernels")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((cl_int)clKernel, (OCLObject**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}
	clErr = pKernel->SetKernelArg(uiArgIndex, szArgSize, pArgValue);
	return CL_ERR_OUT(clErr);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelInfo(cl_kernel clKernel, 
									cl_kernel_info clParamName, 
									size_t szParamValueSize, 
									void * pParamValue, 
									size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter GetKernelInfo (clKernel=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		clKernel, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == m_pKernels)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pKernels")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((cl_int)clKernel, (OCLObject**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	return pKernel->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelWorkGroupInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelWorkGroupInfo(cl_kernel clKernel, 
											 cl_device_id clDevice, 
											 cl_kernel_work_group_info clParamName, 
											 size_t szParamValueSize, 
											 void *	pParamValue, 
											 size_t * pszParamValueSizeRet)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateBuffer(cl_context clContext, 
								   cl_mem_flags clFlags, 
								   size_t szSize, 
								   void * pHostPtr, 
								   cl_int * pErrcodeRet)
{
	InfoLog(m_pLoggerClient, L"Enter CreateBuffer (clContext=%d, clFlags=%d, szSize=%d, pHostPtr=%d, pErrcodeRet=%d)", 
		clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	Buffer * pBuffer = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		ErrLog(m_pLoggerClient, L"m_pContexts->GetOCLObject(%d, %d) = %ws , pContext = %d", clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pContext->CreateBuffer(clFlags, szSize, pHostPtr, &pBuffer);
	if (CL_FAILED(clErr))
	{
		ErrLog(m_pLoggerClient, L"pContext->CreateBuffer(%d, %d, %d, %d) = %ws", clFlags, szSize, pHostPtr, &pBuffer, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pBuffer, pBuffer->GetId(), false);
	if (CL_FAILED(clErr))
	{
		ErrLog(m_pLoggerClient, L"m_pMemObjects->AddObject(%d, %d, false) = %ws", pBuffer, pBuffer->GetId(), ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_SUCCESS;
	}
	return (cl_mem)pBuffer->GetId();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage2D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage2D(cl_context clContext, 
									cl_mem_flags clFlags, 
									const cl_image_format * clImageFormat, 
									size_t szImageWidth, 
									size_t szImageHeight, 
									size_t szImageRowPitch, 
									void * pHostPtr, 
									cl_int * pErrcodeRet)
{
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_ERR_NOT_SUPPORTED;
	}
	return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage3D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage3D(cl_context clContext, 
									cl_mem_flags clFlags, 
									const cl_image_format * clImageFormat, 
									size_t szImageWidth, 
									size_t szImageHeight, 
									size_t szImageDepth, 
									size_t szImageRowPitch, 
									size_t szImageSlicePitch, 
									void * pHostPtr, 
									cl_int * pErrcodeRet)
{
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_ERR_NOT_SUPPORTED;
	}
	return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainMemObject(cl_mem clMemObj)
{
	InfoLog(m_pLoggerClient, L"Enter RetainMemObject (clMemObj=%d)", clMemObj);

	if (NULL == m_pMemObjects)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pMemObjects")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((cl_int)clMemObj, (OCLObject**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clMemObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}
	return pMemObj->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseMemObject(cl_mem clMemObj)
{
	InfoLog(m_pLoggerClient, L"Enter RetainMemObject (clMemObj=%d)", clMemObj);

	if (NULL == m_pMemObjects)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pMemObjects")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((cl_int)clMemObj, (OCLObject**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clMemObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}
	clErr =  pMemObj->Release();
	if (CL_FAILED(clErr))
	{
		CL_ERR_OUT(clErr);
	}
	cl_uint uiRefCount = pMemObj->GetReferenceCount();
	if (0 == uiRefCount)
	{
		// TODO: handle release memomry object
		Context *pContext = (Context*)pMemObj->GetContext();
		clErr = pContext->RemoveMemObject(clMemObj);
		if (CL_FAILED(clErr))
		{
			CL_ERR_OUT(clErr);
		}
		//TODO: set dirty object
		clErr = m_pMemObjects->RemoveObject((cl_int)clMemObj, NULL, true);
		if (CL_FAILED(clErr))
		{
			CL_ERR_OUT(clErr);
		}
	}
	GarbageCollector();
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSupportedImageFormats
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSupportedImageFormats(cl_context clContext, 
											   cl_mem_flags clFlags, 
											   cl_mem_object_type clImageType, 
											   cl_uint uiNumEntries, 
											   cl_image_format * pclImageFormats, 
											   cl_uint * puiNumImageFormats)
{
	return CL_ERR_NOT_SUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetMemObjectInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetMemObjectInfo(cl_mem clMemObj, 
									   cl_mem_info clParamName, 
									   size_t szParamValueSize, 
									   void * pParamValue, 
									   size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter GetMemObjectInfo (clMemObj=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		clMemObj, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == m_pMemObjects)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pMemObjects")
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((cl_int)clMemObj, (OCLObject**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		ErrLog(m_pLoggerClient, L"GetOCLObject(%d, %d) returned %ws", clMemObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}

	return pMemObj->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetImageInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetImageInfo(cl_mem clImage, 
								   cl_image_info clParamName, 
								   size_t szParamValueSize, 
								   void * pParamValue, 
								   size_t * pszParamValueSizeRet)
{
	return CL_ERR_NOT_SUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_sampler ContextModule::CreateSampler(cl_context clContext, 
										cl_bool bNormalizedCoords, 
										cl_addressing_mode clAddressingMode, 
										cl_filter_mode clFilterMode, 
										cl_int * pErrcodeRet)
{
	if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_ERR_NOT_SUPPORTED;
	}
	return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainSampler(cl_sampler clSampler)
{
	return CL_ERR_NOT_SUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseSampler(cl_sampler clSampler)
{
	return CL_ERR_NOT_SUPPORTED;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSamplerInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSamplerInfo(cl_sampler clSampler, 
									 cl_sampler_info clParamName, 
									 size_t szParamValueSize, 
									 void * pParamValue, 
									 size_t * pszParamValueSizeRet)
{
	return CL_ERR_NOT_SUPPORTED;
}
Context* ContextModule::GetContext(cl_context clContext) const
{
#ifdef _DEBUG   // arnonp remark: No need for that, if _DEBUG is part of the assert iplementation
	assert ( NULL != m_pContexts );
#endif
	Context * pContext = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((cl_int)clContext, (OCLObject**)&pContext);
	if (CL_SUCCEEDED(clErr))
	{
		return pContext;
	}
	return NULL;
}

Kernel* ContextModule::GetKernel(cl_kernel clKernel) const
{
	assert ( NULL != m_pKernels );

	Kernel* pKernel = NULL;
	cl_err_code clErr = m_pKernels->GetOCLObject((cl_int)clKernel, (OCLObject**)&pKernel);
	if (CL_SUCCEEDED(clErr))
	{
		return pKernel;
	}
	return NULL;
}

MemoryObject * ContextModule::GetMemoryObject(const cl_mem clMemObjId)
{
	MemoryObject * pMemoryObject = NULL;
	cl_err_code clErr = m_pMemObjects->GetOCLObject((cl_int)clMemObjId, (OCLObject**)&pMemoryObject);
	if (CL_SUCCEEDED(clErr))
	{
		return pMemoryObject;
	}
	return NULL;
}

void ContextModule::GarbageCollector()
{
	m_pKernels->GarbageCollector();
	m_pPrograms->GarbageCollector();
	m_pMemObjects->GarbageCollector();
	m_pContexts->GarbageCollector();
}