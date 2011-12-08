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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  ContextModule.h
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cl_framework.h"
#include "icontext.h"
#include "icontext_gl.h"
#include "ocl_config.h"
#include "ocl_itt.h"
#include "cl_objects_map.h"

#include <Logger.h>
#if defined (DX9_MEDIA_SHARING)
#include <d3d9.h>
#include <basetsd.h>
#include "ocl_object_base.h"
#endif

namespace Intel { namespace OpenCL { namespace Framework {

	class PlatformModule;
	class Device;
    class FissionableDevice;
	class Context;
	template <class HandleType> class OCLObjectsMap;
	class MemoryObject;
    class Kernel;
    struct D3D9ResourceInfo;

	/**********************************************************************************************
	* Class name:	ContextModule
	*
	* Description:	context module class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class ContextModule : public OCLObjectBase, IContext, IContextGL
	{
	
	public:

		/******************************************************************************************
		* Function: 	ContextModule
		* Description:	The Context Module class constructor
		* Arguments:	pPlatformModule [in] -	pointer to the platform module	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ContextModule(PlatformModule *pPlatformModule);

		/******************************************************************************************
		* Function: 	~ContextModule
		* Description:	The Context Module class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~ContextModule();

		/******************************************************************************************
		* Function: 	Initialize    
		* Description:	Initialize the context module object
		*				and load devices
		* Arguments:		
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		cl_err_code		Initialize(ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData);

		/******************************************************************************************
		* Function: 	Release    
		* Description:	Release the context module's resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The release operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		Release(  bool bTerminate );

		/******************************************************************************************
		* Function: 	GetContext    
		* Description:	Gets a pointer to a context accourding to its cl_context value
		* Arguments:	clContext [in] - a valid context handle               	
		* Return value:	Returns the context object if valid, else returns NULL.
		******************************************************************************************/
        Context*        GetContext( cl_context clContext );

		/******************************************************************************************
		* Function: 	GetKernel
		* Description:	Gets a pointer to a kernel object accourding to its cl_kernel value
		* Arguments:	clKernel [in] - a valid memory kernel handle
		* Return value:	Returns the kernel object if valid, else returns NULL.
		******************************************************************************************/
        Kernel*         GetKernel( cl_kernel clKernel );

		/******************************************************************************************
		* Function: 	GetMemoryObject    
		* Description:	Gets a pointer to a memory object according to its cl_mem value
		* Arguments:	clMemObjId [in] - a valid memory object handle               	
		* Return value:	Returns the memory object if valid, else returns NULL.
		******************************************************************************************/
		MemoryObject * GetMemoryObject(const cl_mem clMemObjId);

		/******************************************************************************************
		* Function: 	GetGPAData    
		* Description:	Returns a pointer to the GPA data object.
		* Arguments:	None               	
		* Return value:	Returns a pointer to the GPA data object.
		******************************************************************************************/
		ocl_gpa_data * GetGPAData() const { return m_pGPAData; }

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
		virtual cl_err_code	RetainProgram(cl_program clProgram);
		virtual cl_err_code ReleaseProgram(cl_program clProgram);
		virtual cl_int BuildProgram(cl_program clProgram, cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const char * pcOptions, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void * pUserData);
		virtual cl_int GetProgramInfo(cl_program clProgram, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		virtual cl_int GetProgramBuildInfo(cl_program clProgram, cl_device_id clDevice, cl_program_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		// kernel methods
		virtual cl_kernel CreateKernel(cl_program clProgram, const char * pscKernelName, cl_int * piErr);
		virtual cl_int CreateKernelsInProgram(cl_program clProgram, cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet);
		virtual cl_int RetainKernel(cl_kernel clKernel);
		virtual cl_int ReleaseKernel(cl_kernel clKernel);
		virtual cl_int SetKernelArg(cl_kernel clKernel, cl_uint	uiArgIndex, size_t szArgSize, const void * pszArgValue);
		virtual cl_int GetKernelInfo(cl_kernel clKernel, cl_kernel_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		virtual cl_int GetKernelWorkGroupInfo(cl_kernel clKernel, cl_device_id pDevice, cl_kernel_work_group_info clParamName, size_t szParamValueSize, void *	pParamValue, size_t * pszParamValueSizeRet);
		// memory object methods
		virtual cl_mem CreateBuffer(cl_context clContext, cl_mem_flags clFlags, size_t szSize, void * pHostPtr, cl_int * pErrcodeRet);
		virtual cl_mem CreateSubBuffer(cl_mem buffer, cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, cl_int * pErrcodeRet);
        virtual cl_mem CreateImage(cl_context context, cl_mem_flags flags, const cl_image_format *image_format, const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret);
		virtual cl_mem CreateImage2D(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, size_t szImageWidth, size_t szImageHeight, size_t szImageRowPitch, void * pHostPtr, cl_int * pErrcodeRet);
		virtual cl_mem CreateImage3D(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, size_t szImageWidth, size_t szImageHeight, size_t szImageDepth, size_t szImageRowPitch, size_t szImageSlicePitch, void * pHostPtr, cl_int * pErrcodeRet);
#if 0   // disabled until changes in the spec regarding 2D image arrays are made
        virtual cl_mem CreateImage2DArray(cl_context clContext, cl_mem_flags clFlags, const cl_image_format * clImageFormat, cl_image_array_type clImageArrayType, const size_t * pszImageWidth, const size_t * pszImageHeight, size_t clNumImages, size_t clImageRowPitch, size_t szImageSlicePitch, void * pHostPtr, cl_int *	pErrcodeRet);
#endif
		virtual cl_int RetainMemObject(cl_mem clMemObj);
		virtual cl_int ReleaseMemObject(cl_mem clMemObj);
		virtual cl_int GetSupportedImageFormats(cl_context clContext, cl_mem_flags clFlags, cl_mem_object_type clImageType, cl_uint uiNumEntries, cl_image_format * pclImageFormats, cl_uint * puiNumImageFormats);
		virtual cl_int GetMemObjectInfo(cl_mem clMemObj, cl_mem_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		virtual cl_int GetImageInfo(cl_mem clImage, cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		virtual cl_int SetMemObjectDestructorCallback (cl_mem memObj,mem_dtor_fn pfn_notify,void *pUserData);
		// sampler methods
		virtual cl_sampler CreateSampler(cl_context clContext, cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, cl_int * pErrcodeRet);
		virtual cl_int RetainSampler(cl_sampler clSampler);
		virtual cl_int ReleaseSampler(cl_sampler clSampler);
		virtual cl_int GetSamplerInfo(cl_sampler clSampler, cl_sampler_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		///////////////////////////////////////////////////////////////////////////////////////////
		// IContextGL methods
		///////////////////////////////////////////////////////////////////////////////////////////
		virtual cl_mem CreateFromGLBuffer(cl_context clContext, cl_mem_flags clMemFlags, GLuint glBufObj, int * pErrcodeRet);
		virtual cl_mem CreateFromGLTexture2D(cl_context clContext, cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, cl_int * pErrcodeRet);
		virtual cl_mem CreateFromGLTexture3D(cl_context clContext, cl_mem_flags clMemFlags, GLenum glTextureTarget, GLint glMipLevel, GLuint glTexture, cl_int * pErrcodeRet);
		virtual cl_mem CreateFromGLRenderbuffer(cl_context clContext, cl_mem_flags clMemFlags, GLuint glRenderBuffer, cl_int * pErrcodeRet);
		virtual cl_int GetGLObjectInfo(cl_mem clMemObj, cl_gl_object_type * pglObjectType, GLuint * pglObjectName);
		virtual cl_int GetGLTextureInfo(cl_mem clMemObj, cl_gl_texture_info clglPramName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

        // Direct3D 9 Sharing methods
#if defined (DX9_MEDIA_SHARING)
        virtual cl_mem CreateFromD3D9Surface(cl_context context, cl_mem_flags flags, IDirect3DSurface9 *resource, HANDLE sharehandle, UINT plane, cl_int *errcode_ret);
#endif
#if defined (DX9_SHARING)
        virtual cl_mem CreateFromD3D9VertexBuffer(cl_context context, cl_mem_flags flags, IDirect3DVertexBuffer9* resource, cl_int* errcode_ret);
        virtual cl_mem CreateFromD3D9IndexBuffer(cl_context context, cl_mem_flags flags, IDirect3DIndexBuffer9* resource, cl_int* errcode_ret);
        virtual cl_mem CreateFromD3D9Texture(cl_context context, cl_mem_flags flags, IDirect3DTexture9 *resource, UINT miplevel, cl_int *errcode_ret);
        virtual cl_mem CreateFromD3D9CubeTexture(cl_context context, cl_mem_flags flags, IDirect3DCubeTexture9 *resource, D3DCUBEMAP_FACES facetype, UINT miplevel, cl_int *errcode_ret);
        virtual cl_mem CreateFromD3D9VolumeTexture(cl_context context, cl_mem_flags flags, IDirect3DVolumeTexture9 *resource, UINT miplevel, cl_int *errcode_ret);
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

	private:

		cl_err_code CheckImageParameters(cl_mem_flags clMemFlags,
										const cl_image_format * clImageFormat,
                                         size_t szImageWidth,
                                         size_t szImageHeight,
                                         size_t szImageDepth,
                                         size_t szImageRowPitch,
                                         size_t szImageSlicePitch,
                                         void * pHostPtr);

		// get pointers to device objects according to the device ids
		cl_err_code GetRootDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices);
        cl_err_code GetDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, FissionableDevice ** ppDevices);

#if defined (DX9_MEDIA_SHARING)
        cl_mem CreateFromD3D9Resource(cl_context clContext, cl_mem_flags flags,
            D3D9ResourceInfo* const pResourceInfo, cl_int *pErrcodeRet,
            cl_mem_object_type clObjType, cl_uint uiDimCnt, const D3DFORMAT d3dFormat, UINT plane = MAXUINT);
#endif

		PlatformModule *						m_pPlatformModule; // handle to the platform module

		OCLObjectsMap<_cl_context_int>			m_mapContexts;	// map list of contexts
		OCLObjectsMap<_cl_program_int>			m_mapPrograms;	// map list of programs
		OCLObjectsMap<_cl_kernel_int>			m_mapKernels;		// map list of kernels
		OCLObjectsMap<_cl_mem_int>				m_mapMemObjects;	// map list of all memory objects
		OCLObjectsMap<_cl_sampler_int>			m_mapSamplers;	// map list of all memory objects

		ocl_entry_points *						m_pOclEntryPoints;

		ocl_gpa_data *							m_pGPAData;

		DECLARE_LOGGER_CLIENT;
	};

}}}
