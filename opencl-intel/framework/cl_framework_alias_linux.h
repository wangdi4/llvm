// INTEL CONFIDENTIAL
//
// Copyright 2006-2019 Intel Corporation.
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

#pragma once

#include "CL/cl.h"
#include "CL/cl_usm_ext.h"

namespace Intel { namespace OpenCL { namespace Framework {

DECLARE_ALIAS(clGetPlatformIDs);
DECLARE_ALIAS(clGetPlatformInfo);
DECLARE_ALIAS(clGetDeviceIDs);
DECLARE_ALIAS(clGetDeviceInfo);
DECLARE_ALIAS(clCreateContext);
DECLARE_ALIAS(clCreateContextFromType);
DECLARE_ALIAS(clRetainContext);
DECLARE_ALIAS(clReleaseContext);
DECLARE_ALIAS(clGetContextInfo);
DECLARE_ALIAS(clCreateCommandQueue);
DECLARE_ALIAS(clRetainCommandQueue);
DECLARE_ALIAS(clReleaseCommandQueue);
DECLARE_ALIAS(clGetCommandQueueInfo);
DECLARE_ALIAS(clCreateBuffer);
DECLARE_ALIAS(clCreateImage);
DECLARE_ALIAS(clCreateImage2D);
DECLARE_ALIAS(clCreateImage3D);
DECLARE_ALIAS(clRetainMemObject);
DECLARE_ALIAS(clReleaseMemObject);
DECLARE_ALIAS(clGetSupportedImageFormats);
DECLARE_ALIAS(clGetMemObjectInfo);
DECLARE_ALIAS(clGetImageInfo);
DECLARE_ALIAS(clCreateSampler);
DECLARE_ALIAS(clCreateSamplerWithProperties);
DECLARE_ALIAS(clRetainSampler);
DECLARE_ALIAS(clReleaseSampler);
DECLARE_ALIAS(clGetSamplerInfo);
DECLARE_ALIAS(clCreateProgramWithSource);
DECLARE_ALIAS(clCreateProgramWithBinary);
DECLARE_ALIAS(clCreateProgramWithBuiltInKernels);
DECLARE_ALIAS(clRetainProgram);
DECLARE_ALIAS(clReleaseProgram);
DECLARE_ALIAS(clBuildProgram);
DECLARE_ALIAS(clCompileProgram);
DECLARE_ALIAS(clLinkProgram);
DECLARE_ALIAS(clUnloadCompiler);
DECLARE_ALIAS(clUnloadPlatformCompiler);
DECLARE_ALIAS(clGetProgramInfo);
DECLARE_ALIAS(clGetProgramBuildInfo);
DECLARE_ALIAS(clCreateKernel);
DECLARE_ALIAS(clCreateKernelsInProgram);
DECLARE_ALIAS(clRetainKernel);
DECLARE_ALIAS(clReleaseKernel);
DECLARE_ALIAS(clSetKernelArg);
DECLARE_ALIAS(clGetKernelInfo);
DECLARE_ALIAS(clGetKernelWorkGroupInfo);
DECLARE_ALIAS(clWaitForEvents);
DECLARE_ALIAS(clGetEventInfo);
DECLARE_ALIAS(clRetainEvent);
DECLARE_ALIAS(clReleaseEvent);
DECLARE_ALIAS(clGetEventProfilingInfo);
DECLARE_ALIAS(clFlush);
DECLARE_ALIAS(clFinish);
DECLARE_ALIAS(clEnqueueReadBuffer);
DECLARE_ALIAS(clEnqueueWriteBuffer);
DECLARE_ALIAS(clEnqueueCopyBuffer);
DECLARE_ALIAS(clEnqueueReadImage);
DECLARE_ALIAS(clEnqueueWriteImage);
DECLARE_ALIAS(clEnqueueCopyImage);
DECLARE_ALIAS(clEnqueueCopyImageToBuffer);
DECLARE_ALIAS(clEnqueueCopyBufferToImage);
DECLARE_ALIAS(clEnqueueMapBuffer);
DECLARE_ALIAS(clEnqueueMapImage);
DECLARE_ALIAS(clEnqueueUnmapMemObject);
DECLARE_ALIAS(clEnqueueNDRangeKernel);
DECLARE_ALIAS(clEnqueueTask);
DECLARE_ALIAS(clEnqueueNativeKernel);
DECLARE_ALIAS(clEnqueueMarker);
DECLARE_ALIAS(clEnqueueWaitForEvents);
DECLARE_ALIAS(clEnqueueBarrier);
DECLARE_ALIAS(clGetExtensionFunctionAddress);
DECLARE_ALIAS(clGetExtensionFunctionAddressForPlatform);
DECLARE_ALIAS(clSetEventCallback);
DECLARE_ALIAS(clCreateSubBuffer);
DECLARE_ALIAS(clSetMemObjectDestructorCallback);
DECLARE_ALIAS(clCreateUserEvent);
DECLARE_ALIAS(clSetUserEventStatus);
DECLARE_ALIAS(clEnqueueReadBufferRect);
DECLARE_ALIAS(clEnqueueWriteBufferRect);
DECLARE_ALIAS(clEnqueueCopyBufferRect);
DECLARE_ALIAS(clCreateSubDevices);
DECLARE_ALIAS(clRetainDevice);
DECLARE_ALIAS(clReleaseDevice);
DECLARE_ALIAS(clGetKernelArgInfo);
DECLARE_ALIAS(clCreateImage2DArrayINTEL);
DECLARE_ALIAS(clEnqueueFillBuffer);
DECLARE_ALIAS(clEnqueueMigrateMemObjects);
DECLARE_ALIAS(clCreateSubDevices);
DECLARE_ALIAS(clReleaseDevice);
DECLARE_ALIAS(clEnqueueBarrierWithWaitList);
DECLARE_ALIAS(clCompileProgram);
DECLARE_ALIAS(clLinkProgram);
DECLARE_ALIAS(clEnqueueMarkerWithWaitList);
DECLARE_ALIAS(clEnqueueFillImage);
DECLARE_ALIAS(clSVMAlloc);
DECLARE_ALIAS(clSVMFree);
DECLARE_ALIAS(clEnqueueSVMFree);
DECLARE_ALIAS(clEnqueueSVMMemcpy);
DECLARE_ALIAS(clEnqueueSVMMemFill);
DECLARE_ALIAS(clEnqueueSVMMap);
DECLARE_ALIAS(clEnqueueSVMUnmap);
DECLARE_ALIAS(clSetKernelArgSVMPointer);
DECLARE_ALIAS(clSetKernelExecInfo);
DECLARE_ALIAS(clCreatePipe);
DECLARE_ALIAS(clCreatePipeINTEL);
DECLARE_ALIAS(clGetPipeInfo);
DECLARE_ALIAS(clCreateCommandQueueWithProperties);
DECLARE_ALIAS(clCreateProgramWithIL);
DECLARE_ALIAS(clGetHostTimer);
DECLARE_ALIAS(clGetDeviceAndHostTimer);
DECLARE_ALIAS(clCloneKernel);
DECLARE_ALIAS(clSetDefaultDeviceCommandQueue);
DECLARE_ALIAS(clEnqueueSVMMigrateMem);
DECLARE_ALIAS(clGetKernelSubGroupInfo);
DECLARE_ALIAS(clGetKernelSubGroupInfoKHR);
DECLARE_ALIAS(clGetProfileDataDeviceIntelFPGA);
DECLARE_ALIAS(clMapHostPipeIntelFPGA);
DECLARE_ALIAS(clUnmapHostPipeIntelFPGA);
DECLARE_ALIAS(clReadPipeIntelFPGA);
DECLARE_ALIAS(clWritePipeIntelFPGA);
DECLARE_ALIAS(clGetDeviceFunctionPointerINTEL);
DECLARE_ALIAS(clCreateProgramWithILKHR);
DECLARE_ALIAS(clCreateTracingHandleINTEL);
DECLARE_ALIAS(clSetTracingPointINTEL);
DECLARE_ALIAS(clDestroyTracingHandleINTEL);
DECLARE_ALIAS(clEnableTracingINTEL);
DECLARE_ALIAS(clDisableTracingINTEL);
DECLARE_ALIAS(clGetTracingStateINTEL);
DECLARE_ALIAS(clHostMemAllocINTEL);
DECLARE_ALIAS(clDeviceMemAllocINTEL);
DECLARE_ALIAS(clSharedMemAllocINTEL);
DECLARE_ALIAS(clMemFreeINTEL);
DECLARE_ALIAS(clGetMemAllocInfoINTEL);
DECLARE_ALIAS(clSetKernelArgMemPointerINTEL);
DECLARE_ALIAS(clEnqueueMemsetINTEL);
DECLARE_ALIAS(clEnqueueMemcpyINTEL);
DECLARE_ALIAS(clEnqueueMigrateMemINTEL);
DECLARE_ALIAS(clEnqueueMemAdviseINTEL);
}}}
