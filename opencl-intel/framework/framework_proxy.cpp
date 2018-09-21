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

KHRicdVendorDispatch        FrameworkProxy::ICDDispatchTable;
SOCLCRTDispatchTable        FrameworkProxy::CRTDispatchTable;
ocl_entry_points            FrameworkProxy::OclEntryPoints;


FrameworkProxy * FrameworkProxy::m_pInstance = nullptr;
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
    ICDDispatchTable.clGetPlatformIDs = (KHRpfn_clGetPlatformIDs)GET_ALIAS(clGetPlatformIDs);
    ICDDispatchTable.clGetPlatformInfo = (KHRpfn_clGetPlatformInfo)GET_ALIAS(clGetPlatformInfo);
    ICDDispatchTable.clGetDeviceIDs = (KHRpfn_clGetDeviceIDs)GET_ALIAS(clGetDeviceIDs);
    ICDDispatchTable.clGetDeviceInfo = (KHRpfn_clGetDeviceInfo)GET_ALIAS(clGetDeviceInfo);
    ICDDispatchTable.clCreateContext = (KHRpfn_clCreateContext)GET_ALIAS(clCreateContext);
    ICDDispatchTable.clCreateContextFromType = (KHRpfn_clCreateContextFromType)GET_ALIAS(clCreateContextFromType);
    ICDDispatchTable.clRetainContext = (KHRpfn_clRetainContext)GET_ALIAS(clRetainContext);
    ICDDispatchTable.clReleaseContext = (KHRpfn_clReleaseContext)GET_ALIAS(clReleaseContext);
    ICDDispatchTable.clGetContextInfo = (KHRpfn_clGetContextInfo)GET_ALIAS(clGetContextInfo);
    ICDDispatchTable.clCreateCommandQueue = (KHRpfn_clCreateCommandQueue)GET_ALIAS(clCreateCommandQueue);
	ICDDispatchTable.clCreateCommandQueueWithProperties = (KHRpfn_clCreateCommandQueueWithProperties)GET_ALIAS(clCreateCommandQueueWithProperties);
    ICDDispatchTable.clRetainCommandQueue = (KHRpfn_clRetainCommandQueue)GET_ALIAS(clRetainCommandQueue);
    ICDDispatchTable.clReleaseCommandQueue = (KHRpfn_clReleaseCommandQueue)GET_ALIAS(clReleaseCommandQueue);
    ICDDispatchTable.clGetCommandQueueInfo = (KHRpfn_clGetCommandQueueInfo)GET_ALIAS(clGetCommandQueueInfo);
    ICDDispatchTable.clSetCommandQueueProperty = nullptr;
    ICDDispatchTable.clCreateBuffer = (KHRpfn_clCreateBuffer)GET_ALIAS(clCreateBuffer);
    ICDDispatchTable.clCreateImage = (KHRpfn_clCreateImage)GET_ALIAS(clCreateImage);
    ICDDispatchTable.clCreateImage2D = (KHRpfn_clCreateImage2D)GET_ALIAS(clCreateImage2D);
    ICDDispatchTable.clCreateImage3D = (KHRpfn_clCreateImage3D)GET_ALIAS(clCreateImage3D);
    ICDDispatchTable.clRetainMemObject = (KHRpfn_clRetainMemObject)GET_ALIAS(clRetainMemObject);
    ICDDispatchTable.clReleaseMemObject = (KHRpfn_clReleaseMemObject)GET_ALIAS(clReleaseMemObject);
    ICDDispatchTable.clGetSupportedImageFormats = (KHRpfn_clGetSupportedImageFormats)GET_ALIAS(clGetSupportedImageFormats);
    ICDDispatchTable.clGetMemObjectInfo = (KHRpfn_clGetMemObjectInfo)GET_ALIAS(clGetMemObjectInfo);
    ICDDispatchTable.clGetImageInfo = (KHRpfn_clGetImageInfo)GET_ALIAS(clGetImageInfo);
    ICDDispatchTable.clCreateSampler = (KHRpfn_clCreateSampler)GET_ALIAS(clCreateSampler);
	ICDDispatchTable.clCreateSamplerWithProperties = (KHRpfn_clCreateSamplerWithProperties)GET_ALIAS(clCreateSamplerWithProperties);
    ICDDispatchTable.clRetainSampler = (KHRpfn_clRetainSampler)GET_ALIAS(clRetainSampler);
    ICDDispatchTable.clReleaseSampler = (KHRpfn_clReleaseSampler)GET_ALIAS(clReleaseSampler);
    ICDDispatchTable.clGetSamplerInfo = (KHRpfn_clGetSamplerInfo)GET_ALIAS(clGetSamplerInfo);
    ICDDispatchTable.clCreateProgramWithSource = (KHRpfn_clCreateProgramWithSource)GET_ALIAS(clCreateProgramWithSource);
    ICDDispatchTable.clSetDefaultDeviceCommandQueue = (KHRpfn_clSetDefaultDeviceCommandQueue)GET_ALIAS(clSetDefaultDeviceCommandQueue);
    ICDDispatchTable.clCreateProgramWithBinary = (KHRpfn_clCreateProgramWithBinary)GET_ALIAS(clCreateProgramWithBinary);
    ICDDispatchTable.clCreateProgramWithBuiltInKernels = (KHRpfn_clCreateProgramWithBuiltInKernels)GET_ALIAS(clCreateProgramWithBuiltInKernels);
    ICDDispatchTable.clCreateProgramWithIL = (KHRpfn_clCreateProgramWithIL)GET_ALIAS(clCreateProgramWithIL);	
    ICDDispatchTable.clRetainProgram = (KHRpfn_clRetainProgram)GET_ALIAS(clRetainProgram);
    ICDDispatchTable.clReleaseProgram = (KHRpfn_clReleaseProgram)GET_ALIAS(clReleaseProgram);
    ICDDispatchTable.clBuildProgram = (KHRpfn_clBuildProgram)GET_ALIAS(clBuildProgram);
    ICDDispatchTable.clCompileProgram = (KHRpfn_clCompileProgram)GET_ALIAS(clCompileProgram);
    ICDDispatchTable.clLinkProgram = (KHRpfn_clLinkProgram)GET_ALIAS(clLinkProgram);
    ICDDispatchTable.clUnloadCompiler = (KHRpfn_clUnloadCompiler)GET_ALIAS(clUnloadCompiler);
    ICDDispatchTable.clUnloadPlatformCompiler = (KHRpfn_clUnloadPlatformCompiler)GET_ALIAS(clUnloadPlatformCompiler);
    ICDDispatchTable.clGetProgramInfo = (KHRpfn_clGetProgramInfo)GET_ALIAS(clGetProgramInfo);
    ICDDispatchTable.clGetProgramBuildInfo = (KHRpfn_clGetProgramBuildInfo)GET_ALIAS(clGetProgramBuildInfo);
    ICDDispatchTable.clCreateKernel = (KHRpfn_clCreateKernel)GET_ALIAS(clCreateKernel);
    ICDDispatchTable.clCreateKernelsInProgram = (KHRpfn_clCreateKernelsInProgram)GET_ALIAS(clCreateKernelsInProgram);
    ICDDispatchTable.clRetainKernel = (KHRpfn_clRetainKernel)GET_ALIAS(clRetainKernel);
    ICDDispatchTable.clReleaseKernel = (KHRpfn_clReleaseKernel)GET_ALIAS(clReleaseKernel);
    ICDDispatchTable.clSetKernelArg = (KHRpfn_clSetKernelArg)GET_ALIAS(clSetKernelArg);
    ICDDispatchTable.clGetKernelInfo = (KHRpfn_clGetKernelInfo)GET_ALIAS(clGetKernelInfo);
    ICDDispatchTable.clCloneKernel = (KHRpfn_clCloneKernel)GET_ALIAS(clCloneKernel);
    ICDDispatchTable.clGetHostTimer = (KHRpfn_clGetHostTimer)GET_ALIAS(clGetHostTimer);
    ICDDispatchTable.clGetDeviceAndHostTimer = (KHRpfn_clGetDeviceAndHostTimer)GET_ALIAS(clGetDeviceAndHostTimer);
    ICDDispatchTable.clGetKernelWorkGroupInfo = (KHRpfn_clGetKernelWorkGroupInfo)GET_ALIAS(clGetKernelWorkGroupInfo);
    ICDDispatchTable.clGetKernelSubGroupInfo = (KHRpfn_clGetKernelSubGroupInfo)GET_ALIAS(clGetKernelSubGroupInfo);
    ICDDispatchTable.clWaitForEvents = (KHRpfn_clWaitForEvents)GET_ALIAS(clWaitForEvents);
    ICDDispatchTable.clGetEventInfo = (KHRpfn_clGetEventInfo)GET_ALIAS(clGetEventInfo);
    ICDDispatchTable.clRetainEvent = (KHRpfn_clRetainEvent)GET_ALIAS(clRetainEvent);
    ICDDispatchTable.clReleaseEvent = (KHRpfn_clReleaseEvent)GET_ALIAS(clReleaseEvent);
    ICDDispatchTable.clGetEventProfilingInfo = (KHRpfn_clGetEventProfilingInfo)GET_ALIAS(clGetEventProfilingInfo);
    ICDDispatchTable.clFlush = (KHRpfn_clFlush)GET_ALIAS(clFlush);
    ICDDispatchTable.clFinish = (KHRpfn_clFinish)GET_ALIAS(clFinish);
    ICDDispatchTable.clEnqueueReadBuffer = (KHRpfn_clEnqueueReadBuffer)GET_ALIAS(clEnqueueReadBuffer);
    ICDDispatchTable.clEnqueueWriteBuffer = (KHRpfn_clEnqueueWriteBuffer)GET_ALIAS(clEnqueueWriteBuffer);
    ICDDispatchTable.clEnqueueCopyBuffer = (KHRpfn_clEnqueueCopyBuffer)GET_ALIAS(clEnqueueCopyBuffer);
    ICDDispatchTable.clEnqueueFillBuffer = (KHRpfn_clEnqueueFillBuffer)GET_ALIAS(clEnqueueFillBuffer);
    ICDDispatchTable.clEnqueueFillImage  = (KHRpfn_clEnqueueFillImage)GET_ALIAS(clEnqueueFillImage);
    ICDDispatchTable.clEnqueueReadImage = (KHRpfn_clEnqueueReadImage)GET_ALIAS(clEnqueueReadImage);
    ICDDispatchTable.clEnqueueWriteImage = (KHRpfn_clEnqueueWriteImage)GET_ALIAS(clEnqueueWriteImage);
    ICDDispatchTable.clEnqueueCopyImage = (KHRpfn_clEnqueueCopyImage)GET_ALIAS(clEnqueueCopyImage);
    ICDDispatchTable.clEnqueueCopyImageToBuffer = (KHRpfn_clEnqueueCopyImageToBuffer)GET_ALIAS(clEnqueueCopyImageToBuffer);
    ICDDispatchTable.clEnqueueCopyBufferToImage = (KHRpfn_clEnqueueCopyBufferToImage)GET_ALIAS(clEnqueueCopyBufferToImage);
    ICDDispatchTable.clEnqueueMapBuffer = (KHRpfn_clEnqueueMapBuffer)GET_ALIAS(clEnqueueMapBuffer);
    ICDDispatchTable.clEnqueueMapImage = (KHRpfn_clEnqueueMapImage)GET_ALIAS(clEnqueueMapImage);
    ICDDispatchTable.clEnqueueUnmapMemObject = (KHRpfn_clEnqueueUnmapMemObject)GET_ALIAS(clEnqueueUnmapMemObject);
    ICDDispatchTable.clEnqueueNDRangeKernel = (KHRpfn_clEnqueueNDRangeKernel)GET_ALIAS(clEnqueueNDRangeKernel);
    ICDDispatchTable.clEnqueueTask = (KHRpfn_clEnqueueTask)GET_ALIAS(clEnqueueTask);
    ICDDispatchTable.clEnqueueNativeKernel = (KHRpfn_clEnqueueNativeKernel)GET_ALIAS(clEnqueueNativeKernel);
    ICDDispatchTable.clEnqueueMarker = (KHRpfn_clEnqueueMarker)GET_ALIAS(clEnqueueMarker);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (KHRpfn_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);
    ICDDispatchTable.clEnqueueBarrierWithWaitList = (KHRpfn_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clEnqueueWaitForEvents = (KHRpfn_clEnqueueWaitForEvents)GET_ALIAS(clEnqueueWaitForEvents);
    ICDDispatchTable.clEnqueueBarrier = (KHRpfn_clEnqueueBarrier)GET_ALIAS(clEnqueueBarrier);
    ICDDispatchTable.clGetExtensionFunctionAddress = (KHRpfn_clGetExtensionFunctionAddress)GET_ALIAS(clGetExtensionFunctionAddress);
    ICDDispatchTable.clGetExtensionFunctionAddressForPlatform = (KHRpfn_clGetExtensionFunctionAddressForPlatform)GET_ALIAS(clGetExtensionFunctionAddressForPlatform);
#ifdef WIN32
    ICDDispatchTable.clCreateFromGLBuffer = (KHRpfn_clCreateFromGLBuffer)GET_ALIAS(clCreateFromGLBuffer);
    ICDDispatchTable.clCreateFromGLTexture = (KHRpfn_clCreateFromGLTexture)GET_ALIAS(clCreateFromGLTexture);
    ICDDispatchTable.clCreateFromGLTexture2D = (KHRpfn_clCreateFromGLTexture2D)GET_ALIAS(clCreateFromGLTexture2D);
    ICDDispatchTable.clCreateFromGLTexture3D = (KHRpfn_clCreateFromGLTexture3D)GET_ALIAS(clCreateFromGLTexture3D);
    ICDDispatchTable.clCreateFromGLRenderbuffer = (KHRpfn_clCreateFromGLRenderbuffer)GET_ALIAS(clCreateFromGLRenderbuffer);
    ICDDispatchTable.clGetGLObjectInfo = (KHRpfn_clGetGLObjectInfo)GET_ALIAS(clGetGLObjectInfo);
    ICDDispatchTable.clGetGLTextureInfo = (KHRpfn_clGetGLTextureInfo)GET_ALIAS(clGetGLTextureInfo);
    ICDDispatchTable.clEnqueueAcquireGLObjects = (KHRpfn_clEnqueueAcquireGLObjects)GET_ALIAS(clEnqueueAcquireGLObjects);
    ICDDispatchTable.clEnqueueReleaseGLObjects = (KHRpfn_clEnqueueReleaseGLObjects)GET_ALIAS(clEnqueueReleaseGLObjects);
    ICDDispatchTable.clGetGLContextInfoKHR = (KHRpfn_clGetGLContextInfoKHR)GET_ALIAS(clGetGLContextInfoKHR);
#else
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
#endif
    ICDDispatchTable.clGetDeviceIDsFromD3D10KHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10BufferKHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10Texture2DKHR = nullptr;
    ICDDispatchTable.clCreateFromD3D10Texture3DKHR = nullptr;
    ICDDispatchTable.clEnqueueAcquireD3D10ObjectsKHR = nullptr;
    ICDDispatchTable.clEnqueueReleaseD3D10ObjectsKHR = nullptr;
    ICDDispatchTable.clSetEventCallback = (KHRpfn_clSetEventCallback)GET_ALIAS(clSetEventCallback);
    ICDDispatchTable.clCreateSubBuffer = (KHRpfn_clCreateSubBuffer)GET_ALIAS(clCreateSubBuffer);
    ICDDispatchTable.clSetMemObjectDestructorCallback = (KHRpfn_clSetMemObjectDestructorCallback)GET_ALIAS(clSetMemObjectDestructorCallback);
    ICDDispatchTable.clCreateUserEvent = (KHRpfn_clCreateUserEvent)GET_ALIAS(clCreateUserEvent);
    ICDDispatchTable.clSetUserEventStatus = (KHRpfn_clSetUserEventStatus)GET_ALIAS(clSetUserEventStatus);
    ICDDispatchTable.clEnqueueReadBufferRect = (KHRpfn_clEnqueueReadBufferRect)GET_ALIAS(clEnqueueReadBufferRect);
    ICDDispatchTable.clEnqueueWriteBufferRect = (KHRpfn_clEnqueueWriteBufferRect)GET_ALIAS(clEnqueueWriteBufferRect);
    ICDDispatchTable.clEnqueueCopyBufferRect = (KHRpfn_clEnqueueCopyBufferRect)GET_ALIAS(clEnqueueCopyBufferRect);
    ICDDispatchTable.clEnqueueMigrateMemObjects = (KHRpfn_clEnqueueMigrateMemObjects)GET_ALIAS(clEnqueueMigrateMemObjects);
    ICDDispatchTable.clCreateSubDevices = (KHRpfn_clCreateSubDevices)GET_ALIAS(clCreateSubDevices);
    ICDDispatchTable.clRetainDevice = (KHRpfn_clRetainDevice)GET_ALIAS(clRetainDevice);
    ICDDispatchTable.clReleaseDevice = (KHRpfn_clReleaseDevice)GET_ALIAS(clReleaseDevice);       
    ICDDispatchTable.clGetKernelArgInfo = (KHRpfn_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);    

    ICDDispatchTable.clEnqueueBarrierWithWaitList = (KHRpfn_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clCompileProgram = (KHRpfn_clCompileProgram)GET_ALIAS(clCompileProgram);
    ICDDispatchTable.clLinkProgram = (KHRpfn_clLinkProgram)GET_ALIAS(clLinkProgram);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (KHRpfn_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);
    
	ICDDispatchTable.clSVMAlloc = (KHRpfn_clSVMAlloc)GET_ALIAS(clSVMAlloc);
	ICDDispatchTable.clSVMFree = (KHRpfn_clSVMFree)GET_ALIAS(clSVMFree);
	ICDDispatchTable.clEnqueueSVMFree = (KHRpfn_clEnqueueSVMFree)GET_ALIAS(clEnqueueSVMFree);
	ICDDispatchTable.clEnqueueSVMMemcpy = (KHRpfn_clEnqueueSVMMemcpy)GET_ALIAS(clEnqueueSVMMemcpy);
	ICDDispatchTable.clEnqueueSVMMemFill = (KHRpfn_clEnqueueSVMMemFill)GET_ALIAS(clEnqueueSVMMemFill);
	ICDDispatchTable.clEnqueueSVMMap = (KHRpfn_clEnqueueSVMMap)GET_ALIAS(clEnqueueSVMMap);
	ICDDispatchTable.clEnqueueSVMMigrateMem = (KHRpfn_clEnqueueSVMMigrateMem)GET_ALIAS(clEnqueueSVMMigrateMem);
	ICDDispatchTable.clEnqueueSVMUnmap = (KHRpfn_clEnqueueSVMUnmap)GET_ALIAS(clEnqueueSVMUnmap);
	ICDDispatchTable.clSetKernelArgSVMPointer = (KHRpfn_clSetKernelArgSVMPointer)GET_ALIAS(clSetKernelArgSVMPointer);
	ICDDispatchTable.clSetKernelExecInfo = (KHRpfn_clSetKernelExecInfo)GET_ALIAS(clSetKernelExecInfo);

	ICDDispatchTable.clCreatePipe = (KHRpfn_clCreatePipe)GET_ALIAS(clCreatePipe);
	ICDDispatchTable.clGetPipeInfo = (KHRpfn_clGetPipeInfo)GET_ALIAS(clGetPipeInfo);

	ICDDispatchTable.clMapHostPipeIntelFPGA = (KHRpfn_clMapHostPipeIntelFPGA)GET_ALIAS(clMapHostPipeIntelFPGA);
	ICDDispatchTable.clUnmapHostPipeIntelFPGA = (KHRpfn_clUnmapHostPipeIntelFPGA)GET_ALIAS(clUnmapHostPipeIntelFPGA);
	ICDDispatchTable.clReadPipeIntelFPGA = (KHRpfn_clReadPipeIntelFPGA)GET_ALIAS(clReadPipeIntelFPGA);
	ICDDispatchTable.clWritePipeIntelFPGA = (KHRpfn_clWritePipeIntelFPGA)GET_ALIAS(clWritePipeIntelFPGA);


    /// Extra functions for Common Runtime
    CRTDispatchTable.clGetKernelArgInfo = (KHRpfn_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);
#if defined DX_MEDIA_SHARING
    CRTDispatchTable.clGetDeviceIDsFromDX9INTEL = (INTELpfn_clGetDeviceIDsFromDX9INTEL)GET_ALIAS(clGetDeviceIDsFromDX9INTEL);
    CRTDispatchTable.clCreateFromDX9MediaSurfaceINTEL = (INTELpfn_clCreateFromDX9MediaSurfaceINTEL)GET_ALIAS(clCreateFromDX9MediaSurfaceINTEL);
    CRTDispatchTable.clEnqueueAcquireDX9ObjectsINTEL = (INTELpfn_clEnqueueAcquireDX9ObjectsINTEL)GET_ALIAS(clEnqueueAcquireDX9ObjectsINTEL);
    CRTDispatchTable.clEnqueueReleaseDX9ObjectsINTEL = (INTELpfn_clEnqueueReleaseDX9ObjectsINTEL)GET_ALIAS(clEnqueueReleaseDX9ObjectsINTEL);

    ICDDispatchTable.clGetDeviceIDsFromDX9MediaAdapterKHR = (KHRpfn_clGetDeviceIDsFromDX9MediaAdapterKHR)GET_ALIAS(clGetDeviceIDsFromDX9MediaAdapterKHR);
    ICDDispatchTable.clCreateFromDX9MediaSurfaceKHR = (KHRpfn_clCreateFromDX9MediaSurfaceKHR)GET_ALIAS(clCreateFromDX9MediaSurfaceKHR);
    ICDDispatchTable.clEnqueueAcquireDX9MediaSurfacesKHR = (KHRpfn_clEnqueueAcquireDX9MediaSurfacesKHR)GET_ALIAS(clEnqueueAcquireDX9MediaSurfacesKHR);
    ICDDispatchTable.clEnqueueReleaseDX9MediaSurfacesKHR = (KHRpfn_clEnqueueReleaseDX9MediaSurfacesKHR)GET_ALIAS(clEnqueueReleaseDX9MediaSurfacesKHR);

    ICDDispatchTable.clGetDeviceIDsFromD3D11KHR = (KHRpfn_clGetDeviceIDsFromD3D11KHR)GET_ALIAS(clGetDeviceIDsFromD3D11KHR);
    ICDDispatchTable.clCreateFromD3D11BufferKHR = (KHRpfn_clCreateFromD3D11BufferKHR)GET_ALIAS(clCreateFromD3D11BufferKHR);
    ICDDispatchTable.clCreateFromD3D11Texture2DKHR = (KHRpfn_clCreateFromD3D11Texture2DKHR)GET_ALIAS(clCreateFromD3D11Texture2DKHR);
    ICDDispatchTable.clCreateFromD3D11Texture3DKHR = (KHRpfn_clCreateFromD3D11Texture3DKHR)GET_ALIAS(clCreateFromD3D11Texture3DKHR);
    ICDDispatchTable.clEnqueueAcquireD3D11ObjectsKHR = (KHRpfn_clEnqueueAcquireD3D11ObjectsKHR)GET_ALIAS(clEnqueueAcquireD3D11ObjectsKHR);
    ICDDispatchTable.clEnqueueReleaseD3D11ObjectsKHR = (KHRpfn_clEnqueueReleaseD3D11ObjectsKHR)GET_ALIAS(clEnqueueReleaseD3D11ObjectsKHR);
#else
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
#endif
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
#if defined (_WIN32)
    // On Windows OS kills all threads at shutdown except of one that is used to
    // call atexit() and DllMain(). As all thread killing is done when threads
    // are in an arbitrary state we cannot assume that they are not owning some
    // lock or that they freed their per-thread resources. As our OpenCL
    // implementation objects lifetime is based on reference counted objects we
    // cannot assume that performing normal shutdown will not block or will free
    // resources. So on Windows we should just block our external APIs to avoid
    // global object destructors from DLLs to enter our OpenCL DLLs.
    return true;
#endif
    // On FPGA emulator we should not kill contexts, execution modules, etc.
    // because program can contain `while (true)` kernels which can not be
    // finished using regular finish operation on command queue.
    if (FPGA_EMU_DEVICE == m_pConfig->GetDeviceMode())
    {
        return true;
    }
    // On Linux all threads are alive and fully functional at atexit() time - do
    // the full shutdown.
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Destroy()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Destroy()
{
    if (nullptr != m_pInstance)
    {
        if (m_pInstance->NeedToDisableAPIsAtShutdown())
        {
            // If this function is being called during process shutdown AND we
            // should just disable external APIs. Do not delete or release
            // anything as it may cause a deadlock.
            if (TERMINATED != gGlobalState)
            {
                m_pInstance->Release(true);
                delete m_pInstance;
            }
        }
        else
        {
            m_pInstance->Release(true);
            delete m_pInstance;
        }
        m_pInstance = nullptr;
    }
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
        m_pInstance->m_pContextModule->ShutDown(true);
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
///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::TerminateProcess()
///////////////////////////////////////////////////////////////////////////////////////////////////
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
    if (isDllUnloadingState())
    {
        UnregisterDllCallback( cb );
        cb(AT_EXIT_GLB_PROCESSING_STARTED, AT_EXIT_DLL_UNLOADING_MODE,
            m_pInstance->NeedToDisableAPIsAtShutdown());
        cb(AT_EXIT_GLB_PROCESSING_DONE, AT_EXIT_DLL_UNLOADING_MODE,
            m_pInstance->NeedToDisableAPIsAtShutdown());
    }
    else
    {
        TerminateProcess();
    }
}

void FrameworkProxy::TerminateProcess()
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
               m_pInstance->NeedToDisableAPIsAtShutdown());
        }
    }

    if (nullptr != m_pInstance)
    {
        if (!m_pInstance->NeedToDisableAPIsAtShutdown())
        {
            m_pInstance->m_pContextModule->ShutDown(true);
        }
    }

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
               m_pInstance->NeedToDisableAPIsAtShutdown());
        }
        m_at_exit_cbs.clear();
    }

#if defined(_DEBUG)
    if (!m_pInstance->NeedToDisableAPIsAtShutdown())
    {
        DumpSharedPts("TerminateProcess - only SharedPtrs local to intelocl DLL", true);
    }
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Instance()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy* FrameworkProxy::Instance()
{
    if (nullptr == m_pInstance)
    {
        OclAutoMutex cs(&m_initializationMutex);
        if (nullptr == m_pInstance)
        {
            m_pInstance = new FrameworkProxy();            
        }
    }
    return m_pInstance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::GetTaskExecutor()
///////////////////////////////////////////////////////////////////////////////////////////////////
Intel::OpenCL::TaskExecutor::ITaskExecutor*  FrameworkProxy::GetTaskExecutor() const
{
    if (nullptr == m_pTaskExecutor)
    {
        // Initialize TaskExecutor
        OclAutoMutex cs(&m_initializationMutex);
        if (nullptr == m_pTaskExecutor)
        {
            LOG_INFO(TEXT("%s"), "Initialize Executor");
            m_pTaskExecutor = TaskExecutor::GetTaskExecutor();
            // Here we pass value of CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE env variable
            // as additional required stack for executor threads because
            // local variables located on stack
            size_t additionalStackSize = m_pConfig->GetForcedLocalMemSize();
            additionalStackSize += m_pConfig->GetForcedPrivateMemSize();
            m_pTaskExecutor->Init(g_pUserLogger, TE_AUTO_THREADS, &m_GPAData,
                additionalStackSize, m_pConfig->GetDeviceMode());
        }
    }

    return m_pTaskExecutor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::ActivateTaskExecutor()
///////////////////////////////////////////////////////////////////////////////////////////////////
bool FrameworkProxy::ActivateTaskExecutor() const
{
    ITaskExecutor* pTaskExecutor = GetTaskExecutor();

    OclAutoMutex cs(&m_initializationMutex);

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

