// Copyright (c) 2006-2012 Intel Corporation
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  ContextModule.h
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cl_framework.h"
#include "ocl_config.h"
#include "ocl_itt.h"
#include "cl_objects_map.h"
#include "Context.h"
#include "GenericMemObj.h"
#include <Logger.h>
#if defined (DX_MEDIA_SHARING)
#include <d3d9.h>
#include <basetsd.h>
#include "CL\cl_d3d9.h"
#if defined (DX_MEDIA_SHARING)
#include "d3d9_definitions.h"
#endif
#endif

namespace Intel { namespace OpenCL { namespace Framework {

    class PlatformModule;
    class Device;
    class FissionableDevice;
    class Context;
    template <class HandleType, class ObjectType> class OCLObjectsMap;
    class MemoryObject;
    class Kernel;
    class IOclCommandQueueBase;
    class OclCommandQueue;
    template<typename T> struct D3DResourceInfo;

    /**********************************************************************************************
    * Class name:    ContextModule
    *
    * Description:    context module class
    * Author:        Uri Levy
    * Date:            December 2008
    **********************************************************************************************/
    class ContextModule
    {
    
    public:

        /******************************************************************************************
        * Function:     ContextModule
        * Description:    The Context Module class constructor
        * Arguments:    pPlatformModule [in] -    pointer to the platform module    
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        ContextModule(PlatformModule *pPlatformModule);

        /******************************************************************************************
        * Function:     ~ContextModule
        * Description:    The Context Module class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        virtual ~ContextModule();

        /******************************************************************************************
        * Function:     Initialize    
        * Description:    Initialize the context module object
        *                and load devices
        * Arguments:        
        * Return value:    CL_SUCCESS - The initializtion operation succeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/        
        cl_err_code        Initialize(ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData);

        /******************************************************************************************
        * Function:     Release    
        * Description:    Release the context module's resources
        * Arguments:        
        * Return value:    CL_SUCCESS - The release operation succeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        cl_err_code        Release(  bool bTerminate );

        /******************************************************************************************
        * Function:     GetContext    
        * Description:    Gets a pointer to a context accourding to its cl_context value
        * Arguments:    clContext [in] - a valid context handle                   
        * Return value:    Returns the context object if valid, else returns NULL.
        ******************************************************************************************/
        SharedPtr<Context>        GetContext( cl_context clContext );

        /******************************************************************************************
        * Function:     GetKernel
        * Description:    Gets a pointer to a kernel object accourding to its cl_kernel value
        * Arguments:    clKernel [in] - a valid memory kernel handle
        * Return value:    Returns the kernel object if valid, else returns NULL.
        ******************************************************************************************/
        SharedPtr<Kernel>         GetKernel( cl_kernel clKernel );

        /******************************************************************************************
        * Function:     GetMemoryObject    
        * Description:    Gets a pointer to a memory object according to its cl_mem value
        * Arguments:    clMemObjId [in] - a valid memory object handle                   
        * Return value:    Returns the memory object if valid, else returns NULL.
        ******************************************************************************************/
        SharedPtr<MemoryObject> GetMemoryObject(const cl_mem clMemObjId);

        /******************************************************************************************
        * Function:     GetGPAData    
        * Description:    Returns a pointer to the GPA data object.
        * Arguments:    None                   
        * Return value:    Returns a pointer to the GPA data object.
        ******************************************************************************************/
        ocl_gpa_data * GetGPAData() const { return m_pGPAData; }

        /******************************************************************************************
        * Function:     IsTerminating    
        * Description:    Returns whether this ContextModule is in the process of terminating
        * Arguments:    None                   
        * Return value:    Returns whether this ContextModule is in the process of terminating
        ******************************************************************************************/
        bool IsTerminating() const { return m_bIsTerminating; }

        /******************************************************************************************
        * Function:     ShutDown    
        * Description:    ShutDown everything
        * Arguments:    bool to skip waiting for queues                   
        * Return value:    none
        ******************************************************************************************/
        void ShutDown( bool wait_for_finish );

        ///////////////////////////////////////////////////////////////////////////////////////////
        // IContext methods
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual cl_context CreateContext(const cl_context_properties * clProperties, cl_uint uiNumDevices, const cl_device_id *pDevices, logging_fn pfnNotify, void *pUserData, cl_int *pRrrcodeRet);
        virtual cl_context CreateContextFromType(const cl_context_properties * clProperties, cl_device_type clDeviceType, logging_fn pfnNotify, void * pUserData, cl_int * pErrcodeRet);
        virtual cl_int RetainContext(cl_context context);
        virtual cl_int ReleaseContext(cl_context context);
        virtual cl_int GetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret);
        // program methods
        virtual cl_program CreateProgramWithSource(cl_context clContext, cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, cl_int * pErrcodeRet);
        virtual cl_program CreateProgramWithBinary(cl_context clContext, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, cl_int * pErrRet);
        virtual cl_program CreateProgramWithBuiltInKernels(cl_context clContext, cl_uint uiNumDevices, const cl_device_id *  pclDeviceList, const char *szKernelNames, cl_int *pErrcodeRet);

        virtual cl_err_code    RetainProgram(cl_program clProgram);
        virtual cl_err_code ReleaseProgram(cl_program clProgram);
        virtual cl_int CompileProgram(cl_program clProgram, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, cl_uint num_input_headers, const cl_program* pclInputHeaders, const char **header_include_names, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData);
        virtual cl_program LinkProgram(cl_context clContext, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, cl_uint uiNumInputPrograms, const cl_program* pclInputPrograms, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData, cl_int *pErrcodeRet);
        virtual cl_int BuildProgram(cl_program clProgram, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData);
        virtual cl_int GetProgramInfo(cl_program clProgram, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        virtual cl_int GetProgramBuildInfo(cl_program clProgram, cl_device_id clDevice, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        // kernel methods
        virtual cl_kernel CreateKernel(cl_program clProgram, const char * pscKernelName, cl_int * piErr);
        virtual cl_int CreateKernelsInProgram(cl_program clProgram, cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet);
        virtual cl_int RetainKernel(cl_kernel clKernel);
        virtual cl_int ReleaseKernel(cl_kernel clKernel);
        virtual cl_int SetKernelArg(cl_kernel clKernel, cl_uint    uiArgIndex, size_t szArgSize, const void * pszArgValue);
        virtual cl_int GetKernelInfo(cl_kernel clKernel, cl_kernel_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        virtual cl_int GetKernelWorkGroupInfo(cl_kernel clKernel, cl_device_id pDevice, cl_kernel_work_group_info clParamName, size_t szParamValueSize, void *    pParamValue, size_t * pszParamValueSizeRet);
        
        // memory object methods
        virtual cl_mem CreateBuffer(cl_context clContext, cl_mem_flags clFlags, size_t szSize, void * pHostPtr, cl_int * pErrcodeRet);
        virtual cl_mem CreateSubBuffer(cl_mem buffer, cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, cl_int * pErrcodeRet);
        virtual cl_mem CreateImage2D(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, size_t szImageWidth, size_t szImageHeight, size_t szImageRowPitch, void * pHostPtr, cl_int * pErrcodeRet);
        virtual cl_mem CreateImage3D(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, size_t szImageWidth, size_t szImageHeight, size_t szImageDepth, size_t szImageRowPitch, size_t szImageSlicePitch, void * pHostPtr, cl_int * pErrcodeRet);
        virtual cl_mem CreateImage(cl_context context, cl_mem_flags flags, const cl_image_format *image_format, const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret);        
        virtual cl_mem Create2DImageFromImage(cl_context context, cl_mem_flags flags, const cl_image_format* pImageFormat, const cl_image_desc* pImageDesc, cl_mem otherImgHandle, cl_int* iErrcodeRet);
        virtual cl_mem CreateImageArray(cl_context clContext, cl_mem_flags clFlags, const cl_image_format* clImageFormat, const cl_image_desc* pClImageDesc, void* pHostPtr, cl_int* pErrcodeRet);
        virtual cl_int RetainMemObject(cl_mem clMemObj);
        virtual cl_int ReleaseMemObject(cl_mem clMemObj);
        virtual cl_int GetSupportedImageFormats(cl_context clContext, cl_mem_flags clFlags, cl_mem_object_type clImageType, cl_uint uiNumEntries, cl_image_format * pclImageFormats, cl_uint * puiNumImageFormats);
        virtual cl_int GetMemObjectInfo(cl_mem clMemObj, cl_mem_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        virtual cl_int GetImageInfo(cl_mem clImage, cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        virtual cl_int SetMemObjectDestructorCallback (cl_mem memObj,mem_dtor_fn pfn_notify,void *pUserData);
        // sampler methods
        virtual cl_sampler CreateSampler(cl_context clContext, cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, cl_int * pErrcodeRet);
        virtual cl_sampler CreateSamplerWithProperties (cl_context clContext, const cl_sampler_properties *pSamplerProperties, cl_int *pErrcodeRet);
        virtual cl_int RetainSampler(cl_sampler clSampler);
        virtual cl_int ReleaseSampler(cl_sampler clSampler);
        virtual cl_int GetSamplerInfo(cl_sampler clSampler, cl_sampler_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // IContextGL methods
        ///////////////////////////////////////////////////////////////////////////////////////////
        virtual cl_mem CreateFromGLBuffer(cl_context clContext, cl_mem_flags clMemFlags, GLuint glBufObj, int * pErrcodeRet);
        virtual cl_mem CreateFromGLTexture(cl_context clContext, cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, cl_int * pErrcodeRet);
        virtual cl_mem CreateFromGLRenderbuffer(cl_context clContext, cl_mem_flags clMemFlags, GLuint glRenderBuffer, cl_int * pErrcodeRet);
        virtual cl_int GetGLObjectInfo(cl_mem clMemObj, cl_gl_object_type * pglObjectType, GLuint * pglObjectName);
        virtual cl_int GetGLTextureInfo(cl_mem clMemObj, cl_gl_texture_info clglPramName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

        // Direct3D 9 Sharing methods
#if defined (DX_MEDIA_SHARING)
        virtual cl_mem CreateFromD3D9Surface(cl_context context, cl_mem_flags flags,
            cl_dx9_media_adapter_type_khr adapterType, cl_dx9_surface_info_khr* pSurfaceInfo, UINT plane, cl_int *errcode_ret, const ID3DSharingDefinitions& d3d9Definitions);
        virtual cl_mem CreateFromD3D11Buffer(cl_context context, cl_mem_flags flags, ID3D11Buffer* pResource, cl_int* pErrcodeRet);
        virtual cl_mem CreateFromD3D11Texture2D(cl_context context, cl_mem_flags flags, ID3D11Texture2D* pResource, UINT uiSubresource, cl_int* pErrcodeRet);
        virtual cl_mem CreateFromD3D11Texture3D(cl_context context, cl_mem_flags flags, ID3D11Texture3D* pResource, UINT uiSubresource, cl_int* pErrcodeRet);
#endif

        /////////////////////////////////////////////////////////////////////
        // OpenCL 1.2 functions
        /////////////////////////////////////////////////////////////////////

        virtual cl_int GetKernelArgInfo(cl_kernel clKernel,
                                                cl_uint argIndx,
                                                cl_kernel_arg_info paramName,
                                                size_t      szParamValueSize,
                                                void *      pParamValue,
                                                size_t *    pszParamValueSizeRet);

        /////////////////////////////////////////////////////////////////////
        // OpenCL 2.0 functions
        /////////////////////////////////////////////////////////////////////

        // SVM

        void* SVMAlloc(cl_context context, cl_svm_mem_flags flags, size_t size, unsigned int uiAlignment);
        void SVMFree(cl_context context, void* pSvmPtr);        
        cl_int SetKernelArgSVMPointer(cl_kernel clKernel, cl_uint uiArgIndex, const void* pArgValue);
        cl_int SetKernelExecInfo(cl_kernel clKernel, cl_kernel_exec_info paramName, size_t szParamValueSize, const void* pParamValue);

        // pipes

        cl_mem CreatePipe(cl_context context, cl_mem_flags flags, cl_uint uiPipePacketSize,    cl_uint uiPipeMaxPackets, const cl_pipe_properties* pProperties, void* pHostPtr, size_t* pSizeRet,
            cl_int* piErrcodeRet);

        cl_int GetPipeInfo(cl_mem pipe, cl_pipe_info paramName, size_t szParamValueSize, void *pParamValue, size_t* pszParamValueSizeRet);

        ///////////////////////////////////////////////////////////////////////
        // Utility functions
        ///////////////////////////////////////////////////////////////////////
        void CommandQueueCreated( OclCommandQueue* queue );
        void CommandQueueRemoved( OclCommandQueue* queue );

        void RegisterMappedMemoryObject( MemoryObject* pMemObj );
        void UnRegisterMappedMemoryObject( MemoryObject* pMemObj );

    private:
        
        ContextModule(const ContextModule&);
        ContextModule& operator=(const ContextModule&);

        cl_err_code CheckMemObjectParameters(cl_mem_flags clMemFlags,
                                        const cl_image_format * clImageFormat,
                                        cl_mem_object_type clMemObjType,
                                         size_t szImageWidth,
                                         size_t szImageHeight,
                                         size_t szImageDepth,
                                         size_t szImageRowPitch,
                                         size_t szImageSlicePitch,
                                         size_t szArraySize,
                                         void * pHostPtr);

        /**
         * Check context specific limitations (device capabilities).
         * @param pContext
         * @param image_type
         * @param image_width
         * @param image_height
         * @param image_depth
         * @param array_size
         * @param pImgBufferHostPtr
         * @return CL_SUCCESS if all parameters OK.
         */
        cl_err_code CheckContextSpecificParameters(SharedPtr<Context>pContext,
                                        const cl_mem_object_type image_type,
                                        const size_t image_width,
                                        const size_t image_height,
                                        const size_t image_depth,
                                        const size_t array_size,
                                        const void* pImgBufferHostPtr = NULL,
                                        cl_mem_flags bufFlags = 0);


        // get pointers to device objects according to the device ids
        cl_err_code GetRootDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, SharedPtr<Device>* ppDevices);
        cl_err_code GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, SharedPtr<FissionableDevice>* ppDevices);

        template<size_t DIM, cl_mem_object_type OBJ_TYPE> cl_mem CreateScalarImage(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, size_t szImageWidth, size_t szImageHeight, size_t szImageDepth, size_t szImageRowPitch, size_t szImageSlicePitch, void * pHostPtr, cl_int * pErrcodeRet, bool bIsImageBuffer = false);

        template<size_t DIM, cl_mem_object_type OBJ_TYPE>
        cl_mem CreateImageBuffer(cl_context context, cl_mem_flags clFlags, const cl_image_format* clImageFormat, const cl_image_desc& desc, cl_mem buffer, cl_int* pErrcodeRet);

        void RemoveAllMemObjects( bool preserve_user_handles );
        void RemoveAllSamplers( bool preserve_user_handles );
        void RemoveAllKernels( bool preserve_user_handles );
        void RemoveAllPrograms( bool preserve_user_handles );

        bool Check2DImageFromBufferPitch(const ConstSharedPtr<GenericMemObject>& pBuffer, const cl_image_desc& desc, const cl_image_format& format) const;

#if defined (DX_MEDIA_SHARING)
        template<typename RESOURCE_TYPE, typename DEV_TYPE>
        cl_mem CreateFromD3DResource(cl_context clContext, cl_mem_flags flags, D3DResourceInfo<RESOURCE_TYPE>* const pResourceInfo, cl_int *pErrcodeRet,
            cl_mem_object_type clObjType, cl_uint uiDimCnt, UINT plane = MAXUINT);
#endif

        PlatformModule *                        m_pPlatformModule; // handle to the platform module

        OCLObjectsMap<_cl_context_int>          m_mapContexts;     // map list of contexts
        OCLObjectsMap<_cl_program_int>          m_mapPrograms;     // map list of programs
        OCLObjectsMap<_cl_kernel_int>           m_mapKernels;      // map list of kernels
        OCLObjectsMap<_cl_mem_int>              m_mapMemObjects;   // map list of all memory objects
        OCLObjectsMap<_cl_sampler_int>          m_mapSamplers;     // map list of all memory objects
        std::map<void*, SharedPtr<Context> >    m_mapSVMBuffers;   // map list of all svm objects

        Intel::OpenCL::Utils::LifetimeObjectContainer<OclCommandQueue> m_setQueues;    // set of all queues including invisible to user
        Intel::OpenCL::Utils::LifetimeObjectContainer<MemoryObject>    m_setMappedMemObjects; // set of all memory objects that were mapped at least once in a history

        ocl_entry_points *                      m_pOclEntryPoints;

        ocl_gpa_data *                          m_pGPAData;
        bool                                    m_bIsTerminating;        

        DECLARE_LOGGER_CLIENT;
    };

    template<size_t DIM, cl_mem_object_type OBJ_TYPE>
    cl_mem ContextModule::CreateScalarImage(cl_context clContext, 
        cl_mem_flags clFlags, 
        const cl_image_format * clImageFormat, 
        size_t szImageWidth, 
        size_t szImageHeight, 
        size_t szImageDepth, 
        size_t szImageRowPitch, 
        size_t szImageSlicePitch, 
        void * pHostPtr, 
        cl_int * pErrcodeRet,
        bool bIsImageBuffer)
    {
        assert(DIM >= 1 && DIM <= 3);
        LOG_INFO(TEXT("Enter CreateScalarImage (clContext=%p, clFlags=0x%X, cl_data_type=0x%x, cl_channel_order=0x%x, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d, pHostPtr=%p, pErrcodeRet=%x)"), 
            (void*)clContext, clFlags, clImageFormat->image_channel_data_type, clImageFormat->image_channel_order, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, pHostPtr, pErrcodeRet);

        SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
        if (NULL == pContext)
        {
            LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_INVALID_CONTEXT;
            }
            return CL_INVALID_HANDLE;
        }

        // Do some initial (not context specific) parameter checking
        // check input memory flags
        cl_err_code clErr = CheckMemObjectParameters(clFlags, clImageFormat, OBJ_TYPE, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, 0, pHostPtr);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("%s"), TEXT("Parameter check failed"));
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = clErr;
            }
            return CL_INVALID_HANDLE;
        }

        clErr = CheckContextSpecificParameters(pContext, OBJ_TYPE, szImageWidth, szImageHeight, szImageDepth, 0);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("%s"), TEXT("Context specific parameter check failed"));
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = clErr;
            }
            return CL_INVALID_HANDLE;
        }

        // Create image from context
        const size_t szDims[] = {szImageWidth, szImageHeight, szImageDepth}, szPitches[] = {szImageRowPitch, szImageSlicePitch};
        SharedPtr<MemoryObject> pImage = NULL;
        clErr = pContext->CreateImage<DIM, OBJ_TYPE>(clFlags, clImageFormat, pHostPtr, szDims, szPitches, &pImage, bIsImageBuffer);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("pContext->CreateImage(%d, %d, %d, %d, %d, %d, %d, %d, %d) = %s"), clFlags, clImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, &pImage, ClErrTxt(clErr));
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_ERR_OUT(clErr);
            }
            return CL_INVALID_HANDLE;
        }
        clErr = m_mapMemObjects.AddObject(pImage, false);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %S"), pImage.GetPtr(), pImage->GetHandle(), ClErrTxt(clErr))
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
        return pImage->GetHandle();
    }

#if defined (DX_MEDIA_SHARING)

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_mem ContextModule::CreateFromD3DResource(cl_context clContext, cl_mem_flags clMemFlags, D3DResourceInfo<RESOURCE_TYPE>* const pResourceInfo, cl_int *pErrcodeRet,
                                             cl_mem_object_type clObjType, cl_uint uiDimCnt, UINT plane)
{
    SharedPtr<Context> pContext = NULL;
    SharedPtr<MemoryObject> pMemObj = NULL;

    cl_err_code clErr = CheckMemObjectParameters(clMemFlags, NULL, 0, 0, 0, 0, 0, 0, 0, NULL);
    if (CL_FAILED(clErr))
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    pContext = m_mapContexts.GetOCLObject((_cl_context_int*)clContext).DynamicCast<Context>();
    
    if (CL_FAILED(clErr) || NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d, %d) = %s , pContext = %d"), clContext, pContext, ClErrTxt(clErr), pContext);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }
    
    SharedPtr<D3DContext<RESOURCE_TYPE, DEV_TYPE>> pD3DContext = pContext.DynamicCast<D3DContext<RESOURCE_TYPE, DEV_TYPE>>();
    if (NULL == pD3DContext)
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
            *pErrcodeRet = pD3DContext->GetD3dDefinitions().GetInvalidResource();
        }
        return CL_INVALID_HANDLE;
    }
    /* check if context was created against the same Direct3D 9 device from which resource was
        created */
    DEV_TYPE* const pResourceDevice = pD3DContext->GetDevice(pResourceInfo->m_pResource);
    if (NULL == pResourceDevice)
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_VALUE;
        }
        return CL_INVALID_HANDLE;
    }
    pResourceDevice->Release(); // we don't need the device, just its address
    // Matt is aware that there is a hole in the spec regarding checking this device type
    const ID3DSharingDefinitions& d3dSharingDefs = pD3DContext->GetD3dDefinitions();
    const ID3D9Definitions* const d3d9Defs = dynamic_cast<const ID3D9Definitions*>(&d3dSharingDefs);
    if (NULL != d3d9Defs && d3d9Defs->GetContextAdapterDxva() != pD3DContext->m_iDeviceType &&
        pResourceDevice != pD3DContext->GetD3DDevice())
    {
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = pD3DContext->GetD3dDefinitions().GetInvalidResource();
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
    clErr = pD3DContext->CreateD3DResource(clMemFlags, pResourceInfo, &pMemObj, clObjType, uiDimCnt, plane);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("pD3DContext->CreateD3DResource(%d, %d, %d, %d) = %s"), clMemFlags, pResourceInfo, &pMemObj, ClErrTxt(clErr));
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_ERR_OUT(clErr);
        }
        return CL_INVALID_HANDLE;
    }
    clErr = m_mapMemObjects.AddObject(pMemObj, false);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%d, %d, false) = %s"), pMemObj, pMemObj->GetHandle(), ClErrTxt(clErr));
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

#endif

template<size_t DIM, cl_mem_object_type OBJ_TYPE>
cl_mem ContextModule::CreateImageBuffer(cl_context context, cl_mem_flags clFlags, const cl_image_format* clImageFormat, const cl_image_desc& desc, cl_mem buffer, cl_int* pErrcodeRet)
{
    cl_err_code clErr = CL_SUCCESS;    

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();

    if (NULL == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), context);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

    SharedPtr<GenericMemObject> pBuffer = m_mapMemObjects.GetOCLObject((_cl_mem_int*)buffer).DynamicCast<GenericMemObject>();
    if (CL_FAILED(clErr) || NULL == pBuffer)
    {
        LOG_ERROR(TEXT("GetOCLObject(%d, %d) returned %s"), buffer, &pBuffer, ClErrTxt(clErr));
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

    clErr = CheckContextSpecificParameters(pContext, (cl_mem_object_type)OBJ_TYPE, desc.image_width, desc.image_height, desc.image_depth, desc.image_array_size, pBuffer->GetHostPtr(), bufFlags);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("%s"), TEXT("Context specific parameter check failed"), "");
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = clErr;
        }
        return CL_INVALID_HANDLE;
    }

    size_t szDims[] = { desc.image_width, desc.image_height, desc.image_depth},
        szPitches[] = { desc.image_row_pitch, desc.image_slice_pitch };    
    if (GenericMemObjectBackingStore::calculate_size(clGetPixelBytesCount(clImageFormat), DIM, szDims, szPitches) > pBuffer->GetSize())
    {
        LOG_ERROR(TEXT("Size of image must be <= size of buffer object data store"), "");
        if (pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }    

    if (OBJ_TYPE == CL_MEM_OBJECT_IMAGE2D && !Check2DImageFromBufferPitch(pBuffer, desc, *clImageFormat))
    {
        LOG_ERROR(TEXT("pitch isn't a multiple of the maximum of the CL_DEVICE_IMAGE_PITCH_ALIGNMENT value for all devices in the context associated with image_desc->buffer"), "");
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
    }

    /* if the CL_MEM_READ_WRITE, CL_MEM_READ_ONLY or CL_MEM_WRITE_ONLY values are not specified in flags, they are inherited from the corresponding memory access qualifers associated with
       mem_object */
    if (0 == (clFlags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)))
    {
        clFlags |= pBuffer->GetFlags() & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
    }
    /* The CL_MEM_USE_HOST_PTR, CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR values cannot be specified in flags but are inherited from the corresponding memory access qualifiers associated with
       mem_object. */
    clFlags |= pBuffer->GetFlags() & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
    /* If the CL_MEM_HOST_WRITE_ONLY, CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS values are not specified in flags, they are inherited from the corresponding memory access qualifiers
       associated with mem_object. */
    if (0 == (clFlags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)))
    {
        clFlags |= pBuffer->GetFlags() & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);
    }

    void* const pBufferData = pBuffer->GetBackingStoreData();
    const cl_mem clImgBuf = CreateScalarImage<DIM, OBJ_TYPE>(context, clFlags, clImageFormat, desc.image_width, desc.image_height, desc.image_depth, desc.image_row_pitch,
                                                             desc.image_slice_pitch, pBufferData, pErrcodeRet, true);
    if (CL_INVALID_HANDLE == clImgBuf)
    {
        return clImgBuf;
    }

    SharedPtr<MemoryObject> pImgBuf = m_mapMemObjects.GetOCLObject((_cl_mem_int*)clImgBuf).DynamicCast<MemoryObject>();
    assert(pImgBuf);
    pImgBuf->SetParent(pBuffer);
    return clImgBuf;
}

}}}
