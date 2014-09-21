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
    m_icdDispatchTable . __NAME__ = (__ADDRESS__);

namespace OCLCRT
{
    extern CrtModule crt_ocl_module;
}

IcdDispatchMgr::IcdDispatchMgr()
{
    REGISTER_DISPATCH_ENTRYPOINT( clGetPlatformIDs , clGetPlatformIDs )
    REGISTER_DISPATCH_ENTRYPOINT( clGetPlatformInfo , clGetPlatformInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clGetDeviceIDs , clGetDeviceIDs )
    REGISTER_DISPATCH_ENTRYPOINT( clGetDeviceInfo , clGetDeviceInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateContext , clCreateContext )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateContextFromType , clCreateContextFromType )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainContext , clRetainContext )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseContext , clReleaseContext )
    REGISTER_DISPATCH_ENTRYPOINT( clGetContextInfo , clGetContextInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateCommandQueue , clCreateCommandQueue )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainCommandQueue , clRetainCommandQueue )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseCommandQueue , clReleaseCommandQueue )
    REGISTER_DISPATCH_ENTRYPOINT( clGetCommandQueueInfo , clGetCommandQueueInfo )
    // clSetCommandQueueProperty
    REGISTER_DISPATCH_ENTRYPOINT( clCreateBuffer , clCreateBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateImage2D , clCreateImage2D )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateImage3D , clCreateImage3D )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainMemObject , clRetainMemObject )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseMemObject , clReleaseMemObject )
    REGISTER_DISPATCH_ENTRYPOINT( clGetSupportedImageFormats , clGetSupportedImageFormats )
    REGISTER_DISPATCH_ENTRYPOINT( clGetMemObjectInfo , clGetMemObjectInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clGetImageInfo , clGetImageInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateSampler , clCreateSampler )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainSampler , clRetainSampler )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseSampler , clReleaseSampler )
    REGISTER_DISPATCH_ENTRYPOINT( clGetSamplerInfo , clGetSamplerInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateProgramWithSource , clCreateProgramWithSource )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateProgramWithBinary , clCreateProgramWithBinary )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainProgram , clRetainProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseProgram , clReleaseProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clBuildProgram , clBuildProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clUnloadCompiler , clUnloadCompiler )
    REGISTER_DISPATCH_ENTRYPOINT( clGetProgramInfo , clGetProgramInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clGetProgramBuildInfo , clGetProgramBuildInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateKernel , clCreateKernel )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateKernelsInProgram , clCreateKernelsInProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainKernel , clRetainKernel )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseKernel , clReleaseKernel )
    REGISTER_DISPATCH_ENTRYPOINT( clSetKernelArg , clSetKernelArg )
    REGISTER_DISPATCH_ENTRYPOINT( clGetKernelInfo , clGetKernelInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clGetKernelWorkGroupInfo , clGetKernelWorkGroupInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clWaitForEvents , clWaitForEvents )
    REGISTER_DISPATCH_ENTRYPOINT( clGetEventInfo , clGetEventInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainEvent , clRetainEvent )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseEvent , clReleaseEvent )
    REGISTER_DISPATCH_ENTRYPOINT( clGetEventProfilingInfo , clGetEventProfilingInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clFlush , clFlush )
    REGISTER_DISPATCH_ENTRYPOINT( clFinish , clFinish )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueReadBuffer , clEnqueueReadBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueWriteBuffer , clEnqueueWriteBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueCopyBuffer , clEnqueueCopyBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueReadImage , clEnqueueReadImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueWriteImage , clEnqueueWriteImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueCopyImage , clEnqueueCopyImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueCopyImageToBuffer , clEnqueueCopyImageToBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueCopyBufferToImage , clEnqueueCopyBufferToImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueMapBuffer , clEnqueueMapBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueMapImage , clEnqueueMapImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueUnmapMemObject , clEnqueueUnmapMemObject )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueNDRangeKernel , clEnqueueNDRangeKernel )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueTask , clEnqueueTask )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueNativeKernel , clEnqueueNativeKernel )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueMarker , clEnqueueMarker )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueWaitForEvents , clEnqueueWaitForEvents )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueBarrier , clEnqueueBarrier )
    REGISTER_DISPATCH_ENTRYPOINT( clGetExtensionFunctionAddress , clGetExtensionFunctionAddress )
    // clCreateFromGLBuffer
    // clCreateFromGLTexture2D
    // clCreateFromGLTexture3D
    // clCreateFromGLRenderbuffer
    // clGetGLObjectInfo
    // clGetGLTextureInfo
    // clEnqueueAcquireGLObjects
    // clEnqueueReleaseGLObjects
    REGISTER_DISPATCH_ENTRYPOINT( clGetGLContextInfoKHR , clGetGLContextInfoKHR )

#ifdef _WIN32
    REGISTER_DISPATCH_ENTRYPOINT( clGetDeviceIDsFromDX9MediaAdapterKHR , clGetDeviceIDsFromDX9MediaAdapterKHR )
    REGISTER_DISPATCH_ENTRYPOINT( clGetDeviceIDsFromD3D10KHR , clGetDeviceIDsFromD3D10KHR )
    // clCreateFromD3D10BufferKHR
    // clCreateFromD3D10Texture2DKHR
    // clCreateFromD3D10Texture3DKHR
    // clEnqueueAcquireD3D10ObjectsKHR
    // clEnqueueReleaseD3D10ObjectsKHR

    REGISTER_DISPATCH_ENTRYPOINT( clGetDeviceIDsFromD3D11KHR , clGetDeviceIDsFromD3D11KHR )
#endif

    REGISTER_DISPATCH_ENTRYPOINT( clSetEventCallback , clSetEventCallback )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateSubBuffer , clCreateSubBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clSetMemObjectDestructorCallback , clSetMemObjectDestructorCallback )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateUserEvent , clCreateUserEvent )
    REGISTER_DISPATCH_ENTRYPOINT( clSetUserEventStatus , clSetUserEventStatus )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueReadBufferRect , clEnqueueReadBufferRect )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueWriteBufferRect , clEnqueueWriteBufferRect )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueCopyBufferRect , clEnqueueCopyBufferRect )

    REGISTER_DISPATCH_ENTRYPOINT( clCreateEventFromGLsyncKHR , clCreateEventFromGLsyncKHR )

    REGISTER_DISPATCH_ENTRYPOINT( clCreateSubDevices , clCreateSubDevices )
    REGISTER_DISPATCH_ENTRYPOINT( clRetainDevice , clRetainDevice )
    REGISTER_DISPATCH_ENTRYPOINT( clReleaseDevice , clReleaseDevice )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateImage , clCreateImage )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateProgramWithBuiltInKernels , clCreateProgramWithBuiltInKernels )
    REGISTER_DISPATCH_ENTRYPOINT( clCompileProgram , clCompileProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clLinkProgram , clLinkProgram )
    REGISTER_DISPATCH_ENTRYPOINT( clUnloadPlatformCompiler , clUnloadPlatformCompiler )
    REGISTER_DISPATCH_ENTRYPOINT( clGetKernelArgInfo , clGetKernelArgInfo )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueFillBuffer , clEnqueueFillBuffer )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueFillImage , clEnqueueFillImage )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueMigrateMemObjects , clEnqueueMigrateMemObjects )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueMarkerWithWaitList , clEnqueueMarkerWithWaitList )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueBarrierWithWaitList , clEnqueueBarrierWithWaitList )
    REGISTER_DISPATCH_ENTRYPOINT( clGetExtensionFunctionAddressForPlatform , clGetExtensionFunctionAddressForPlatform )
    // clCreateFromGLTexture

    
    // clCreateFromD3D11BufferKHR
    // clCreateFromD3D11Texture2DKHR
    // clCreateFromD3D11Texture3DKHR
    // clCreateFromDX9MediaSurfaceKHR
    // clEnqueueAcquireD3D11ObjectsKHR
    // clEnqueueReleaseD3D11ObjectsKHR

    REGISTER_DISPATCH_ENTRYPOINT( clSVMAlloc , clSVMAlloc )
    REGISTER_DISPATCH_ENTRYPOINT( clSVMFree , clSVMFree )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueSVMFree , clEnqueueSVMFree )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueSVMMemcpy , clEnqueueSVMMemcpy )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueSVMMemFill , clEnqueueSVMMemFill )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueSVMMap , clEnqueueSVMMap )
    REGISTER_DISPATCH_ENTRYPOINT( clEnqueueSVMUnmap , clEnqueueSVMUnmap )
    REGISTER_DISPATCH_ENTRYPOINT( clSetKernelArgSVMPointer , clSetKernelArgSVMPointer )
    REGISTER_DISPATCH_ENTRYPOINT( clSetKernelExecInfo , clSetKernelExecInfo )

    REGISTER_DISPATCH_ENTRYPOINT( clCreateSamplerWithProperties , clCreateSamplerWithProperties )
    REGISTER_DISPATCH_ENTRYPOINT( clCreateCommandQueueWithProperties , clCreateCommandQueueWithProperties)

    //disabling this now as there is no CPU nor GPU support
    //when reenabling GPU support a check must be added in clGetKernelSubGroupInfoKHR implementation
    //so that CPU version is never called as this would cause a crash
    //REGISTER_DISPATCH_ENTRYPOINT( clGetKernelSubGroupInfoKHR , clGetKernelSubGroupInfoKHR )

    // clEnqueueAcquireDX9MediaSurfacesKHR
    // clEnqueueReleaseDX9MediaSurfacesKHR

    REGISTER_DISPATCH_ENTRYPOINT( clCreatePipe , clCreatePipe )
    REGISTER_DISPATCH_ENTRYPOINT( clGetPipeInfo , clGetPipeInfo )
}

CrtModule::CrtModule():
m_deviceInfoMapGuard(m_deviceInfoMap),
m_contextInfoGuard(m_contextInfo),
m_CrtPlatformVersion(OPENCL_INVALID)
{
    m_initializeState   = NOT_INITIALIZED;
    m_crtPlatformId     = NULL;
}

crt_err_code CrtModule::PatchClDeviceID(cl_device_id& inDeviceId)
{
    inDeviceId->dispatch->clCreateContext = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clCreateContext;
    inDeviceId->dispatch->clGetDeviceInfo = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clGetDeviceInfo;
    inDeviceId->dispatch->clCreateSubDevices = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clCreateSubDevices;
    inDeviceId->dispatch->clReleaseDevice    = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clReleaseDevice;
    inDeviceId->dispatch->clRetainDevice     = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clRetainDevice;
    return CRT_SUCCESS;
}

crt_err_code CrtModule::PatchClContextID(cl_context& inContextId, KHRicdVendorDispatch* origDispatchTable)
{
    inContextId->dispatch->clGetContextInfo = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clGetContextInfo;
    inContextId->dispatch->clReleaseContext = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clReleaseContext;
    inContextId->dispatch->clRetainContext  = crt_ocl_module.m_icdDispatchMgr.m_icdDispatchTable.clRetainContext;
    return CRT_SUCCESS;
}

bool OCLCRT::isSupportedContextType(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id *devices)
{
    // Check for unsupported context properties
    if( properties != NULL )
    {
        while( properties[ 0 ] != NULL )
        {
            switch( properties[ 0 ] )
            {
            case CL_GL_CONTEXT_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_EGL_DISPLAY_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_GLX_DISPLAY_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_CGL_SHAREGROUP_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_WGL_HDC_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
#ifdef _WIN32
            case CL_CONTEXT_ADAPTER_D3D9_KHR:
            case CL_CONTEXT_ADAPTER_D3D9EX_KHR:
            case CL_CONTEXT_ADAPTER_DXVA_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_CONTEXT_D3D10_DEVICE_KHR:
            case CL_CONTEXT_D3D11_DEVICE_KHR:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_CONTEXT_D3D9_DEVICE_INTEL:
            case CL_CONTEXT_D3D9EX_DEVICE_INTEL:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
            case CL_CONTEXT_DXVA_DEVICE_INTEL:
                if( properties[ 1 ] != NULL )
                {
                    return false;
                }
                break;
#endif
            default:
                break;
            }
            properties += 2;
        }
    }

    // Check for unsupported devices types for shared context
    for( cl_uint i = 0; i < num_devices; i++)
    {
        cl_device_type deviceType;
        cl_int error = clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);
        if( CL_SUCCESS != error ||
            ( CL_DEVICE_TYPE_CPU != deviceType && CL_DEVICE_TYPE_GPU != deviceType ))
        {
            return false;
        }
    }

    return true;
}

crt_err_code CrtModule::Initialize()
{
    Utils::OclAutoMutex CS(&m_mutex);   // Critical section

    if (m_initializeState == NOT_INITIALIZED)
    {
        crt_err_code res = CRT_SUCCESS;

        char platformVersionStr[ MAX_STRLEN ] = {0};

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

            KHRpfn_clGetExtensionFunctionAddress clGetExtFuncAddr = (KHRpfn_clGetExtensionFunctionAddress)pCrtPlatform->m_lib.GetFunctionPtrByName("clGetExtensionFunctionAddress");
            if( NULL == clGetExtFuncAddr )
            {
                res = CRT_FAIL;
                break;
            }
            KHRpfn_clGetPlatformIDs pfn_clGetPlatformIDs =  (KHRpfn_clGetPlatformIDs)(ptrdiff_t)clGetExtFuncAddr("clIcdGetPlatformIDsKHR");
            if (NULL == pfn_clGetPlatformIDs || !(CL_SUCCESS == pfn_clGetPlatformIDs(1, &pCrtPlatform->m_platformIdDEV, NULL)))
            {
                delete pCrtPlatform;
                continue;
            }

            m_oclPlatforms.push_back( pCrtPlatform );

            // Query the Platform extensions for each platform
            size_t extSize = 0;
            if( CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_EXTENSIONS,
                0,
                NULL,
                &extSize) )
            {
                res = CRT_FAIL;
                break;
            }

            pCrtPlatform->m_supportedExtensionsStr = new char[extSize];
            if( !pCrtPlatform->m_supportedExtensionsStr )
            {
                res = CRT_FAIL;
                break;
            }

            if( CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_EXTENSIONS,
                extSize,
                pCrtPlatform->m_supportedExtensionsStr,
                NULL) )
            {
                res = CRT_FAIL;
                break;
            }
            pCrtPlatform->m_supportedExtensions = GetCrtExtension(pCrtPlatform->m_supportedExtensionsStr);

            // Query the ICD Suffix for each platform
            if( CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_ICD_SUFFIX_KHR,
                0,
                NULL,
                &extSize) )
            {
                res = CRT_FAIL;
                break;
            }

            pCrtPlatform->m_icdSuffix = new char[extSize];
            if( !pCrtPlatform->m_icdSuffix )
            {
                res = CRT_FAIL;
                break;
            }

            if( CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                pCrtPlatform->m_platformIdDEV,
                CL_PLATFORM_ICD_SUFFIX_KHR,
                extSize,
                pCrtPlatform->m_icdSuffix,
                NULL) )
            {
                res = CRT_FAIL;
                break;
            }

            // Get the underlying platform version
            if( CL_SUCCESS != pCrtPlatform->m_platformIdDEV->dispatch->clGetPlatformInfo(
                                    pCrtPlatform->m_platformIdDEV,
                                    CL_PLATFORM_VERSION,
                                    MAX_STRLEN,
                                    platformVersionStr,
                                    NULL ) )
            {
                res = CRT_FAIL;
                break;
            }

            cl_uint curPlatVer = GetPlatformVersion( platformVersionStr );
            if( curPlatVer == 0 )
            {
                res = CRT_FAIL;
                break;
            }

            OCLCRT::crt_ocl_module.m_CrtPlatformVersion = 
				( OCLCRT::crt_ocl_module.m_CrtPlatformVersion >= curPlatVer ) ? 
				OCLCRT::crt_ocl_module.m_CrtPlatformVersion : 
				curPlatVer;
                
            cl_uint num_devices = 0;
            pCrtPlatform->m_platformIdDEV->dispatch->clGetDeviceIDs(
                                    pCrtPlatform->m_platformIdDEV,
                                    CL_DEVICE_TYPE_ALL,
                                    0,
                                    NULL,
                                    &num_devices);
            if( num_devices == 0 )
            {
                res = CRT_FAIL;
                break;
            }
            cl_device_id* pDevices = new cl_device_id[num_devices];
            if( !(CL_SUCCESS == pCrtPlatform->m_platformIdDEV->dispatch->clGetDeviceIDs(
                                    pCrtPlatform->m_platformIdDEV,
                                    CL_DEVICE_TYPE_ALL,
                                    num_devices,
                                    pDevices,
                                    NULL)) )
            {
                res = CRT_FAIL;
                break;
            }

            for( cl_uint j=0; j < num_devices; j++ )
            {
                CrtDeviceInfo* pDevInfo = new CrtDeviceInfo;
                if (NULL == pDevInfo)
                {
                    res = CRT_FAIL;
                    break;
                }

                if( CL_SUCCESS != pDevices[j]->dispatch->clGetDeviceInfo(
                                    pDevices[j],
                                    CL_DEVICE_EXECUTION_CAPABILITIES,
                                    sizeof( pDevInfo->m_deviceCapabilities ),
                                    &pDevInfo->m_deviceCapabilities,
                                    0) )
                {
                    res = CRT_FAIL;
                    break;
                }

                if( CL_SUCCESS != pDevices[j]->dispatch->clGetDeviceInfo( pDevices[j],
                                    CL_DEVICE_TYPE,
                                    sizeof (cl_device_type ),
                                    &pDevInfo->m_devType,
                                    NULL ) )
                {
                    res = CRT_FAIL;
                    break;
                }

                // This is not a sub-device
                pDevInfo->m_isRootDevice = true;
                pDevInfo->m_crtPlatform = pCrtPlatform;
                // Store original device dispatch table entries
                pDevInfo->m_origDispatchTable = *(pDevices[j]->dispatch);

                // store the device types we have loaded;
                // sometimes we load only CPU or GPU; in case the other
                // doesn't exist
                m_availableDeviceTypes |= pDevInfo->m_devType;
                m_deviceInfoMap[pDevices[j]] = pDevInfo;
            }
            // Now we have all devices, patch dispatch table
            for( cl_uint j=0; j < num_devices; j++ )
            {
                crt_ocl_module.PatchClDeviceID(pDevices[j]);
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

    cl_bool cl_context_platform_set         = CL_FALSE;
    cl_bool cl_ctx_interop_user_sync_set    = CL_FALSE;
    cl_bool cl_gl_context_khr_set           = CL_FALSE;
    cl_bool cl_wgl_hdc_khr_set              = CL_FALSE;
#ifdef _WIN32
    cl_bool cl_d3d10_device_khr_set         = CL_FALSE;
    cl_bool cl_d3d9_device_intel_set        = CL_FALSE;
    cl_bool cl_dxva9_device_intel_set       = CL_FALSE;
    cl_bool cl_d3d11_device_khr_set         = CL_FALSE;
#else
#ifdef __linux__
    cl_bool cl_glx_display_khr_set          = CL_FALSE;
#endif
#ifdef LIBVA_SHARING
    cl_bool cl_va_api_display_intel_set     = CL_FALSE;
#endif
#endif

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
                if( ( cl_platform_id )( properties[ 1 ] ) != crt_ocl_module.m_crtPlatformId )
                {
                    errCode = CL_INVALID_PLATFORM;
                }
                break;
            case CL_CONTEXT_INTEROP_USER_SYNC:
                if( ( OCLCRT::crt_ocl_module.m_CrtPlatformVersion < OPENCL_1_2 ) ||
                    ( cl_ctx_interop_user_sync_set == CL_TRUE ) )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_ctx_interop_user_sync_set = CL_TRUE;
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
#ifdef _WIN32
            case CL_CONTEXT_D3D10_DEVICE_KHR:
                if( cl_d3d10_device_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_d3d10_device_khr_set = CL_TRUE;
                break;
            case CL_CONTEXT_D3D11_DEVICE_KHR:
                if( cl_d3d11_device_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_d3d11_device_khr_set = CL_TRUE;
                break;
            case CL_CONTEXT_D3D9_DEVICE_INTEL:
            case CL_CONTEXT_D3D9EX_DEVICE_INTEL:
            case CL_CONTEXT_ADAPTER_D3D9_KHR:
            case CL_CONTEXT_ADAPTER_D3D9EX_KHR:
                if( cl_d3d9_device_intel_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_d3d9_device_intel_set = CL_TRUE;
                break;
            case CL_CONTEXT_DXVA_DEVICE_INTEL:
            case CL_CONTEXT_ADAPTER_DXVA_KHR:
                if( cl_dxva9_device_intel_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_dxva9_device_intel_set  = CL_TRUE;
                break;
#else
#ifdef __linux__
            case CL_GLX_DISPLAY_KHR:
                if( cl_glx_display_khr_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_glx_display_khr_set = CL_TRUE;
                break;
#endif //__linux__
#ifdef LIBVA_SHARING
            case CL_CONTEXT_VA_API_DISPLAY_INTEL:
                if( cl_va_api_display_intel_set == CL_TRUE )
                {
                    return CL_INVALID_PROPERTY;
                }
                cl_va_api_display_intel_set = CL_TRUE;
                break;
#endif
#endif     
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

    if( !properties )
    {
        if( props )
        {
            *props = NULL;
        }
        return CRT_SUCCESS;
    }

    cl_context_properties* clProperties = NULL;
    cl_uint num_entries = 0;
    if (dup)
    {
        while (NULL != *p)
        {
            p+=2;
            num_entries+=2;
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
            break;
        default:
            *clProperties = *properties;
            *(clProperties+1) = *(properties+1);
            break;
        }
        properties+=2;
        clProperties+=2;
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

            if( p->m_supportedExtensionsStr != NULL )
            {
                delete[] p->m_supportedExtensionsStr;
                p->m_supportedExtensionsStr = NULL;
            }

            if( p->m_icdSuffix != NULL )
            {
                delete[] p->m_icdSuffix;
                p->m_icdSuffix = NULL;
            }

            delete p;
        }
        m_oclPlatforms.pop_back();
    }
    // Delete all device info
    for (DEV_INFO_MAP::iterator itr = m_deviceInfoMap.begin(); itr != m_deviceInfoMap.end(); itr++)
    {
        if (itr->second)
        {
            delete itr->second;
            itr->second = NULL;
        }
    }

    if( m_common_extensions != NULL )
    {
        delete[] m_common_extensions;
        m_common_extensions = NULL;
    }
    if( m_crtPlatformId != NULL )
    {
        delete m_crtPlatformId;
        m_crtPlatformId = NULL;
    }


    m_deviceInfoMap.clear();
    m_oclPlatforms.clear();
}

CrtModule::~CrtModule()
{
    Shutdown();
}
