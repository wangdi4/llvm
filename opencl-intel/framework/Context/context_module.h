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

#pragma once
#include "cl_framework.h"
#include "ocl_config.h"
#include "ocl_itt.h"
#include "cl_objects_map.h"
#include "Context.h"
#include "GenericMemObj.h"
#include <Logger.h>

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
        virtual cl_program CreateProgramWithIL(cl_context clContext, const unsigned char* pIL, size_t length, cl_int* pErrcodeRet);
        virtual cl_program CreateProgramWithBinary(cl_context clContext, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, cl_int * pErrRet);
        virtual cl_program CreateProgramWithBuiltInKernels(cl_context clContext, cl_uint uiNumDevices, const cl_device_id *  pclDeviceList, const char *szKernelNames, cl_int *pErrcodeRet);

        virtual cl_err_code RetainProgram(cl_program clProgram);
        virtual cl_err_code ReleaseProgram(cl_program clProgram);
        virtual cl_int CompileProgram(cl_program clProgram, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, cl_uint num_input_headers, const cl_program* pclInputHeaders, const char **header_include_names, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData);
        virtual cl_program LinkProgram(cl_context clContext, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, cl_uint uiNumInputPrograms, const cl_program* pclInputPrograms, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData, cl_int *pErrcodeRet);
        virtual cl_int BuildProgram(cl_program clProgram, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData);
        virtual cl_int GetProgramInfo(cl_program clProgram, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        virtual cl_int GetProgramBuildInfo(cl_program clProgram, cl_device_id clDevice, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        // kernel methods
        virtual cl_kernel CreateKernel(cl_program clProgram, const char * pscKernelName, cl_int * piErr);
        virtual cl_kernel CloneKernel(cl_kernel source_kernel, cl_int * pErr);
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

        cl_int GetKernelSubGroupInfo(cl_kernel kernel, cl_device_id device,
                                     cl_kernel_sub_group_info param_name, size_t input_value_size,
                                     const void* input_value, size_t param_value_size,
                                     void* param_value, size_t* param_value_size_ret);
        cl_mem CreatePipe(cl_context context, cl_mem_flags flags, cl_uint uiPipePacketSize,    cl_uint uiPipeMaxPackets, const cl_pipe_properties* pProperties, void* pHostPtr, size_t* pSizeRet,
            cl_int* piErrcodeRet);

        cl_int GetPipeInfo(cl_mem pipe, cl_pipe_info paramName, size_t szParamValueSize, void *pParamValue, size_t* pszParamValueSizeRet);

        ///////////////////////////////////////////////////////////////////////
        // IntelFPGA functions
        ///////////////////////////////////////////////////////////////////////

        void* MapHostPipeIntelFPGA(cl_mem pipe, cl_map_flags flags,
                                   size_t requestedSize, size_t* pMappedSize,
                                   cl_int* pError);
        cl_int UnmapHostPipeIntelFPGA( cl_mem pipe, void* pMappedPtr,
                                       size_t sizeToUnmap,
                                       size_t* pUnmappedSize);
        cl_int ReadPipeIntelFPGA( cl_mem pipe, void* pDst );
        cl_int WritePipeIntelFPGA( cl_mem pipe, const void* pSrc);
        cl_int GetProfileDataDeviceIntelFPGA(
            cl_device_id device_id, cl_program program,
            cl_bool read_enqueue_kernels, cl_bool read_auto_enqueued,
            cl_bool clear_counters_after_readback, size_t param_value_size,
            void *param_value, size_t *param_value_size_ret,
            cl_int *errcode_ret);

        ///////////////////////////////////////////////////////////////////////
        // cl_intel_function_pointers functions
        ///////////////////////////////////////////////////////////////////////

        cl_int GetDeviceFunctionPointer(cl_device_id device,
            cl_program program, const char* func_name,
            cl_ulong* function_pointer_ret);

        ///////////////////////////////////////////////////////////////////////
        // cl_intel_unified_shared_memory functions
        ///////////////////////////////////////////////////////////////////////

        void* USMHostAlloc(cl_context context,
                           const cl_mem_properties_intel* properties,
                           size_t size, cl_uint alignment,
                           cl_int* errcode_ret);
        void* USMDeviceAlloc(cl_context context, cl_device_id device,
                             const cl_mem_properties_intel* properties,
                             size_t size, cl_uint alignment,
                             cl_int* errcode_ret);
        void* USMSharedAlloc(cl_context context, cl_device_id device,
                             const cl_mem_properties_intel* properties,
                             size_t size, cl_uint alignment,
                             cl_int* errcode_ret);
        cl_int USMFree(cl_context context, const void* ptr);
        cl_int GetMemAllocInfoINTEL(cl_context context, const void* ptr,
                                    cl_mem_info_intel param_name,
                                    size_t param_value_size, void* param_value,
                                    size_t* param_value_size_ret);

        // Set a pointer into a Unified Shared Memory allocation as an argument
        // to a kernel
        cl_int SetKernelArgUSMPointer(cl_kernel clKernel, cl_uint uiArgIndex,
                                      const void* pArgValue);

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
                                         void * pHostPtr,
                                         SharedPtr<Context> pContext);

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

        PlatformModule *                        m_pPlatformModule; // handle to the platform module

        OCLObjectsMap<_cl_context_int>          m_mapContexts;     // map list of contexts
        OCLObjectsMap<_cl_program_int>          m_mapPrograms;     // map list of programs
        OCLObjectsMap<_cl_kernel_int>           m_mapKernels;      // map list of kernels
        OCLObjectsMap<_cl_mem_int>              m_mapMemObjects;   // map list of all memory objects
        OCLObjectsMap<_cl_sampler_int>          m_mapSamplers;     // map list of all memory objects
        std::map<void*, SharedPtr<Context> >    m_mapSVMBuffers;   // map list of all svm objects
        // map list of all unified shared memory objects
        std::map<void*, SharedPtr<Context> >    m_mapUSMBuffers;

        Intel::OpenCL::Utils::LifetimeObjectContainer<OclCommandQueue> m_setQueues; // set of all queues including invisible to user
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
        if (0 == pContext)
        {
            LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), clContext);
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_INVALID_CONTEXT;
            }
            return CL_INVALID_HANDLE;
        }
        if (pContext->IsFPGAEmulator())
        {
            if (NULL != pErrcodeRet)
            {
                *pErrcodeRet = CL_INVALID_OPERATION;
            }
            return CL_INVALID_HANDLE;
        }

        // Do some initial (not context specific) parameter checking
        // check input memory flags
        cl_err_code clErr = CheckMemObjectParameters(clFlags, clImageFormat, OBJ_TYPE, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, 0, pHostPtr, pContext);
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

template<size_t DIM, cl_mem_object_type OBJ_TYPE>
cl_mem ContextModule::CreateImageBuffer(cl_context context, cl_mem_flags clFlags, const cl_image_format* clImageFormat, const cl_image_desc& desc, cl_mem buffer, cl_int* pErrcodeRet)
{
    cl_err_code clErr = CL_SUCCESS;

    SharedPtr<Context> pContext = m_mapContexts.GetOCLObject((_cl_context_int*)context).DynamicCast<Context>();

    if (0 == pContext)
    {
        LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%d) = NULL"), context);
        if (NULL != pErrcodeRet)
        {
            *pErrcodeRet = CL_INVALID_CONTEXT;
        }
        return CL_INVALID_HANDLE;
    }

    SharedPtr<GenericMemObject> pBuffer = m_mapMemObjects.GetOCLObject((_cl_mem_int*)buffer).DynamicCast<GenericMemObject>();
    if (CL_FAILED(clErr) || 0 == pBuffer)
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
