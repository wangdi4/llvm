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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include "crt_module.h"
#include "crt_internals.h"
#include <iostream>
#include <memory>
#include <string>
using namespace OCLCRT;

#define REGISTER_DISPATCH_ENTRYPOINT(__NAME__,__ADDRESS__) \
    m_crtDispatchTable.##__NAME__ = (__ADDRESS__);


namespace OCLCRT
{
    extern CrtModule crt_ocl_module;
};


IcdDispatchMgr::IcdDispatchMgr()
{
    REGISTER_DISPATCH_ENTRYPOINT(clGetPlatformIDs,clGetPlatformIDs);
    REGISTER_DISPATCH_ENTRYPOINT(clGetPlatformInfo,clGetPlatformInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetDeviceIDs,clGetDeviceIDs);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateContext,clCreateContext);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateContextFromType,clCreateContextFromType);
    REGISTER_DISPATCH_ENTRYPOINT(clGetContextInfo,clGetContextInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetDeviceInfo,clGetDeviceInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateProgramWithSource,clCreateProgramWithSource);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateProgramWithBinary,clCreateProgramWithBinary);
    REGISTER_DISPATCH_ENTRYPOINT(clBuildProgram,clBuildProgram);
    REGISTER_DISPATCH_ENTRYPOINT(clGetProgramInfo,clGetProgramInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetProgramBuildInfo,clGetProgramBuildInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainProgram,clRetainProgram);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseProgram,clReleaseProgram);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateKernel,clCreateKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clGetKernelInfo,clGetKernelInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateKernelsInProgram,clCreateKernelsInProgram);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainKernel,clRetainKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseKernel,clReleaseKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clSetKernelArg,clSetKernelArg);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateBuffer,clCreateBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateSubBuffer,clCreateSubBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateCommandQueue,clCreateCommandQueue);
    REGISTER_DISPATCH_ENTRYPOINT(clGetCommandQueueInfo,clGetCommandQueueInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueReadBuffer,clEnqueueReadBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueWriteBuffer,clEnqueueWriteBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueCopyBuffer,clEnqueueCopyBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueMapBuffer,clEnqueueMapBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueMapImage,clEnqueueMapImage);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueReadBufferRect,clEnqueueReadBufferRect);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueWriteBufferRect,clEnqueueWriteBufferRect);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueCopyBufferRect,clEnqueueCopyBufferRect);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateImage2D,clCreateImage2D);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateImage3D,clCreateImage3D);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueReadImage,clEnqueueReadImage);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueWriteImage,clEnqueueWriteImage);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueCopyImage,clEnqueueCopyImage);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueCopyImageToBuffer,clEnqueueCopyImageToBuffer);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueCopyBufferToImage,clEnqueueCopyBufferToImage);
    REGISTER_DISPATCH_ENTRYPOINT(clFlush,clFlush);
    REGISTER_DISPATCH_ENTRYPOINT(clFinish,clFinish);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueNDRangeKernel,clEnqueueNDRangeKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseCommandQueue,clReleaseCommandQueue);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainCommandQueue,clRetainCommandQueue);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseContext,clReleaseContext);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainContext,clRetainContext);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseMemObject,clReleaseMemObject);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainMemObject,clRetainMemObject);
    REGISTER_DISPATCH_ENTRYPOINT(clSetMemObjectDestructorCallback,clSetMemObjectDestructorCallback);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateSubDevicesEXT,clCreateSubDevicesEXT);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseDeviceEXT,clReleaseDeviceEXT);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainDeviceEXT,clRetainDeviceEXT);
    REGISTER_DISPATCH_ENTRYPOINT(clWaitForEvents,clWaitForEvents);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseEvent,clReleaseEvent);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainEvent,clRetainEvent);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateUserEvent,clCreateUserEvent);
    REGISTER_DISPATCH_ENTRYPOINT(clGetEventInfo,clGetEventInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clSetEventCallback,clSetEventCallback);
    REGISTER_DISPATCH_ENTRYPOINT(clSetUserEventStatus,clSetUserEventStatus);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueNDRangeKernel,clEnqueueNDRangeKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueTask,clEnqueueTask);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueMarker,clEnqueueMarker);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueBarrier,clEnqueueBarrier);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueWaitForEvents,clEnqueueWaitForEvents);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueNativeKernel,clEnqueueNativeKernel);
    REGISTER_DISPATCH_ENTRYPOINT(clEnqueueUnmapMemObject,clEnqueueUnmapMemObject);
    REGISTER_DISPATCH_ENTRYPOINT(clCreateSampler,clCreateSampler);
    REGISTER_DISPATCH_ENTRYPOINT(clRetainSampler,clRetainSampler);
    REGISTER_DISPATCH_ENTRYPOINT(clReleaseSampler,clReleaseSampler);
    REGISTER_DISPATCH_ENTRYPOINT(clGetImageInfo,clGetImageInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetSamplerInfo,clGetSamplerInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetEventProfilingInfo,clGetEventProfilingInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetKernelWorkGroupInfo,clGetKernelWorkGroupInfo);
    REGISTER_DISPATCH_ENTRYPOINT(clGetSupportedImageFormats,clGetSupportedImageFormats);    

        /// Extensions
    REGISTER_DISPATCH_ENTRYPOINT(clGetGLContextInfoKHR,clGetGLContextInfoKHR);
    REGISTER_DISPATCH_ENTRYPOINT(clGetDeviceIDsFromD3D10KHR,clGetDeviceIDsFromD3D10KHR);
};

CrtModule::CrtModule():
m_deviceInfoMapGuard(m_deviceInfoMap),
m_contextInfoGuard(m_contextInfo)
{
    m_initializeState = NOT_INITIALIZED;
}

crt_err_code CrtModule::PatchClDeviceID(cl_device_id& inDeviceId, KHRicdVendorDispatch* origDispatchTable)
{
        /// Store original device dispatch table entries
    if (origDispatchTable)
        *(origDispatchTable) = *(inDeviceId->dispatch);

    inDeviceId->dispatch->clCreateContext = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clCreateContext;
    inDeviceId->dispatch->clGetDeviceInfo = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clGetDeviceInfo;
    inDeviceId->dispatch->clCreateSubDevicesEXT = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clCreateSubDevicesEXT;
    inDeviceId->dispatch->clReleaseDeviceEXT = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clReleaseDeviceEXT;
    inDeviceId->dispatch->clRetainDeviceEXT = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clRetainDeviceEXT;
    return CRT_SUCCESS;
}

crt_err_code CrtModule::PatchClContextID(cl_context& inContextId, KHRicdVendorDispatch* origDispatchTable)
{
    inContextId->dispatch->clGetContextInfo = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clGetContextInfo;
    inContextId->dispatch->clReleaseContext = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clReleaseContext;
    inContextId->dispatch->clRetainContext  = crt_ocl_module.m_icdDispatchMgr.m_crtDispatchTable.clRetainContext;
    return CRT_SUCCESS;
}

bool OCLCRT::isGLContext(const cl_context_properties* properties)
{
    cl_context_properties hGL = NULL, hDC = NULL;
    if ( NULL == properties)
    {
        return false;
    }
    while (NULL != *properties)
    {
        switch (*properties)
        {
        case CL_GL_CONTEXT_KHR:
            hGL = *(properties+1);
            break;
        case  CL_WGL_HDC_KHR:
            hDC = *(properties+1);
            break;
        }
        properties+=2;
    }
    return ( (NULL != hGL) || (NULL != hDC) );
}

crt_err_code CrtModule::Initialize()
{
    Utils::OclAutoMutex CS(&m_mutex);   // Critical section

    if (m_initializeState == NOT_INITIALIZED)
    {
        crt_err_code res = CRT_SUCCESS;

        m_crtPlatformId = new _cl_platform_id_crt;

        if (!m_crtPlatformId || m_crtConfig.Init() == CRT_FAIL)
        {
            m_initializeState = INITIALIZE_ERROR;
            return CRT_FAIL;
        }
        for (cl_uint i = 0; i < m_crtConfig.getNumPlatforms(); i++)
        {
            const std::string& libName = m_crtConfig.getPlatformLibName(i);
            CrtPlatform* pCrtPlatform = new CrtPlatform;
            if (!pCrtPlatform)
            {
                res = CRT_FAIL;
                break;
            }
            if (CRT_FAIL == pCrtPlatform->m_lib.Load(libName.c_str()))
            {
                delete pCrtPlatform;
                continue;
            }

            m_oclPlatforms.push_back(pCrtPlatform);

            KHRpfn_clGetExtensionFunctionAddress pfn_clGetExtFuncAddr = (KHRpfn_clGetExtensionFunctionAddress)pCrtPlatform->m_lib.GetFunctionPtrByName("clGetExtensionFunctionAddress");
            KHRpfn_clGetPlatformIDs pfn_clGetPlatformIDs =  (KHRpfn_clGetPlatformIDs)pfn_clGetExtFuncAddr("clIcdGetPlatformIDsKHR");
            if (NULL == pfn_clGetPlatformIDs || !(CL_SUCCESS == pfn_clGetPlatformIDs(1, &pCrtPlatform->m_platformIdDEV, NULL)))
            {
                res = CRT_FAIL;
                break;
            }
                /// Query the Platform extensions for each platform
            size_t extSize = 0;
            if (CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_EXTENSIONS,
                0,
                NULL,
                &extSize))
            {
                res = CRT_FAIL;
                break;
            }

            pCrtPlatform->m_supportedExtensionsStr = new char[extSize];
            if (!pCrtPlatform->m_supportedExtensionsStr)
            {
                res = CRT_FAIL;
                break;
            }

            if (CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_EXTENSIONS,
                extSize,
                pCrtPlatform->m_supportedExtensionsStr,
                NULL))
            {
                res = CRT_FAIL;
                break;
            }
            pCrtPlatform->m_supportedExtensions = GetCrtExtension(pCrtPlatform->m_supportedExtensionsStr);

                /// Query the ICD Suffix for each platform
            if (CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_ICD_SUFFIX_KHR,
                0,
                NULL,
                &extSize))
            {
                res = CRT_FAIL;
                break;
            }

            pCrtPlatform->m_icdSuffix = new char[extSize];
            if (!pCrtPlatform->m_icdSuffix)
            {
                res = CRT_FAIL;
                break;
            }

            if (CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_ICD_SUFFIX_KHR,
                extSize,
                pCrtPlatform->m_icdSuffix,
                NULL))
            {
                res = CRT_FAIL;
                break;
            }

            cl_uint num_devices = 0;
            pCrtPlatform->m_platformIdDEV->dispatch->clGetDeviceIDs(
                                    pCrtPlatform->m_platformIdDEV,
                                    CL_DEVICE_TYPE_DEFAULT,
                                    0,
                                    NULL,
                                    &num_devices);
            if (num_devices == 0)
            {
                res = CRT_FAIL;
                break;
            }
            cl_device_id* pDevices = new cl_device_id[num_devices];
            if (!(CL_SUCCESS == pCrtPlatform->m_platformIdDEV->dispatch->clGetDeviceIDs(
                                    pCrtPlatform->m_platformIdDEV,
                                    CL_DEVICE_TYPE_DEFAULT,
                                    num_devices,
                                    pDevices,
                                    NULL)))
            {
                res = CRT_FAIL;
                break;
            }

            for (cl_uint j=0; j < num_devices; j++)
            {
                CrtDeviceInfo* pDevInfo = new CrtDeviceInfo;
                if (NULL == pDevInfo)
                {
                    res = CRT_FAIL;
                    break;
                }

                if (CL_SUCCESS != pDevices[j]->dispatch->clGetDeviceInfo(
                                    pDevices[j],
                                    CL_DEVICE_EXECUTION_CAPABILITIES,
                                    sizeof( pDevInfo->m_deviceCapabilities ),
                                    &pDevInfo->m_deviceCapabilities,
                                    0))
                {
                    res = CRT_FAIL;
                    break;
                }

                    /// This is not a sub-device
                pDevInfo->m_isRootDevice = true;
                pDevInfo->m_platformIdDEV = pCrtPlatform->m_platformIdDEV;
                crt_ocl_module.PatchClDeviceID(pDevices[j], &pDevInfo->m_origDispatchTable);

                m_deviceInfoMap[pDevices[j]] = pDevInfo;
            }

            delete[] pDevices;
        }

        m_defaultDeviceType = CRT_DEFAULT_DEVICE_TYPE;

        if (m_deviceInfoMap.size() == 0)
        {
            // We couldn't load any device
            res = CRT_FAIL;
        }

        if (res == CRT_FAIL)
        {
            Shutdown();
            m_initializeState = INITIALIZE_ERROR;
            return CRT_FAIL;
        }
    }
    m_initializeState = INITIALIZE_OK;
    return CRT_SUCCESS;
}


cl_int CrtModule::isValidProperties(const cl_context_properties* properties)
{
    cl_int errCode = CL_SUCCESS;

    cl_bool cl_context_platform_set   = CL_FALSE;
    cl_bool cl_gl_context_khr_set     = CL_FALSE;
    cl_bool cl_wgl_hdc_khr_set        = CL_FALSE;
    cl_bool cl_d3d10_device_khr_set   = CL_FALSE;
    cl_bool cl_d3d9_device_intel_set  = CL_FALSE;
    cl_bool cl_dxva9_device_intel_set = CL_FALSE;

    if( properties != NULL )
    {
        while( *properties != NULL )
        {
            switch( properties[ 0 ] )
            {
            case CL_CONTEXT_PLATFORM:
                if( cl_context_platform_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_context_platform_set = CL_TRUE;
                if( (cl_platform_id)(properties[ 1 ]) != crt_ocl_module.m_crtPlatformId )
                {
                    errCode = CL_INVALID_PLATFORM;
                }
                break;
            case CL_GL_CONTEXT_KHR:
                if( cl_gl_context_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_gl_context_khr_set = CL_TRUE;
                break;
            case CL_WGL_HDC_KHR:
                if( cl_wgl_hdc_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_wgl_hdc_khr_set = CL_TRUE;
                break;
            case CL_CONTEXT_D3D10_DEVICE_KHR:
                if( cl_d3d10_device_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_d3d10_device_khr_set = CL_TRUE;
                break;
            case CL_CONTEXT_D3D9_DEVICE_INTEL:
            case CL_CONTEXT_D3D9EX_DEVICE_INTEL:
                if( cl_d3d9_device_intel_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_d3d9_device_intel_set = CL_TRUE;
                break;
            case CL_CONTEXT_DXVA9_DEVICE_INTEL:
                if( cl_dxva9_device_intel_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_dxva9_device_intel_set  = CL_TRUE;
                break;
            default:
                return CL_INVALID_PROPERTY;
            }
            properties += 2;
        }
    }
    return errCode;
}



crt_err_code OCLCRT::ReplacePlatformId(const cl_context_properties* properties, cl_platform_id& pId, cl_context_properties** props, bool dup)
{
    void** p = (void**)properties;

    if (!properties)
    {
        *props = NULL;
        return CRT_SUCCESS;
    }

    cl_context_properties* clProperties = NULL;
    cl_uint num_entries = 0;
    if (dup)
    {
        while (NULL != *p)
        {
            p++;
            num_entries++;
        }
        // We create a new props to be passed to the underlying
        // platform. this is deleted by the calling function.
        *props = new cl_context_properties[num_entries+1];
        if (*props == NULL)
        {
            return CRT_FAIL;
        }

        clProperties = *props;
    }
    else
    {
        // const_cast is legal here since this flow will occur only
        // when we need to return the properties so we need to change them
        clProperties = const_cast<cl_context_properties*>(properties);
    }
    while (NULL != *properties)
    {
        switch (*properties)
        {
        case CL_CONTEXT_PLATFORM:
            *clProperties = CL_CONTEXT_PLATFORM;
            *(clProperties+1) = (cl_context_properties)pId;
            properties+=1;
            clProperties+=1;
            break;
        default:
            *clProperties = *properties;
            break;
        }
        properties+=1;
        clProperties+=1;
    }
    *clProperties = 0;
    return CRT_SUCCESS;
}


void CrtModule::Shutdown()
{
    // Delete all create platforms
    while (!m_oclPlatforms.empty())
    {
        CrtPlatform* p = m_oclPlatforms.back();
        if (p)
        {
            p->m_lib.Close();

            if (p->m_supportedExtensionsStr)
            {
                delete[] p->m_supportedExtensionsStr;
            }

            if (p->m_icdSuffix)
            {
                delete[] p->m_icdSuffix;
            }

            delete p;
        }
        m_oclPlatforms.pop_back();
    }
        /// Delete all device info
    for (DEV_INFO_MAP::iterator itr = m_deviceInfoMap.begin(); itr != m_deviceInfoMap.end(); itr++)
    {
        if (itr->second)
        {
            delete itr->second;
            itr->second = NULL;
        }
    }

    if (m_common_extensions)
        delete m_common_extensions;
    if (m_crtPlatformId)
    {
        delete m_crtPlatformId;
    }


    m_deviceInfoMap.clear();
    m_oclPlatforms.clear();
}

CrtModule::~CrtModule()
{
    Shutdown();
}
