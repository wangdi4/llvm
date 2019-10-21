// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <limits.h>
#include <assert.h>
#include <algorithm>

#include "Context.h"
#include "Device.h"
#include "program_with_source.h"
#include "program_with_binary.h"
#include "program_with_il.h"
#include "program_builtin_kernels.h"
#include "program_for_link.h"
#include "program_service.h"
#include "sampler.h"
#include "cl_sys_defines.h"
#include "context_module.h"
#include "svm_buffer.h"
#include "usm_buffer.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/MemoryObject.h"
#include "ocl_itt.h"
#include "cl_shared_ptr.hpp"
#include <cl_utils.h>
#include <cl_objects_map.h>
#include <cl_local_array.h>
#include <framework_proxy.h>
#include "pipe.h"
#include "cl_user_logger.h"

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

#ifdef __GNUC__
    #define UNUSED_ATTR __attribute__((unused))
#else
    #define UNUSED_ATTR
#endif

// Function to compare two image formats.
static bool compareImageFormats(cl_image_format f1, cl_image_format f2);
// Function to get format map key
static cl_ulong getFormatsKey(cl_mem_object_type clObjType , cl_mem_flags clMemFlags);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>*ppDevices, logging_fn pfnNotify,
                 void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData, ContextModule& contextModule)
    : OCLObject<_cl_context_int>(nullptr, "Context"), // Context doesn't have conext to belong
    m_bTEActivated(false), m_ppExplicitRootDevices(nullptr),
    m_ppAllDevices(nullptr), m_pDeviceIds(nullptr),
    m_pOriginalDeviceIds(nullptr), m_devTypeMask(0),
    m_pclContextProperties(nullptr), m_fpgaEmulator(false), m_eyeqEmulator(false),
    m_pfnNotify(nullptr), m_pUserData(nullptr), m_ulMaxMemAllocSize(0),
    m_MemObjectsHeap(nullptr), m_contextModule(contextModule)
{

    INIT_LOGGER_CLIENT(TEXT("Context"), LL_DEBUG);
    LOG_DEBUG(TEXT("%s"), TEXT("Context constructor enter"));

    m_programService.SetContext(this);
    LOG_INFO(TEXT("%s"), "TaskExecutor->Activate()");
    m_bTEActivated = FrameworkProxy::Instance()->ActivateTaskExecutor();
    if ( !m_bTEActivated )
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }

    m_szArraySize    = 0;
    m_sz1dImgBufSize = 0;

    m_sz2dWidth      = 0;
    m_sz2dHeight     = 0;

    m_sz3dWidth      = 0;
    m_sz3dHeight     = 0;
    m_sz3dDepth      = 0;

    m_bSupportsSvmSystem = false;

    m_bSupportsUsmHost = false;
    m_bSupportsUsmDevice = false;
    m_bSupportsUsmSharedSingle = false;
    m_bSupportsUsmSharedCross = false;
    m_bSupportsUsmSharedSystem = false;

    m_ppAllDevices = nullptr;
    m_ppExplicitRootDevices = nullptr;
    m_pDeviceIds = nullptr;
    m_pOriginalDeviceIds = nullptr;
    m_pGPAData = pGPAData;

    if ((0 != clCreateHeap( 0, 0, &m_MemObjectsHeap )) || (nullptr == m_MemObjectsHeap))
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }

    assert ((nullptr != ppDevices) && (uiNumDevices > 0));
    m_uiNumRootDevices = uiNumRootDevices;

    m_ppAllDevices = new SharedPtr<FissionableDevice>[uiNumDevices];
    if (nullptr == m_ppAllDevices)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }
    m_ppExplicitRootDevices = new SharedPtr<Device>[m_uiNumRootDevices];
    if (nullptr == m_ppExplicitRootDevices)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_ppAllDevices;
        return;
    }

    m_pDeviceIds = new cl_device_id[uiNumDevices];
    if (nullptr == m_pDeviceIds)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_ppAllDevices;
        delete[] m_ppExplicitRootDevices;
        return;
    }
    m_pOriginalDeviceIds = new cl_device_id[uiNumDevices];
    if (nullptr == m_pOriginalDeviceIds)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_pDeviceIds;
        delete[] m_ppAllDevices;
        delete[] m_ppExplicitRootDevices;
        return;
    }

    cl_uint curRoot = 0;
    for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
    {
        if (m_mapDevices.AddObject(ppDevices[ui], false) == CL_ERR_KEY_ALLREADY_EXISTS)
        {
            continue;  // the spec says that duplicate devices specified in devices are ignored
        }
        m_ppAllDevices[ui] = ppDevices[ui];
        m_pDeviceIds[ui] = ppDevices[ui]->GetHandle();
        m_pOriginalDeviceIds[ui] = ppDevices[ui]->GetHandle();

        // Create a set of all root devices implicitly/explicitly defined in the context.
        m_allRootDevices.insert(ppDevices[ui]->GetRootDevice());

        // Create a list of all explicit root devices in the context.
        if (ppDevices[ui]->IsRootLevelDevice())
        {
            assert(curRoot < m_uiNumRootDevices);
            // GetRootDevice used just for the purpose of casting to SharedPtr<Device>.
            m_ppExplicitRootDevices[curRoot++] = ppDevices[ui]->GetRootDevice();
        }

        cl_bitfield devType = ppDevices[ui]->GetRootDevice()->GetDeviceType();
        m_devTypeMask |= devType;
    }
    uiNumDevices = m_mapDevices.Count();    // handle the case of duplicate devices
    m_pOriginalNumDevices = uiNumDevices;

    m_uiContextPropCount = 0;
    m_pclContextProperties = nullptr;
    if (nullptr == clProperties)
    {
        m_pclContextProperties = new cl_context_properties[1];
        if (nullptr != m_pclContextProperties)
        {
            m_pclContextProperties[0] = 0;
        }
    }
    else
    {
        // count the number of properties;
        while (0 != clProperties[m_uiContextPropCount])
        {
//            m_mapPropertyMap[clProperties[m_uiContextPropCount]] = clProperties[m_uiContextPropCount+1];
            m_uiContextPropCount+=2;
        }
        m_uiContextPropCount++; // last property = NULL;
        // allocate new buffer for context's properties
        m_pclContextProperties = new cl_context_properties[m_uiContextPropCount];
        if (nullptr != m_pclContextProperties)
        {
            MEMCPY_S(m_pclContextProperties, m_uiContextPropCount * sizeof(cl_context_properties), clProperties, m_uiContextPropCount * sizeof(cl_context_properties));
        }
    }

    const OCLConfig* pOclConfig = FrameworkProxy::Instance()->GetOCLConfig();
    if (FPGA_EMU_DEVICE == pOclConfig->GetDeviceMode())
    {
        m_fpgaEmulator = true;
    }
    if (EYEQ_EMU_DEVICE == pOclConfig->GetDeviceMode())
    {
        m_eyeqEmulator = true;
    }

    m_pfnNotify = pfnNotify;
    m_pUserData = pUserData;

    //
    // For each device in this context, "create" it (either really create or increment its ref count)
    //
    cl_err_code ret = CL_SUCCESS;
    cl_uint idx = 0;
    for(; idx < uiNumDevices && CL_SUCCEEDED(ret); idx++)
    {
        // Need to make insure that instance of DeviceAgent exists
        if (ppDevices[idx]->IsRootLevelDevice())
        {
            ret = ppDevices[idx]->GetRootDevice()->CreateInstance();
        }
    }
    if ( CL_FAILED(ret) )
    {
        for(cl_uint ui=0; ui<(idx-1); ++ui)
        {
            m_mapDevices.RemoveObject(ppDevices[ui]->GetHandle());
            if (m_ppAllDevices[ui]->IsRootLevelDevice())
            {
                m_ppAllDevices[ui]->GetRootDevice()->CloseDeviceInstance();
            }
        }
        *pclErr = ret;
        m_mapDevices.ReleaseAllObjects(false);
        return;
    }
    GetMaxImageDimensions(m_sz2dWidth, m_sz2dHeight, m_sz3dWidth, m_sz3dHeight,
        m_sz3dDepth, m_szArraySize, m_sz1dImgBufSize);

    // calculate m_bSupportsSvmSystem
    const tSetOfDevices* pDevices = GetAllRootDevices();
    for (tSetOfDevices::const_iterator iter = pDevices->begin(); iter != pDevices->end(); iter++)
    {
        cl_device_svm_capabilities svmCaps;
        const cl_err_code err = (*iter)->GetInfo(CL_DEVICE_SVM_CAPABILITIES, sizeof(svmCaps), &svmCaps, nullptr);
        if (CL_SUCCEEDED(err) && (svmCaps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM))
        {
            m_bSupportsSvmSystem = true;
            break;
        }
    }

    // unified shared memory
    for (tSetOfDevices::const_iterator iter = pDevices->begin();
         iter != pDevices->end(); iter++)
    {
        cl_unified_shared_memory_capabilities_intel usmCaps;

        cl_err_code err = (*iter)->GetInfo(
            CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL, sizeof(usmCaps), &usmCaps,
            nullptr);
        if (CL_SUCCEEDED(err) && (usmCaps &
                                  CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL))
            m_bSupportsUsmHost = true;

        err = (*iter)->GetInfo(CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL,
                               sizeof(usmCaps), &usmCaps, nullptr);
        if (CL_SUCCEEDED(err) && (usmCaps &
                                  CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL))
            m_bSupportsUsmDevice = true;

        err = (*iter)->GetInfo(
            CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
            sizeof(usmCaps), &usmCaps, nullptr);
        if (CL_SUCCEEDED(err) && (usmCaps &
                                  CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL))
            m_bSupportsUsmSharedSingle = true;

        err = (*iter)->GetInfo(
            CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
            sizeof(usmCaps), &usmCaps, nullptr);
        if (CL_SUCCEEDED(err) && (usmCaps &
                                  CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL))
            m_bSupportsUsmSharedCross = true;

        err = (*iter)->GetInfo(
            CL_DEVICE_SHARED_SYSTEM_MEM_CAPABILITIES_INTEL,
            sizeof(usmCaps), &usmCaps, nullptr);
        if (CL_SUCCEEDED(err) && (usmCaps &
                                  CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL))
            m_bSupportsUsmSharedSystem = true;
    }

    *((ocl_entry_points*)(&m_handle)) = *pOclEntryPoints;

    *pclErr = CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
void Context::Cleanup( bool bTerminate )
{
    if (bTerminate || m_contextModule.IsTerminating())
    {
        // If terminate, do nothing since devices are off already
        return;
    }

    // Close all device instances. Device will decide whether to close or just decrease ref count
    cl_uint uiNumDevices = m_mapDevices.Count();
    for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
    {
        m_mapDevices.RemoveObject(m_ppAllDevices[ui]->GetHandle());
        // The pendency to the device implicitly removed by RemoveObject()
        if (m_ppAllDevices[ui]->IsRootLevelDevice())
        {
            m_ppAllDevices[ui]->GetRootDevice()->CloseDeviceInstance();
        }
    }
    if ( m_bTEActivated )
    {
        LOG_INFO(TEXT("%s"), TEXT("GetTaskExecutor()->Deactivate();"));
        FrameworkProxy::Instance()->DeactivateTaskExecutor();
        m_bTEActivated = false;
    }
    delete this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
    LOG_INFO(TEXT("%s"), TEXT("Context destructor enter"));
    LOG_DEBUG(TEXT("CONTEXT_TEST: Context destructor enter. (id = %d)"), m_iId);

    //
    // Free private resources
    //

    if (nullptr != m_MemObjectsHeap)
    {
        clDeleteHeap( m_MemObjectsHeap );
    }

    RELEASE_LOGGER_CLIENT;

    m_mapPrograms.Clear();

    m_mapDevices.Clear();
 
    if (nullptr != m_ppAllDevices)
    {
        delete[] m_ppAllDevices;
        m_ppAllDevices = nullptr;
    }
    if (nullptr != m_ppExplicitRootDevices)
    {
        delete[] m_ppExplicitRootDevices;
        m_ppExplicitRootDevices = nullptr;
    }
    if (nullptr != m_pDeviceIds)
    {
        delete[] m_pDeviceIds;
        m_pDeviceIds = nullptr;
    }
    if (nullptr != m_pOriginalDeviceIds)
    {
        delete[] m_pOriginalDeviceIds;
        m_pOriginalDeviceIds = nullptr;
    }
    m_mapMemObjects.Clear();
    m_mapSamplers.Clear();

    if (nullptr != m_pclContextProperties)
    {
        delete []m_pclContextProperties;
        m_pclContextProperties = nullptr;
    }

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
    while ( !m_OsEventPool.IsEmpty() )
    {
        delete m_OsEventPool.PopFront();
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) const
{
    LOG_DEBUG(TEXT("Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), param_name, param_value_size, param_value, param_value_size_ret);

    size_t szParamValueSize = 0;
    cl_uint uiVal;
    const void * pValue = nullptr;

    cl_err_code clErrRet = CL_SUCCESS;
    switch ( (cl_context_info)param_name )
    {

    case CL_CONTEXT_REFERENCE_COUNT:
        szParamValueSize = sizeof(cl_uint);
        pValue = &m_uiRefCount;
        break;
    case CL_CONTEXT_DEVICES:
        szParamValueSize = sizeof(cl_device_id) * m_pOriginalNumDevices;
        pValue = m_pOriginalDeviceIds;
        break;
    case CL_CONTEXT_PROPERTIES:
        szParamValueSize = sizeof(cl_context_properties) * (m_uiContextPropCount);
        pValue = m_pclContextProperties;
        break;
    case CL_CONTEXT_NUM_DEVICES:
        szParamValueSize = sizeof(cl_uint);
        uiVal = (cl_uint)m_mapDevices.Count();
        pValue = &uiVal;
        break;

    default:
        LOG_ERROR(TEXT("param_name (=%d) isn't valid"), param_name);
        return CL_INVALID_VALUE;
    }
    if (CL_FAILED(clErrRet))
    {
        return clErrRet;
    }

    // if param_value_size < actual value size return CL_INVALID_VALUE
    if (nullptr != param_value && param_value_size < szParamValueSize)
    {
        LOG_ERROR(TEXT("param_value_size (=%d) < szParamValueSize (=%d)"), param_value_size, szParamValueSize);
        return CL_INVALID_VALUE;
    }

    // return param value size
    if (nullptr != param_value_size_ret)
    {
        *param_value_size_ret = szParamValueSize;
    }

    if (nullptr != param_value && szParamValueSize > 0)
    {
        MEMCPY_S(param_value, param_value_size, pValue, szParamValueSize);
    }

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithIL
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithIL(const unsigned char* pIL, const size_t length, SharedPtr<Program>* ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramWithSource enter. pIL=%d, length=%d, ppProgram=%d"), pIL, length, ppProgram);

    // check input parameters
    if (nullptr == ppProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
        return CL_INVALID_VALUE;
    }
    cl_err_code clErrRet = CL_SUCCESS;
    // create new program object
    SharedPtr<Program> pProgram = ProgramWithIL::Allocate(this, pIL, length, &clErrRet);
    if (NULL == pProgram)
    {
        if (CL_SUCCESS != clErrRet)
        {
            return clErrRet;
        }
        return CL_OUT_OF_HOST_MEMORY;
    }
    pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

    // add program object to programs map list
    m_mapPrograms.AddObject(pProgram);
    *ppProgram = pProgram;
    return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithSource(cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, SharedPtr<Program>* ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramWithSource enter. uiCount=%d, ppcStrings=%d, szLengths=%d, ppProgram=%d"), uiCount, ppcStrings, szLengths, ppProgram);

    // check input parameters
    if (nullptr == ppProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
        return CL_INVALID_VALUE;
    }
    cl_err_code clErrRet = CL_SUCCESS;
    // create new program object
    SharedPtr<Program> pProgram = ProgramWithSource::Allocate(this, uiCount,ppcStrings, szLengths, &clErrRet);
    if (NULL == pProgram)
    {
        if (CL_SUCCESS != clErrRet)
        {
            return clErrRet;
        }
        return CL_OUT_OF_HOST_MEMORY;
    }
    pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

    // add program object to programs map list
    m_mapPrograms.AddObject(pProgram);
    *ppProgram = pProgram;
    return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramForLink
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramForLink(cl_uint                IN  uiNumDevices,
                                          const cl_device_id *    IN  pclDeviceList,
                                          SharedPtr<Program>*    OUT ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramFromLink enter. uiNumDevices=%d, pclDeviceList=%d, ppProgram=%d"), uiNumDevices, pclDeviceList, ppProgram);

    cl_err_code clErrRet = CL_SUCCESS;

    // check input parameters
    if (nullptr == ppProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
        return CL_INVALID_VALUE;
    }

    if (nullptr == pclDeviceList || 0 == uiNumDevices)
    {
        // invalid input args
        LOG_ERROR(TEXT("%s"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices"));
        return CL_INVALID_VALUE;
    }

    // get devices
    SharedPtr<FissionableDevice>* ppDevices = new SharedPtr<FissionableDevice>[uiNumDevices];
    if (nullptr == ppDevices)
    {
        // can't allocate memory for devices
        LOG_ERROR(TEXT("%s"), TEXT("Can't allocated memory for devices"));
        return CL_OUT_OF_HOST_MEMORY;
    }

    // check devices
    bool bRes = GetDevicesFromList(uiNumDevices, pclDeviceList, ppDevices);
    if (false == bRes)
    {
        LOG_ERROR(TEXT("%s"), TEXT("GetDevicesFromList(uiNumDevices, pclDeviceList) = false"));
        delete[] ppDevices;
        return CL_INVALID_DEVICE;
    }

    // create program object
    SharedPtr<Program> pProgram = ProgramForLink::Allocate(this, uiNumDevices, ppDevices, &clErrRet);
    delete[] ppDevices;

    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("Out of memory for creating program"));
        return CL_OUT_OF_HOST_MEMORY;
    }
    pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

    m_mapPrograms.AddObject(pProgram);
    *ppProgram = pProgram;
    return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CompileProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CompileProgram(cl_program    IN  clProgram,
                           cl_uint                IN  uiNumDevices,
                           const cl_device_id*    IN  pclDeviceList,
                           cl_uint                IN  uiNumHeaders,
                           const cl_program*    IN  pclHeaders,
                           const char**         IN  pszHeadersNames, 
                           const char*          IN  szOptions, 
                           pfnNotifyBuildDone   IN  pfn_notify,
                           void*                IN  user_data)
{
    SharedPtr<Program> pProg =m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProg)
    {
        LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }

    SharedPtr<Program>* ppHeaders = nullptr;

    if (0 < uiNumHeaders)
    {
        // This array will be freed by the program service
        ppHeaders = new SharedPtr<Program>[uiNumHeaders];
        if (nullptr == ppHeaders)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        for (unsigned int i = 0; i < uiNumHeaders; ++i)
        {
            ppHeaders[i] = m_mapPrograms.GetOCLObject((_cl_program_int*)pclHeaders[i]).DynamicCast<Program>();
            if (NULL == ppHeaders[i])
            {
                delete[] ppHeaders;
                LOG_ERROR(TEXT("One of the header programs %d isn't valid program"), clProgram);
                return CL_INVALID_PROGRAM;
            }
        }
    }

    cl_int clErr = m_programService.CompileProgram(pProg, uiNumDevices, pclDeviceList, 
                       uiNumHeaders, ppHeaders, pszHeadersNames, szOptions, pfn_notify, user_data);

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::LinkProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::LinkProgram(cl_program                IN  clProgram,
                                cl_uint                    IN  uiNumDevices,
                                const cl_device_id*        IN  pclDeviceList,
                                cl_uint                    IN  uiNumBinaries,
                                const cl_program*        IN  pclBinaries,
                                const char*             IN  szOptions, 
                                pfnNotifyBuildDone      IN  pfn_notify,
                                void*                   IN  user_data)
{
    SharedPtr<Program> pProg = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProg)
    {
        LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }

    SharedPtr<Program>* ppBinaries = nullptr;

    if (0 < uiNumBinaries)
    {
        // This array will be freed by the program service
        ppBinaries = new SharedPtr<Program>[uiNumBinaries];
        if (nullptr == ppBinaries)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        for (unsigned int i = 0; i < uiNumBinaries; ++i)
        {
            ppBinaries[i] = m_mapPrograms.GetOCLObject((_cl_program_int*)pclBinaries[i]).DynamicCast<Program>();
            if (NULL== ppBinaries[i])
            {
                delete[] ppBinaries;
                LOG_ERROR(TEXT("One of the binaries programs %d isn't valid program"), clProgram);
                return CL_INVALID_PROGRAM;
            }
        }
    }

    cl_int clErr = m_programService.LinkProgram(pProg, uiNumDevices, pclDeviceList,
                        uiNumBinaries, ppBinaries, szOptions, pfn_notify, user_data);

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::BuildProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::BuildProgram(cl_program            IN  clProgram,
                                  cl_uint                IN  uiNumDevices,
                                  const cl_device_id*   IN  pclDeviceList,
                                  const char*           IN  szOptions, 
                                  pfnNotifyBuildDone    IN  pfn_notify,
                                  void*                 IN  user_data)
{
    SharedPtr<Program> pProg = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram).DynamicCast<Program>();
    if (NULL == pProg)
    {
        LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
        return CL_INVALID_PROGRAM;
    }

    cl_int clErr = m_programService.BuildProgram(pProg, uiNumDevices, pclDeviceList, szOptions, pfn_notify, user_data);
    
    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetDeviceByIndex
///////////////////////////////////////////////////////////////////////////////////////////////////
SharedPtr<FissionableDevice> Context::GetDeviceByIndex(cl_uint uiDeviceIndex)
{
    return m_mapDevices.GetObjectByIndex((cl_int)uiDeviceIndex).DynamicCast<FissionableDevice>();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CheckDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
    LOG_DEBUG(TEXT("CheckDevices enter. uiNumDevices=%d, pclDevices=%d"), uiNumDevices, pclDevices);
    if (0 == uiNumDevices || nullptr == pclDevices)
    {
        // invalid inputs
        LOG_ERROR(TEXT("%s"), TEXT("0 == uiNumDevices || NULL == pclDevices"));
        return false;
    }
    for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
    {
        SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int*)pclDevices[ui]).DynamicCast<FissionableDevice>();
        if (NULL == pDevice)
        {
            LOG_ERROR(TEXT("device %d wasn't found in this context"), pclDevices[ui]);
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetDevicesFromList
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::GetDevicesFromList(cl_uint uiNumDevices, const cl_device_id * pclDevices, SharedPtr<FissionableDevice>* ppDevices)
{
    LOG_DEBUG(TEXT("GetDeviceFromList enter. uiNumDevices=%d, pclDevices=%d"), uiNumDevices, pclDevices);
    if (0 == uiNumDevices || nullptr == pclDevices)
    {
        // invalid inputs
        LOG_ERROR(TEXT("%s"), TEXT("0 == uiNumDevices || NULL == pclDevices"));
        return false;
    }

    for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
    {
        ppDevices[ui] = m_mapDevices.GetOCLObject((_cl_device_id_int*)pclDevices[ui]).DynamicCast<FissionableDevice>();
        if (NULL == ppDevices[ui])
        {
            LOG_ERROR(TEXT("device %d wasn't found in this context"), pclDevices[ui]);
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetGPAData
///////////////////////////////////////////////////////////////////////////////////////////////////
ocl_gpa_data * Context::GetGPAData() const
{
    return m_pGPAData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithBinary(cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, SharedPtr<Program>* ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramWithBinary enter. uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d, ppProgram=%d"),
        uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, ppProgram);

    cl_err_code clErrRet = CL_SUCCESS;

    if (nullptr == pclDeviceList || 0 == uiNumDevices || nullptr == pszLengths || nullptr == ppBinaries)
    {
        // invalid input args
        LOG_ERROR(TEXT("%s"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries"));
        return CL_INVALID_VALUE;
    }
    // check items in pszLengths and in ppBinaries
    for (cl_uint ui=0; ui<uiNumDevices; ++ui)
    {
        if (0 == pszLengths[ui] || nullptr == ppBinaries[ui])
        {
            LOG_ERROR(TEXT("0 == pszLengths[%d] || NULL == ppBinaries[%d]"), ui, ui);
            if (nullptr != piBinaryStatus)
            {
                piBinaryStatus[ui] = CL_INVALID_VALUE;
            }
            return CL_INVALID_VALUE;
        }
    }

    // get devices
    SharedPtr<FissionableDevice>* ppDevices = new SharedPtr<FissionableDevice>[uiNumDevices];
    if (nullptr == ppDevices)
    {
        // can't allocate memory for devices
        LOG_ERROR(TEXT("%s"), TEXT("Can't allocated memory for devices"));
        return CL_OUT_OF_HOST_MEMORY;
    }

    // check devices
    bool bRes = GetDevicesFromList(uiNumDevices, pclDeviceList, ppDevices);
    if (false == bRes)
    {
        LOG_ERROR(TEXT("%s"), TEXT("GetDevicesFromList(uiNumDevices, pclDeviceList) = false"));
        delete[] ppDevices;
        return CL_INVALID_DEVICE;
    }

    // create program object
    SharedPtr<Program> pProgram = ProgramWithBinary::Allocate(this, uiNumDevices, ppDevices, pszLengths, ppBinaries, piBinaryStatus, &clErrRet);
    delete[] ppDevices;

    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("%s"), TEXT("Out of memory for creating program"));
        return CL_OUT_OF_HOST_MEMORY;
    }
    pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

    m_mapPrograms.AddObject(pProgram);
    *ppProgram = pProgram;
    return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithBuiltInKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithBuiltInKernels(cl_uint IN uiNumDevices,
                                                    const cl_device_id * IN pclDeviceList,
                                                    const char IN *szKernelNames,
                                                    SharedPtr<Program>* OUT ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramWithBuiltInKernels enter. uiNumDevices=%d, pclDeviceList=%d, ppProgram=%p"),
        uiNumDevices, pclDeviceList, ppProgram);

    cl_err_code clErrRet = CL_SUCCESS;

    if (nullptr == pclDeviceList || 0 == uiNumDevices || nullptr == szKernelNames || nullptr == ppProgram)
    {
        // invalid input args
        LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == szKernelNames || NULL == ppProgram"));
        return CL_INVALID_VALUE;
    }

    // get devices
    SharedPtr<FissionableDevice>* ppDevices = new SharedPtr<FissionableDevice>[uiNumDevices];
    if (nullptr == ppDevices)
    {
        // can't allocate memory for devices
        LOG_ERROR(TEXT("%S"), TEXT("Can't allocated memory for devices"));
        return CL_OUT_OF_HOST_MEMORY;
    }

    // check devices
    bool bRes = GetDevicesFromList(uiNumDevices, pclDeviceList, ppDevices);
    if (false == bRes)
    {
        LOG_ERROR(TEXT("%S"), TEXT("GetDevicesFromList(uiNumDevices, pclDeviceList) = false"));
        delete[] ppDevices;
        return CL_INVALID_DEVICE;
    }

    // create program object
    SharedPtr<Program> pProgram = ProgramWithBuiltInKernels::Allocate(this, uiNumDevices, ppDevices, szKernelNames, &clErrRet);
    delete[] ppDevices;

    if (NULL == pProgram)
    {
        LOG_ERROR(TEXT("%S"), TEXT("Out of memory for creating program"));
        return CL_OUT_OF_HOST_MEMORY;
    }
    pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

    m_mapPrograms.AddObject(pProgram);
    *ppProgram = pProgram;
    return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveProgram(cl_program clProgramId)
{
    LOG_DEBUG(TEXT("Enter RemoveProgram (clProgramId=%d)"), clProgramId);

    return m_mapPrograms.RemoveObject((_cl_program_int*)clProgramId);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveMemObject(cl_mem clMem)
{
    LOG_DEBUG(TEXT("Enter RemoveMemObject (clMem=%d)"), clMem);

    return m_mapMemObjects.RemoveObject((_cl_mem_int*)clMem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveSampler(cl_sampler clSampler)
{
    LOG_DEBUG(TEXT("Enter RemoveSampler (clSampler=%d)"), clSampler);

    return m_mapSamplers.RemoveObject((_cl_sampler_int*)clSampler);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, SharedPtr<MemoryObject>* ppBuffer)
{
    LOG_DEBUG(TEXT("Enter CreateBuffer (cl_mem_flags=%llu, szSize=%u, pHostPtr=%d, ppBuffer=%d)"),
        (unsigned long long) clFlags, szSize, pHostPtr, ppBuffer);

    assert ( nullptr != ppBuffer );

    cl_ulong ulMaxMemAllocSize = GetMaxMemAllocSize();
    LOG_DEBUG(TEXT("GetMaxMemAllocSize() = %d"), ulMaxMemAllocSize);

    // check buffer size
    if (szSize == 0 || szSize > ulMaxMemAllocSize)
    {
        LOG_ERROR(TEXT("szSize == %d, ulMaxMemAllocSize =%d"), szSize, ulMaxMemAllocSize);
        return CL_INVALID_BUFFER_SIZE;
    }

    cl_err_code clErr;
    clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_MEM_OBJECT_BUFFER, CL_MEMOBJ_GFX_SHARE_NONE, this, ppBuffer);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new buffer, returned: %s"), ClErrTxt(clErr));
        return clErr;
    }

    clErr = (*ppBuffer)->Initialize(clFlags, nullptr, 1, &szSize, nullptr, pHostPtr, 0);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error Initialize new buffer, returned: %s"), ClErrTxt(clErr));
        (*ppBuffer)->Release();
        return clErr;
    }
    m_mapMemObjects.AddObject(*ppBuffer);

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateSubBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateSubBuffer(SharedPtr<MemoryObject> pBuffer, cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
                                     const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    LOG_DEBUG(TEXT("Enter CreateSubBuffer (cl_mem_flags=%d, buffer_create_type=%d, ppBuffer=%d)"),
        (unsigned long long) clFlags, buffer_create_type, ppBuffer);

    assert ( nullptr != ppBuffer );


    // Parameters check
    if ( CL_BUFFER_CREATE_TYPE_REGION != buffer_create_type )
    {
        return CL_INVALID_VALUE;
    }

    if ( nullptr == buffer_create_info )
    {
        return CL_INVALID_VALUE;
    }
    const cl_buffer_region* region = reinterpret_cast<const cl_buffer_region*>(buffer_create_info);
    assert(region);

    if (region->size == 0 )
    {
        return CL_INVALID_BUFFER_SIZE;
    }

    if ( (region->origin + region->size) > pBuffer->GetSize()  )
    {
        return CL_INVALID_VALUE;
    }

    if (CL_SUCCESS != pBuffer->ValidateChildFlags(clFlags))
    {
        return CL_INVALID_VALUE;
    }

    // Copy access flags from parent, if none supplied.
    cl_mem_flags pflags = pBuffer->GetFlags();
    if ( !(clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
    {
        clFlags |= (pflags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE));
    }
    if ( !(clFlags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY)) )
    {
        clFlags |= (pflags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY));
    }
    // These flags cannot be specified in flags but are inherited from the corresponding memory access qualifiers associated with buffer.
    clFlags |= pflags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR);


    cl_err_code clErr;
    clErr = pBuffer->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error initializing sub buffer, returned: %s"), ClErrTxt(clErr));
        return clErr;
    }


    m_mapMemObjects.AddObject(*ppBuffer);

    return clErr;
}

// Context::clCreateImageArray
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateImageArray(cl_mem_flags clFlags, const cl_image_format* pclImageFormat, void* pHostPtr, const cl_image_desc* pClImageDesc,
                                      SharedPtr<MemoryObject>* ppImageArr)
{
    assert(nullptr != ppImageArr);
    assert(CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type || CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type);
    
    if (pClImageDesc->image_array_size < 1 || pClImageDesc->image_array_size > m_szArraySize || pClImageDesc->image_width < 1 || pClImageDesc->image_width > m_sz2dWidth ||
        (CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type && (pClImageDesc->image_height < 1 || pClImageDesc->image_height > m_sz2dHeight)))
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }

    const size_t pixelBytesCnt = clGetPixelBytesCount(pclImageFormat);
    // handle the mess in the spec, where for 1D image array the slice pitch defines the size in bytes of each 1D image
    const size_t szPitchDim1 =
        CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type ?
        /// array is 1D image array
            (0 == pClImageDesc->image_slice_pitch ?
            (0 == pClImageDesc->image_row_pitch ?
             pClImageDesc->image_width * pixelBytesCnt : pClImageDesc->image_row_pitch) : pClImageDesc->image_slice_pitch)
        /// array is not 1D image array
            : (0 == pClImageDesc->image_row_pitch ? pClImageDesc->image_width * pixelBytesCnt : pClImageDesc->image_row_pitch);

    const size_t szPitchDim2 =
        CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type ?
        0 :
        0 == pClImageDesc->image_slice_pitch ? szPitchDim1 * pClImageDesc->image_height : pClImageDesc->image_slice_pitch;

    cl_err_code clErr = CheckSupportedImageFormatByMemFlags(clFlags, *pclImageFormat, pClImageDesc->image_type);
    if (CL_FAILED(clErr))
    {
        return clErr;
    }

    // flags and imageFormat are validated by Image2D and MemoryObject contained inside Image2DArray and hostPtr by MemoryObject::initialize.
    clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, pClImageDesc->image_type, CL_MEMOBJ_GFX_SHARE_NONE, this, ppImageArr);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new Image3D, returned: %s"), ClErrTxt(clErr));
        return clErr;
    }

    size_t dim[3] = {pClImageDesc->image_width};
    const size_t pitch[2] = {szPitchDim1, szPitchDim2};

    if (CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type)
    {
        dim[1] = pClImageDesc->image_array_size;
        clErr = (*ppImageArr)->Initialize(clFlags, pclImageFormat, 2, dim, pitch, pHostPtr, 0);
    }
    else
    {
        dim[1] = pClImageDesc->image_height;
        dim[2] = pClImageDesc->image_array_size;
        clErr = (*ppImageArr)->Initialize(clFlags, pclImageFormat, 3, dim, pitch, pHostPtr, 0);
    }
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error Initialize new buffer, returned: %s"), ClErrTxt(clErr));
        (*ppImageArr)->Release();
        return clErr;
    }
    m_mapMemObjects.AddObject(*ppImageArr);
    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetSupportedImageFormats
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetSupportedImageFormats(cl_mem_flags clFlags,
                                              cl_mem_object_type clType,
                                              cl_uint uiNumEntries,
                                              cl_image_format * pclImageFormats,
                                              cl_uint * puiNumImageFormats)
{
    LOG_DEBUG(TEXT("Enter GetSupportedImageFormats(clFlags=%d, clType=%d, uiNumEntries=%d, pclImageFormats=%d, puiNumImageFormats=%d"),
        clFlags, clType, uiNumEntries, pclImageFormats, puiNumImageFormats);

    if ( (uiNumEntries == 0 && pclImageFormats != nullptr) )
    {
        LOG_ERROR(TEXT("%s"), TEXT("uiNumEntries == 0 && pclImageFormats != NULL"));
        return CL_INVALID_VALUE;
    }

	if (CL_MEM_OBJECT_BUFFER == clType || CL_MEM_OBJECT_PIPE == clType)
    {
        LOG_ERROR(TEXT("%s"), TEXT("clType != CL_MEM_OBJECT_IMAGE2D && clType != CL_MEM_OBJECT_IMAGE3D"));
        return CL_INVALID_VALUE;
    }

    if ( ((clFlags & CL_MEM_READ_ONLY) && (clFlags & CL_MEM_WRITE_ONLY)) ||
         ((clFlags & CL_MEM_READ_ONLY) && (clFlags & CL_MEM_READ_WRITE)) ||
         ((clFlags & CL_MEM_WRITE_ONLY) && (clFlags & CL_MEM_READ_WRITE)))
    {
        return CL_INVALID_VALUE;
    }

    if (  !(clFlags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_KERNEL_READ_AND_WRITE)) )
    {
        if ( !(clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) )
        {
            return CL_INVALID_VALUE;
        }
        // default value;
        clFlags = clFlags | CL_MEM_READ_WRITE;
    }

    // get supported image types from all devices
    // TODO: prepare the minimum overlapping list from all devices
    // Need to iterate only over root devices
    assert(m_mapDevices.Count() == 1);

    cl_err_code clErr = CL_SUCCESS;
    for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
    {
        clErr = m_ppAllDevices[ui]->GetDeviceAgent()->clDevGetSupportedImageFormats(clFlags, clType, uiNumEntries, pclImageFormats, puiNumImageFormats);
        if (CL_FAILED(clErr))
        {
            return clErr;
        }
    }

    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxMemAllocSize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_ulong Context::GetMaxMemAllocSize()
{
    if ( 0 != m_ulMaxMemAllocSize )
    {
        return m_ulMaxMemAllocSize;
    }

    LOG_DEBUG(TEXT("%s"), TEXT("Enter GetMaxMemAllocSize"));

    cl_ulong ulGlobalMemSize = 0;

    for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
    {
        cl_err_code clErr = m_ppAllDevices[ui]->GetInfo(CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &ulGlobalMemSize, nullptr);
        if (CL_FAILED(clErr))
        {
            continue;
        }
        m_ulMaxMemAllocSize = (0 == m_ulMaxMemAllocSize) ? ulGlobalMemSize :
                                    (ulGlobalMemSize < m_ulMaxMemAllocSize) ? ulGlobalMemSize : m_ulMaxMemAllocSize;
    }

    return m_ulMaxMemAllocSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxImageDimensions
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetMaxImageDimensions(size_t &psz2dWidth,
                                           size_t &psz2dHeight,
                                           size_t &psz3dWidth,
                                           size_t &psz3dHeight,
                                           size_t &psz3dDepth,
                                           size_t &pszArraySize,
                                           size_t &psz1dImgBufSize)
{
    LOG_DEBUG(TEXT("%s"), TEXT("Enter GetMaxAllowedImageWidth"));

    size_t sz2dWith = 0, sz2dHeight = 0, szMax2dWith = 0, szMax2dHeight = 0;
    size_t sz3dWith = 0, sz3dHeight = 0, szMax3dWith = 0, szMax3dHeight = 0, sz3dDepth = 0, szMax3dDepth = 0;
    size_t szArraySize = 0, szMaxArraySize = 0;
    size_t sz1dImgBufSize = 0, szMax1dImgBufSize = 0;
    cl_err_code clErr = CL_SUCCESS;

    for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
    {
        SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetObjectByIndex(ui).DynamicCast<FissionableDevice>();
        if (NULL == pDevice)
        {
            continue;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sz2dWith, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax2dWith = ((0 == ui) || (sz2dWith < szMax2dWith)) ? sz2dWith : szMax2dWith;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sz2dHeight, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax2dHeight = ((0 == ui) || (sz2dHeight < szMax2dHeight)) ? sz2dHeight : szMax2dHeight;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sz3dWith, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax3dWith = ((0 == ui) || (sz3dWith < szMax3dWith)) ? sz3dWith : szMax3dWith;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sz3dHeight, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax3dHeight = ((0 == ui) || (sz3dHeight < szMax3dHeight)) ? sz3dHeight : szMax3dHeight;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sz3dDepth, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax3dDepth = ((0 == ui) || (sz3dDepth < szMax3dDepth)) ? sz3dDepth : szMax3dDepth;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(size_t), &szArraySize, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMaxArraySize = ((0 == ui) || (szArraySize < szMaxArraySize)) ? szArraySize : szMaxArraySize;
        }
        clErr = pDevice->GetInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(size_t), &sz1dImgBufSize, nullptr);
        if (CL_SUCCEEDED(clErr))
        {
            szMax1dImgBufSize = 0 == ui || sz1dImgBufSize < szMax1dImgBufSize ? sz1dImgBufSize : szMax1dImgBufSize;
        }
    }

    psz2dWidth = szMax2dWith;
    psz2dHeight = szMax2dHeight;
    psz3dWidth = szMax3dWith;
    psz3dHeight = szMax3dHeight;
    psz3dDepth = szMax3dDepth;
    pszArraySize = szMaxArraySize;
    psz1dImgBufSize = szMax1dImgBufSize;
    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::NotifyError
///////////////////////////////////////////////////////////////////////////////////////////////////
void Context::NotifyError(const char * pcErrInfo, const void * pPrivateInfo, size_t szCb)
{
    if (nullptr != m_pfnNotify)
    {        
        if (nullptr != g_pUserLogger && g_pUserLogger->IsApiLoggingEnabled())
        {
            std::stringstream stream;
            stream << "clCreateContext callback(" << pcErrInfo << ", " << pPrivateInfo << ", " << szCb << ")" << std::endl;
            g_pUserLogger->PrintString(stream.str());
        }
        m_pfnNotify(pcErrInfo, pPrivateInfo, szCb, m_pUserData);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateSampler(cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, SharedPtr<Sampler>* ppSampler)
{
    assert ( nullptr != ppSampler );
    LOG_DEBUG(TEXT("Enter CreateSampler (bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, ppSampler=%d)"),
        bNormalizedCoords, clAddressingMode, clFilterMode, ppSampler);

#ifdef _DEBUG
    assert ( nullptr != ppSampler );
#endif

    SharedPtr<Sampler> pSampler = Sampler::Allocate(&m_handle);
    cl_err_code clErr = pSampler->Initialize(this, bNormalizedCoords, clAddressingMode, clFilterMode);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new Sampler, returned: %s"), ClErrTxt(clErr));
        pSampler->Release();
        return clErr;
    }

    m_mapSamplers.AddObject(pSampler);

    *ppSampler = pSampler;
    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
SharedPtr<Sampler> Context::GetSampler(cl_sampler clSamplerId)
{
    return m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSamplerId).DynamicCast<Sampler>();
}
SharedPtr<FissionableDevice>* Context::GetDevices(cl_uint * puiNumDevices)
{
    if (nullptr != puiNumDevices)
    {
        *puiNumDevices = m_mapDevices.Count();
    }
    return m_ppAllDevices;
}

SharedPtr<Device>* Context::GetExplicitlyAssociatedRootDevices(cl_uint* puiNumDevices)
{
    if (nullptr != puiNumDevices)
    {
        *puiNumDevices = m_uiNumRootDevices;
    }
    return m_ppExplicitRootDevices;
}

const tSetOfDevices *Context::GetAllRootDevices() const
{
    return &m_allRootDevices;
}

cl_device_id * Context::GetDeviceIds(cl_uint * puiNumDevices)
{
    if (nullptr != puiNumDevices)
    {
        *puiNumDevices = m_mapDevices.Count();
    }
    return m_pDeviceIds;
}

cl_dev_subdevice_id Context::GetSubdeviceId(cl_device_id id)
{
    SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int*)id).DynamicCast<FissionableDevice>();
    if (NULL == pDevice)
    {
        return 0;
    }
    SharedPtr<SubDevice> pSubdevice = pDevice.DynamicCast<SubDevice>();
    if (NULL == pSubdevice)
    {
        return 0;
    }
    return pSubdevice->GetSubdeviceId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CheckSupportedImageFormat
// Calculate the supported file formats for context.
// UNION of all device capabilities (see clGetSupportedImageFormats).
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CheckSupportedImageFormat( const cl_image_format* pclImageFormat, cl_mem_flags clMemFlags, cl_mem_object_type clObjType)
{
    // Check for invalid format
    if (nullptr == pclImageFormat || (0 == clGetPixelBytesCount(pclImageFormat)) )
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    // Calculate supported format key
    cl_mem_flags key = getFormatsKey(clObjType, clMemFlags);

    tImageFormatMap::iterator mapIT;
    {    // Critical section
        OclAutoMutex mu(&m_muFormatsMap);
        mapIT = m_mapSupportedFormats.find(key);
        // First access to the key, need to get formats from devices
        if ( m_mapSupportedFormats.end() == mapIT )
        {
            if ( 0 == CalculateSupportedImageFormats(clMemFlags, clObjType) )
            {
                return CL_IMAGE_FORMAT_NOT_SUPPORTED;
            }
            mapIT = m_mapSupportedFormats.find(key);
        }
    }

    // Now look for the format in relevant list
    tImageFormatList::iterator listIT = mapIT->second.begin();
    while (mapIT->second.end() != listIT)
    {
        if ( (pclImageFormat->image_channel_order == listIT->image_channel_order) &&
            (pclImageFormat->image_channel_data_type == listIT->image_channel_data_type) )
        {
            return CL_SUCCESS;
        }
        listIT++;
    }

    return CL_IMAGE_FORMAT_NOT_SUPPORTED;
}


/**
 * Calculate the supported file formats for context.
 * UNION of all device capabilities (see clGetSupportedImageFormats).
 * @param clMemFlags
 * @param clObjType
 * @return size of supported image formats list.
 */
size_t Context::CalculateSupportedImageFormats( const cl_mem_flags clMemFlags, cl_mem_object_type clObjType )
{
    // Calculate supported format key
    cl_mem_flags key = getFormatsKey(clObjType, clMemFlags);

    OclAutoMutex mu(&m_muFormatsMap);

    tImageFormatMap::iterator mapIT = m_mapSupportedFormats.find(key);

    // Found the supported formats list, no need to calculate it.
    if ( m_mapSupportedFormats.end() != mapIT )
    {
        return mapIT->second.size();
    }

    cl_err_code          clErr = CL_SUCCESS;
    cl_uint              maxFormatCount = 0;
    cl_image_format*     pFormats = nullptr;
    tImageFormatList     imageFormatsList;
    const tSetOfDevices *rDevSet = GetAllRootDevices();
    bool                 exitWithErr = false;

    // Go through the devices and accumulate formats (union)
    tSetOfDevices::const_iterator rDev;
    for ( rDev = rDevSet->begin() ;
            rDev != rDevSet->end() ; ++rDev)
    {
        cl_uint devSpecificFormatsCount(0);

        // find number of formats to expect.
        clErr = (*rDev)->GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clObjType,
                0, nullptr, &devSpecificFormatsCount);
        if (CL_FAILED(clErr))
        {
            exitWithErr = true;
            break;
        }

        if (maxFormatCount < devSpecificFormatsCount)
        {
            if (pFormats) delete[] pFormats;
            maxFormatCount = devSpecificFormatsCount;
            pFormats = new cl_image_format[maxFormatCount];
        }

        // get formats
        clErr = (*rDev)->GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clObjType,
                devSpecificFormatsCount, pFormats, nullptr);
        if (CL_FAILED(clErr))
        {
            exitWithErr = true;
            break;
        }

        std::sort(&pFormats[0], &pFormats[devSpecificFormatsCount], compareImageFormats);

        if (rDev != rDevSet->begin())
        {
            // not first device
            tImageFormatList tempFormatsList;
            std::set_union(&pFormats[0], &pFormats[devSpecificFormatsCount],
                    imageFormatsList.begin(), imageFormatsList.end(),
                    tempFormatsList.begin(), compareImageFormats);

            imageFormatsList = tempFormatsList;
        } else {
            // only for first device, add all formats
            for (unsigned int ui=0; ui<devSpecificFormatsCount; ++ui)
            {
                imageFormatsList.push_back(pFormats[ui]);
            }
        }
    }

    if (!exitWithErr)
    {
        if (rDev == rDevSet->begin() && rDev == rDevSet->end())
        {
            assert(0 && "CalculateSupportedImageFormats: No root devices for context.");
            exitWithErr = true;
        }
    }

    if (pFormats) delete[] pFormats;

    if (exitWithErr)
    {
        imageFormatsList.clear();
    }
    m_mapSupportedFormats[key] = imageFormatsList;
    return imageFormatsList.size();
}

/**
 * Comparison function for image formats.
 * Sorting order:  image_channel_data_type, image_channel_order.
 * @param f1
 * @param f2
 * @return true if f1 is smaller than f2, false otherwise.
 */
bool compareImageFormats(cl_image_format f1, cl_image_format f2)
{
    if (f1.image_channel_data_type < f2.image_channel_data_type) return true;
    if (f1.image_channel_data_type > f2.image_channel_data_type) return false;

    // here only if image_channel_data_type is equal.
    if (f1.image_channel_order < f2.image_channel_order) return true;

    // in case of equality, or image_channel_order not smaller:
    return false;
}

static cl_ulong getFormatsKey(cl_mem_object_type clObjType , cl_mem_flags clMemFlags)
{
    return clObjType << 16 | clMemFlags;
}

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT

Intel::OpenCL::Utils::OclOsDependentEvent* Context::GetOSEvent()
{
    Intel::OpenCL::Utils::OclOsDependentEvent* pOsEvent = nullptr;
    bool exists = m_OsEventPool.TryPop(pOsEvent);
    if (!exists)
    {
        pOsEvent = new Intel::OpenCL::Utils::OclOsDependentEvent();
        if ( nullptr != pOsEvent )
        {
            bool UNUSED_ATTR initOK = pOsEvent->Init();
            assert(initOK && "OclEvent Failed to setup OS_DEPENDENT event");
        }
    }

    return pOsEvent;
}

void Context::RecycleOSEvent(Intel::OpenCL::Utils::OclOsDependentEvent* pEvent)
{
    assert(pEvent && "pEvent == NULL, not expected");
    pEvent->Reset();
    m_OsEventPool.PushBack(pEvent);
}

#endif

cl_err_code Context::CheckSupportedImageFormatByMemFlags(cl_mem_flags clFlags, const cl_image_format& clImageFormat, cl_mem_object_type objType)
{    
    assert(((CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY) & clFlags) != (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY));
    // Need to perform inverse checking, becuase CL_MEM_READ_WRITE value is 0
    // If WRITE_ONLY flag is not set check for read image support
    if (0 == (CL_MEM_WRITE_ONLY & clFlags))
    {
        const cl_err_code clErr = CheckSupportedImageFormat(&clImageFormat, CL_MEM_READ_ONLY, objType);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Image format not supported: %S"), ClErrTxt(clErr));
            return clErr;
        }
    }
    // If READ_ONLY flag is not set check for write image support
    if (0 == (CL_MEM_READ_ONLY & clFlags))
    {
        const cl_err_code clErr = CheckSupportedImageFormat(&clImageFormat, CL_MEM_WRITE_ONLY, objType);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Image format not supported: %S"), ClErrTxt(clErr));
            return clErr;
        }
    }
    return CL_SUCCESS;
}

cl_int Context::SetKernelArgSVMPointer(const SharedPtr<Kernel>& pKernel, cl_uint uiArgIndex, const void* pArgValue)
{
    cl_err_code err = pKernel->SetKernelArg(uiArgIndex, sizeof(void*), pArgValue, true);
    return CL_ERR_OUT(err);
}

cl_int Context::SetKernelExecInfo(const SharedPtr<Kernel>& pKernel, cl_kernel_exec_info paramName, size_t szParamValueSize, const void* pParamValue)
{
    switch (paramName)
    {
    case CL_KERNEL_EXEC_INFO_SVM_PTRS:
        {
            if (szParamValueSize == 0 || szParamValueSize % sizeof(void*) != 0)
            {
                return CL_INVALID_VALUE;
            }
            std::vector<SharedPtr<SVMBuffer> > svmBufs;
            for (size_t i = 0; i < szParamValueSize / sizeof(void*); i++)
            {
                SharedPtr<SVMBuffer> pSvmBuf = GetSVMBufferContainingAddr(((void**)pParamValue)[i]);
                if (NULL == pSvmBuf)
                {
                    return CL_INVALID_VALUE;
                }
                svmBufs.push_back(pSvmBuf);
            }
            pKernel->SetNonArgSvmBuffers(svmBufs);
            break;
        }
    case CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM:
        {
            if (szParamValueSize < sizeof(cl_bool))
            {
                return CL_INVALID_VALUE;
            }
            if (CL_TRUE == *(cl_bool*)pParamValue && !pKernel->GetContext()->DoesSupportSvmSystem())
            {
                return CL_INVALID_OPERATION;
            }
            pKernel->SetSvmFineGrainSystem(CL_TRUE == *(cl_bool*)pParamValue);
            break;
        }
    case CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL:
        {
            if (szParamValueSize == 0 || szParamValueSize % sizeof(void*) != 0)
                return CL_INVALID_VALUE;
            std::vector<SharedPtr<USMBuffer> > usmBufs;
            for (size_t i = 0; i < szParamValueSize / sizeof(void*); i++)
            {
                SharedPtr<USMBuffer> buf = GetUSMBufferContainingAddr(
                                           ((void**)pParamValue)[i]);
                if (nullptr == buf.GetPtr())
                    return CL_INVALID_VALUE;
                usmBufs.push_back(buf);
            }
            pKernel->SetNonArgUsmBuffers(usmBufs);
            break;
        }
    case CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL:
        {
            if (szParamValueSize < sizeof(cl_bool))
                return CL_INVALID_VALUE;
            if (CL_TRUE == *(cl_bool*)pParamValue &&
                !pKernel->GetContext()->DoesSupportUsmHost())
                return CL_INVALID_OPERATION;
            pKernel->SetUsmIndirectHost(CL_TRUE == *(cl_bool*)pParamValue);
            break;
        }
    case CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL:
        {
            if (szParamValueSize < sizeof(cl_bool))
                return CL_INVALID_VALUE;
            if (CL_TRUE == *(cl_bool*)pParamValue &&
                !pKernel->GetContext()->DoesSupportUsmDevice())
                return CL_INVALID_OPERATION;
            pKernel->SetUsmIndirectDevice(CL_TRUE == *(cl_bool*)pParamValue);
            break;
        }
    case CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL:
        {
            if (szParamValueSize < sizeof(cl_bool))
                return CL_INVALID_VALUE;
            if (CL_TRUE == *(cl_bool*)pParamValue &&
                !(pKernel->GetContext()->DoesSupportUsmSharedSingle() ||
                  pKernel->GetContext()->DoesSupportUsmSharedCross() ||
                  pKernel->GetContext()->DoesSupportUsmSharedSystem()))
                return CL_INVALID_OPERATION;
            pKernel->SetUsmIndirectShared(CL_TRUE == *(cl_bool*)pParamValue);
            break;
        }
    default:
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

void* Context::SVMAlloc(cl_svm_mem_flags flags, size_t size, unsigned int uiAlignment)
{
    const cl_svm_mem_flags oldMemFlags = flags & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);
    if (oldMemFlags != 0 && oldMemFlags != CL_MEM_READ_WRITE && oldMemFlags != CL_MEM_WRITE_ONLY && oldMemFlags != CL_MEM_READ_ONLY)
    {
        LOG_ERROR(TEXT("CL_MEM_READ_WRITE or CL_MEM_WRITE_ONLY and CL_MEM_READ_ONLY are mutually exclusive"), "");
        return nullptr;
    }

    const tSetOfDevices& devices = *GetAllRootDevices();
    for (tSetOfDevices::const_iterator iter = devices.begin(); iter != devices.end(); iter++)
    {
        cl_device_svm_capabilities devSvmCapabilities;
        cl_err_code err = (*iter)->GetInfo(CL_DEVICE_SVM_CAPABILITIES, sizeof(devSvmCapabilities), &devSvmCapabilities, nullptr);
        ASSERT_RET_VAL(CL_SUCCEEDED(err), "device can't handle CL_DEVICE_SVM_CAPABILITIES query", nullptr);
        if (((flags & CL_MEM_SVM_FINE_GRAIN_BUFFER) && !(devSvmCapabilities & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)) ||
            ((flags & CL_MEM_SVM_ATOMICS) && !(devSvmCapabilities & CL_DEVICE_SVM_ATOMICS)))
        {
            LOG_ERROR(TEXT("CL_MEM_SVM_FINE_GRAIN_BUFFER or CL_MEM_SVM_ATOMICS is specified in flags and these are not supported by all devices in context"), "");
            return nullptr;
        }

        cl_ulong ulDevMaxAllocSize;
        err = (*iter)->GetInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(ulDevMaxAllocSize), &ulDevMaxAllocSize, nullptr);
        ASSERT_RET_VAL(CL_SUCCEEDED(err), "device can't handle CL_DEVICE_MAX_MEM_ALLOC_SIZE query", nullptr);
        if (size > ulDevMaxAllocSize)
        {
            LOG_ERROR(TEXT("size is > CL_DEVICE_MAX_ALLOC_SIZE value for a device in context"), "");
            return nullptr;
        }
    }

    // do the real work:
    if (0 == oldMemFlags)
    {
        flags |= CL_MEM_READ_WRITE;
    }
    if (0 == uiAlignment)
    {
        uiAlignment = sizeof(cl_long16);    // this is the default alignment
    }

    SharedPtr<SVMBuffer> pSvmBuf = SVMBuffer::Allocate(this);
    if (NULL == pSvmBuf)
    {
        return nullptr;
    }
    // these flags aren't needed anymore
    pSvmBuf->Initialize(flags & ~(CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS), nullptr, 1, &size, nullptr, nullptr, 0);

    OclAutoWriter mu(&m_svmBuffersRwlock);
    void* pSvmPtr = pSvmBuf->GetBackingStoreData();
    m_svmBuffers[pSvmPtr] = pSvmBuf;
    ASSERT_RET_VAL(IS_ALIGNED_ON(pSvmPtr, uiAlignment), "pSvmPtr isn't aligned as user requested", nullptr);
    return pSvmPtr;
}

void Context::SVMFree(void* pSvmPtr)
{
    OclAutoWriter mu(&m_svmBuffersRwlock);
    if (m_svmBuffers.find(pSvmPtr) == m_svmBuffers.end())
    {
        LOG_ERROR(TEXT("pSvmPtr isn't an SVM buffer"), "");
        return;
    }
    m_svmBuffers.erase(pSvmPtr);
}

SharedPtr<SVMBuffer> Context::GetSVMBufferContainingAddr(void* ptr)
{
    OclAutoReader mu(&m_svmBuffersRwlock);
    std::map<void*, SharedPtr<SVMBuffer> >::iterator iter = m_svmBuffers.upper_bound(ptr);
    if (iter == m_svmBuffers.begin())
    {
        return nullptr;
    }
    const SharedPtr<SVMBuffer>& pSvmBuffer = (--iter)->second;

    if (((char*)ptr >= (char*)pSvmBuffer->GetAddr()) &&
        ((char*)ptr < (char*)pSvmBuffer->GetAddr() + pSvmBuffer->GetSize()))
    {
        return pSvmBuffer;
    }
    return nullptr;
}

ConstSharedPtr<SVMBuffer> Context::GetSVMBufferContainingAddr(const void* ptr) const
{
    return const_cast<Context*>(this)->GetSVMBufferContainingAddr(const_cast<void*>(ptr));
}

bool Context::IsSVMPointer(const void* ptr) const
{
    if (nullptr != ptr &&
        NULL != GetSVMBufferContainingAddr(ptr))
    {
        return true;
    }
    return false;
}

cl_mem_alloc_flags_intel Context::ParseUSMAllocProperties(
    const cl_mem_properties_intel* properties, cl_int* errcode_ret)
{
    cl_mem_alloc_flags_intel flags = 0;

    // check properties
    std::map<cl_mem_alloc_flags_intel, cl_mem_alloc_flags_intel> propertyMap;
    if (nullptr != properties)
    {
        size_t i = 0;

        while (0 != properties[i])
        {
            if (propertyMap.find(properties[i]) != propertyMap.end())
            {
                LOG_ERROR(TEXT("%s"), TEXT(
                    "the same property name is specified more than once"));
                *errcode_ret = CL_INVALID_PROPERTY;
                return flags;
            }

            static std::vector<cl_context_properties> legalProperties = {
                CL_MEM_ALLOC_FLAGS_INTEL
            };

            cl_context_properties currentProperty = properties[i];
            if (std::none_of(
                    legalProperties.begin(), legalProperties.end(),
                    [currentProperty](const cl_context_properties& prop) {
                        return prop == currentProperty;
                    }))
            {
                LOG_ERROR(TEXT("%s"), TEXT(
                    "property name is not a supported property name"));
                *errcode_ret = CL_INVALID_PROPERTY;
                return flags;
            }
            propertyMap[properties[i]] = properties[i + 1];
            i += 2;
        }
        if (propertyMap.find(CL_MEM_ALLOC_FLAGS_INTEL) != propertyMap.end())
        {
            cl_mem_alloc_flags_intel property =
                propertyMap[CL_MEM_ALLOC_FLAGS_INTEL];
            if (CL_MEM_ALLOC_WRITE_COMBINED_INTEL != property)
            {
                *errcode_ret = CL_INVALID_PROPERTY;
                return flags;
            }
            flags = property;
        }
    }

    *errcode_ret = CL_SUCCESS;
    return flags;
}

void* Context::USMAlloc(cl_unified_shared_memory_type_intel type,
                        cl_device_id device,
                        const cl_mem_properties_intel* properties,
                        size_t size, unsigned int alignment,
                        cl_int* errcode_ret)
{
    cl_int err;
    cl_mem_alloc_flags_intel flags = ParseUSMAllocProperties(properties,
                                                             &err);
#define USM_ALLOC_ERR_RET(err) \
        { \
            if (errcode_ret) \
                *errcode_ret = err; \
            return nullptr; \
        }
    if (CL_SUCCESS != err)
        USM_ALLOC_ERR_RET(err);

    // Check if device is valid
    cl_uint numDevices;
    SharedPtr<FissionableDevice> pDevice;
    const SharedPtr<FissionableDevice> *pDevices;
    if (device)
    {
        numDevices = 1;
        bool res = GetDevicesFromList(numDevices, &device, &pDevice);
        if (false == res)
        {
            LOG_ERROR(TEXT("%s"), TEXT("GetDevicesFromList returns false"));
            USM_ALLOC_ERR_RET(CL_INVALID_DEVICE);
        }
        pDevices = &pDevice;
    }
    else if (CL_MEM_TYPE_DEVICE_INTEL == type)
    {
        LOG_ERROR(TEXT("%s"), TEXT("device is nullptr"));
        USM_ALLOC_ERR_RET(CL_INVALID_DEVICE);
    }
    else
    {
        numDevices = m_mapDevices.Count();
        pDevices = m_ppAllDevices;
    }

    // Check device capabilities
    bool capSupported = false;
    bool sizeSupported = false;
    for (cl_uint i = 0; i < numDevices; i++)
    {
        SharedPtr<Device> rootDevice = pDevices[i]->GetRootDevice();

        cl_unified_shared_memory_capabilities_intel hostCap;
        cl_unified_shared_memory_capabilities_intel deviceCap;
        cl_unified_shared_memory_capabilities_intel singleSharedCap;
        cl_unified_shared_memory_capabilities_intel crossSharedCap;
        err = rootDevice->GetInfo(CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL,
            sizeof(hostCap), &hostCap, nullptr);
        if (CL_SUCCESS != err)
            USM_ALLOC_ERR_RET(err);
        err = rootDevice->GetInfo(CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL,
            sizeof(deviceCap), &deviceCap, nullptr);
        if (CL_SUCCESS != err)
            USM_ALLOC_ERR_RET(err);
        err = rootDevice->GetInfo(
            CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
            sizeof(singleSharedCap), &singleSharedCap, nullptr);
        if (CL_SUCCESS != err)
            USM_ALLOC_ERR_RET(err);
        err = rootDevice->GetInfo(
            CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL,
            sizeof(crossSharedCap), &crossSharedCap, nullptr);
        if (CL_SUCCESS != err)
            USM_ALLOC_ERR_RET(err);
        if ((type == CL_MEM_TYPE_HOST_INTEL && 0 != hostCap) ||
            (type == CL_MEM_TYPE_DEVICE_INTEL && 0 != deviceCap) ||
            (type == CL_MEM_TYPE_SHARED_INTEL &&
             (0 != singleSharedCap || 0 != crossSharedCap)))
        {
            capSupported = true;
        }

        cl_ulong ulDevMaxAllocSize;
        err = rootDevice->GetInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE,
            sizeof(ulDevMaxAllocSize), &ulDevMaxAllocSize, nullptr);
        if (CL_SUCCESS != err)
            USM_ALLOC_ERR_RET(CL_INVALID_OPERATION);
        if (size > 0 && size <= ulDevMaxAllocSize)
        {
            sizeSupported = true;
        }
    }
    if (!capSupported)
    {
        LOG_ERROR(TEXT("no devices support this type of USM allocation"), "");
        USM_ALLOC_ERR_RET(CL_INVALID_OPERATION);
    }
    if (!sizeSupported)
    {
        LOG_ERROR(TEXT("size is zero or greater than "
                       "CL_DEVICE_MAX_MEM_ALLOC_SIZE for all devices"), "");
        USM_ALLOC_ERR_RET(CL_INVALID_BUFFER_SIZE);
    }

    // do the real work
    SharedPtr<USMBuffer> usmBuf = USMBuffer::Allocate(this);
    if (nullptr == usmBuf.GetPtr())
        USM_ALLOC_ERR_RET(CL_OUT_OF_HOST_MEMORY);
    usmBuf->SetType(type);
    usmBuf->SetDevice(device);
    // CL_MEM_ALLOC_WRITE_COMBINED_INTEL is used for write-combining uncached
    // memory which helps to accelerate cases when the host is writing to
    // device memory. This flag is ignored for CPU device.
    usmBuf->SetFlags(flags);

    flags |= CL_MEM_READ_WRITE;
    size_t forceAlignment = 0;
    if (0 != alignment)
    {
        if (0 != (alignment & (alignment - 1))) // if is not a power of 2
            USM_ALLOC_ERR_RET(CL_INVALID_VALUE);
        forceAlignment = alignment;
    }

    // these flags aren't needed anymore
    err = usmBuf->Initialize(flags & ~CL_MEM_ALLOC_WRITE_COMBINED_INTEL,
        nullptr, 1, &size, nullptr, nullptr, 0, forceAlignment);
    if (CL_SUCCESS != err)
        USM_ALLOC_ERR_RET(err);

    OclAutoWriter mu(&m_usmBuffersRwlock);
    void* usmPtr = usmBuf->GetBackingStoreData();
    m_usmBuffers[usmPtr] = usmBuf;

    if (0 != forceAlignment && !IS_ALIGNED_ON(usmPtr, forceAlignment))
        USM_ALLOC_ERR_RET(CL_OUT_OF_RESOURCES);

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;
    return usmPtr;
}

void* Context::USMHostAlloc(const cl_mem_properties_intel* properties,
                            size_t size,
                            cl_uint alignment,
                            cl_int* errcode_ret)
{
    cl_unified_shared_memory_type_intel type = CL_MEM_TYPE_HOST_INTEL;
    return USMAlloc(type, nullptr, properties, size, alignment, errcode_ret);
}

void* Context::USMDeviceAlloc(cl_device_id device,
                              const cl_mem_properties_intel* properties,
                              size_t size,
                              cl_uint alignment,
                              cl_int* errcode_ret)
{
    cl_unified_shared_memory_type_intel type = CL_MEM_TYPE_DEVICE_INTEL;
    return USMAlloc(type, device, properties, size, alignment, errcode_ret);
}

void* Context::USMSharedAlloc(cl_device_id device,
                              const cl_mem_properties_intel* properties,
                              size_t size,
                              cl_uint alignment,
                              cl_int* errcode_ret)
{
    cl_unified_shared_memory_type_intel type = CL_MEM_TYPE_SHARED_INTEL;
    return USMAlloc(type, device, properties, size, alignment, errcode_ret);
}

cl_int Context::USMFree(void* usmPtr)
{
    OclAutoWriter mu(&m_usmBuffersRwlock);
    if (m_usmBuffers.find(usmPtr) == m_usmBuffers.end())
    {
        LOG_ERROR(TEXT("usmPtr isn't an USM buffer"), "");
        return CL_INVALID_VALUE;
    }
    m_usmBuffers.erase(usmPtr);
    return CL_SUCCESS;
}

cl_int Context::GetMemAllocInfoINTEL(const void* ptr,
                                     cl_mem_info_intel param_name,
                                     size_t param_value_size,
                                     void* param_value,
                                     size_t* param_value_size_ret)
{
    ConstSharedPtr<USMBuffer> buffer = GetUSMBufferContainingAddr(ptr);
    bool notUSM = (nullptr == buffer.GetPtr());

    size_t valueSize = 0;
    const void *value = nullptr;
    cl_unified_shared_memory_type_intel type;
    cl_mem_alloc_flags_intel flags;
    const void *basePtr;
    size_t size;
    cl_device_id device;

    type = notUSM ? CL_MEM_TYPE_UNKNOWN_INTEL : buffer->GetType();
    switch (param_name)
    {
    case CL_MEM_ALLOC_TYPE_INTEL:
        valueSize = sizeof(cl_unified_shared_memory_type_intel);
        value = &type;
        break;
    case CL_MEM_ALLOC_BASE_PTR_INTEL:
        valueSize = sizeof(void*);
        basePtr = (type == CL_MEM_TYPE_UNKNOWN_INTEL) ? nullptr :
            buffer->GetAddr();
        value = &basePtr;
        break;
    case CL_MEM_ALLOC_SIZE_INTEL:
        valueSize = sizeof(size_t);
        size = (type == CL_MEM_TYPE_UNKNOWN_INTEL) ? 0 : buffer->GetSize();
        value = &size;
        break;
    case CL_MEM_ALLOC_DEVICE_INTEL:
        valueSize = sizeof(cl_device_id);
        device = ((type == CL_MEM_TYPE_DEVICE_INTEL) ||
            ((type == CL_MEM_TYPE_SHARED_INTEL) && buffer->GetDevice())) ?
            buffer->GetDevice() : nullptr;
        value = &device;
        break;
    case CL_MEM_ALLOC_FLAGS_INTEL:
        valueSize = sizeof(cl_mem_alloc_flags_intel);
        flags = (type == CL_MEM_TYPE_UNKNOWN_INTEL) ? 0 : buffer->GetFlags();
        value = &flags;
        break;
    default:
        LOG_ERROR(TEXT("param_name (=%d) isn't valid"), param_name);
        return CL_INVALID_VALUE;
    }
    if (nullptr != param_value && param_value_size < valueSize)
        return CL_INVALID_VALUE;
    if (nullptr != param_value_size_ret)
        *param_value_size_ret = valueSize;
    if (nullptr != param_value && valueSize > 0)
    {
        MEMCPY_S(param_value, param_value_size, value, valueSize);
    }

    return CL_SUCCESS;
}

SharedPtr<USMBuffer> Context::GetUSMBufferContainingAddr(void* ptr)
{
    OclAutoReader mu(&m_usmBuffersRwlock);
    std::map<void*, SharedPtr<USMBuffer> >::iterator iter =
        m_usmBuffers.upper_bound(ptr);
    if (iter == m_usmBuffers.begin())
        return nullptr;

    const SharedPtr<USMBuffer>& usmBuffer = (--iter)->second;

    if (((char*)ptr >= (char*)usmBuffer->GetAddr()) &&
        ((char*)ptr < (char*)usmBuffer->GetAddr() + usmBuffer->GetSize()))
        return usmBuffer;

    return nullptr;
}

ConstSharedPtr<USMBuffer> Context::GetUSMBufferContainingAddr(const void* ptr)
    const
{
    return const_cast<Context*>(this)->GetUSMBufferContainingAddr(
        const_cast<void*>(ptr));
}

bool Context::IsUSMPointer(const void* ptr) const
{
    return (nullptr != ptr) &&
        (nullptr != GetUSMBufferContainingAddr(ptr).GetPtr());
}

cl_err_code Context::CreatePipe(cl_mem_flags flags, cl_uint uiPipePacketSize,
                                cl_uint uiPipeMaxPackets,
                                SharedPtr<MemoryObject>& pPipe,
                                void* pHostPtr)
{
	cl_err_code err = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_MEM_OBJECT_PIPE, CL_MEMOBJ_GFX_SHARE_NONE, this, &pPipe);
	if (CL_FAILED(err))
	{
		return err;
	}
	err = pPipe.StaticCast<Pipe>()->Initialize(flags, uiPipePacketSize, uiPipeMaxPackets, pHostPtr);
	if (CL_FAILED(err))
	{
		return err;
	}
	m_mapMemObjects.AddObject(pPipe);
	return CL_SUCCESS;
}

void* Context::MapPipe(SharedPtr<Pipe>& pPipe, cl_map_flags flags,
                       size_t requestedSize, size_t* pMappedSize,
                       cl_err_code* pError)
{
    return pPipe->Map(flags, requestedSize, pMappedSize, pError);
}

cl_err_code Context::UnmapPipe(SharedPtr<Pipe>& pPipe, void* pMappedPtr,
                               size_t sizeToUnmap, size_t* pUnmappedSize)
{
    return pPipe->Unmap(pMappedPtr, sizeToUnmap, pUnmappedSize);
}

cl_err_code Context::ReadPipe(SharedPtr<Pipe>& pPipe, void* pDst)
{
    return pPipe->ReadPacket(pDst);
}

cl_err_code Context::WritePipe(SharedPtr<Pipe>& pPipe, const void* pSrc)
{
    return pPipe->WritePacket(pSrc);
}
