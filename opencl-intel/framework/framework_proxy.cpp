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


#include "framework_proxy.h"
#include "Logger.h"
#include "cl_sys_info.h"
#include "cl_sys_defines.h"
#include <task_executor.h>
#include <cl_shared_ptr.hpp>
#include "cl_shutdown.h"

#if defined (_WIN32)
#include <windows.h>
#else
#include "cl_secure_string_linux.h"
#include "cl_framework_alias_linux.h"
#endif
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::TaskExecutor;

// no local atexit handler - only global
USE_SHUTDOWN_HANDLER(nullptr);

cl_monitor_init

cl_icd_dispatch             FrameworkProxy::ICDDispatchTable;
SOCLCRTDispatchTable        FrameworkProxy::CRTDispatchTable;
ocl_entry_points            FrameworkProxy::OclEntryPoints;

OclSpinMutex FrameworkProxy::m_initializationMutex;

volatile FrameworkProxy::GLOBAL_STATE FrameworkProxy::gGlobalState = FrameworkProxy::WORKING;
THREAD_LOCAL bool                     FrameworkProxy::m_bIgnoreAtExit = false;

std::set<Intel::OpenCL::Utils::at_exit_dll_callback_fn>   FrameworkProxy::m_at_exit_cbs;

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::FrameworkProxy()
{
    m_pPlatformModule = nullptr;
    m_pContextModule = nullptr;
    m_pExecutionModule = nullptr;
    m_pFileLogHandler = nullptr;
    m_pConfig = nullptr;
    m_pLoggerClient = nullptr;
    m_pTaskExecutor = nullptr;
    m_pTaskList     = nullptr;
    m_uiTEActivationCount = 0;

    RegisterGlobalAtExitNotification        ( this );
#ifndef _WIN32
    // on Linux Logger is implemented as a separate DLL
    Logger::RegisterGlobalAtExitNotification( this );
#endif
    TE_RegisterGlobalAtExitNotification     ( this );

    Initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// ~FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::~FrameworkProxy()
{
}

void FrameworkProxy::InitOCLEntryPoints()
{
    OclEntryPoints.icdDispatch = &ICDDispatchTable;
    OclEntryPoints.crtDispatch = &CRTDispatchTable;

    /// ICD functions
    ICDDispatchTable.clGetPlatformIDs = (cl_api_clGetPlatformIDs)GET_ALIAS(clGetPlatformIDs);
    ICDDispatchTable.clGetPlatformInfo = (cl_api_clGetPlatformInfo)GET_ALIAS(clGetPlatformInfo);
    ICDDispatchTable.clGetDeviceIDs = (cl_api_clGetDeviceIDs)GET_ALIAS(clGetDeviceIDs);
    ICDDispatchTable.clGetDeviceInfo = (cl_api_clGetDeviceInfo)GET_ALIAS(clGetDeviceInfo);
    ICDDispatchTable.clCreateContext = (cl_api_clCreateContext)GET_ALIAS(clCreateContext);
    ICDDispatchTable.clCreateContextFromType = (cl_api_clCreateContextFromType)GET_ALIAS(clCreateContextFromType);
    ICDDispatchTable.clSetContextDestructorCallback =
        (cl_api_clSetContextDestructorCallback)GET_ALIAS(
            clSetContextDestructorCallback);
    ICDDispatchTable.clSetProgramReleaseCallback =
        (cl_api_clSetProgramReleaseCallback)GET_ALIAS(
            clSetProgramReleaseCallback);
    ICDDispatchTable.clRetainContext = (cl_api_clRetainContext)GET_ALIAS(clRetainContext);
    ICDDispatchTable.clReleaseContext = (cl_api_clReleaseContext)GET_ALIAS(clReleaseContext);
    ICDDispatchTable.clGetContextInfo = (cl_api_clGetContextInfo)GET_ALIAS(clGetContextInfo);
    ICDDispatchTable.clCreateCommandQueue = (cl_api_clCreateCommandQueue)GET_ALIAS(clCreateCommandQueue);
    ICDDispatchTable.clCreateCommandQueueWithProperties = (cl_api_clCreateCommandQueueWithProperties)GET_ALIAS(clCreateCommandQueueWithProperties);
    ICDDispatchTable.clRetainCommandQueue = (cl_api_clRetainCommandQueue)GET_ALIAS(clRetainCommandQueue);
    ICDDispatchTable.clReleaseCommandQueue = (cl_api_clReleaseCommandQueue)GET_ALIAS(clReleaseCommandQueue);
    ICDDispatchTable.clGetCommandQueueInfo = (cl_api_clGetCommandQueueInfo)GET_ALIAS(clGetCommandQueueInfo);
    ICDDispatchTable.clSetCommandQueueProperty = nullptr;
    ICDDispatchTable.clCreateBuffer = (cl_api_clCreateBuffer)GET_ALIAS(clCreateBuffer);
    ICDDispatchTable.clCreateBufferWithProperties =
        (cl_api_clCreateBufferWithProperties)GET_ALIAS(
            clCreateBufferWithProperties);
    ICDDispatchTable.clCreateImage = (cl_api_clCreateImage)GET_ALIAS(clCreateImage);
    ICDDispatchTable.clCreateImageWithProperties =
        (cl_api_clCreateImageWithProperties)GET_ALIAS(
            clCreateImageWithProperties);
    ICDDispatchTable.clCreateImage2D = (cl_api_clCreateImage2D)GET_ALIAS(clCreateImage2D);
    ICDDispatchTable.clCreateImage3D = (cl_api_clCreateImage3D)GET_ALIAS(clCreateImage3D);
    ICDDispatchTable.clRetainMemObject = (cl_api_clRetainMemObject)GET_ALIAS(clRetainMemObject);
    ICDDispatchTable.clReleaseMemObject = (cl_api_clReleaseMemObject)GET_ALIAS(clReleaseMemObject);
    ICDDispatchTable.clGetSupportedImageFormats = (cl_api_clGetSupportedImageFormats)GET_ALIAS(clGetSupportedImageFormats);
    ICDDispatchTable.clGetMemObjectInfo = (cl_api_clGetMemObjectInfo)GET_ALIAS(clGetMemObjectInfo);
    ICDDispatchTable.clGetImageInfo = (cl_api_clGetImageInfo)GET_ALIAS(clGetImageInfo);
    ICDDispatchTable.clCreateSampler = (cl_api_clCreateSampler)GET_ALIAS(clCreateSampler);
    ICDDispatchTable.clCreateSamplerWithProperties = (cl_api_clCreateSamplerWithProperties)GET_ALIAS(clCreateSamplerWithProperties);
    ICDDispatchTable.clRetainSampler = (cl_api_clRetainSampler)GET_ALIAS(clRetainSampler);
    ICDDispatchTable.clReleaseSampler = (cl_api_clReleaseSampler)GET_ALIAS(clReleaseSampler);
    ICDDispatchTable.clGetSamplerInfo = (cl_api_clGetSamplerInfo)GET_ALIAS(clGetSamplerInfo);
    ICDDispatchTable.clCreateProgramWithSource = (cl_api_clCreateProgramWithSource)GET_ALIAS(clCreateProgramWithSource);
    ICDDispatchTable.clSetDefaultDeviceCommandQueue = (cl_api_clSetDefaultDeviceCommandQueue)GET_ALIAS(clSetDefaultDeviceCommandQueue);
    ICDDispatchTable.clCreateProgramWithBinary = (cl_api_clCreateProgramWithBinary)GET_ALIAS(clCreateProgramWithBinary);
    ICDDispatchTable.clCreateProgramWithBuiltInKernels = (cl_api_clCreateProgramWithBuiltInKernels)GET_ALIAS(clCreateProgramWithBuiltInKernels);
    ICDDispatchTable.clCreateProgramWithIL = (cl_api_clCreateProgramWithIL)GET_ALIAS(clCreateProgramWithIL);
    ICDDispatchTable.clRetainProgram = (cl_api_clRetainProgram)GET_ALIAS(clRetainProgram);
    ICDDispatchTable.clReleaseProgram = (cl_api_clReleaseProgram)GET_ALIAS(clReleaseProgram);
    ICDDispatchTable.clBuildProgram = (cl_api_clBuildProgram)GET_ALIAS(clBuildProgram);
    ICDDispatchTable.clCompileProgram = (cl_api_clCompileProgram)GET_ALIAS(clCompileProgram);
    ICDDispatchTable.clLinkProgram = (cl_api_clLinkProgram)GET_ALIAS(clLinkProgram);
    ICDDispatchTable.clUnloadCompiler = (cl_api_clUnloadCompiler)GET_ALIAS(clUnloadCompiler);
    ICDDispatchTable.clUnloadPlatformCompiler = (cl_api_clUnloadPlatformCompiler)GET_ALIAS(clUnloadPlatformCompiler);
    ICDDispatchTable.clGetProgramInfo = (cl_api_clGetProgramInfo)GET_ALIAS(clGetProgramInfo);
    ICDDispatchTable.clGetProgramBuildInfo = (cl_api_clGetProgramBuildInfo)GET_ALIAS(clGetProgramBuildInfo);
    ICDDispatchTable.clCreateKernel = (cl_api_clCreateKernel)GET_ALIAS(clCreateKernel);
    ICDDispatchTable.clCreateKernelsInProgram = (cl_api_clCreateKernelsInProgram)GET_ALIAS(clCreateKernelsInProgram);
    ICDDispatchTable.clRetainKernel = (cl_api_clRetainKernel)GET_ALIAS(clRetainKernel);
    ICDDispatchTable.clReleaseKernel = (cl_api_clReleaseKernel)GET_ALIAS(clReleaseKernel);
    ICDDispatchTable.clSetKernelArg = (cl_api_clSetKernelArg)GET_ALIAS(clSetKernelArg);
    ICDDispatchTable.clGetKernelInfo = (cl_api_clGetKernelInfo)GET_ALIAS(clGetKernelInfo);
    ICDDispatchTable.clCloneKernel = (cl_api_clCloneKernel)GET_ALIAS(clCloneKernel);
    ICDDispatchTable.clGetHostTimer = (cl_api_clGetHostTimer)GET_ALIAS(clGetHostTimer);
    ICDDispatchTable.clGetDeviceAndHostTimer = (cl_api_clGetDeviceAndHostTimer)GET_ALIAS(clGetDeviceAndHostTimer);
    ICDDispatchTable.clGetKernelWorkGroupInfo = (cl_api_clGetKernelWorkGroupInfo)GET_ALIAS(clGetKernelWorkGroupInfo);
    ICDDispatchTable.clGetKernelSubGroupInfo = (cl_api_clGetKernelSubGroupInfo)GET_ALIAS(clGetKernelSubGroupInfo);
    ICDDispatchTable.clGetKernelSubGroupInfoKHR = (cl_api_clGetKernelSubGroupInfoKHR)GET_ALIAS(clGetKernelSubGroupInfoKHR);
    ICDDispatchTable.clWaitForEvents = (cl_api_clWaitForEvents)GET_ALIAS(clWaitForEvents);
    ICDDispatchTable.clGetEventInfo = (cl_api_clGetEventInfo)GET_ALIAS(clGetEventInfo);
    ICDDispatchTable.clRetainEvent = (cl_api_clRetainEvent)GET_ALIAS(clRetainEvent);
    ICDDispatchTable.clReleaseEvent = (cl_api_clReleaseEvent)GET_ALIAS(clReleaseEvent);
    ICDDispatchTable.clGetEventProfilingInfo = (cl_api_clGetEventProfilingInfo)GET_ALIAS(clGetEventProfilingInfo);
    ICDDispatchTable.clFlush = (cl_api_clFlush)GET_ALIAS(clFlush);
    ICDDispatchTable.clFinish = (cl_api_clFinish)GET_ALIAS(clFinish);
    ICDDispatchTable.clEnqueueReadBuffer = (cl_api_clEnqueueReadBuffer)GET_ALIAS(clEnqueueReadBuffer);
    ICDDispatchTable.clEnqueueWriteBuffer = (cl_api_clEnqueueWriteBuffer)GET_ALIAS(clEnqueueWriteBuffer);
    ICDDispatchTable.clEnqueueCopyBuffer = (cl_api_clEnqueueCopyBuffer)GET_ALIAS(clEnqueueCopyBuffer);
    ICDDispatchTable.clEnqueueFillBuffer = (cl_api_clEnqueueFillBuffer)GET_ALIAS(clEnqueueFillBuffer);
    ICDDispatchTable.clEnqueueFillImage  = (cl_api_clEnqueueFillImage)GET_ALIAS(clEnqueueFillImage);
    ICDDispatchTable.clEnqueueReadImage = (cl_api_clEnqueueReadImage)GET_ALIAS(clEnqueueReadImage);
    ICDDispatchTable.clEnqueueWriteImage = (cl_api_clEnqueueWriteImage)GET_ALIAS(clEnqueueWriteImage);
    ICDDispatchTable.clEnqueueCopyImage = (cl_api_clEnqueueCopyImage)GET_ALIAS(clEnqueueCopyImage);
    ICDDispatchTable.clEnqueueCopyImageToBuffer = (cl_api_clEnqueueCopyImageToBuffer)GET_ALIAS(clEnqueueCopyImageToBuffer);
    ICDDispatchTable.clEnqueueCopyBufferToImage = (cl_api_clEnqueueCopyBufferToImage)GET_ALIAS(clEnqueueCopyBufferToImage);
    ICDDispatchTable.clEnqueueMapBuffer = (cl_api_clEnqueueMapBuffer)GET_ALIAS(clEnqueueMapBuffer);
    ICDDispatchTable.clEnqueueMapImage = (cl_api_clEnqueueMapImage)GET_ALIAS(clEnqueueMapImage);
    ICDDispatchTable.clEnqueueUnmapMemObject = (cl_api_clEnqueueUnmapMemObject)GET_ALIAS(clEnqueueUnmapMemObject);
    ICDDispatchTable.clEnqueueNDRangeKernel = (cl_api_clEnqueueNDRangeKernel)GET_ALIAS(clEnqueueNDRangeKernel);
    ICDDispatchTable.clEnqueueTask = (cl_api_clEnqueueTask)GET_ALIAS(clEnqueueTask);
    ICDDispatchTable.clEnqueueNativeKernel = (cl_api_clEnqueueNativeKernel)GET_ALIAS(clEnqueueNativeKernel);
    ICDDispatchTable.clEnqueueMarker = (cl_api_clEnqueueMarker)GET_ALIAS(clEnqueueMarker);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (cl_api_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);
    ICDDispatchTable.clEnqueueBarrierWithWaitList = (cl_api_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clEnqueueWaitForEvents = (cl_api_clEnqueueWaitForEvents)GET_ALIAS(clEnqueueWaitForEvents);
    ICDDispatchTable.clEnqueueBarrier = (cl_api_clEnqueueBarrier)GET_ALIAS(clEnqueueBarrier);
    ICDDispatchTable.clGetExtensionFunctionAddress = (cl_api_clGetExtensionFunctionAddress)GET_ALIAS(clGetExtensionFunctionAddress);
    ICDDispatchTable.clGetExtensionFunctionAddressForPlatform = (cl_api_clGetExtensionFunctionAddressForPlatform)GET_ALIAS(clGetExtensionFunctionAddressForPlatform);
    ICDDispatchTable.clCreateFromGLBuffer = nullptr;
    ICDDispatchTable.clCreateFromGLTexture = nullptr;
    ICDDispatchTable.clCreateFromGLTexture2D = nullptr;
    ICDDispatchTable.clCreateFromGLTexture3D = nullptr;
    ICDDispatchTable.clCreateFromGLRenderbuffer = nullptr;
    ICDDispatchTable.clGetGLObjectInfo = nullptr;
    ICDDispatchTable.clGetGLTextureInfo = nullptr;
    ICDDispatchTable.clEnqueueAcquireGLObjects = nullptr;
    ICDDispatchTable.clEnqueueReleaseGLObjects = nullptr;
    ICDDispatchTable.clGetGLContextInfoKHR = nullptr;
    ICDDispatchTable.clGetDeviceIDsFromD3D10KHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10BufferKHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10Texture2DKHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10Texture3DKHR = nullptr;
    ICDDispatchTable.clEnqueueAcquireD3D10ObjectsKHR = nullptr;
    ICDDispatchTable.clEnqueueReleaseD3D10ObjectsKHR = nullptr;
    ICDDispatchTable.clSetEventCallback = (cl_api_clSetEventCallback)GET_ALIAS(clSetEventCallback);
    ICDDispatchTable.clCreateSubBuffer = (cl_api_clCreateSubBuffer)GET_ALIAS(clCreateSubBuffer);
    ICDDispatchTable.clSetMemObjectDestructorCallback = (cl_api_clSetMemObjectDestructorCallback)GET_ALIAS(clSetMemObjectDestructorCallback);
    ICDDispatchTable.clCreateUserEvent = (cl_api_clCreateUserEvent)GET_ALIAS(clCreateUserEvent);
    ICDDispatchTable.clSetUserEventStatus = (cl_api_clSetUserEventStatus)GET_ALIAS(clSetUserEventStatus);
    ICDDispatchTable.clEnqueueReadBufferRect = (cl_api_clEnqueueReadBufferRect)GET_ALIAS(clEnqueueReadBufferRect);
    ICDDispatchTable.clEnqueueWriteBufferRect = (cl_api_clEnqueueWriteBufferRect)GET_ALIAS(clEnqueueWriteBufferRect);
    ICDDispatchTable.clEnqueueCopyBufferRect = (cl_api_clEnqueueCopyBufferRect)GET_ALIAS(clEnqueueCopyBufferRect);
    ICDDispatchTable.clEnqueueMigrateMemObjects = (cl_api_clEnqueueMigrateMemObjects)GET_ALIAS(clEnqueueMigrateMemObjects);
    ICDDispatchTable.clCreateSubDevices = (cl_api_clCreateSubDevices)GET_ALIAS(clCreateSubDevices);
    ICDDispatchTable.clRetainDevice = (cl_api_clRetainDevice)GET_ALIAS(clRetainDevice);
    ICDDispatchTable.clReleaseDevice = (cl_api_clReleaseDevice)GET_ALIAS(clReleaseDevice);
    ICDDispatchTable.clGetKernelArgInfo = (cl_api_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);

    ICDDispatchTable.clEnqueueBarrierWithWaitList = (cl_api_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clCompileProgram = (cl_api_clCompileProgram)GET_ALIAS(clCompileProgram);
    ICDDispatchTable.clLinkProgram = (cl_api_clLinkProgram)GET_ALIAS(clLinkProgram);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (cl_api_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);

    ICDDispatchTable.clSVMAlloc = (cl_api_clSVMAlloc)GET_ALIAS(clSVMAlloc);
    ICDDispatchTable.clSVMFree = (cl_api_clSVMFree)GET_ALIAS(clSVMFree);
    ICDDispatchTable.clEnqueueSVMFree = (cl_api_clEnqueueSVMFree)GET_ALIAS(clEnqueueSVMFree);
    ICDDispatchTable.clEnqueueSVMMemcpy = (cl_api_clEnqueueSVMMemcpy)GET_ALIAS(clEnqueueSVMMemcpy);
    ICDDispatchTable.clEnqueueSVMMemFill = (cl_api_clEnqueueSVMMemFill)GET_ALIAS(clEnqueueSVMMemFill);
    ICDDispatchTable.clEnqueueSVMMap = (cl_api_clEnqueueSVMMap)GET_ALIAS(clEnqueueSVMMap);
    ICDDispatchTable.clEnqueueSVMMigrateMem = (cl_api_clEnqueueSVMMigrateMem)GET_ALIAS(clEnqueueSVMMigrateMem);
    ICDDispatchTable.clEnqueueSVMUnmap = (cl_api_clEnqueueSVMUnmap)GET_ALIAS(clEnqueueSVMUnmap);
    ICDDispatchTable.clSetKernelArgSVMPointer = (cl_api_clSetKernelArgSVMPointer)GET_ALIAS(clSetKernelArgSVMPointer);
    ICDDispatchTable.clSetKernelExecInfo = (cl_api_clSetKernelExecInfo)GET_ALIAS(clSetKernelExecInfo);

    ICDDispatchTable.clCreatePipe = (cl_api_clCreatePipe)GET_ALIAS(clCreatePipe);
    ICDDispatchTable.clGetPipeInfo = (cl_api_clGetPipeInfo)GET_ALIAS(clGetPipeInfo);

    ICDDispatchTable.clSetProgramSpecializationConstant = (cl_api_clSetProgramSpecializationConstant)GET_ALIAS(clSetProgramSpecializationConstant);

    /// Extra functions for Common Runtime
    CRTDispatchTable.clGetKernelArgInfo = (cl_api_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);
    CRTDispatchTable.clGetDeviceIDsFromDX9INTEL = nullptr;
    CRTDispatchTable.clCreateFromDX9MediaSurfaceINTEL = nullptr;
    CRTDispatchTable.clEnqueueAcquireDX9ObjectsINTEL = nullptr;
    CRTDispatchTable.clEnqueueReleaseDX9ObjectsINTEL = nullptr;

    ICDDispatchTable.clGetDeviceIDsFromDX9MediaAdapterKHR = nullptr;
    ICDDispatchTable.clCreateFromDX9MediaSurfaceKHR       = nullptr;
    ICDDispatchTable.clEnqueueAcquireDX9MediaSurfacesKHR  = nullptr;
    ICDDispatchTable.clEnqueueReleaseDX9MediaSurfacesKHR  = nullptr;

    ICDDispatchTable.clGetDeviceIDsFromD3D11KHR           = nullptr;
    ICDDispatchTable.clCreateFromD3D11BufferKHR           = nullptr;
    ICDDispatchTable.clCreateFromD3D11Texture2DKHR        = nullptr;
    ICDDispatchTable.clCreateFromD3D11Texture3DKHR        = nullptr;
    ICDDispatchTable.clEnqueueAcquireD3D11ObjectsKHR      = nullptr;
    ICDDispatchTable.clEnqueueReleaseD3D11ObjectsKHR      = nullptr;
    // Nullify entries which are not relevant for CPU
    CRTDispatchTable.clGetImageParamsINTEL = nullptr;
    CRTDispatchTable.clCreatePerfCountersCommandQueueINTEL = nullptr;
    CRTDispatchTable.clCreateAcceleratorINTEL = nullptr;
    CRTDispatchTable.clGetAcceleratorInfoINTEL = nullptr;
    CRTDispatchTable.clRetainAcceleratorINTEL = nullptr;
    CRTDispatchTable.clReleaseAcceleratorINTEL = nullptr;
    CRTDispatchTable.clCreateProfiledProgramWithSourceINTEL = nullptr;
    CRTDispatchTable.clCreateKernelProfilingJournalINTEL = nullptr;
    CRTDispatchTable.clCreateFromVAMediaSurfaceINTEL = nullptr;
    CRTDispatchTable.clGetDeviceIDsFromVAMediaAdapterINTEL = nullptr;
    CRTDispatchTable.clEnqueueReleaseVAMediaSurfacesINTEL = nullptr;
    CRTDispatchTable.clEnqueueAcquireVAMediaSurfacesINTEL = nullptr;
    CRTDispatchTable.clCreatePipeINTEL = (INTELpfn_clCreatePipeINTEL)GET_ALIAS(clCreatePipeINTEL);
    CRTDispatchTable.clSetDebugVariableINTEL = nullptr;
    CRTDispatchTable.clSetAcceleratorInfoINTEL = nullptr;

    /// Extra CPU specific functions
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Initialize()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Initialize()
{

    // Initialize entry points table
    InitOCLEntryPoints();

    // initialize configuration file
    m_pConfig = new OCLConfig();
    if (nullptr == m_pConfig)
    {
        //Todo: terrible crash imminent
        return;
    }
    m_pConfig->Initialize(GetConfigFilePath());

    bool bUseLogger = m_pConfig->UseLogger();
    if (bUseLogger)
    {
        string str = m_pConfig->GetLogFile();
        if (str != "")
        {
            // Construct file name with process ID
            // Search for file extension
            size_t ext = str.rfind(".");
            if ( string::npos == ext )
            {
                // If "." not found -> no extension
                ext = str.length();
            }
            // Add Process if before the "."
            //Calculate Extension lenght
            std::string procId;
            const unsigned int pid_length = 16;
            procId.resize(pid_length);
            SPRINTF_S(&procId[0], pid_length, "_%d", GetProcessId());
            procId.resize(strlen(&procId[0]));
            str.insert(ext, procId);

            // Prepare log title
            char     strProcName[MAX_PATH];
            GetProcessName(strProcName, MAX_PATH);
            std::string title = "---------------------------------> ";
            title += strProcName;
            title += " <-----------------------------------\n";

            // initialise logger
            m_pFileLogHandler = new FileLogHandler(TEXT("cl_framework"));
            cl_err_code clErrRet = m_pFileLogHandler->Init(LL_DEBUG, str.c_str(), title.c_str());
            if (CL_SUCCEEDED(clErrRet))
            {
                Logger::GetInstance().AddLogHandler(m_pFileLogHandler);
            }
        }
    }
    Logger::GetInstance().SetActive(bUseLogger);

    INIT_LOGGER_CLIENT(TEXT("FrameworkProxy"), LL_DEBUG);
#if defined(USE_ITT)
	m_GPAData.bUseGPA = m_pConfig->EnableITT();
	m_GPAData.bEnableAPITracing = m_pConfig->EnableAPITracing();
	m_GPAData.bEnableContextTracing = m_pConfig->EnableContextTracing();
	m_GPAData.cStatusMarkerFlags = 0;
	if (m_GPAData.bUseGPA)
	{
		if (m_pConfig->ShowQueuedMarker())
			m_GPAData.cStatusMarkerFlags |= ITT_SHOW_QUEUED_MARKER;
		if (m_pConfig->ShowSubmittedMarker())
			m_GPAData.cStatusMarkerFlags |= ITT_SHOW_SUBMITTED_MARKER;
		if (m_pConfig->ShowRunningMarker())
			m_GPAData.cStatusMarkerFlags |= ITT_SHOW_RUNNING_MARKER;
		if (m_pConfig->ShowCompletedMarker())
			m_GPAData.cStatusMarkerFlags |= ITT_SHOW_COMPLETED_MARKER;

		// Create domains
		m_GPAData.pDeviceDomain = __itt_domain_create("OpenCL.Device");
		m_GPAData.pAPIDomain = __itt_domain_create("OpenCL.API");

		#if defined(USE_GPA)
		if (m_GPAData.bEnableContextTracing)
		{
			m_GPAData.pContextDomain = __itt_domain_create("OpenCL.Context");
			
			// Create Context task group
			__itt_string_handle* pContextTrackGroupHandle = __itt_string_handle_create("Context Track Group");
			m_GPAData.pContextTrackGroup = __itt_track_group_create(pContextTrackGroupHandle, __itt_track_group_type_normal);

            // Create task states
            assert(m_GPAData.pContextDomain && "m_GPAData.pContextDomain is nullptr");
            m_GPAData.pWaitingTaskState = __ittx_task_state_create(m_GPAData.pContextDomain, "OpenCL Waiting");
            m_GPAData.pRunningTaskState = __ittx_task_state_create(m_GPAData.pContextDomain, "OpenCL Running");
        }
        #endif

        m_GPAData.pNDRangeHandle = __itt_string_handle_create("NDRange");
		m_GPAData.pReadHandle = __itt_string_handle_create("Read MemoryObject");
		m_GPAData.pWriteHandle = __itt_string_handle_create("Write MemoryObject");
		m_GPAData.pCopyHandle = __itt_string_handle_create("Copy MemoryObject");
		m_GPAData.pFillHandle = __itt_string_handle_create("Fill MemoryObject");
		m_GPAData.pMapHandle = __itt_string_handle_create("Map MemoryObject");
		m_GPAData.pUnmapHandle = __itt_string_handle_create("Unmap MemoryObject");
		m_GPAData.pSyncDataHandle = __itt_string_handle_create("Sync Data");
		m_GPAData.pSizeHandle = __itt_string_handle_create("Size W/H/D");
		m_GPAData.pWorkGroupSizeHandle = __itt_string_handle_create("Work Group Size");
		m_GPAData.pNumberOfWorkGroupsHandle = __itt_string_handle_create("Number of Work Groups");
		m_GPAData.pWorkGroupRangeHandle = __itt_string_handle_create("Work Group Range");
		m_GPAData.pMarkerHandle = __itt_string_handle_create("Marker");
		m_GPAData.pWorkDimensionHandle = __itt_string_handle_create("Work Dimension");
		m_GPAData.pGlobalWorkSizeHandle = __itt_string_handle_create("Global Work Size W/H/D");
		m_GPAData.pLocalWorkSizeHandle = __itt_string_handle_create("Local Work Size W/H/D");
		m_GPAData.pGlobalWorkOffsetHandle = __itt_string_handle_create("Global Work Offset");

        m_GPAData.pStartPos    = __itt_string_handle_create("Start W/H/D");
        m_GPAData.pEndPos      = __itt_string_handle_create("End W/H/D");

        m_GPAData.pIsBlocking = __itt_string_handle_create("Blocking");
        m_GPAData.pNumEventsInWaitList = __itt_string_handle_create("#Events in Wait List");
    }
#endif // ITT
	
	LOG_INFO(TEXT("%s"), "Initialize platform module: m_PlatformModule = new PlatformModule()");
	m_pPlatformModule = new PlatformModule();
	m_pPlatformModule->Initialize(&OclEntryPoints, m_pConfig, &m_GPAData);

    LOG_INFO(TEXT("Initialize context module: m_pContextModule = new ContextModule(%d)"),m_pPlatformModule);
    m_pContextModule = new ContextModule(m_pPlatformModule);
    m_pContextModule->Initialize(&OclEntryPoints, &m_GPAData);

    LOG_INFO(TEXT("Initialize context module: m_pExecutionModule = new ExecutionModule(%d,%d)"), m_pPlatformModule, m_pContextModule);
    m_pExecutionModule = new ExecutionModule(m_pPlatformModule, m_pContextModule);
    m_pExecutionModule->Initialize(&OclEntryPoints, m_pConfig, &m_GPAData);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::NeedToDisableAPIsAtShutdown()
///////////////////////////////////////////////////////////////////////////////////////////////////
bool FrameworkProxy::NeedToDisableAPIsAtShutdown() const
{
    // On Windows OS kills all threads at shutdown except of one that is used to
    // call atexit() and DllMain(). As all thread killing is done when threads
    // are in an arbitrary state we cannot assume that they are not owning some
    // lock or that they freed their per-thread resources. As our OpenCL
    // implementation objects lifetime is based on reference counted objects we
    // cannot assume that performing normal shutdown will not block or will free
    // resources. So on Windows we should just block our external APIs to avoid
    // global object destructors from DLLs to enter our OpenCL DLLs.
    //
    // On FPGA emulator we should not kill contexts, execution modules, etc.
    // because program can contain `while (true)` kernels which can not be
    // finished using regular finish operation on command queue.
    //
    // On Linux all threads are alive and fully functional at atexit() time - so
    // full shutdown is possible.
    //
    // The shutdown mechanism is disabled for all configurations now.
    // The functionality provides is not required by the OpenCL specification
    // and there is no known customer request for it. But it leads to the
    // problems in real application quite often due to problems in the mechanism
    // itself. For example, some applications just hang at exit instead of
    // finishing with leaks. Some crashes due to bugs. Those problems are very
    // hard to debug because environment the logic works in is very specific -
    // multi-thread multi-library process shutdown.

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Destroy()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Destroy() {
#ifdef _WIN32
  // Disable it on linux to avoid conflict with atexit() callback.
  if (Instance()->NeedToDisableAPIsAtShutdown()) {
    // If this function is being called during process shutdown AND we
    // should just disable external APIs. Do not delete or release
    // anything as it may cause a deadlock.
    if (TERMINATED != gGlobalState)
      Instance()->Release(true);
  } else
    Instance()->Release(true);
#else
  // FIXME: Now sycl shutdown process is executed after ocl so that the ocl
  // resources will not be released indeed. This is a workaround to make sure
  // that the user's programs are finally released.
  Instance()->m_pContextModule->Release(true);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Release(bool bTerminate)
{
    if (TERMINATED != gGlobalState)
    {
        // Many modules assume that FrameWorkProxy singleton, execution_module, context_module and platform_module
        // exist all the time -> we must ensure that everything is shut down before deleting them.
        Instance()->m_pContextModule->ShutDown(true);
    }

    if (nullptr != m_pExecutionModule)
    {
        m_pExecutionModule->Release(bTerminate);
        delete m_pExecutionModule;
    }

    if (nullptr != m_pContextModule)
    {
        m_pContextModule->Release(bTerminate);
        delete m_pContextModule;
    }
    
    if (nullptr != m_pPlatformModule)
    {
        m_pPlatformModule->Release(bTerminate);
        delete m_pPlatformModule;
    }

    if (!bTerminate && (nullptr != m_pTaskList))
    {
        // looks like this is the normal deletion - force root device deletion
        m_uiTEActivationCount = 1;
        DeactivateTaskExecutor();
    }
    else
    {
        // TaskExecutor is managed inside it's own DLL and may be already deleted at this point
        // we should avoid deletion of root device here - leave one extra counter
        m_pTaskList     = nullptr;
    }
    m_pTaskExecutor = nullptr;
    
    if (nullptr != m_pFileLogHandler)
    {
        m_pFileLogHandler->Flush();
        delete m_pFileLogHandler;
        m_pFileLogHandler = nullptr;
    }
    if (nullptr != m_pConfig)
    {
        m_pConfig->Release();
        delete m_pConfig;
        m_pConfig = nullptr;
    }
    cl_monitor_summary;
}

void FrameworkProxy::RegisterDllCallback( at_exit_dll_callback_fn cb )
{
    if (nullptr != cb)
    {
        OclAutoMutex cs(&m_initializationMutex);
        m_at_exit_cbs.insert(cb);
    }
}

void FrameworkProxy::UnregisterDllCallback( at_exit_dll_callback_fn cb )
{
    if (nullptr != cb)
    {
        OclAutoMutex cs(&m_initializationMutex);
        m_at_exit_cbs.erase(cb);
    }
}

void FrameworkProxy::AtExitTrigger( at_exit_dll_callback_fn cb )
{
  bool needToDisableAPI = Instance()->NeedToDisableAPIsAtShutdown();

  if (isDllUnloadingState()) {
    UnregisterDllCallback(cb);
    cb(AT_EXIT_GLB_PROCESSING_STARTED, AT_EXIT_DLL_UNLOADING_MODE,
       needToDisableAPI);
    cb(AT_EXIT_GLB_PROCESSING_DONE, AT_EXIT_DLL_UNLOADING_MODE,
       needToDisableAPI);
    }
    else
    {
        TerminateProcess(needToDisableAPI);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::TerminateProcess(bool needToDisableAPI)
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::TerminateProcess(bool needToDisableAPI)
{
    if (WORKING != gGlobalState)
    {
        return;
    }
    
    // references to other DLLs are not safe on Linux at exit 

    // normal shutdown
    gGlobalState = TERMINATING;

    // notify all DLLs that at_exit started 
    {
// The following comment applicable for Windows only.
// Locking this mutex may lead to deadlock due to any other thread
// may acquire mutex and die(killed by OS) without freeing.
// Anyway using mutex during process shutdown is meaningless
// because of there is only one thread is alive.
#ifndef _WIN32
        OclAutoMutex cs(&m_initializationMutex);
#endif // _WIN32
        for (auto it : m_at_exit_cbs)
        {
            at_exit_dll_callback_fn cb = *it;
            cb(AT_EXIT_GLB_PROCESSING_STARTED, AT_EXIT_PROCESS_UNLOADING_MODE,
               needToDisableAPI);
        }
    }

    if (!Instance()->NeedToDisableAPIsAtShutdown())
      Instance()->m_pContextModule->ShutDown(true);

    gGlobalState = TERMINATED;

    // notify all DLLs that at_exit occured 
    {
#ifndef _WIN32
        OclAutoMutex cs(&m_initializationMutex);
#endif // _WIN32
        for (auto it : m_at_exit_cbs)
        {
            at_exit_dll_callback_fn cb = *it;
            cb(AT_EXIT_GLB_PROCESSING_DONE, AT_EXIT_PROCESS_UNLOADING_MODE,
               needToDisableAPI);
        }
        m_at_exit_cbs.clear();
    }

#if defined(_DEBUG)
    if (!Instance()->NeedToDisableAPIsAtShutdown()) {
      DumpSharedPts("TerminateProcess - only SharedPtrs local to intelocl DLL",
                    true);
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Instance()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy *FrameworkProxy::Instance() {
  static FrameworkProxy S;
  return &S;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::GetTaskExecutor()
///////////////////////////////////////////////////////////////////////////////////////////////////
Intel::OpenCL::TaskExecutor::ITaskExecutor*  FrameworkProxy::GetTaskExecutor() const
{
  // teInitialize > 0 means task executor is initialized successfully.
  // teInitialize == 0 means task executor is not initialized succcessfully.
  static int teInitialized = 1;
  if (nullptr == m_pTaskExecutor && 0 != teInitialized) {
    // Initialize TaskExecutor
    OclAutoMutex cs(&m_initializationMutex);
    if (nullptr == m_pTaskExecutor && 0 != teInitialized) {
      LOG_INFO(TEXT("%s"), "Initialize Executor");
      m_pTaskExecutor = TaskExecutor::GetTaskExecutor();
      assert(m_pTaskExecutor);
      auto deviceMode = m_pConfig->GetDeviceMode();
      if (m_pConfig->UseAutoMemory()) {
        teInitialized = m_pTaskExecutor->Init(
            m_pConfig->GetNumTBBWorkers(), &m_GPAData,
            m_pConfig->GetStackDefaultSize(), deviceMode);
      } else {
        // Here we pass value of CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE and
        // CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE env variables
        // as required stack for executor threads because local/private
        // variables are located on stack.
        size_t stackSize = m_pConfig->GetForcedLocalMemSize() +
                           m_pConfig->GetForcedPrivateMemSize() +
                           CPU_DEV_BASE_STACK_SIZE;
        teInitialized =
            m_pTaskExecutor->Init(m_pConfig->GetNumTBBWorkers(),
                                  &m_GPAData, stackSize, deviceMode);
      }
    }
  }

  if (0 == teInitialized)
    return nullptr;

  return m_pTaskExecutor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::ActivateTaskExecutor()
///////////////////////////////////////////////////////////////////////////////////////////////////
bool FrameworkProxy::ActivateTaskExecutor() const
{
    ITaskExecutor* pTaskExecutor = GetTaskExecutor();

    OclAutoMutex cs(&m_initializationMutex);
    // Quit as early as possible if task executor initialization fails.
    if (nullptr == pTaskExecutor)
      return false;

    if (nullptr == m_pTaskList)
    {
        // During shutdown task_executor dll may finish before current dll and destroy all internal objects
        // We can discover this case but we cannot access any task_executor object at that time point because
        // it may be already destroyed. As SharedPtr accesses the object itself to manage counters, we cannot use
        // SharedPointers at all.

        // create root device in flat mode. Use all available HW threads 
        // and allow non-worker threads to participate in execution but do not assume they will join.
        SharedPtr<ITEDevice> pTERootDevice = pTaskExecutor->CreateRootDevice(
                    RootDeviceCreationParam(TE_AUTO_THREADS, TE_ENABLE_MASTERS_JOIN, 1));

        SharedPtr<ITaskList> pTaskList;
        SharedPtr<ITaskList> pTaskList_Immediate;

        if (0 != pTERootDevice)
        {
            pTaskList           = pTERootDevice->CreateTaskList( TE_CMD_LIST_IN_ORDER  );
            pTaskList_Immediate = pTERootDevice->CreateTaskList( TE_CMD_LIST_IMMEDIATE );
        }

        if (0 != pTaskList && 0 != pTaskList_Immediate)
        {
            m_pTaskList = pTaskList.GetPtr();
            m_pTaskList->IncRefCnt();

            m_pTaskList_immediate = pTaskList_Immediate.GetPtr();
            m_pTaskList_immediate->IncRefCnt();
        }
    }

    if (nullptr != m_pTaskList && nullptr != m_pTaskList_immediate)
    {
        ++m_uiTEActivationCount;
        return true;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::ActivateTaskExecutor()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::DeactivateTaskExecutor() const
{
    if (TERMINATED == gGlobalState)
    {
        return;
    }
    
    OclAutoMutex cs(&m_initializationMutex);

    if (nullptr != m_pTaskList && nullptr != m_pTaskList_immediate)
    {
        --m_uiTEActivationCount;

        if (0 == m_uiTEActivationCount)
        {
// This is disabled due to shutdown issue and will be fixed by CMPLRLLVM-20324
#if 0
            // this is the normal deletion - undo the counting here to delete the object
            long ref = m_pTaskList->DecRefCnt();
            if ( 0 == ref )
            {
                m_pTaskList->Cleanup();
            }
            m_pTaskList = nullptr;

            ref = m_pTaskList_immediate->DecRefCnt();
            if ( 0 == ref )
            {
                m_pTaskList_immediate->Cleanup();
            }
            m_pTaskList_immediate = nullptr;
#endif
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::ExecuteImmediate()
///////////////////////////////////////////////////////////////////////////////////////////////////
bool FrameworkProxy::ExecuteImmediate(const Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>& pTask) const
{
    assert(m_pTaskList_immediate);
    if (nullptr == m_pTaskList_immediate)
    {
        return false;
    }

    m_pTaskList_immediate->Enqueue(pTask);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Execute()
///////////////////////////////////////////////////////////////////////////////////////////////////
bool FrameworkProxy::Execute(const Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>& pTask) const
{
    if (nullptr == m_pTaskList)
    {
        return false;
    }

    m_pTaskList->Enqueue(pTask);
    m_pTaskList->Flush();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Execute()
///////////////////////////////////////////////////////////////////////////////////////////////////
void  FrameworkProxy::CancelAllTasks(bool wait_for_finish) const
{
    m_initializationMutex.Lock();
    SharedPtr<ITaskList> tmpTaskList = m_pTaskList;
    m_initializationMutex.Unlock();

    if (0 != tmpTaskList)
    {
        tmpTaskList->Cancel();
        if (wait_for_finish)
        {
            tmpTaskList->WaitForCompletion(nullptr);
        }
    }
}

