// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "CL/cl.h"

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
#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#warning CL_USE_DEPRECATED_OPENCL_1_0_APIS is defined. These APIs are unsupported and untested in OpenCL 1.1!
	ECLARE_ALIAS(clSetCommandQueueProperty);
#endif
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
DECLARE_ALIAS(clRetainSampler);
DECLARE_ALIAS(clReleaseSampler);
DECLARE_ALIAS(clGetSamplerInfo);
DECLARE_ALIAS(clCreateProgramWithSource);
DECLARE_ALIAS(clCreateProgramWithBinary);
DECLARE_ALIAS(clRetainProgram);
DECLARE_ALIAS(clReleaseProgram);
DECLARE_ALIAS(clBuildProgram);
DECLARE_ALIAS(clUnloadCompiler);
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
DECLARE_ALIAS(clCreateFromGLBuffer);
DECLARE_ALIAS(clCreateFromGLTexture2D);
DECLARE_ALIAS(clCreateFromGLTexture3D);
DECLARE_ALIAS(clCreateFromGLRenderbuffer);
DECLARE_ALIAS(clGetGLObjectInfo);
DECLARE_ALIAS(clGetGLTextureInfo);
DECLARE_ALIAS(clEnqueueAcquireGLObjects);
DECLARE_ALIAS(clEnqueueReleaseGLObjects);
DECLARE_ALIAS(clGetGLContextInfoKHR);
//	NULL, // DECLARE_ALIAS(clGetDeviceIDsFromD3D10KHR);
//	NULL, // DECLARE_ALIAS(clCreateFromD3D10BufferKHR);
//	NULL, // DECLARE_ALIAS(clCreateFromD3D10Texture2DKHR);
//	NULL, // DECLARE_ALIAS(clCreateFromD3D10Texture3DKHR);
//	NULL, // DECLARE_ALIAS(clEnqueueAcquireD3D10ObjectsKHR);
//	NULL, // DECLARE_ALIAS(clEnqueueReleaseD3D10ObjectsKHR);
DECLARE_ALIAS(clSetEventCallback);
DECLARE_ALIAS(clCreateSubBuffer);
DECLARE_ALIAS(clSetMemObjectDestructorCallback);
DECLARE_ALIAS(clCreateUserEvent);
DECLARE_ALIAS(clSetUserEventStatus);
DECLARE_ALIAS(clEnqueueReadBufferRect);
DECLARE_ALIAS(clEnqueueWriteBufferRect);
DECLARE_ALIAS(clEnqueueCopyBufferRect);
DECLARE_ALIAS(clCreateSubDevicesEXT);
DECLARE_ALIAS(clRetainDeviceEXT);
DECLARE_ALIAS(clReleaseDeviceEXT);
DECLARE_ALIAS(clGetKernelArgInfo);
DECLARE_ALIAS(clCreateImage2DArrayINTEL);

}}}
