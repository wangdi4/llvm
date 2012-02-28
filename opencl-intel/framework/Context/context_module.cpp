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
#include "sampler.h"
#include "MemoryAllocator/MemoryObject.h"
#include "ocl_itt.h"
#if defined (_WIN32)
#include "gl_context.h"
#include "gl_shr_utils.h"
#include "gl_mem_objects.h"
#if defined (DX9_MEDIA_SHARING)
#include "d3d9_sharing.h"
#include "d3d9_context.h"
#endif
#endif
#include <platform_module.h>
#include <Device.h>
#include <cl_objects_map.h>
#include <cl_utils.h>
#include <assert.h>
#include <set>
#include "GenericMemObj.h"
#include "Image1DBuffer.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//////////////////////////////////////////////////////////////////////////
// ContextModule C'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::ContextModule(PlatformModule *pPlatformModule) : OCLObjectBase("ContextModule")
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

	RELEASE_LOGGER_CLIENT;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::Initialize
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Initialize(ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData)
{
	LOG_INFO(TEXT("%S"), TEXT("ContextModule::Initialize enter"));

	m_pOclEntryPoints = pOclEntryPoints;
	m_pGPAData = pGPAData;
	
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::Release
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Release(  bool bTerminate )
{
	LOG_INFO(TEXT("%S"), TEXT("ContextModule::Release enter"));

	return CL_SUCCESS;
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

	LOG_INFO(TEXT("Enter ContextModule::CreateContext (clProperties=%d, uiNumDevices=%d, pDevices=%d)"), 
		clProperties, uiNumDevices, pDevices);
	
	if (!pDevices)
	{
		LOG_ERROR(TEXT("%S"), TEXT("(!pDevices); return CL_INVALID_VALUE"));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}

	if (!pfnNotify && pUserData)
	{
		LOG_ERROR(TEXT("%S"), TEXT("(!pfnNotify && pUserData); return CL_INVALID_VALUE"));
		if (NULL != pRrrcodeRet)
		{	
			*pRrrcodeRet = CL_INVALID_VALUE;
		}
		return CL_INVALID_HANDLE;
	}

	if (NULL != pRrrcodeRet)
	{
		*pRrrcodeRet = CL_SUCCESS;
	}

	FissionableDevice ** ppDevices = new FissionableDevice * [uiNumDevices];
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
        delete[] ppDevices;
		if (NULL != pRrrcodeRet)
		{				
			*pRrrcodeRet = CL_INVALID_DEVICE;
		}
		return CL_INVALID_HANDLE;
	}

    cl_uint numRootDevices = 0;
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (ppDevices[i]->IsRootLevelDevice())
        {
            ++numRootDevices;
        }
    }

	Context *pContext = NULL;
    // check properties
    if (NULL != clProperties)
    {
        size_t i = 0;
        std::set<cl_context_properties> propertySet;

        while (0 != clProperties[i])
        {
            if (CL_CONTEXT_PLATFORM == clProperties[i] &&
                !m_pPlatformModule->CheckPlatformId((cl_platform_id)clProperties[i + 1]))
            {
                LOG_ERROR(TEXT("%s"), TEXT("platform value specified in properties is not a valid platform"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PLATFORM;
                }
                return CL_INVALID_HANDLE;
            }
            if (propertySet.find(clProperties[i]) != propertySet.end())
            {
                LOG_ERROR(TEXT("%s"), TEXT("the same property name is specified more than once"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PROPERTY;
                }
                return CL_INVALID_HANDLE;
            }
            if (CL_CONTEXT_PLATFORM != clProperties[i]
#if defined (_WIN32)
                && CL_GL_CONTEXT_KHR != clProperties[i] && CL_WGL_HDC_KHR != clProperties[i]
#if defined (DX9_MEDIA_SHARING)
                && CL_CONTEXT_D3D9_DEVICE_INTEL != clProperties[i] &&
                   CL_CONTEXT_D3D9EX_DEVICE_INTEL != clProperties[i] &&
                   CL_CONTEXT_DXVA_DEVICE_INTEL != clProperties[i]
#endif
#endif
                )
            {
                LOG_ERROR(TEXT("%s"), TEXT("context property name in properties is not a supported property name"));
                delete[] ppDevices;
                if (NULL != pRrrcodeRet)
                {
                    *pRrrcodeRet = CL_INVALID_PROPERTY;
                }
                return CL_INVALID_HANDLE;
            }        
            propertySet.insert(clProperties[i]);
            i += 2;
        }
    }
    
#if defined (_WIN32)  //TODO GL support for Linux
	cl_context_properties hGLCtx, hDC;
    bool bGLSharingSupported = false;
	ParseGLContextOptions(clProperties, &hGLCtx, &hDC, &bGLSharingSupported);
#if defined (DX9_MEDIA_SHARING)
    IUnknown* pD3D9Device;
    int iDevType;
    clErrRet = ParseD3D9ContextOptions(clProperties, pD3D9Device, &iDevType);
    if (CL_SUCCESS != clErrRet)
    {
        LOG_ERROR(TEXT("%s"), TEXT("more than one Direct3D 9 device is specified in properties"));
        if (NULL != pRrrcodeRet)
        {
            *pRrrcodeRet = clErrRet;
        }
        delete[] ppDevices;
        return CL_INVALID_HANDLE;
    }
    if (NULL != pD3D9Device && bGLSharingSupported)
    {
        LOG_ERROR(TEXT("%S"), TEXT("CL_INVALID_D3D9_DEVICE_INTEL is set to a non-NULL value and interoperability with OpenGL is also specified."));
        if (NULL != pRrrcodeRet)
        {
            *pRrrcodeRet = CL_INVALID_OPERATION;
        }
        delete[] ppDevices;
        return CL_INVALID_HANDLE;
    }
#endif
#endif
	// Default error in case new() will fail
	clErrRet = CL_OUT_OF_HOST_MEMORY;
#if defined (_WIN32)  //TODO GL support for Linux
	if (bGLSharingSupported)
	{
		pContext = 	new GLContext(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, hGLCtx, hDC, m_pGPAData);
	} else
#endif
#if defined (DX9_MEDIA_SHARING)
    if (NULL != pD3D9Device)
    {
        pContext = new D3D9Context(clProperties, uiNumDevices, numRootDevices, ppDevices,
            pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, m_pGPAData, pD3D9Device, iDevType);
    }
    else
#endif
	{
		pContext = 	new Context(clProperties, uiNumDevices, numRootDevices, ppDevices, pfnNotify, pUserData, &clErrRet, m_pOclEntryPoints, m_pGPAData);
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
		{
			pContext->Release();
		}
		return CL_INVALID_HANDLE;
	}

	delete[] ppDevices;
	
	cl_context clContextId = (cl_context)m_mapContexts.AddObject(pContext);
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

cl_err_code ContextModule::GetRootDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices)
{
	LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d"), uiNumDevices, pclDeviceIds, ppDevices);
	cl_err_code clErrRet = CL_SUCCESS;

#ifdef _DEBUG
	assert ( NULL != m_pPlatformModule );
	assert ( (NULL != ppDevices) && (0 != uiNumDevices) );
#endif

    cl_uint rootId = 0;
	// go through device ids and get the device from the platform module
	FissionableDevice * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pPlatformModule->GetDevice(pclDeviceIds[ui], &pDevice);
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%d, %d) = %d"), pclDeviceIds[ui], &pDevice, clErrRet);
			return clErrRet;
		}
        if (pDevice->IsRootLevelDevice())
        {
            ppDevices[rootId++] = pDevice->GetRootDevice();
        }
	}
	return CL_SUCCESS;
}

cl_err_code ContextModule::GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, FissionableDevice ** ppDevices)
{
    LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%d, pclDeviceIds=%d, ppDevices=%d"), uiNumDevices, pclDeviceIds, ppDevices);
    cl_err_code clErrRet = CL_SUCCESS;

#ifdef _DEBUG
    assert ( NULL != m_pPlatformModule );
    assert ( (NULL != ppDevices) && (0 != uiNumDevices) );
#endif

    // go through device ids and get the device from the platform module
    FissionableDevice * pDevice = NULL;
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
	clErrRet = m_mapContexts.GetOCLObject((_cl_context_int*)context, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
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

	cl_err_code err = m_mapContexts.ReleaseObject((_cl_context_int*)context);
	return ((CL_ERR_KEY_NOT_FOUND == err) ? CL_INVALID_CONTEXT : err);
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

	cl_err_code clErrRet = CL_SUCCESS;
	Context * pContext = NULL;
	// get context from the contexts map list
	clErrRet = m_mapContexts.GetOCLObject((_cl_context_int*)context, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), context, &pContext, clErrRet);
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
	clErrRet = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), clContext, &pContext, clErrRet);
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
	clErrRet = m_mapPrograms.AddObject((OCLObject<_cl_program_int>*)pProgram, false);
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
	// get the context object
	cl_err_code clErrRet = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%d, %d) = %d"), clContext, &pContext, clErrRet);
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
        if (pProgram)
        {
            pProgram->Release();
        }		
		return CL_INVALID_HANDLE;
	}
	clErrRet = m_mapPrograms.AddObject((OCLObject<_cl_program_int>*)pProgram, false);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrRet)
		{
			*pErrRet = CL_OUT_OF_HOST_MEMORY;
		}
		pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet, sizeof(cl_int));
        pProgram->Release();
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
	
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
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

	Program *pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
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
	pContext->AddPendency(this);

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

		clErrRet = m_mapPrograms.RemoveObject((_cl_program_int*)clProgram);
		if (CL_FAILED(clErrRet))
		{
			return CL_ERR_OUT(clErrRet);
		}
	}
	pContext->RemovePendency(this);
	return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CompileProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CompileProgram(cl_program clProgram, 
                                     cl_uint uiNumDevices, 
                                     const cl_device_id * pclDeviceList, 
                                     const char * pcOptions, 
                                     cl_uint num_input_headers, 
                                     const cl_program* pclInputHeaders, 
                                     const char **header_include_names, 
                                     void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
                                     void * pUserData)
{
	LOG_INFO(TEXT("CompileProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, num_input_headers=%d, pclInputHeaders=%d, header_include_names=%d, pUserData=%d"), 
		clProgram, uiNumDevices, pclDeviceList, pcOptions, num_input_headers, pclInputHeaders, header_include_names, pUserData);

    if ((0 == num_input_headers) && ((NULL != pclInputHeaders) || (NULL != header_include_names)))
    {
        return CL_INVALID_VALUE;
    }

    if ((0 != num_input_headers) && ((NULL == pclInputHeaders) || (NULL == header_include_names)))
    {
        return CL_INVALID_VALUE;
    }

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        return CL_INVALID_VALUE;
    }

	// get program from programs map list
	Program * pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

    Context* pContext = pProgram->GetContext();
    cl_int clErr = pContext->CompileProgram(clProgram, uiNumDevices, pclDeviceList, num_input_headers, pclInputHeaders, header_include_names, pcOptions, pfn_notify, pUserData);

	return clErr;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::LinkProgram
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::LinkProgram(cl_context clContext, 
                                      cl_uint uiNumDevices, 
                                      const cl_device_id * pclDeviceList, 
                                      const char * pcOptions, 
                                      cl_uint uiNumInputPrograms, 
                                      const cl_program* pclInputPrograms, 
                                      void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), 
                                      void * pUserData, 
                                      cl_int *pErrcodeRet)
{
	LOG_INFO(TEXT("LinkProgram enter. clContext=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, uiNumInputPrograms=%d, pclInputPrograms=%d, pUserData=%d"), 
		clContext, uiNumDevices, pclDeviceList, pcOptions, uiNumInputPrograms, pclInputPrograms, pUserData);

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_VALUE;
		}
        return CL_INVALID_HANDLE;
    }

    if ((0 == uiNumInputPrograms) || (NULL == pclInputPrograms))
    {
        if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_VALUE;
		}
        return CL_INVALID_HANDLE;
    }

    if ((NULL == pclDeviceList) && (0 < uiNumDevices))
    {
        if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_VALUE;
		}
        return CL_INVALID_HANDLE;
    }

    if ((NULL != pclDeviceList) && (0 == uiNumDevices))
    {
        if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_VALUE;
		}
        return CL_INVALID_HANDLE;
    }

    // get context from contexts map list
	Context * pContext = NULL;
    cl_err_code clErrRet = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErrRet) || NULL == pContext)
	{
		LOG_ERROR(TEXT("context %d isn't valid context"), clContext);
        if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
        return CL_INVALID_HANDLE;
	}

    Program *pProgram = NULL;

    if (0 < uiNumDevices)
    {
    clErrRet = pContext->CreateProgramForLink(uiNumDevices, pclDeviceList, &pProgram);
	    if (CL_FAILED(clErrRet))
	    {
		    if (NULL != pErrcodeRet)
		    {
			    *pErrcodeRet = clErrRet;
		    }
		    return CL_INVALID_HANDLE;
	    }
    }
    else
    {
        size_t uiNumContextDevices = 0;
        cl_device_id* pContextDevices = pContext->GetDeviceIds(&uiNumContextDevices);

        clErrRet = pContext->CreateProgramForLink(uiNumContextDevices, pContextDevices, &pProgram);
	    if (CL_FAILED(clErrRet))
	    {
		    if (NULL != pErrcodeRet)
		    {
			    *pErrcodeRet = clErrRet;
		    }
		    return CL_INVALID_HANDLE;
	    }
    }

    clErrRet = m_mapPrograms.AddObject((OCLObject<_cl_program_int>*)pProgram, false);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		return CL_INVALID_HANDLE;
	}

    clErrRet = pContext->LinkProgram(pProgram->GetHandle(), uiNumDevices, pclDeviceList, uiNumInputPrograms, pclInputPrograms, pcOptions, pfn_notify, pUserData);
    if (CL_FAILED(clErrRet))
	{
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = clErrRet;
		}
		return CL_INVALID_HANDLE;
	}

    if (NULL != pErrcodeRet)
	{
		*pErrcodeRet = CL_SUCCESS;
	}
	return pProgram->GetHandle();
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
	LOG_INFO(TEXT("BuildProgram enter. clProgram=%d, uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d"), 
		clProgram, uiNumDevices, pclDeviceList, pcOptions, pUserData);

    if ((NULL == pfn_notify) && (NULL != pUserData))
    {
        return CL_INVALID_VALUE;
    }

	// get program from programs map list
	Program * pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet) || NULL == pProgram)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

    Context* pContext = pProgram->GetContext();
    cl_int clErr = pContext->BuildProgram(clProgram, uiNumDevices, pclDeviceList, pcOptions, pfn_notify, pUserData);
	
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
	// get program from the programs map list
	clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
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
	// get program from the programs map list
	clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%d, %d) = %d"), clProgram, &pProgram, clErrRet);
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

	// get program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
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
		m_mapKernels.AddObject((OCLObject<_cl_kernel_int>*)pKernel, false);
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

	// get the program object
	Program *pProgram = NULL;
	cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProgram);
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
		m_mapKernels.AddObject((OCLObject<_cl_kernel_int>*)ppKernels[ui], false);
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

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
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

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
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
	pProgram->AddPendency(this);

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
		clErr = m_mapKernels.RemoveObject((_cl_kernel_int*)clKernel);
		if (CL_FAILED(clErr))
		{
			return CL_ERR_OUT(clErr);
		}
	}
	pProgram->RemovePendency(this);
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

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
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

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
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
											 cl_device_id deviceId, 
											 cl_kernel_work_group_info clParamName, 
											 size_t szParamValueSize, 
											 void *	pParamValue, 
											 size_t * pszParamValueSizeRet)
{
	LOG_INFO(TEXT("Enter GetKernelWorkGroupInfo (clKernel=%d, clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		clKernel, deviceId, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	FissionableDevice* pDevice = NULL;
	if ( NULL != deviceId )	// When deviceId is NULL, we should pass this paramter to kernel.
							// In case of single device, it's data should be returned
	{
		clErr = m_pPlatformModule->GetDevice(deviceId, &pDevice);
		if (CL_FAILED(clErr) || NULL == pDevice)
		{
			LOG_ERROR(TEXT("GetDevice(%d, %d) returned %S"), deviceId, &pDevice, ClErrTxt(clErr));
			return CL_INVALID_DEVICE;
		}
	}
	return pKernel->GetWorkGroupInfo(pDevice, clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
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

	Context * pContext = NULL;
	MemoryObject * pBuffer = NULL;
	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_FAILED(clErr) || NULL == pContext)
	{
		LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext)
		if (NULL != pErrcodeRet)
		{
			*pErrcodeRet = CL_INVALID_CONTEXT;
		}
		return CL_INVALID_HANDLE;
	}

	clErr = CheckMemObjectParameters(clFlags, NULL, CL_MEM_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, pHostPtr);
	if ( !((CL_INVALID_IMAGE_FORMAT_DESCRIPTOR == clErr) || (CL_SUCCESS == clErr)) )
	{
		*pErrcodeRet =  clErr;
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
	clErr = m_mapMemObjects.AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
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

    cl_int iNullErr;
    cl_int& iErr = pErrcodeRet ? *pErrcodeRet : iNullErr;

    iErr = CheckMemObjectParameters(clFlags, NULL, CL_MEM_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(iErr))
    {
        return CL_INVALID_HANDLE;
    }
	if (!clBuffer)
	{
		iErr = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}

	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clBuffer, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clBuffer, &pMemObj, ClErrTxt(clErr));
		iErr = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}

	Context * pContext = const_cast<Context *>(pMemObj->GetContext());

	// check memory object is a Buffer not Image2D/3D
	if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
	{
		iErr = CL_INVALID_MEM_OBJECT;		
		return CL_INVALID_HANDLE;
	}
			
	if (NULL != pMemObj->GetParent())
	{
		iErr = CL_INVALID_MEM_OBJECT;
		return CL_INVALID_HANDLE;
	}

	MemoryObject* pBuffer = NULL;
	clErr = pContext->CreateSubBuffer(pMemObj, clFlags, buffer_create_type, buffer_create_info, &pBuffer);
	if (CL_FAILED(clErr))
	{		
		iErr = CL_ERR_OUT(clErr);		
		return CL_INVALID_HANDLE;
	}

	clErr = m_mapMemObjects.AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
		if (NULL != pErrcodeRet)
		{
			iErr = CL_ERR_OUT(clErr);
		}
		return CL_INVALID_HANDLE;
	}
	if (NULL != pErrcodeRet)
	{
		iErr = CL_SUCCESS;
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
    return CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, 0, szImageRowPitch, 0, pHostPtr, pErrcodeRet);
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
    return CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch,
        pHostPtr, pErrcodeRet);
}

/************************************************************************/
/* ContextModule::CreateImage                                              */
/************************************************************************/
cl_mem ContextModule::CreateImage(cl_context context,
                                  cl_mem_flags flags,
                                  const cl_image_format *image_format,
                                  const cl_image_desc *image_desc,
                                  void *host_ptr,
                                  cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateImage(context=%p, flags=%d, image_format=%p, image_desc=%p, host_ptr=%p, errcode_ret=%p"),
        context, flags, image_format, image_desc, host_ptr, errcode_ret);

    cl_mem clMemObj = CL_INVALID_HANDLE;

    if (!image_desc || 0 != image_desc->num_mip_levels || 0 != image_desc->num_samples ||
        (CL_MEM_OBJECT_IMAGE1D_BUFFER != image_desc->image_type && NULL != image_desc->buffer))
    {
        if (errcode_ret)
        {
            *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }
    switch (image_desc->image_type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
        clMemObj = CreateScalarImage<1, CL_MEM_OBJECT_IMAGE1D>(context, flags, image_format, image_desc->image_width, 0, 0, 0, 0, host_ptr, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        clMemObj = CreateImage1DBuffer(context, flags, image_format, image_desc->image_width, image_desc->buffer, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        clMemObj = CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(context, flags, image_format, image_desc->image_width,
            image_desc->image_height, 0, image_desc->image_row_pitch, 0, host_ptr, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        clMemObj = CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(context, flags, image_format, image_desc->image_width,
            image_desc->image_height, image_desc->image_depth, image_desc->image_row_pitch,
            image_desc->image_slice_pitch, host_ptr, errcode_ret);
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        clMemObj = CreateImageArray(context, flags, image_format, image_desc, host_ptr, errcode_ret);
        break;
    default:
        LOG_ERROR(TEXT("unsupported image type (%d)"), image_desc->image_type);
        if (errcode_ret)
        {
            *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }
    return clMemObj;
}

cl_mem ContextModule::CreateImageArray(cl_context clContext,
                                       cl_mem_flags clFlags,
                                       const cl_image_format* clImageFormat,
                                       const cl_image_desc* pClImageDesc,
                                       void* pHostPtr,
                                       cl_int* pErrcodeRet)
{
    Context * pContext = NULL;
    MemoryObject * pImageArr = NULL;

	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
    if (CL_FAILED(clErr) || NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

	// Do some initial (not context specific) parameter checking
	// check input memory flags
	clErr = CheckMemObjectParameters(clFlags, clImageFormat, pClImageDesc->image_type, pClImageDesc->image_width, pClImageDesc->image_height, 0, pClImageDesc->image_row_pitch,
        pClImageDesc->image_slice_pitch, pClImageDesc->image_array_size, pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("%S"), TEXT("Parameter check failed"));
        if (NULL != pErrcodeRet)
        {
		    *pErrcodeRet = clErr;
        }
		return CL_INVALID_HANDLE;
	}

    clErr = pContext->CreateImageArray(clFlags, clImageFormat, pHostPtr, pClImageDesc, &pImageArr);
    if (CL_FAILED(clErr) || NULL == pImageArr)
    {
        LOG_ERROR(TEXT("pContext->CreateImage2DArray(%d, %p, %p, %p, %p) = %S"), clFlags, clImageFormat, pHostPtr, pClImageDesc, &pImageArr, ClErrTxt(clErr));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
	clErr = m_mapMemObjects.AddObject(pImageArr, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pImageArr, pImageArr->GetHandle(), ClErrTxt(clErr))
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
    return pImageArr->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainMemObject(cl_mem clMemObj)
{
	LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%d)"), clMemObj);

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
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
	LOG_DEBUG(TEXT("Enter ReleaseMemObject (clMemObj=%d)"), clMemObj);

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;
	Context *pContext;

	clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
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
	pContext->AddPendency(this);

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
		clErr = m_mapMemObjects.RemoveObject((_cl_mem_int*)clMemObj);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}
	}

	pContext->RemovePendency(this);
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

	Context * pContext = NULL;
	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
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

	cl_err_code clErr = CL_SUCCESS;
	MemoryObject * pMemObj = NULL;

	clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clImage, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErr) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clImage, &pMemObj, ClErrTxt(clErr));
		return CL_INVALID_MEM_OBJECT;
	}

	// If memory object doesnt support this operation it retuns CL_INVALID_MEM_OBJECT
	return pMemObj->GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
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
	clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)memObj, (OCLObject<_cl_mem_int>**)&pMemObj);
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

	Context * pContext = NULL;
	Sampler * pSampler = NULL;
	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
	clErr = m_mapSamplers.AddObject(pSampler, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pSampler, pSampler->GetHandle(), ClErrTxt(clErr))
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

	cl_err_code clErr = CL_SUCCESS;
	Sampler * pSampler = NULL;

	clErr = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
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


	cl_err_code clErr = CL_SUCCESS;
	Sampler* pSampler = NULL;
	Context* pContext = NULL;

	clErr = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
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
	pContext->AddPendency(this);

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
		clErr = m_mapSamplers.RemoveObject((_cl_sampler_int*)clSampler);
		if (CL_FAILED(clErr))
		{
			res = CL_ERR_OUT(clErr);
		}
	}

	pContext->RemovePendency(this);
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

	cl_err_code clErr = CL_SUCCESS;
	Sampler * pSampler = NULL;

	clErr = m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSampler, (OCLObject<_cl_sampler_int>**)&pSampler);
	if (CL_FAILED(clErr) || NULL == pSampler)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clSampler, &pSampler, ClErrTxt(clErr));
		return CL_INVALID_SAMPLER;
	}

	clErr = pSampler->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	return CL_ERR_OUT(clErr);
}

Context* ContextModule::GetContext(cl_context clContext)
{
	Context * pContext = NULL;
	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
	if (CL_SUCCEEDED(clErr))
	{
		return pContext;
	}
	return NULL;
}

Kernel* ContextModule::GetKernel(cl_kernel clKernel)
{
	Kernel* pKernel = NULL;
	cl_err_code clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_SUCCEEDED(clErr))
	{
		return pKernel;
	}
	return NULL;
}

MemoryObject * ContextModule::GetMemoryObject(const cl_mem clMemObjId)
{
	MemoryObject * pMemoryObject = NULL;
	cl_err_code clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObjId, (OCLObject<_cl_mem_int>**)&pMemoryObject);
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

	Context * pContext = NULL;
	MemoryObject * pBuffer = NULL;
    
    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, CL_GL_OBJECT_BUFFER, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
	clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
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
	clErr = m_mapMemObjects.AddObject(pBuffer, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pBuffer, pBuffer->GetHandle(), ClErrTxt(clErr))
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

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;

    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, CL_GL_OBJECT_TEXTURE2D, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
	clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
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
	clErr = m_mapMemObjects.AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;

    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, CL_GL_OBJECT_TEXTURE3D, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
	clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
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
	clErr = m_mapMemObjects.AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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

	Context * pContext = NULL;
	MemoryObject * pMemObj = NULL;
	cl_err_code clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
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
    if (CL_MEM_READ_ONLY != clMemFlags && CL_MEM_WRITE_ONLY != clMemFlags && CL_MEM_READ_WRITE != clMemFlags)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
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
	clErr = m_mapMemObjects.AddObject(pMemObj, false);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr))
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

	// get program from programs map list
	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErrRet) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
		return CL_INVALID_MEM_OBJECT;
	}

	GLMemoryObject* pGLObject= NULL;
	// Check for GL object
	// Check if it's a GL object
	if ( NULL != (pGLObject = dynamic_cast<GLMemoryObject*>(pMemObj)) )
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

	// get program from programs map list
	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemObj, (OCLObject<_cl_mem_int>**)&pMemObj);
	if (CL_FAILED(clErrRet) || NULL == pMemObj)
	{
		LOG_ERROR(TEXT("Object %d isn't a valid object"), clMemObj);
		return CL_INVALID_MEM_OBJECT;
	}

	GLTexture* pGLObject= NULL;
	// Check for GL object
	// Check if it's a GL object
	if ( NULL != (pGLObject = dynamic_cast<GLTexture*>(pMemObj)) )
	{
		return pGLObject->GetGLTextureInfo(clglPramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}

	return CL_INVALID_GL_OBJECT;
#else
	assert (0 && "NOT Implemented on Linux");
	return CL_INVALID_GL_OBJECT;
#endif
}

cl_err_code ContextModule::CheckMemObjectParameters(cl_mem_flags clMemFlags,
										const cl_image_format * clImageFormat,
                                        cl_mem_object_type clMemObjType,
                                         size_t szImageWidth,
                                         size_t szImageHeight,
                                         size_t szImageDepth,
                                         size_t szImageRowPitch,
                                         size_t szImageSlicePitch,
                                         size_t szArraySize,
                                         void * pHostPtr)
{
    // check for illegal flags
    if ((clMemFlags & ~(
        CL_MEM_READ_WRITE |
        CL_MEM_WRITE_ONLY |
        CL_MEM_READ_ONLY |
        CL_MEM_USE_HOST_PTR |
        CL_MEM_ALLOC_HOST_PTR |
        CL_MEM_COPY_HOST_PTR |
        CL_MEM_HOST_WRITE_ONLY |
        CL_MEM_HOST_READ_ONLY |
        CL_MEM_HOST_NO_ACCESS)) != 0)
    {
        return CL_INVALID_VALUE;
    }
    // check for illegal flag combinations
	if ( ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_WRITE_ONLY)) ||
		((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_READ_WRITE)) ||
		((clMemFlags & CL_MEM_WRITE_ONLY) && (clMemFlags & CL_MEM_READ_WRITE))||
		((clMemFlags & CL_MEM_USE_HOST_PTR) && (clMemFlags & CL_MEM_ALLOC_HOST_PTR)) ||
		((clMemFlags & CL_MEM_HOST_WRITE_ONLY) && (clMemFlags & CL_MEM_HOST_READ_ONLY)) ||
		((clMemFlags & CL_MEM_HOST_WRITE_ONLY) && (clMemFlags & CL_MEM_HOST_NO_ACCESS)) ||
		((clMemFlags & CL_MEM_HOST_READ_ONLY) && (clMemFlags & CL_MEM_HOST_NO_ACCESS))
		)
	{
		return CL_INVALID_VALUE;
	}

	if ( (NULL == pHostPtr) && ((0 != szImageRowPitch) ||(0 != szImageSlicePitch)) )
	{
		return CL_INVALID_IMAGE_DESCRIPTOR;
 	}

	if ( (NULL == pHostPtr) && ((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&clMemFlags) )
	{
		return CL_INVALID_HOST_PTR;
	}

	if (CL_MEM_OBJECT_IMAGE1D_BUFFER != clMemObjType && (NULL != pHostPtr) && !((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&clMemFlags) )
	{
		return CL_INVALID_HOST_PTR;
	}

    if (NULL != clImageFormat)
    {
        size_t pixelBytesCnt = clGetPixelBytesCount(clImageFormat);
        
        if (0 == pixelBytesCnt)
        {
            return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
        }
        // Check minimum row pitch size
        size_t szMinRowPitchSize = szImageWidth * pixelBytesCnt;
        if ( (NULL != pHostPtr) && (0 != szImageRowPitch) && ((szImageRowPitch<szMinRowPitchSize)||(szImageRowPitch % pixelBytesCnt)) )
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        // in 1D image array there is no row pitch, just slice pitch
        const size_t szRealRowPitch = 0 == szImageRowPitch || CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType ? szMinRowPitchSize : szImageRowPitch;
        const size_t szMinSlicePitchSize = CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType ? szRealRowPitch : szRealRowPitch * szImageHeight;
        if ( (NULL != pHostPtr) && (0 != szImageSlicePitch) && ((szImageSlicePitch<szMinSlicePitchSize)||(szImageRowPitch != 0 && szImageSlicePitch % szImageRowPitch)) )
        {
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
    }

	return CL_SUCCESS;
}

#if defined (DX9_MEDIA_SHARING)
cl_mem ContextModule::CreateFromD3D9Resource(cl_context clContext, cl_mem_flags clMemFlags,
                                             D3D9ResourceInfo* const pResourceInfo,
                                             cl_int *pErrcodeRet, cl_mem_object_type clObjType,
                                             cl_uint uiDimCnt, const D3DFORMAT d3dFormat, UINT plane)
{
    Context* pContext = NULL;
    MemoryObject* pMemObj = NULL;

    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, 0, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapContexts.GetOCLObject((_cl_context_int*)clContext, (OCLObject<_cl_context_int>**)&pContext);
    
    if (CL_FAILED(clErr) || NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %S , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    
    D3D9Context* const pD3D9Context = dynamic_cast<D3D9Context*>(pContext);    
    if (NULL == pD3D9Context)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    if (NULL == pResourceInfo->m_pResource)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_DX9_RESOURCE_INTEL;
        }
        return CL_INVALID_HANDLE;
    }
    /* check if context was created against the same Direct3D 9 device from which resource was
        created */
    IDirect3DDevice9* pResourceDevice;
    HRESULT res = pResourceInfo->m_pResource->GetDevice(&pResourceDevice);
    if (D3D_OK != res)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    pResourceDevice->Release();
    // Matt is aware that there is a hole in the spec regarding checking this device type
    if (CL_CONTEXT_DXVA_DEVICE_INTEL != pD3D9Context->m_iDeviceType &&
        pResourceDevice != pD3D9Context->GetD3D9Device())
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_DX9_RESOURCE_INTEL;
        }
        return CL_INVALID_HANDLE;
    }
    // check if just one of the allowed flags is set
    if ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & ~CL_MEM_READ_ONLY) ||
        (clMemFlags & CL_MEM_WRITE_ONLY) && (clMemFlags & ~CL_MEM_WRITE_ONLY) ||
        (clMemFlags & CL_MEM_READ_WRITE) && (clMemFlags & ~CL_MEM_READ_WRITE) ||
        (clMemFlags & ~(CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    clErr = pD3D9Context->CreateD3D9Resource(clMemFlags, pResourceInfo, &pMemObj, clObjType, uiDimCnt, d3dFormat, plane);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pD3D9Context->CreateD3D9Resource(%d, %d, %d, %d) = %S"), clMemFlags, pResourceInfo, &pMemObj, ClErrTxt(clErr));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pMemObj, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr));
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
}

cl_mem ContextModule::CreateFromD3D9Surface(cl_context context, cl_mem_flags flags, IDirect3DSurface9 *resource, HANDLE sharehandle,
                                            UINT plane, cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9Surface(context=%p, flags=%d, resource=%p, errcode_ret=%p)"),
        context, flags, resource, errcode_ret);
    if (NULL == resource)
    {
        LOG_ERROR(TEXT("resource is NULL"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_DX9_RESOURCE_INTEL;
        }
        return CL_INVALID_HANDLE;
    }
    D3D9ResourceInfo* const pResourceInfo = new D3D9SurfaceResourceInfo(resource, sharehandle, plane);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    D3DSURFACE_DESC desc;
    HRESULT res = resource->GetDesc(&desc);
    assert(D3D_OK == res);
    // planar surfaces
    if (MAKEFOURCC('N', 'V', '1', '2') == desc.Format)
    {
        if (plane > 1)
        {
            LOG_ERROR(TEXT("invalid plane for format"));
            if (NULL != errcode_ret)
            {
                *errcode_ret = CL_INVALID_VALUE;
            }
            return CL_INVALID_HANDLE;
        }
        return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, desc.Format, plane);
    }
    if (MAKEFOURCC('Y', 'V', '1', '2') == desc.Format)
    {
        if (plane > 2)
        {
            LOG_ERROR(TEXT("invalid plane for format"));
            if (NULL != errcode_ret)
            {
                *errcode_ret = CL_INVALID_VALUE;
            }
            return CL_INVALID_HANDLE;
        }
        return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, desc.Format, plane);
    }
    // non-planar surface
    if (0 != plane)
    {
        LOG_ERROR(TEXT("invalid plane"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_SURFACE, 2, desc.Format, MAXUINT);
}
#endif

#if defined DX9_SHARING

cl_mem ContextModule::CreateFromD3D9VertexBuffer(cl_context context, cl_mem_flags flags, IDirect3DVertexBuffer9* resource, cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9VertexBuffer(context=%p, flags=%d, resource=%p, errcode_ret=%p)"),
        context, flags, resource, errcode_ret);
    D3D9ResourceInfo* const pResourceInfo = new D3D9ResourceInfo(resource);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_DX9_OBJECT_VERTEX_BUFFER, 1, D3DFMT_UNKNOWN);
}

cl_mem ContextModule::CreateFromD3D9IndexBuffer(cl_context context, cl_mem_flags flags, IDirect3DIndexBuffer9* resource, cl_int* errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9IndexBuffer(context=%p, flags=%d, resource=%p, errcode_ret=%p)"),
        context, flags, resource, errcode_ret);
    D3D9ResourceInfo* const pResourceInfo = new D3D9ResourceInfo(resource);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_D3D9_OBJECT_INDEX_BUFFER, 1, D3DFMT_UNKNOWN);
}

cl_mem ContextModule::CreateFromD3D9Texture(cl_context context, cl_mem_flags flags,
                                              IDirect3DTexture9 *resource, UINT miplevel,
                                              cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9Texture(context=%p, flags=%d, resource=%p, miplevel=%d, errcode_ret=%p)"),
        context, flags, resource, miplevel, errcode_ret);
    D3D9TextureResourceInfo* const pResourceInfo = new D3D9TextureResourceInfo(resource, miplevel);
    if (NULL == pResourceInfo)
    {
        LOG_ERROR(TEXT("could not allocate D3DResourceInfo"));
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    D3DSURFACE_DESC desc;
    HRESULT res = resource->GetLevelDesc(miplevel, &desc);
    assert(D3D_OK == res);
    return CreateFromD3D9Resource(context, flags, pResourceInfo, errcode_ret, CL_D3D9_OBJECT_TEXTURE, 2, desc.Format);
}

cl_mem ContextModule::CreateFromD3D9CubeTexture(cl_context context, cl_mem_flags flags,
                                                  IDirect3DCubeTexture9 *resource,
                                                  D3DCUBEMAP_FACES facetype, UINT miplevel,
                                                  cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9CubeTexture(context=%p, flags=%d, resource=%p, facetype=%d, miplevel=%d, errcode_ret=%p)"),
        context, flags, resource, facetype, miplevel, errcode_ret);
    D3D9CubeTextureResourceInfo* const pCubeTextureResourceInfo =
        new D3D9CubeTextureResourceInfo(resource, miplevel, facetype);
    if (NULL == pCubeTextureResourceInfo)
    {
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    D3DSURFACE_DESC desc;
    const HRESULT res = resource->GetLevelDesc(miplevel, &desc);
    assert(D3D_OK == res);
    return CreateFromD3D9Resource(context, flags, pCubeTextureResourceInfo, errcode_ret, CL_D3D9_OBJECT_CUBE_TEXTURE, 2, desc.Format);
}

cl_mem ContextModule::CreateFromD3D9VolumeTexture(cl_context context, cl_mem_flags flags,
                                        IDirect3DVolumeTexture9 *resource, UINT miplevel,
                                        cl_int *errcode_ret)
{
    LOG_DEBUG(TEXT("Enter CreateFromD3D9VolumeTexture(context=%p, flags=%d, resource=%p, miplevel=%d, errcode_ret=%p)"),
        context, flags, resource, miplevel, errcode_ret);
    D3D9TextureResourceInfo* const pTextureResourceInfo =
        new D3D9TextureResourceInfo(resource, miplevel);
    if (NULL == pTextureResourceInfo)
    {
        if (NULL != errcode_ret)
        {
            *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        }
        return CL_INVALID_HANDLE;
    }
    D3DVOLUME_DESC desc;
    const HRESULT res = resource->GetLevelDesc(miplevel, &desc);
    assert(D3D_OK == res);
    return CreateFromD3D9Resource(context, flags, pTextureResourceInfo, errcode_ret, CL_D3D9_OBJECT_VOLUME_TEXTURE, 3, desc.Format);
}
#endif

/////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelArgInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelArgInfo(cl_kernel clKernel,
										cl_uint argIndx,
										cl_kernel_arg_info paramName,
										size_t      szParamValueSize,
										void *      pParamValue,
										size_t *    pszParamValueSizeRet)
{
	LOG_INFO(TEXT("Enter clKernel=%X, argIndx=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%X, pszParamValueSizeRet=%X"), 
		clKernel, argIndx, paramName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;

	clErr = m_mapKernels.GetOCLObject((_cl_kernel_int*)clKernel, (OCLObject<_cl_kernel_int>**)&pKernel);
	if (CL_FAILED(clErr) || NULL == pKernel)
	{
		LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), clKernel, &pKernel, ClErrTxt(clErr));
		return CL_INVALID_KERNEL;
	}

	return pKernel->GetKernelArgInfo(argIndx, paramName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

cl_mem ContextModule::CreateImage1DBuffer(cl_context context, cl_mem_flags clFlags, const cl_image_format* clImageFormat, size_t szImageWidth, cl_mem buffer, cl_int* pErrcodeRet)
{
    cl_err_code clErr = CL_SUCCESS;
    GenericMemObject* pBuffer;

    clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)buffer, (OCLObject<_cl_mem_int>**)&pBuffer);
    if (CL_FAILED(clErr) || NULL == pBuffer)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %S"), buffer, &pBuffer, ClErrTxt(clErr));
        if (pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }

    const cl_mem_flags bufFlags = pBuffer->GetFlags();
    if (((bufFlags & CL_MEM_WRITE_ONLY) && (clFlags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY))) ||
        ((bufFlags & CL_MEM_READ_ONLY) && (clFlags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY))) ||
        (clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) ||
        ((bufFlags & CL_MEM_HOST_WRITE_ONLY) && (clFlags & CL_MEM_HOST_READ_ONLY)) ||
        ((bufFlags & CL_MEM_HOST_READ_ONLY) && (clFlags & CL_MEM_HOST_WRITE_ONLY)) ||
        ((bufFlags & CL_MEM_HOST_NO_ACCESS) && (clFlags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY))))
    {
        LOG_ERROR(TEXT("invalid flags (%d)"), clFlags);
        if (pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    if (szImageWidth * clGetPixelBytesCount(clImageFormat) > pBuffer->GetSize())
    {
        LOG_ERROR(TEXT("The image_width (%d) * size of element in bytes (%d) must be <= size of buffer object data store (%d)"),
            szImageWidth, clGetPixelBytesCount(clImageFormat), pBuffer->GetSize());
        if (pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }
    clFlags |= pBuffer->GetFlags(); // if some flags are not specified, they are inherited from buffer

    void* const pBufferData = pBuffer->GetBackingStoreData();
    const cl_mem clImgBuf = CreateScalarImage<1, CL_MEM_OBJECT_IMAGE1D_BUFFER>(context, clFlags, clImageFormat, szImageWidth, 0, 0, 0, 0, pBufferData, pErrcodeRet, true);
    if (CL_INVALID_HANDLE == clImgBuf)
    {
        return clImgBuf;
    }

    Image1DBuffer* pImgBuf;
    clErr = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clImgBuf, (OCLObject<_cl_mem_int>**)&pImgBuf);
    assert(CL_SUCCEEDED(clErr));
    pImgBuf->SetBuffer(pBuffer);
    return clImgBuf;
}
