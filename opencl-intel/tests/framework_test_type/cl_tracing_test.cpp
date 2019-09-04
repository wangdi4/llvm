// Copyright (c) 2019 Intel Corporation
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

#include "tracing_notify.h"
#include "tracing_api.h"
#include "test_utils.h"

static cl_platform_id GetPlatform() {
    cl_int status = CL_SUCCESS;
    cl_platform_id platform = nullptr;

    status = clGetPlatformIDs(1, &platform, nullptr);
    CheckException("clGetPlatformIDs", CL_SUCCESS, status);

    return platform;
}

static cl_device_id GetDevice() {
    cl_int status = CL_SUCCESS;
    cl_platform_id platform = GetPlatform();
    cl_device_id device = nullptr;

    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, nullptr);
    CheckException("clGetDeviceIDs", CL_SUCCESS, status);

    return device;
}

static cl_function_id functionId = CL_FUNCTION_COUNT;

static uint16_t CallFunctions(cl_device_id device) {
    uint16_t count = 0;

    ++count;
    functionId = CL_FUNCTION_clBuildProgram;
    clBuildProgram(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCloneKernel;
    clCloneKernel(0, 0);

    ++count;
    functionId = CL_FUNCTION_clCompileProgram;
    clCompileProgram(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateBuffer;
    clCreateBuffer(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateCommandQueue;
    clCreateCommandQueue(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateCommandQueueWithProperties;
    clCreateCommandQueueWithProperties(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateContext;
    clCreateContext(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateContextFromType;
    clCreateContextFromType(0, 0, 0, 0, 0);

    ++count;
    cl_image_desc imageDesc = {0};
    functionId = CL_FUNCTION_clCreateImage;
    clCreateImage(0, 0, 0, &imageDesc, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateImage2D;
    clCreateImage2D(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateImage3D;
    clCreateImage3D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateKernel;
    clCreateKernel(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateKernelsInProgram;
    clCreateKernelsInProgram(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreatePipe;
    clCreatePipe(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateProgramWithBinary;
    const size_t length = 32;
    unsigned char binary[length] = {0};
    clCreateProgramWithBinary(0, 0, &device, &length,
        reinterpret_cast<const unsigned char **>(&binary), 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateProgramWithBuiltInKernels;
    clCreateProgramWithBuiltInKernels(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateProgramWithIL;
    clCreateProgramWithIL(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateProgramWithSource;
    clCreateProgramWithSource(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateSampler;
    clCreateSampler(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateSamplerWithProperties;
    clCreateSamplerWithProperties(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateSubBuffer;
    clCreateSubBuffer(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateSubDevices;
    clCreateSubDevices(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clCreateUserEvent;
    clCreateUserEvent(0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueBarrier;
    clEnqueueBarrier(0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueBarrierWithWaitList;
    clEnqueueBarrierWithWaitList(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueCopyBuffer;
    clEnqueueCopyBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueCopyBufferRect;
    clEnqueueCopyBufferRect(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueCopyBufferToImage;
    clEnqueueCopyBufferToImage(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueCopyImage;
    clEnqueueCopyImage(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueCopyImageToBuffer;
    clEnqueueCopyImageToBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueFillBuffer;
    clEnqueueFillBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueFillImage;
    clEnqueueFillImage(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueMapBuffer;
    clEnqueueMapBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueMapImage;
    clEnqueueMapImage(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueMarker;
    clEnqueueMarker(0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueMarkerWithWaitList;
    clEnqueueMarkerWithWaitList(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueMigrateMemObjects;
    clEnqueueMigrateMemObjects(0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueNDRangeKernel;
    clEnqueueNDRangeKernel(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueNativeKernel;
    clEnqueueNativeKernel(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueReadBuffer;
    clEnqueueReadBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueReadBufferRect;
    clEnqueueReadBufferRect(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueReadImage;
    clEnqueueReadImage(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMFree;
    clEnqueueSVMFree(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMMap;
    clEnqueueSVMMap(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMMemFill;
    clEnqueueSVMMemFill(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMMemcpy;
    clEnqueueSVMMemcpy(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMMigrateMem;
    clEnqueueSVMMigrateMem(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueSVMUnmap;
    clEnqueueSVMUnmap(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueTask;
    clEnqueueTask(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueUnmapMemObject;
    clEnqueueUnmapMemObject(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueWaitForEvents;
    clEnqueueWaitForEvents(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueWriteBuffer;
    clEnqueueWriteBuffer(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueWriteBufferRect;
    clEnqueueWriteBufferRect(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clEnqueueWriteImage;
    clEnqueueWriteImage(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clFinish;
    clFinish(0);

    ++count;
    functionId = CL_FUNCTION_clFlush;
    clFlush(0);

    ++count;
    functionId = CL_FUNCTION_clGetCommandQueueInfo;
    clGetCommandQueueInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetContextInfo;
    clGetContextInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetDeviceAndHostTimer;
    clGetDeviceAndHostTimer(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetDeviceIDs;
    clGetDeviceIDs(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetDeviceInfo;
    clGetDeviceInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetEventInfo;
    clGetEventInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetEventProfilingInfo;
    clGetEventProfilingInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetExtensionFunctionAddress;
    clGetExtensionFunctionAddress("test");

    ++count;
    functionId = CL_FUNCTION_clGetExtensionFunctionAddressForPlatform;
    clGetExtensionFunctionAddressForPlatform(0, "test");

    ++count;
    functionId = CL_FUNCTION_clGetHostTimer;
    clGetHostTimer(0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetImageInfo;
    clGetImageInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetKernelArgInfo;
    clGetKernelArgInfo(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetKernelInfo;
    clGetKernelInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetKernelSubGroupInfo;
    clGetKernelSubGroupInfo(0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetKernelWorkGroupInfo;
    clGetKernelWorkGroupInfo(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetMemObjectInfo;
    clGetMemObjectInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetPipeInfo;
    clGetPipeInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetPlatformIDs;
    clGetPlatformIDs(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetPlatformInfo;
    clGetPlatformInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetProgramBuildInfo;
    clGetProgramBuildInfo(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetProgramInfo;
    clGetProgramInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetSamplerInfo;
    clGetSamplerInfo(0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clGetSupportedImageFormats;
    clGetSupportedImageFormats(0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clLinkProgram;
    clLinkProgram(0, 0, 0, 0, 0, 0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clReleaseCommandQueue;
    clReleaseCommandQueue(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseContext;
    clReleaseContext(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseDevice;
    clReleaseDevice(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseEvent;
    clReleaseEvent(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseKernel;
    clReleaseKernel(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseMemObject;
    clReleaseMemObject(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseProgram;
    clReleaseProgram(0);

    ++count;
    functionId = CL_FUNCTION_clReleaseSampler;
    clReleaseSampler(0);

    ++count;
    functionId = CL_FUNCTION_clRetainCommandQueue;
    clRetainCommandQueue(0);

    ++count;
    functionId = CL_FUNCTION_clRetainContext;
    clRetainContext(0);

    ++count;
    functionId = CL_FUNCTION_clRetainDevice;
    clRetainDevice(0);

    ++count;
    functionId = CL_FUNCTION_clRetainEvent;
    clRetainEvent(0);

    ++count;
    functionId = CL_FUNCTION_clRetainKernel;
    clRetainKernel(0);

    ++count;
    functionId = CL_FUNCTION_clRetainMemObject;
    clRetainMemObject(0);

    ++count;
    functionId = CL_FUNCTION_clRetainProgram;
    clRetainProgram(0);

    ++count;
    functionId = CL_FUNCTION_clRetainSampler;
    clRetainSampler(0);

    ++count;
    functionId = CL_FUNCTION_clSVMAlloc;
    clSVMAlloc(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSVMFree;
    clSVMFree(0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetDefaultDeviceCommandQueue;
    clSetDefaultDeviceCommandQueue(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetEventCallback;
    clSetEventCallback(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetKernelArg;
    clSetKernelArg(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetKernelArgSVMPointer;
    clSetKernelArgSVMPointer(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetKernelExecInfo;
    clSetKernelExecInfo(0, 0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetMemObjectDestructorCallback;
    clSetMemObjectDestructorCallback(0, 0, 0);

    ++count;
    functionId = CL_FUNCTION_clSetUserEventStatus;
    clSetUserEventStatus(0, 0);

    ++count;
    functionId = CL_FUNCTION_clUnloadCompiler;
    clUnloadCompiler();

    ++count;
    functionId = CL_FUNCTION_clUnloadPlatformCompiler;
    clUnloadPlatformCompiler(0);

    ++count;
    functionId = CL_FUNCTION_clWaitForEvents;
    clWaitForEvents(0, 0);

    return count;
}

static uint16_t enterCount = 0;
static uint16_t exitCount = 0;

static void callback(cl_function_id fid,
                     cl_callback_data *callbackData,
                     void *userData) {
    if (fid == functionId) {
        if (callbackData->site == CL_CALLBACK_SITE_ENTER) {
            ++enterCount;
        } else if (callbackData->site == CL_CALLBACK_SITE_EXIT) {
            ++exitCount;
        }
    }
}

bool clTracingCheckExtensionsTest() {
    void* ptr = nullptr;

    ptr = clGetExtensionFunctionAddress("clCreateTracingHandleINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddress("clSetTracingPointINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddress("clDestroyTracingHandleINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddress("clEnableTracingINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddress("clDisableTracingINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddress("clGetTracingStateINTEL");
    if (ptr == nullptr) {
        return false;
    }

    return true;
}

bool clTracingCheckExtensionsForPlatformTest() {
    void* ptr = nullptr;
    cl_platform_id platform = GetPlatform();

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clCreateTracingHandleINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clSetTracingPointINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clDestroyTracingHandleINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clEnableTracingINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clDisableTracingINTEL");
    if (ptr == nullptr) {
        return false;
    }

    ptr = clGetExtensionFunctionAddressForPlatform(platform,
        "clGetTracingStateINTEL");
    if (ptr == nullptr) {
        return false;
    }

    return true;
}

bool clTracingCheckInvalidArgsTest() {
    cl_tracing_handle handle = nullptr;
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();

    status = clCreateTracingHandleINTEL(nullptr, callback, nullptr, &handle);
    if (status != CL_INVALID_VALUE || handle != nullptr) {
        return false;
    }

    status = clCreateTracingHandleINTEL(device, nullptr, nullptr, &handle);
    if (status != CL_INVALID_VALUE || handle != nullptr) {
        return false;
    }

    status = clCreateTracingHandleINTEL(device, callback, nullptr, nullptr);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clSetTracingPointINTEL(nullptr,
        CL_FUNCTION_clBuildProgram, CL_TRUE);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clDestroyTracingHandleINTEL(nullptr);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clEnableTracingINTEL(nullptr);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clDisableTracingINTEL(nullptr);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clSetTracingPointINTEL(nullptr,
        CL_FUNCTION_clBuildProgram, CL_FALSE);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    cl_bool enabled = CL_FALSE;
    status = clGetTracingStateINTEL(nullptr, &enabled);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    return true;
}

bool clTracingCheckInvactiveHandleTest() {
    cl_tracing_handle handle = nullptr;
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();

    status = clCreateTracingHandleINTEL(device, callback, nullptr, &handle);
    CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);

    status = clDisableTracingINTEL(handle);
    if (status != CL_INVALID_VALUE) {
        return false;
    }

    status = clDestroyTracingHandleINTEL(handle);
    CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);

    return true;
}

bool clTracingCheckTooManyHandlesTest() {
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();

    cl_tracing_handle handle[HostSideTracing::TRACING_MAX_HANDLE_COUNT + 1] =
        { nullptr };

    for (uint32_t i = 0;
            i < HostSideTracing::TRACING_MAX_HANDLE_COUNT + 1; ++i) {
        status = clCreateTracingHandleINTEL(device, callback,
            nullptr, &(handle[i]));
        CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);
    }

    for (uint32_t i = 0; i < HostSideTracing::TRACING_MAX_HANDLE_COUNT; ++i) {
        status = clEnableTracingINTEL(handle[i]);
        CheckException("clEnableTracingINTEL", CL_SUCCESS, status);
    }

    status = clEnableTracingINTEL(
        handle[HostSideTracing::TRACING_MAX_HANDLE_COUNT]);
    if (status != CL_OUT_OF_RESOURCES) {
        return false;
    }

    for (uint32_t i = 0; i < HostSideTracing::TRACING_MAX_HANDLE_COUNT; ++i) {
        status = clDisableTracingINTEL(handle[i]);
        CheckException("clDisableTracingINTEL", CL_SUCCESS, status);
    }

    for (uint32_t i = 0;
            i < HostSideTracing::TRACING_MAX_HANDLE_COUNT + 1; ++i) {
        status = clDestroyTracingHandleINTEL(handle[i]);
        CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);
    }

    return true;
}

bool clTracingFlowCheckTest() {
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();

    cl_tracing_handle handle1 = nullptr;
    status = clCreateTracingHandleINTEL(device, callback, nullptr, &handle1);
    CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);
    cl_tracing_handle handle2 = nullptr;
    status = clCreateTracingHandleINTEL(device, callback, nullptr, &handle2);
    CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);

    status = clSetTracingPointINTEL(handle1, CL_FUNCTION_clBuildProgram, CL_TRUE);
    CheckException("clSetTracingPointINTEL", CL_SUCCESS, status);
    status = clSetTracingPointINTEL(handle2, CL_FUNCTION_clBuildProgram, CL_TRUE);
    CheckException("clSetTracingPointINTEL", CL_SUCCESS, status);

    cl_bool enabled = CL_FALSE;
    status = clGetTracingStateINTEL(handle1, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_FALSE) {
        return false;
    }
    status = clGetTracingStateINTEL(handle2, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_FALSE) {
        return false;
    }

    status = clEnableTracingINTEL(handle1);
    CheckException("clEnableTracingINTEL", CL_SUCCESS, status);
    status = clEnableTracingINTEL(handle2);
    CheckException("clEnableTracingINTEL", CL_SUCCESS, status);

    status = clGetTracingStateINTEL(handle1, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_TRUE) {
        return false;
    }
    status = clGetTracingStateINTEL(handle2, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_TRUE) {
        return false;
    }

    functionId = CL_FUNCTION_clBuildProgram;
    clBuildProgram(0, 0, 0, 0, 0, 0);

    if (2 != enterCount || 2 != exitCount) {
        return false;
    }

    status = clDisableTracingINTEL(handle1);
    CheckException("clDisableTracingINTEL", CL_SUCCESS, status);
    status = clDisableTracingINTEL(handle2);
    CheckException("clDisableTracingINTEL", CL_SUCCESS, status);

    status = clGetTracingStateINTEL(handle1, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_FALSE) {
        return false;
    }
    status = clGetTracingStateINTEL(handle2, &enabled);
    CheckException("clGetTracingStateINTEL", CL_SUCCESS, status);
    if (enabled != CL_FALSE) {
        return false;
    }

    status = clDestroyTracingHandleINTEL(handle1);
    CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);
    status = clDestroyTracingHandleINTEL(handle2);
    CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);

    return true;
}

bool clTracingFunctionsEnabledCheckTest() {
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();
    cl_tracing_handle handle = nullptr;

    status = clCreateTracingHandleINTEL(device, callback, nullptr, &handle);
    CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);

    for (uint32_t i = 0; i < CL_FUNCTION_COUNT; ++i) {
        status = clSetTracingPointINTEL(handle, (cl_function_id)i, CL_TRUE);
        CheckException("clSetTracingPointINTEL", CL_SUCCESS, status);
    }

    status = clEnableTracingINTEL(handle);
    CheckException("clEnableTracingINTEL", CL_SUCCESS, status);

    uint16_t count = CallFunctions(device);
    if (count != enterCount || count != exitCount) {
        return false;
    }

    status = clDisableTracingINTEL(handle);
    CheckException("clDisableTracingINTEL", CL_SUCCESS, status);

    status = clDestroyTracingHandleINTEL(handle);
    CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);

    return true;
}

bool clTracingFunctionsDisabledCheckTest() {
    cl_int status = CL_SUCCESS;
    cl_device_id device = GetDevice();
    cl_tracing_handle handle = nullptr;

    status = clCreateTracingHandleINTEL(device, callback, nullptr, &handle);
    CheckException("clCreateTracingHandleINTEL", CL_SUCCESS, status);

    for (uint32_t i = 0; i < CL_FUNCTION_COUNT; ++i) {
        status = clSetTracingPointINTEL(handle, (cl_function_id)i, CL_FALSE);
        CheckException("clSetTracingPointINTEL", CL_SUCCESS, status);
    }

    status = clEnableTracingINTEL(handle);
    CheckException("clEnableTracingINTEL", CL_SUCCESS, status);

    CallFunctions(device);
    if (0 != enterCount || 0 != exitCount) {
        return false;
    }

    status = clDisableTracingINTEL(handle);
    CheckException("clDisableTracingINTEL", CL_SUCCESS, status);

    status = clDestroyTracingHandleINTEL(handle);
    CheckException("clDestroyTracingHandleINTEL", CL_SUCCESS, status);

    return true;
}
