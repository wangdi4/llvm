///////////////////////////////////////////////////////////
//  ContextModule.cpp
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:03:03 PM
//  Original author: Uri Levy
///////////////////////////////////////////////////////////

#include "context_module.h"
#include "Context.h"
#include "program.h"
#include "kernel.h"
#include "cl_buffer.h"
#include "image.h"
#include "sampler.h"
#if defined (_WIN32)
#include "gl_context.h"
#include "gl_shr_utils.h"
#include "gl_mem_objects.h"
#endif
#include <platform_module.h>
#include <Device.h>
#include <cl_objects_map.h>
#include <cl_utils.h>
#include <assert.h>
#include "ocl_itt.h"
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//////////////////////////////////////////////////////////////////////////
// ContextModule C'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::ContextModule(PlatformModule *pPlatformModule)
{
	INIT_LOGGER_CLIENT(L"ContextModule",LL_DEBUG);

	LOG_INFO(TEXT("%S"), TEXT("ContextModule constructor enter"));

	m_pPlatformModule = pPlatformModule;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule D'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::~ContextModule()
{
	LOG_INFO(TEXT("%S"), TEXT("ContextModule destructor enter"));

	if(NULL != m_pKernels) delete m_pKernels;
	if(NULL != m_pPrograms) delete m_pPrograms;
	if(NULL != m_pMemObjects) delete m_pMemObjects;
	if(NULL != m_pSamplers) delete m_pSamplers;
	if(NULL != m_pContexts) delete m_pContexts;

	RELEASE_LOGGER_CLIENT;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::Initialize
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pOclConfig)
{
	LOG_INFO(TEXT("%S"), TEXT("ContextModule::Initialize enter"));

	m_pContexts = new OCLObjectsMap<_cl_context_int>();
	m_pPrograms = new OCLObjectsMap<_cl_program_int>();
	m_pKernels = new OCLObjectsMap<_cl_kernel_int>();
	m_pMemObjects = new OCLObjectsMap<_cl_mem_int>();
	m_pSamplers = new OCLObjectsMap<_cl_sampler_int>();

	m_pOclEntryPoints = pOclEntryPoints;
	m_bUseTaskalyzer = pOclConfig->UseTaskalyzer();

	// Initialize Stage Marker flags
	m_cStageMarkerFlags = 0;

	if (m_bUseTaskalyzer)
	{
		if (pOclConfig->ShowQueuedMarker())
			m_cStageMarkerFlags += GPA_SHOW_QUEUED_MARKER;			// Set first bit
		if (pOclConfig->ShowSubmittedMarker())
			m_cStageMarkerFlags += GPA_SHOW_SUBMITTED_MARKER;		// Set second bit
		if (pOclConfig->ShowRunningMarker())
			m_cStageMarkerFlags += GPA_SHOW_RUNNING_MARKER;			// Set third bit
		if (pOclConfig->ShowCompletedMarker())
			m_cStageMarkerFlags += GPA_SHOW_COMPLETED_MARKER;		// Set fourth bit
	}

	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::Release
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Release(  bool bTerminate )
{
	LOG_INFO(TEXT("%S"), TEXT("ContextModule::Release enter"));

	cl_err_code clErrRet = CL_SUCCESS;
	if (NULL != m_pContexts)
	{
		m_pContexts->ReleaseAllObjects();
	}

	if (NULL != m_pPrograms)
	{
		m_pPrograms->Clear();
	}

	if (NULL != m_pKernels)
	{
		m_pKernels->Clear();
	}

	if (NULL != m_pMemObjects)
	{
		m_pMemObjects->Clear();
	}

	if (NULL != m_pSamplers)
	{
		m_pSamplers->Clear();
	}

	return clErrRet;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateContext
//////////////////////////////////////////////////////////////////////////
cl_context	ContextModule::CreateContext(const cl_context_properties * clProperties,
										 cl_uint uiNumDevices,
										 const cl_device_id *pDevices,
										 logging_fn pfnNotify,
										 void *pUserData,
										 cl_err_code *pRrrcodeRet)
{
	//cl_start;

	LOG_INFO(TEXT("Enter ContextModule::CreateContext (clProperties=%d, uiNumDevices=%d, pDevices=%d)"), clProperties, uiNumDevices, pDevices);
	
	if (NULL != pRrrcodeRet)
	{
		*pRrrcodeRet = CL_SUCCESS;
	}
	
	if ((NULL == pDevices) || (0 == uiNumDevices))
	{
		LOG_ERROR(TEXT("%S"), TEXT("(NULL == pDevices) || (0 == uiNumDevices); return CL_INVALID_VALUE"));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}

	Device ** ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		LOG_ERROR(TEXT("%S"), TEXT("Failed to allocate memory for devices: new Device[uiNumDevices] = NULL"));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_OUT_OF_HOST_MEMORY;
		}
		return CL_INVALID_HANDLE;
	}

	cl_err_code clErrRet = GetDevices(uiNumDevices, pDevices, ppDevices);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pRrrcodeRet)
		{	
			delete[] ppDevices;
			*pRrrcodeRet = CL_INVALID_DEVICE;
		}
		return CL_INVALID_HANDLE;
	}

	Context *pContext = NULL;
#if defined (_WIN32)  //TODO GL support for Linux
	cl_context_properties hGLCtx, hDC;
	ParseGLContextOptions(clProperties, &hGLCtx, &hDC);
#endif
	// Default error in case new() will fail
	clErrRet = CL_OUT_OF_HOST_MEMORY;
#if defined (_WIN32)  //TODO GL support for Linux
	if ( (NULL != hGLCtx) || (NULL != hDC) )
	{
		pContext = 	new GLContext(clProperties, uiNumDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, hGLCtx, hDC, m_bUseTaskalyzer, m_cStageMarkerFlags);
	} else
#endif
	{
		pContext = 	new Context(clProperties, uiNumDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, m_bUseTaskalyzer, m_cStageMarkerFlags);
	}
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("%S"), TEXT("Create context failed"));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = clErrRet;
		}
        delete[] ppDevices;
		if ( NULL != pContext )
			pContext->Release();
		return CL_INVALID_HANDLE;
	}

	delete[] ppDevices;
	
	cl_context clContextId = (cl_context)m_pContexts->AddObject(pContext);
    LOG_INFO(TEXT("CONTEXT_TEST: New context created. (id = %d)"), clContextId);

	return clContextId;
}
cl_context ContextModule::CreateContextFromType(const cl_context_properties * clProperties, 
												cl_device_type clDeviceType, 
												logging_fn pfnNotify, 
												void * pUserData, 
												cl_int * pErrcodeRet)
{
	//cl_start;
	LOG_INFO(TEXT("Enter ContextModule::CreateContextFromType (clProperties=%d, clDeviceType=%d, pfnNotify=%d, pUserData=%d, pErrcodeRet=%d)"),
		clProperties, clDeviceType, pfnNotify, pUserData, pErrcodeRet);
	
#ifdef _DEBUG
	assert (NULL != m_pPlatformModule);
#endif
	cl_uint uiNumDevices = 0;
	
	// TODO: Handle new spec
	cl_err_code clErrRet = m_pPlatformModule->GetDeviceIDs(NULL, clDeviceType, 0, NULL, &uiNumDevices);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("GetDeviceIDs(%d, 0, NULL, %d) = %S"), clDeviceType, &uiNumDevices, ClErrTxt(clErrRet));
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		return CL_INVALID_HANDLE;
	}

	cl_device_id * pDevices = new cl_device_id[uiNumDevices];
	if (NULL == pDevices)
	{
		LOG_ERROR(TEXT("new cl_device_id[%d] = NULL"), uiNumDevices);
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
		}
		return CL_INVALID_HANDLE;
	}

	// TODO: Handle new spec
	clErrRet = m_pPlatformModule->GetDeviceIDs(NULL, clDeviceType, uiNumDevices, pDevices, NULL);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("GetDeviceIDs(%d, %d, %d, NULL) = %S"), clDeviceType, uiNumDevices, pDevices, ClErrTxt(clErrRet));
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		delete[] pDevices;
		return CL_INVALID_HANDLE;
	}
	cl_context clContext = CreateContext(clProperties, uiNumDevices, pDevices, pfnNotify, pUserData, pErrcodeRet);
	delete[] pDevices;
	//cl_return clContext;
	return clContext;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CheckDevices
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices)
{
	LOG_DEBUG(TEXT("ContextModule::CheckDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d"), uiNumDevices, pclDeviceIds, ppDevices);
	cl_err_code clErrRet = CL_SUCCESS;

#ifdef _DEBUG
	assert ( NULL != m_pPlatformModule );
	assert ( (NULL != ppDevices) && (0 != uiNumDevices) );
#endif

	// go through device ids and get the device from the platform module
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pPlatformModule->GetDevice(pclDeviceIds[ui], &pDevice);
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%d, %d) = %d"), pclDeviceIds[ui], &pDevice, clErrRet);
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
	LOG_INFO(TEXT("ContextModule::RetainContext enter. context=%d"), context);
	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	if (NULL == m_pContexts)
	{
		LOG_ERROR(TEXT("%S"), TEXT("m_pContexts == NULL; return CL_ERR_INITILIZATION_FAILED"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	clErrRet = m_pContexts->GetOCLObject((_cl_context_int*)context, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
		return CL_INVALID_CONTEXT;
	}
	return pContext->Retain();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseContext(cl_context context)
{
	LOG_INFO(TEXT("ContextModule::ReleaseContext enter. context=%d"), context);
	if (NULL == m_pContexts)
	{
		LOG_ERROR(TEXT("%S"), TEXT("m_pContexts == NULL; return CL_ERR_INITILIZATION_FAILED"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	return m_pContexts->ReleaseObject((_cl_context_int*)context);
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
	LOG_DEBUG(TEXT("ContextModule::GetContextInfo enter. context=%d, param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
		context, param_name, param_value_size, param_value, param_value_size_ret);

#ifdef _DEBUG
	assert( NULL != m_pContexts ); 
#endif

	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	// get context from the contexts map list
	clErrRet = m_pContexts->GetOCLObject((_cl_context_int*)context, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
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
	LOG_INFO(TEXT("CreateProgramWithSource enter. clContext=%d, uiCount=%d, ppcStrings=%d, szLengths=%d, pErrcodeRet=%d"), 
		clContext, uiCount, ppcStrings, szLengths, pErrcodeRet);

	cl_err_code clErrRet = CL_SUCCESS;
	// get the context from the contexts map list
	Context * pContext = NULL;
	if (NULL == m_pContexts || NULL == m_pPrograms)
	{
		LOG_ERROR(TEXT("%S"), TEXT("m_pContexts == NULL || NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED"));
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %d"), clContext, &pContext, clErrRet);
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
	clErrRet = m_pPrograms->AddObject((OCLObject<_cl_program_int>*)pProgram, false);
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
	return pProgram->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBinary
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBinary(cl_context				clContext,
												  cl_uint					uiNumDevices,
												  const cl_device_id *		pclDeviceList,
												  const size_t *			pszLengths,
												  const unsigned char **	ppBinaries,
												  cl_int *					piBinaryStatus,
												  cl_int *					pErrRet)
{
	LOG_INFO(TEXT("CreateProgramWithBinary enter. clContext=%d, uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d"), 
		clContext, uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus);
	if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
	{
		// invalid value
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries"));
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
		LOG_ERROR(TEXT("%S"), TEXT("m_pContexts == NULL || NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED"));
		if (NULL != pErrRet)
		{
			*pErrRet = CL_ERR_INITILIZATION_FAILED;
		}
		return CL_INVALID_HANDLE;
	}
	// get the context object
	cl_err_code clErrRet = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %d"), clContext, &pContext, clErrRet);
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
		pProgram->Release();
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_pPrograms->AddObject((OCLObject<_cl_program_int>*)pProgram, false);
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
	return pProgram->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code	ContextModule::RetainProgram(cl_program clProgram)
{
	LOG_INFO(TEXT("Enter RetainProgram (clProgram=%d)"), clProgram);

#ifdef _DEBUG
	assert("Programs map list isn't initialized" && (NULL != m_pPrograms));
#endif
	
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("program %d is invalid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}
	return pProgram->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseProgram(cl_program clProgram)
{
	LOG_INFO(TEXT("Enter ReleaseProgram (clProgram=%d)"), clProgram);

#ifdef _DEBUG
	assert("Programs map list isn't initialized" && (NULL != m_pPrograms));
#endif

	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("program %d is invalid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}
	Context * pContext = const_cast<Context *>(pProgram->GetContext());
	if (NULL == pContext)
	{
		return CL_INVALID_PROGRAM;
	}
	//Prevent deletion of context by program release before we're ready
	pContext->AddPendency();

	long newRef = pProgram->Release();
	if (newRef < 0)
	{
		return CL_INVALID_PROGRAM;
	}
	else if (0 == newRef)
	{
		clErrRet = pContext->RemoveProgram(clProgram);
		if (CL_FAILED(clErrRet))
		{
			return CL_ERR_OUT(clErrRet);
		}

		clErrRet = m_pPrograms->RemoveObject((_cl_program_int*)clProgram);
		if (CL_FAILED(clErrRet))
		{
			return CL_ERR_OUT(clErrRet);
		}
	}
	pContext->RemovePendency();
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::BuildProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::BuildProgram(cl_program clProgram, 
								   cl_uint uiNumDevices, 
								   const cl_device_id * pclDeviceList, 
								   const char * pcOptions, 
								   void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
								   void * pUserData)
{
	//cl_start;

	LOG_INFO(TEXT("BuildProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d"), 
		clProgram, uiNumDevices, pclDeviceList, pcOptions, pUserData);

	if (NULL == m_pPrograms)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pPrograms"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get program from programs map list
	Program * pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

	cl_int clErr = pProgram->Build(uiNumDevices, pclDeviceList, pcOptions, pfn_notify, pUserData);
	//cl_return clErr;
	return clErr;
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
	LOG_INFO(TEXT("GetProgramInfo enter. clProgram=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d"), 
		clProgram, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	
	cl_err_code clErrRet = CL_SUCCESS;
	Program * pProgram = NULL;
	if (NULL == m_pPrograms)
	{
		LOG_ERROR(TEXT("%S"), TEXT("m_pPrograms == NULL; return CL_ERR_INITILIZATION_FAILED"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get program from the programs map list
	clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pPrograms->GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
		return CL_INVALID_PROGRAM;
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
	LOG_INFO(TEXT("GetProgramBuildInfo enter. clProgram=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d"), 
		clProgram, clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	
	cl_err_code clErrRet = CL_SUCCESS;
	Program * pProgram = NULL;
	if (NULL == m_pPrograms)
	{
		LOG_ERROR(TEXT("%S"), TEXT("m_pPrograms == NULL; return CL_ERR_INITILIZATION_FAILED"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	// get program from the programs map list
	clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_pPrograms->GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
		return CL_INVALID_PROGRAM;
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
	LOG_INFO(TEXT("CreateKernel enter. clProgram=%d, pscKernelName=%d, piErr=%d"), clProgram, pscKernelName, piErr);

#ifdef _DEBUG
	assert ( (NULL != m_pPrograms) && (NULL != m_pKernels) );
#endif

	// get program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("clProgram is invalid program"));
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
		m_pKernels->AddObject((OCLObject<_cl_kernel_int>*)pKernel, false);
		if (NULL != piErr)
		{
			*piErr = CL_SUCCESS;
		}
		// return handle
		return pKernel->GetHandle();
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
	LOG_INFO(TEXT("CreateKernelsInProgram enter. clProgram=%d, uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d"), 
		clProgram, uiNumKernels, pclKernels, puiNumKernelsRet);

	// check invalid input
	if (NULL == m_pPrograms || NULL == m_pKernels)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pPrograms || NULL == m_pKernels"));
		return CL_ERR_FAILURE;
	}
	
	// get the program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("clProgram is invalid program"));
		return CL_INVALID_PROGRAM;
	}

	// create all kernels for the program
	clErrRet = pProgram->CreateAllKernels(uiNumKernels, pclKernels, puiNumKernelsRet);
	if (CL_FAILED(clErrRet))
	{
		return CL_ERR_OUT(clErrRet);
	}
	//No point in creating user-invisible kernels
	if (NULL == pclKernels)
	{
		return CL_SUCCESS;
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
		m_pKernels->AddObject((OCLObject<_cl_kernel_int>*)ppKernels[ui], false);
	}
	
	delete[] ppKernels;
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainKernel(cl_kernel clKernel)
{
	LOG_INFO(TEXT("Enter RetainKernel (clKernel=%d)"), clKernel);

	if (NULL == m_pKernels)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pKernels"))
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
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
	LOG_INFO(TEXT("Enter ReleaseKernel (clKernel=%d)"), clKernel);

	if (NULL == m_pKernels)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pKernels"));
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	Program * pProgram = const_cast<Program *>(pKernel->GetProgram());
	if (NULL == pProgram)
	{
		return CL_INVALID_KERNEL;
	}
	//Prevent deletion of program by kernel release until we're ready
	pProgram->AddPendency();

	long newRef = pKernel->Release();
	cl_int err = CL_SUCCESS;
	if (newRef < 0)
	{
		LOG_ERROR(TEXT("pKernel->Release() returned %ld"), newRef);
		err = CL_INVALID_KERNEL;
	}
	else if (0 == newRef)
	{
		clErr = pProgram->RemoveKernel(clKernel);
		if (CL_FAILED(clErr))
		{
			return CL_ERR_OUT(clErr);
		}
		// remove kernel form kernels list and add it to the dirty kernels list		
		clErr = m_pKernels->RemoveObject((_cl_kernel_int*)clKernel);
		if (CL_FAILED(clErr))
		{
			return CL_ERR_OUT(clErr);
		}
	}
	pProgram->RemovePendency();
	return err;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArg
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetKernelArg(cl_kernel clKernel, 
								   cl_uint	uiArgIndex, 
								   size_t szArgSize, 
								   const void * pArgValue)
{
	LOG_DEBUG(TEXT("Enter SetKernelArg (clKernel=%d, uiArgIndex=%d, szArgSize=%d, pszArgValue=%d)"), 
		clKernel, uiArgIndex, szArgSize, pArgValue);

	if (NULL == m_pKernels)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pKernels"));
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
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
	LOG_INFO(TEXT("Enter GetKernelInfo (clKernel=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clKernel, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	assert ( "Initialization Failure" && (NULL != m_pKernels) );

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
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
	LOG_INFO(TEXT("Enter GetKernelWorkGroupInfo (clKernel=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clKernel, clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	assert ( "Initialization Failure" && (NULL != m_pKernels) );

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	return pKernel->GetWorkGroupInfo(clDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
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
	LOG_DEBUG(TEXT("Enter CreateBuffer (clContext=%d, clFlags=%d, szSize=%d, pHostPtr=%d, pErrcodeRet=%d)"), 
		clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	Buffer * pBuffer = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pContext->CreateBuffer(clFlags, szSize, pHostPtr, &pBuffer);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pContext->CreateBuffer(%d, %d, %d, %d) = %S"), clFlags, szSize, pHostPtr, &pBuffer, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
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
	return pBuffer->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSubBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateSubBuffer(cl_mem				clBuffer,
								   cl_mem_flags				clFlags, 
								   cl_buffer_create_type	buffer_create_type, 
								   const void *				buffer_create_info, 
								   cl_int *					pErrcodeRet)
{
	LOG_INFO(TEXT("Enter CreateSubBuffer (clFlags=%d, cl_buffer_create_type=%d, pErrcodeRet=%d)"), 
		clFlags, buffer_create_type, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clBuffer, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clBuffer, &pMemObj, ClErrTxt(clErr));
		*pErrcodeRet = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}

	Context * pContext = const_cast<Context *>(pMemObj->GetContext());

	// check memory object is a Buffer not Image2D/3D
	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		*pErrcodeRet = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}
			
	Buffer* pParentBuffer = dynamic_cast<Buffer*>(pMemObj);
	if (pParentBuffer->IsSubBuffer())
	{
		*pErrcodeRet = CL_INVALID_MEM_OBJECT;
		return CL_INVALID_HANDLE;
	}

	Buffer* pBuffer = NULL;
	clErr = pContext->CreateSubBuffer(pParentBuffer, clFlags, buffer_create_type, buffer_create_info, &pBuffer);
	if (CL_FAILED(clErr))
	{		
		*pErrcodeRet = CL_ERR_OUT(clErr);		
		return CL_INVALID_HANDLE;
	}

	clErr = m_pMemObjects->AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
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

	return pBuffer->GetHandle();	
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
	LOG_DEBUG(TEXT("Enter CreateImage2D (clContext=%d, clFlags=%d, clImageFormat=%d, szImageWidth=%d, szImageHeight=%d, szImageRowPitch=%d, pHostPtr=%d, pErrcodeRet=%d)"), 
		clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, szImageRowPitch, pHostPtr, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	Image2D * pImage2d = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pContext->CreateImage2D(clFlags, clImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, &pImage2d);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pContext->CreateImage2D(%d, %d, %d, %d, %d, %d, %d) = %S"), clFlags, clImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, &pImage2d, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pImage2d, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pImage2d, pImage2d->GetHandle(), ClErrTxt(clErr))
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
	return pImage2d->GetHandle();
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
	LOG_DEBUG(TEXT("Enter CreateImage3D (clContext=%d, clFlags=%d, clImageFormat=%d, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d, pHostPtr=%d, pErrcodeRet=%d)"), 
		clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, pHostPtr, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	Image3D * pImage3d = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pContext->CreateImage3D(clFlags, clImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, &pImage3d);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pContext->CreateImage3D(%d, %d, %d, %d, %d, %d, %d, %d, %d) = %S"), clFlags, clImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, &pImage3d, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pImage3d, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pImage3d, pImage3d->GetHandle(), ClErrTxt(clErr))
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
	return pImage3d->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainMemObject(cl_mem clMemObj)
{
	LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clMemObj);

#ifdef _DEBUG
	assert ("memory objects map list wasn't initiaized" && NULL != m_pMemObjects);
#endif

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clMemObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}
	return pMemObj->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseMemObject(cl_mem clMemObj)
{
	LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clMemObj);

	if (NULL == m_pMemObjects)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pMemObjects"));
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;
	Context *pContext;

	clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clMemObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}
	pContext = const_cast<Context *>(pMemObj->GetContext());
	if (!pContext)
	{
		return CL_INVALID_MEM_OBJECT;
	}

	//Prevent deletion of context by program release before we're ready
	pContext->AddPendency();

	long newRef = pMemObj->Release();

	cl_int res = CL_SUCCESS;
	if (newRef < 0)
	{
		return CL_INVALID_MEM_OBJECT;
	}
	else if (0 == newRef)
	{
		// TODO: handle release memory object
		clErr = pContext->RemoveMemObject(clMemObj);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}

		clErr = m_pMemObjects->RemoveObject((_cl_mem_int*)clMemObj);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}
	}

	pContext->RemovePendency();
	return res;
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
	LOG_INFO(TEXT("Enter GetSupportedImageFormats (clContext=%d, clFlags=%d, clImageType=%d, uiNumEntries=%d, pclImageFormats=%d, puiNumImageFormats=%d)"), 
		clContext, clFlags, clImageType, uiNumEntries, pclImageFormats, puiNumImageFormats);

#ifdef _DEBUG
	assert ("Initialization error" && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext);
		return CL_INVALID_CONTEXT;
	}
	return pContext->GetSupportedImageFormats(clFlags, clImageType, uiNumEntries, pclImageFormats, puiNumImageFormats);
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
	LOG_INFO(TEXT("Enter GetMemObjectInfo (clMemObj=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clMemObj, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == m_pMemObjects)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pMemObjects"));
		return CL_ERR_FAILURE;
	}
	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clMemObj, &pMemObj, ClErrTxt(clErr));
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
	LOG_INFO(TEXT("Enter GetImageInfo (clImage=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clImage, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	assert( "initialization error" && (NULL != m_pMemObjects) );

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clImage, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clImage, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}

	if (pMemObj->GetType() == CL_MEM_OBJECT_IMAGE2D)
	{
		return ((Image2D*)pMemObj)->GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}
	if (pMemObj->GetType() == CL_MEM_OBJECT_IMAGE3D)
	{
		return ((Image3D*)pMemObj)->GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}
	return CL_INVALID_MEM_OBJECT;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetMemObjectDestructorCallback (cl_mem memObj,
													  mem_dtor_fn pfn_notify,													  
													  void *pUserData)
{
	cl_err_code clErr = CL_SUCCESS;

	MemoryObject * pMemObj = NULL;
	clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)memObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), memObj, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}

	// if pfn_notify is NULL. the following register function will return CL_INVALID_VALUE
	clErr = pMemObj->registerDtorNotifierCallback(pfn_notify,pUserData);
	return clErr;
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
	LOG_DEBUG(TEXT("Enter CreateSampler (clContext=%d, bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, pErrcodeRet=%d)"), 
		clContext, bNormalizedCoords, clAddressingMode, clFilterMode, pErrcodeRet);

#ifdef _DEBUG
	assert (NULL != m_pMemObjects && NULL != m_pContexts);
#endif

	Context * pContext = NULL;
	Sampler * pSampler = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pContext->CreateSampler(bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pContext->CreateSampler(%d, %d, %d, %d) = %S"), bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pSamplers->AddObject(pSampler, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pSampler, pSampler->GetHandle(), ClErrTxt(clErr))
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
	return pSampler->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainSampler(cl_sampler clSampler)
{
	LOG_DEBUG(TEXT("Enter RetainSampler (clSampler=%d)"), clSampler);

#ifdef _DEBUG
	assert ("Samplers map list wasn't initialized" && NULL != m_pSamplers);
#endif
	
	cl_err_code clErr = CL_SUCCESS;
	Sampler * pSampler = NULL;

	clErr = m_pSamplers->GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
	if (CL_FAILED(clErr) || NULL == pSampler)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clSampler, &pSampler, ClErrTxt(clErr));
		return CL_INVALID_SAMPLER;
	}
	return pSampler->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseSampler(cl_sampler clSampler)
{
	LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clSampler);

#ifdef _DEBUG
	assert ("Samplers map list wasn't initiaized" && NULL != m_pSamplers);
#endif

	cl_err_code clErr = CL_SUCCESS;
	Sampler* pSampler = NULL;
	Context* pContext = NULL;

	clErr = m_pSamplers->GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
	if (CL_FAILED(clErr) || NULL == pSampler)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clSampler, &pSampler, ClErrTxt(clErr));
		return CL_INVALID_SAMPLER;
	}
	pContext = const_cast<Context *>(pSampler->GetContext());
	if (!pContext)
	{
		return CL_INVALID_SAMPLER;
	}
	//Prevent deletion of context by program release before we're ready
	pContext->AddPendency();

	long newRef = pSampler->Release();
	cl_int res = CL_SUCCESS;
	if (newRef < 0)
	{
		return CL_INVALID_SAMPLER;
	}
	else if (0 == newRef)
	{
		clErr = pContext->RemoveSampler(clSampler);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}		
		clErr = m_pSamplers->RemoveObject((_cl_sampler_int*)clSampler);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}
	}

	pContext->RemovePendency();
	return res;
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
	LOG_INFO(TEXT("Enter GetSamplerInfo (clSampler=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clSampler, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	assert( "initialization error" && (NULL != m_pSamplers) );

	cl_err_code clErr = CL_SUCCESS;
	Sampler * pSampler = NULL;

	clErr = m_pSamplers->GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
	if (CL_FAILED(clErr) || NULL == pSampler)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clSampler, &pSampler, ClErrTxt(clErr));
		return CL_INVALID_SAMPLER;
	}

	clErr = pSampler->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	return CL_ERR_OUT(clErr);
}
Context* ContextModule::GetContext(cl_context clContext) const
{
	assert ( NULL != m_pContexts );
	Context * pContext = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
	cl_err_code clErr = m_pKernels->GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_SUCCEEDED(clErr))
	{
		return pKernel;
	}
	return NULL;
}

MemoryObject * ContextModule::GetMemoryObject(const cl_mem clMemObjId)
{
	MemoryObject * pMemoryObject = NULL;
	cl_err_code clErr = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObjId, (OCLObject<_cl_mem_int>**)&pMemoryObject);
	if (CL_SUCCEEDED(clErr))
	{
		return pMemoryObject;
	}
	return NULL;
}

cl_mem ContextModule::CreateFromGLBuffer(cl_context clContext, 
						  cl_mem_flags clMemFlags, 
						  GLuint glBufObj, 
						  int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter CreateFromGLBuffer (clContext=%d, clFlags=%d, pErrcodeRet=%d)"), 
		clContext, clMemFlags, pErrcodeRet);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	Context * pContext = NULL;
	Buffer * pBuffer = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
			if (NULL != pErrcodeRet)
			{
				*pErrcodeRet = CL_INVALID_CONTEXT;
			}
			return CL_INVALID_HANDLE;
	}

	GLContext* pGLContext = dynamic_cast<GLContext*>(pContext);
	if ( NULL == pGLContext )
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}

	clErr = pGLContext->CreateGLBuffer(clMemFlags, glBufObj, &pBuffer);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pGLContext->CreateGLBuffer(%d, %d, %d, %d) = %S"), clMemFlags, glBufObj, &pBuffer, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
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
	return pBuffer->GetHandle();
#else
	assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_mem ContextModule::CreateFromGLTexture2D(cl_context clContext, 
							 cl_mem_flags clMemFlags, 
							 GLenum glTextureTarget, 
							 GLint glMipLevel, 
							 GLuint glTexture, 
							 cl_int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter CreateFromGLTexture2D (clContext=%d, clFlags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%, pErrcodeRet=%d)"), 
		clContext, clMemFlags, glTextureTarget, glMipLevel, glTexture, pErrcodeRet);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}

	GLContext* pGLContext = dynamic_cast<GLContext*>(pContext);
	if ( NULL == pGLContext )
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pGLContext->CreateGLTexture2D(clMemFlags, glTextureTarget, glMipLevel, glTexture, &pMemObj);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pGLContext->CreateGLTexture2D(%d, %d, %d, %d, %d) = %S"), clMemFlags, glTextureTarget, glMipLevel, glTexture,
			&pMemObj, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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
	return pMemObj->GetHandle();
#else
	assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_mem ContextModule::CreateFromGLTexture3D(cl_context clContext, 
							 cl_mem_flags clMemFlags, 
							 GLenum glTextureTarget, 
							 GLint glMipLevel, 
							 GLuint glTexture, 
							 cl_int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter CreateFromGLTexture3D (clContext=%d, clFlags=%d, glTextureTarget=%d, glMipLevel=%d, glTexture=%, pErrcodeRet=%d)"), 
		clContext, clMemFlags, glTextureTarget, glMipLevel, glTexture, pErrcodeRet);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
			if (NULL != pErrcodeRet)
			{
				*pErrcodeRet = CL_INVALID_CONTEXT;
			}
			return CL_INVALID_HANDLE;
	}

	GLContext* pGLContext = dynamic_cast<GLContext*>(pContext);
	if ( NULL == pGLContext )
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pGLContext->CreateGLTexture3D(clMemFlags, glTextureTarget, glMipLevel, glTexture, &pMemObj);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pGLContext->CreateGLTexture3D(%d, %d, %d, %d, %d) = %S"), clMemFlags, glTextureTarget, glMipLevel, glTexture,
			&pMemObj, ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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
	return pMemObj->GetHandle();
#else
	assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_mem ContextModule::CreateFromGLRenderbuffer(cl_context clContext, 
								cl_mem_flags clMemFlags, 
								GLuint glRenderBuffer, 
								cl_int * pErrcodeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter CreateFromGLRenderbuffer (clContext=%d, clFlags=%d, glRenderBuffer=%d, pErrcodeRet=%d)"), 
		clContext, clMemFlags, glRenderBuffer, pErrcodeRet);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_pContexts->GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
			if (NULL != pErrcodeRet)
			{
				*pErrcodeRet = CL_INVALID_CONTEXT;
			}
			return CL_INVALID_HANDLE;
	}

	GLContext* pGLContext = dynamic_cast<GLContext*>(pContext);
	if ( NULL == pGLContext )
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}
	clErr = pGLContext->CreateGLRenderBuffer(clMemFlags, glRenderBuffer, &pMemObj);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("pGLContext->CreateGLRenderBuffer(%d, %d, %d) = %S"), clMemFlags, glRenderBuffer, &pMemObj, ClErrTxt(clErr))
			if (NULL != pErrcodeRet)
			{
				*pErrcodeRet = CL_ERR_OUT(clErr);
			}
			return CL_INVALID_HANDLE;
	}
	clErr = m_pMemObjects->AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_pMemObjects->AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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
	return pMemObj->GetHandle();
#else
	assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_HANDLE; 
#endif
}

cl_int ContextModule::GetGLObjectInfo(cl_mem clMemObj, 
					   cl_gl_object_type * pglObjectType, 
					   GLuint * pglObjectName)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter GetGLObjectInfo (clMemObj=%d, pglObjectType=%d, pglObjectName=%d)"), 
		clMemObj, pglObjectType, pglObjectName);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	// get program from programs map list
	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErrRet) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
		return CL_INVALID_MEM_OBJECT;
	}

	GLMemoryObject* pGLObject= NULL;
	// Check for GL object
	// Check if it's a GL object
	if ( NULL != (pGLObject = static_cast<GLMemoryObject*>(dynamic_cast<GLBuffer*>(pMemObj))) )
	{
		return pGLObject->GetGLObjectInfo(pglObjectType, pglObjectName);
	}
	if ( NULL != (pGLObject = static_cast<GLMemoryObject*>(dynamic_cast<GLTexture2D*>(pMemObj))) )
	{
		return pGLObject->GetGLObjectInfo(pglObjectType, pglObjectName);
	}
	if ( NULL != (pGLObject = static_cast<GLMemoryObject*>(dynamic_cast<GLTexture3D*>(pMemObj))) )
	{
		return pGLObject->GetGLObjectInfo(pglObjectType, pglObjectName);
	}
	if ( NULL != (pGLObject = static_cast<GLMemoryObject*>(dynamic_cast<GLRenderBuffer*>(pMemObj))) )
	{
		return pGLObject->GetGLObjectInfo(pglObjectType, pglObjectName);
	}
	
	return CL_INVALID_GL_OBJECT;
#else
	assert (0 && "NOT Implemented on Linux");
  return CL_INVALID_GL_OBJECT; 
#endif
}

cl_int ContextModule::GetGLTextureInfo(cl_mem clMemObj, 
									   cl_gl_texture_info clglPramName, 
									   size_t szParamValueSize, 
									   void * pParamValue, 
									   size_t * pszParamValueSizeRet)
{
#if defined (_WIN32)  //TODO GL support for Linux
	LOG_DEBUG(TEXT("Enter GetGLTextureInfo (clMemObj=%d, cl_gl_texture_info=%d)"), 
		clMemObj, clglPramName);

	assert (NULL != m_pMemObjects && NULL != m_pContexts);

	// get program from programs map list
	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErrRet) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
		return CL_INVALID_MEM_OBJECT;
	}

	GLTexture* pGLObject= NULL;
	// Check for GL object
	// Check if it's a GL object
	if ( NULL != (pGLObject = static_cast<GLTexture*>(dynamic_cast<GLTexture2D*>(pMemObj))) )
	{
		return pGLObject->GetGLTextureInfo(clglPramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}
	if ( NULL != (pGLObject = static_cast<GLTexture*>(dynamic_cast<GLTexture3D*>(pMemObj))) )
	{
		return pGLObject->GetGLTextureInfo(clglPramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}

	return CL_INVALID_GL_OBJECT;
#else
	assert (0 && "NOT Implemented on Linux");
	return CL_INVALID_GL_OBJECT;
#endif
}
