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
//  iplatform.h
//  Implementation of the IContext interface
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_ICONTEXT_H_)
#define OCL_ICONTEXT_H_

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	IContext
	*
	* Description:	IContext iterface - outlines the context OpneCL related functions
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class IContext
	{
	public:
        virtual ~IContext(){};
		/******************************************************************************************
		* Function: 	CreateContext    
		* Description:	creates an OpenCL context. An OpenCL context is created with one or more 
		*				devices. Contexts are used by the OpenCL runtime for managing objects such 
		*				as command-queues, memory, program and kernel objects and for executing 
		*				kernels on one or more devices specified in the context
		* Arguments:	clProperties [in] -	is reserved and must be zero	
		*				uiNumDevices [in] -	is the number of devices specified in the pDevices 
		*									argument
		*				pDevices [in] -		is a pointer to a list of unique devices returned by 
		*									GetDeviceIDs. If more than one device is specified in 
		*									pDevices, an implementation-defined selection criteria 
		*									may be applied to determine if the list of devices 
		*									specified can be used together to create a context
		*				pfnNotify [in] -	is a callback function that can be registered by the 
		*									application. This callback function will be used by the 
		*									OpenCL implementation to report information on errors 
		*									that occur in this context. This callback function may 
		*									be called asynchronously by the OpenCL implementation.
		*									It is the application’s responsibility to ensure that 
		*									the callback function is thread-safe. The parameters to 
		*									this callback function are:
		*									errinfo					is a pointer to an error string.
		*									private_info and cb		represent a pointer 
		*															to binary data that is returned 
		*															by the OpenCL implementation  
		*															that can be used to log  
		*															additional information helpful 
		*															in debugging the error.
		*									user_data				is a pointer to user supplied 
		*															data.
		*									If pfnNotify is NULL, no callback function is registered
		*				pUserData [in]		will be passed as the user_data argument when pfnNotify 
		*									is called. pUserData can be NULL
		*				pRrrcodeRet [out]	will return an appropriate error code. If pRrrcodeRet is 
		*									NULL, no error code is returned
		* Return value:	CL_INVALID_VALUE 		if properties is not zero or if devices is NULL or if 
		*										num_devices is equal to zero
		*				CL_INVALID_DEVICE 		if devices contains an invalid device
		*				CL_INVALID_DEVICE_LIST	if more than one device is specified in devices and 
		*										the list of devices specified cannot be used together 
		*										to create a context
		*				CL_DEVICE_NOT_AVAILABLE if a device in devices is currently not available even 
		*										though the device was returned by clGetDeviceIDs
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources required 
		*										by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_context	CreateContext(	const cl_context_properties * IN  clProperties,
											cl_uint                       IN  uiNumDevices,
											const cl_device_id *          IN  pDevices,
											logging_fn                    IN  pfnNotify,
											void *                        IN  pUserData,
											cl_int *                      OUT pRrrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateContextFromType    
		* Description:	creates an OpenCL context from a device type that identifies the specific 
		*				device(s) to use
		* Arguments:	clProperties [in] -	is reserved and must be zero	
		*				clDeviceType [in] -	is a bit-field that identifies the type of device
		*				pfnNotify [in] -	described in CreateContext...
		*				pUserData [in] -	described in CreateContext...
		*				pErrrcodeRet [out]	will return an appropriate error code. If pErrrcodeRet is 
		*									NULL, no error code is returned
		* Return value:	CL_INVALID_VALUE 		if clProperties is not zero
		*				CL_INVALID_DEVICE_TYPE	if clDeviceType is not a valid value
		*				CL_DEVICE_NOT_AVAILABLE if no devices that match clDeviceType are currently 
		*										available
		*				CL_DEVICE_NOT_FOUND		if no devices that match clDeviceType were found
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources required 
		*										by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_context CreateContextFromType(	const cl_context_properties * IN  clProperties,
													cl_device_type                IN  clDeviceType,
													logging_fn                    IN  pfnNotify,
													void *                        IN  pUserData,
													cl_int *                      OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	RetainContext    
		* Description:	increments the context reference count
		* Arguments:	context [in] -	specifies the OpenCL context being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		
		virtual cl_int	RetainContext( cl_context IN context ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseContext    
		* Description:	decrements the context reference count. After the context reference count 
		*				becomes zero and all the objects attached to context (such as memory 
		*				objects, command-queues) are released, the context is deleted.
		* Arguments:	context [in] -	specifies the OpenCL context being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_int ReleaseContext( cl_context IN context ) = 0;

		/******************************************************************************************
		* Function: 	GetContextInfo    
		* Description:	can be used to query information about a context
		* Arguments:	context [in] -				specifies the OpenCL context being queried	
		*				param_name [in] -			is an enum that specifies the information to 
		*											query
		*				param_value [out] -			is a pointer to memory where the appropriate  
		*											result being queried is returned. If 
		*											param_value is NULL, it is ignored
		*				param_value_size [in] -		specifies the size in bytes of memory pointed 
		*											to by param_value. This size must be greater 
		*											than or equal to the size of return type
		*				param_value_size_ret [out]-	returns the actual size in bytes of data being 
		*											queried by param_value. If param_value_size_ret 
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		*				CL_INVALID_VALUE		if param_name is not one of the supported values 
		*										or if size in bytes specified by param_value_size
		*										is < size of return type and param_value is not a 
		*										NULL value
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_int GetContextInfo(	cl_context      IN  context,
										cl_context_info IN  param_name,
										size_t          IN  param_value_size,
										void *          OUT param_value,
										size_t *        OUT param_value_size_ret ) = 0;


		/******************************************************************************************
		* Function: 	CreateBuffer    
		* Description:	A buffer object is created using the following function
		* Arguments:	clContext [in]		is a valid OpenCL context used to create the buffer 
		*									object	
		*				clFlags [in]		is a bit-field that is used to specify allocation and 
		*									usage information such as the memory arena that should
		*									be used to allocate the buffer object and how it will 
		*									be used
		*				szSize [in]			is the size in bytes of the buffer memory object to be
		*									allocated.
		*				pHostPtr [in]		is a pointer to the buffer data that may already be 
		*									allocated by the application. The size of the buffer 
		*									that pHostPtr points to must be >= size bytes. Passing 
		*									in a pointer to an already allocated buffer on the host
		*									and using it as a buffer object allows applications to 
		*									share data efficiently with kernels and the host
		*				pErrcodeRet [out]	will return an appropriate error code. If pErrcodeRet
		*									is NULL, no error code is returned
		* Return value:	CL_SUCCESS				the buffer object is created successfully
		*				CL_INVALID_CONTEXT		if context is not a valid context
		*				CL_INVALID_VALUE		if values specified in clFlags are not valid
		*				CL_INVALID_BUFFER_SIZE	if szSize is 0 or is greater than 
		*										CL_DEVICE_MAX_MEM_ALLOC_SIZE
		*				CL_INVALID_HOST_PTR		if pHostPtr is NULL and CL_MEM_USE_HOST_PTR or 
		*										CL_MEM_COPY_HOST_PTR are set in clFlags or if 
		*										pHostPtr is not NULL but CL_MEM_COPY_HOST_PTR or
		*										CL_MEM_USE_HOST_PTR are not set in clFlags
		*				CL_MEM_OBJECT_ALLOCATION_FAILURE	if there is a failure to allocate 
		*														memory for buffer object
		*				CL_INVALID_OPERATION	if the buffer object cannot be created for all 
		*										devices in context
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_mem CreateBuffer(	cl_context   IN  clContext, 
										cl_mem_flags IN  clFlags, 
										size_t       IN  szSize, 
										void *       IN  pHostPtr, 
										cl_int *     OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateImage2D    
		* Description:	An image (1D, or 2D) object is created using the following function
		* Arguments:	clContext [in]			is a valid OpenCL context on which the image object
		*										is to be created	
		*				clFlags [in]			is a bit-field that is used to specify allocation
		*										and usage information about the image memory object
		*										being created
		*				szSize [in]				is the size in bytes of the buffer memory object to
		*										be allocated.
		*				clImageFormat [in]		s a pointer to a structure that describes format 
		*										properties of the image to be allocated
		*				szImageWidth [in]		is the width of the image in pixels. This must be
		*										value greater than or equal to 1.
		*				szImageHeight [in]		is the height of the image in pixels. This must be
		*										value greater than or equal to 1.
		*				szImageRowPitch [in]	is the scan-line pitch in bytes. This must be 0 if 
		*										pHostPtr is NULL and can be either 0 or >= 
		*										szImageWidth * szSize of element in bytes if 
		*										pHostPtr is not NULL. If pHostPtr is not NULL and 
		*										szImageRowPitch = 0, szImageRowPitch is calculated
		*										as szImageWidth * szSize of element in bytes. If 
		*										szImageRowPitch is not 0, it must be a multiple of 
		*										the image element size in bytes
		*				pHostPtr [in]			is a pointer to the image data that may already be 
		*										allocated by the application. The size of the buffer 
		*										that pHostPtr points to must be >= szImageRowPitch 
		*										* szImageHeight. The size of each element in bytes 
		*										must be a power of 2. Passing in a pointer to an 
		*										already allocated buffer on the host and using it 
		*										as a memory object allows applications to share 
		*										data efficiently with kernels and the host. The 
		*										image data specified by pHostPtr is stored as a 
		*										linear sequence of adjacent scanlines. Each scanline
		*										is stored as a linear sequence of image elements
		*				pErrcodeRet [out]		will return an appropriate error code. If 
		*										pErrcodeRet is NULL, no error code is returned
		* Return value:	valid non-zero image object and:
		*				CL_SUCCESS				if the image object is created successfully
		*
		*				NULL value with one of the following error values:
		*				CL_INVALID_CONTEXT		if context is not a valid context
		*				CL_INVALID_VALUE		if values specified in clFlags are not valid
		*				CL_INVALID_IMAGE_SIZE	if szImageWidth or szImageHeight are 0 or if they
		*										exceed values specified in CL_DEVICE_IMAGE2D_MAX_WIDTH 
		*										or CL_DEVICE_IMAGE2D_MAX_HEIGHT respectively or if
		*										values specified by szImageRowPitch do not follow 
		*										rules described in the argument description above
		*				CL_INVALID_HOST_PTR		if pHostPtr is NULL and CL_MEM_USE_HOST_PTR or 
		*										CL_MEM_COPY_HOST_PTR are set in clFlags or if 
		*										pHostPtr is not NULL but CL_MEM_COPY_HOST_PTR or
		*										CL_MEM_USE_HOST_PTR are not set in clFlags
		*				CL_INVALID_OPERATION	if the image object as specified by the 
		*										clImageFormat, clFlags and dimensions cannot be 
		*										created for all devices in context that support
		*										images or if there are no devices in context that
		*										support images
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		*				CL_INVALID_IMAGE_FORMAT_DESCRIPTOR	if values specified in clImageFormat 
		*													are not valid or if clImageFormat is
		*													NULL
		*				CL_IMAGE_FORMAT_NOT_SUPPORTED		if the clImageFormat is not supported
		*				CL_MEM_OBJECT_ALLOCATION_FAILURE	if there is a failure to allocate 
		*													memory for image object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_mem CreateImage2D(	cl_context              IN  clContext,
										cl_mem_flags            IN  clFlags,
										const cl_image_format * IN  clImageFormat,
										size_t                  IN  szImageWidth,
										size_t                  IN  szImageHeight,
										size_t                  IN  szImageRowPitch,
										void *                  IN  pHostPtr,
										cl_int *                OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateImage3D    
		* Description:	A 3D image object is created using the following function
		* Arguments:	clContext [in]			is a valid OpenCL context on which the image object
		*										is to be created	
		*				clFlags [in]			is a bit-field that is used to specify allocation
		*										and usage information about the image memory object
		*										being created
		*				szSize [in]				is the size in bytes of the buffer memory object to
		*										be allocated.
		*				clImageFormat [in]		s a pointer to a structure that describes format 
		*										properties of the image to be allocated
		*				szImageWidth [in]		is the width of the image in pixels. This must be
		*										value greater than or equal to 1.
		*				szImageHeight [in]		is the height of the image in pixels. This must be
		*										value greater than or equal to 1.
		*				szImageDepth [in]		is the depth of the image in pixels. This must be a
		*										value > 1
		*				szImageRowPitch [in]	is the scan-line pitch in bytes. This must be 0 if 
		*										pHostPtr is NULL and can be either 0 or >= 
		*										szImageWidth * szSize of element in bytes if 
		*										pHostPtr is not NULL. If pHostPtr is not NULL and 
		*										szImageRowPitch = 0, szImageRowPitch is calculated
		*										as szImageWidth * szSize of element in bytes. If 
		*										szImageRowPitch is not 0, it must be a multiple of 
		*										the image element size in bytes.
		*				szImageSlicePitch [in]	is the size in bytes of each 2D slice in the 3D 
		*										image. This must be 0 if pHostPtr is NULL and can 
		*										be either 0 or >= szImageRowPitch * szImageHeight
		*										if pHostPtr is not NULL. If pHostPtr is not NULL 
		*										and szImageSlicePitch = 0, szImageSlicePitch is 
		*										calculated as szImageRowPitch * szImageHeight
		*				pHostPtr [in]			is a pointer to the image data that may already be
		*										allocated by the application. The size of the 
		*										buffer that pHostPtr points to must be >= 
		*										szImageSlicePitch * szImageDepth. The size of each
		*										element in bytes must be a power of 2. Passing in a
		*										pointer to an already allocated buffer on the host
		*										and using it as a memory object allows applications
		*										to share data efficiently with kernels and the host.
		*										The image data specified by pHostPtr is stored as a
		*										linear sequence of adjacent 2D slices. Each 2D 
		*										slice is a linear sequence of adjacent scanlines. 
		*										Each scanline is a linear sequence of image elements
		*				pErrcodeRet [out]		will return an appropriate error code. If 
		*										pErrcodeRet is NULL, no error code is returned
		* Return value:	valid non-zero image object and:
		*				CL_SUCCESS				if the image object is created successfully
		*
		*				NULL value with one of the following error values:
		*				CL_INVALID_CONTEXT		if context is not a valid context
		*				CL_INVALID_VALUE		if values specified in clFlags are not valid
		*				CL_INVALID_IMAGE_SIZE	if szImageWidth, szImageHeight or szImageDepth are 
		*										0 or if they exceed values specified in 
		*										CL_DEVICE_IMAGE3D_MAX_WIDTH, 
		*										CL_DEVICE_IMAGE3D_MAX_HEIGHT or
		*										CL_DEVICE_IMAGE3D_MAX_DEPTH respectively or if
		*										values specified by szImageRowPitch and 
		*										szImageSlicePitch do not follow rules described in
		*										the argument description above
		*				CL_INVALID_HOST_PTR		if pHostPtr is NULL and CL_MEM_USE_HOST_PTR or 
		*										CL_MEM_COPY_HOST_PTR are set in clFlags or if 
		*										pHostPtr is not NULL but CL_MEM_COPY_HOST_PTR or
		*										CL_MEM_USE_HOST_PTR are not set in clFlags
		*				CL_INVALID_OPERATION	if the image object as specified by the 
		*										clImageFormat, clFlags and dimensions cannot be 
		*										created for all devices in context that support
		*										images or if there are no devices in context that
		*										support images
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		*				CL_INVALID_IMAGE_FORMAT_DESCRIPTOR	if values specified in clImageFormat 
		*													are not valid or if clImageFormat is
		*													NULL
		*				CL_IMAGE_FORMAT_NOT_SUPPORTED		if the clImageFormat is not supported
		*				CL_MEM_OBJECT_ALLOCATION_FAILURE	if there is a failure to allocate 
		*													memory for image object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_mem CreateImage3D(	cl_context              IN  clContext,
										cl_mem_flags            IN  clFlags,
										const cl_image_format * IN  clImageFormat,
										size_t                  IN  szImageWidth,
										size_t                  IN  szImageHeight,
										size_t                  IN  szImageDepth,
										size_t                  IN  szImageRowPitch,
										size_t                  IN  szImageSlicePitch,
										void *                  IN  pHostPtr,
										cl_int *                OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	RetainMemObject    
		* Description:	increments the memobj reference count. CreateBuffer and CreateImage{2D|3D}
		*				perform an implicit retain.
		* Arguments:	clMemObj [in]		the memory object to be retain
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is not a valid memory object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/                        
		virtual cl_int RetainMemObject( cl_mem IN clMemObj ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseMemObject    
		* Description:	decrements the clMemObj reference count. After the clMemObj reference count
		*				becomes zero and commands queued for execution on a command-queue(s) that 
		*				use clMemObj have finished, the memory object is deleted
		* Arguments:	clMemObj [in]		the memory object to be released
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is not a valid memory object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/		
		virtual cl_int ReleaseMemObject( cl_mem IN clMemObj ) = 0;


		/******************************************************************************************
		* Function: 	GetSupportedImageFormats    
		* Description:	can be used to get the list of image formats supported by an OpenCL 
		*				implementation when the following information about an image memory object
		*				is specified:
		*					- Context
		*					- Image type – 2D or 3D image
		*					- Image object allocation information
		* Arguments:	cl_context [in]				is a valid OpenCL context on which the image 
		*											object(s) will be created
		*				clFlags [in]				is a bit-field that is used to specify 
		*											allocation and usage information about the 
		*											image memory object being created
		*				clImageType [in]			describes the image type and must be either 
		*											CL_MEM_OBJECT_IMAGE2D or CL_MEM_OBJECT_IMAGE3D
		*				uiNumEntries [in]			specifies the number of entries that can be 
		*											returned in the memory location given by
		*											pclImageFormats
		*				pclImageFormats [out]		is a pointer to a memory location where the 
		*											list of supported image formats are returned.
		*											Each entry describes a cl_image_format structure
		*											supported by the OpenCL implementation. If 
		*											pclImageFormats is NULL, it is ignored
		*				puiNumImageFormats [out]	is the actual number of supported image formats 
		*											for a specific context and values specified by 
		*											clFlags. If puiNumImageFormats is NULL, it is 
		*											ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid context
		*				CL_INVALID_VALUE		if flags or clImageType are not valid, or if 
		*										uiNumEntries is 0 and pclImageFormats is not NULL
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_int GetSupportedImageFormats(cl_context           IN  clContext,
												cl_mem_flags         IN  clFlags,
												cl_mem_object_type   IN  clImageType,
												cl_uint              IN  uiNumEntries,
												cl_image_format *    OUT pclImageFormats,
												cl_uint *            OUT puiNumImageFormats ) = 0;

		/******************************************************************************************
		* Function: 	GetMemObjectInfo    
		* Description:	get information that is common to all memory objects (buffer and image 
		*				objects)
		* Arguments:	clMemObj [in]				specifies the memory object being queried 
		*				clParamName [in]			specifies the information to query 
		*				pParamValue [in]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If 
		*											pParamValue is NULL, it is ignored
		*				szParamValueSize [in]		is used to specify the size in bytes of memory
		*											pointed to by pParamValue. This size must be 
		*											>= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data being
		*											queried by pParamValue. If pszParamValueSizeRet
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is a not a valid memory object
		*				CL_INVALID_VALUE		if clParamName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/                                    
		virtual cl_int GetMemObjectInfo(cl_mem          IN  clMemObj,
										cl_mem_info		IN  clParamName, 
										size_t          IN  szParamValueSize,
										void *          OUT pParamValue,
										size_t *        OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		* Function: 	GetImageInfo    
		* Description:	get information specific to an image object created with CreateImage{2D|3D}
		* Arguments:	clImage [in]				specifies the image object being queried.
		*				clParamName [in]			specifies the information to query 
		*				pParamValue [in]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If 
		*											pParamValue is NULL, it is ignored
		*				szParamValueSize [in]		is used to specify the size in bytes of memory
		*											pointed to by pParamValue. This size must be 
		*											>= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data being
		*											queried by pParamValue. If pszParamValueSizeRet
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is a not a valid memory object
		*				CL_INVALID_VALUE		if clParamName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_int GetImageInfo(cl_mem          IN  clImage,
									cl_image_info	IN  clParamName, 
									size_t			IN  szParamValueSize,
									void *          OUT pParamValue,
									size_t *        OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateSampler    
		* Description:	creates a sampler object
		* Arguments:	clContext [in]			must be a valid OpenCL context 
		*				bNormalizedCoords [in]	determines if the image coordinates specified are 
		*										normalized (if bNormalizedCoords is CL_TRUE) or not
		*										(if bNormalizedCoords is CL_FALSE).
		*				clAddressingMode [in]	specifies how out of range image coordinates are 
		*										handled when reading from an image. This can be set
		*										to CL_ADDRESS_REPEAT, CL_ADDRESS_CLAMP_TO_EDGE,
		*										CL_ADDRESS_CLAMP and CL_ADDRESS_NONE
		*				clFilterMode [in]		specifies the type of filter that must be applied 
		*										when reading an image. This can be CL_FILTER_NEAREST
		*										or CL_FILTER_LINEAR
		*				pErrcodeRet [out]		will return an appropriate error code. If 
		*										pErrcodeRet is NULL, no error code is returned
		* Return value:	valid non-zero sampler object and:
		*				CL_SUCCESS				if the sampler object is created successfully
		*
		*				NULL value with one of the following error values:
		*				CL_INVALID_CONTEXT		if clContext is not a valid context
		*				CL_INVALID_VALUE		if clAddressingMode, clFilterMode or 
		*										bNormalizedCoords or combination of these argument
		*										values are not valid
		*				CL_INVALID_OPERATION	if images are not supported by any device 
		*										associated with context
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host.
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/   		
		virtual cl_sampler CreateSampler(	cl_context			IN  clContext,
											cl_bool				IN  bNormalizedCoords,
											cl_addressing_mode	IN  clAddressingMode,
											cl_filter_mode		IN  clFilterMode,
											cl_int *			OUT pErrcodeRet ) = 0;
		
		/******************************************************************************************
		* Function: 	RetainSampler    
		* Description:	increments the clSampler reference count. CreateSampler does an implicit 
		*				retain
		* Arguments:	clSampler [in]			the sampler object to be retain
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_SAMPLER		if clSampler is not a valid sampler object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/ 
		virtual cl_int RetainSampler( cl_sampler IN clSampler ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseSampler    
		* Description:	decrements the clSampler reference count. The sampler object is deleted 
		*				after the reference count becomes zero and commands queued for execution 
		*				on a command-queue(s) that use sampler have finished
		* Arguments:	clSampler [in]			the sampler object to be released
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_SAMPLER		if clSampler is not a valid sampler object
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/ 
		virtual cl_int ReleaseSampler( cl_sampler IN clSampler ) = 0;

		/******************************************************************************************
		* Function: 	GetSamplerInfo    
		* Description:	returns information about the sampler object
		* Arguments:	clSampler [in]				specifies the sampler being queried
		*				clParamName [in]			specifies the information to query 
		*				pParamValue [in]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If 
		*											pParamValue is NULL, it is ignored
		*				szParamValueSize [in]		is used to specify the size in bytes of memory
		*											pointed to by pParamValue. This size must be 
		*											>= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data being
		*											queried by pParamValue. If pszParamValueSizeRet
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_SAMPLER		if clSampler is a not a valid sampler object
		*				CL_INVALID_VALUE		if clParamName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/
		virtual cl_int GetSamplerInfo(	cl_sampler		IN  clSampler,
										cl_sampler_info	IN  clParamName,
										size_t			IN  szParamValueSize,
										void *			OUT pParamValue,
										size_t *		OUT pszParamValueSizeRet ) = 0;


		/******************************************************************************************
		* Function: 	CreateProgramWithSource    
		* Description:	creates a program object for a context, and loads the source code specified 
		*				by the text strings in the strings array into the program object. The 
		*				devices associated with the program object are the devices associated with 
		*				context
		* Arguments:	clContext [in] -	must be a valid OpenCL context	
		*				ppcStrings [in] -	is an array of uiCount pointers to optionally null-
		*									terminated character strings that make up the source
		*									code
		*				szLengths [in] -	is an array with the number of chars in each string 
		*									(the string length). If an element in lengths is zero, 
		*									its accompanying string is null-terminated. If lengths 
		*									is NULL, all strings in the strings argument are 
		*									considered null-terminated. Any length value passed in 
		*									that is greater than zero excludes the null terminator 
		*									in its count
		*				pErrcodeRet [out] -	will return an appropriate error code. If errcode_ret 
		*									is NULL, no error code is returned
		* Return value:	CL_SUCCESS					the program object is created successfully
		*				CL_INVALID_CONTEXT			if context is not a valid OpenCL context
		*				CL_INVALID_VALUE			if count is zero or if strings or any entry in
		*											strings is NULL
		*				CL_COMPILER_NOT_AVAILABLE	if a compiler is not available i.e. 
		*											CL_DEVICE_COMPILER_AVAILABLE is set to 
		*											CL_FALSE
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the 
		*											host
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_program CreateProgramWithSource(	cl_context     IN  clContext,
													cl_uint        IN  uiCount,
													const char **  IN  ppcStrings,
													const size_t * IN  szLengths,
													cl_int *       OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateProgramWithBinary    
		* Description:	creates a program object for a context, and loads the binary bits specified 
		*				by binary into the program object
		* Arguments:	clContext [in] 		must be a valid OpenCL context	
		*				pclDeviceList [in]	is a pointer to a list of devices that are in context. 
		*									device_list must be a non-NULL value. The binaries are
		*									loaded for devices specified in this list.
		*									The devices associated with the program object will be 
		*									the list of devices specified by device_list. The list 
		*									of devices specified by pclDeviceList must be devices 
		*									associated with context.
		*				uiNumDevices [in] 	is the number of devices listed in pclDeviceList 
		*				pszLengths [in] 	is an array of the size in bytes of the program binaries
		*									to be loaded for devices specified by pclDeviceList
		*				ppBinaries [in] 	is an array of pointers to program binaries to be loaded
		*									for devices specified by pclDeviceList. For each device 
		*									given by pclDeviceList[i], the pointer to the program 
		*									binary for that device is given by ppBinaries[i] and the
		*									length of this corresponding binary is given by 
		*									pszLengths[i]. pszLengths[i] cannot be zero and 
		*									ppBinaries[i] cannot be a NULL pointer
		*									The program binaries specified by binaries contain the 
		*									bits that describe the program executable that will be 
		*									run on the device(s) associated with context. The 
		*									program binary can consist of either or both:
		*									-	Device-specific executable(s), and/or,
		*									-	Implementation-specific intermediate representation
		*										(IR) which will be converted to the device-specific 
		*										executable.
		*				piBinaryStatus [in]	returns whether the program binary for each device 
		*									specified in device_list wasloaded successfully or not
		*									It is an array of num_devices entries and returns 
		*									CL_SUCCESS in piBinaryStatus[i] if binary was 
		*									successfully loaded for device specified by 
		*									pclDeviceList[i]; otherwise returns CL_INVALID_VALUE if
		*									pszLengths[i] is zero or if ppBinaries[i] is a NULL 
		*									value or CL_INVALID_BINARY in piBinaryStatus[i] if 
		*									program binary is not a valid binary for the specified 
		*									device. If binary_status is NULL, it is ignored.
		*				pErrRet [in]		will return an appropriate error code. If pErrRet is 
		*									NULL, no error code isreturned
		* Return value:	CL_SUCCESS					the program object is created successfully
		*				CL_INVALID_CONTEXT			if context is not a valid OpenCL context
		*				CL_INVALID_VALUE			if pclDeviceList is NULL or uiNumDevicesis
		*											zero or if pszLengths or ppBinaries are NULL 
		*											or if any entry in pszLengths[i] is zero or 
		*											ppBinaries[i] is NULL
		*				CL_INVALID_DEVICE			if OpenCL devices listed in pclDeviceList are 
		*											not in the list of devices associated with 
		*											context
		*				CL_INVALID_BINARY			if an invalid program binary was encountered
		*											for any device. piBinaryStatus will return 
		*											specific status for each device
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the 
		*											host
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_program CreateProgramWithBinary(	cl_context				IN  clContext,
													cl_uint					IN  uiNumDevices,
													const cl_device_id *	IN  pclDeviceList,
													const size_t *			IN  pszLengths,
													const unsigned char **	IN  ppBinaries,
													cl_int *				OUT piBinaryStatus,
													cl_int *				OUT pErrRet ) = 0;

		/******************************************************************************************
		* Function: 	RetainProgram    
		* Description:	increments the context reference count
		* Arguments:	clProgram [in] -	specifies the OpenCL program being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_PROGRAM		if clProgram is not a valid OpenCL program
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/		
		virtual cl_err_code	RetainProgram( cl_program IN clProgram ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseProgram    
		* Description:	decrements the program reference count. The program object is deleted after
		*				all kernel objects associated with program have been deleted and the 
		*				program reference count becomes zero.
		* Arguments:	clProgram [in] -	specifies the OpenCL program being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_PROGRAM		if clProgram is not a valid OpenCL program
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_err_code ReleaseProgram( cl_program IN clProgram ) = 0;

		/******************************************************************************************
		* Function: 	BuildProgram    
		* Description:	builds (compiles & links) a program executable from the program source or 
		*				binary for all the devices or a specific device(s) in the OpenCL context 
		*				associated with program. OpenCL allows program executables to be built 
		*				using the source or the binary
		* Arguments:	clProgram [in]		is the program object	
		*				uiNumDevices [in]	is the number of devices listed in pclDeviceList
		*				pclDeviceList [in]	is a pointer to a list of devices associated with 
		*									program. If device_list is a NULL value, the program 
		*									executable is built for all devices associated with 
		*									program for which a source or binary has been loaded.
		*									If pclDeviceList is a non-NULL value, the program
		*									executable is built for devices specified in this list
		*									for which a source or binary has been loaded
		*				pcOptions [in]		is a pointer to a string that describes the build 
		*									options to be used for building the program executable
		*				pfn_notify [in]		is a function pointer to a notification routine. The 
		*									notification routine allows an application to register
		*									a callback function which will be called when the 
		*									program executable has been built (successfully or 
		*									unsuccessfully). If pfn_notify is not NULL, 
		*									BuildProgram does not need to wait for the build to 
		*									complete and can return immediately. If pfn_notify is
		*									NULL, BuildProgram does not return until the build has
		*									completed. This callback function may be called 
		*									asynchronously by the OpenCL implementation. It is the
		*									application’s responsibility to ensure that the callback
		*									function is thread-safe
		*				pUserData [in]		will be passed as an argument when pfn_notify is called.
		*									pUserData can be NULL
		* Return value:	CL_SUCCESS					if the function is executed successfully
		*				CL_INVALID_PROGRAM			if clProgram is not a valid OpenCL program
		*				CL_INVALID_VALUE			if pclDeviceList is NULL and num_devices is 
		*											greater than zero, or if pclDeviceList is not 
		*											NULL and num_devices is zero
		*				CL_INVALID_DEVICE			if OpenCL devices listed in pclDeviceList are
		*											not in the list of devices associated with 
		*											program
		*				CL_INVALID_BINARY			if program is created with 
		*											CreateWithProgramBinary and devices listed in
		*											pclDeviceList do not have a valid program 
		*											binary loaded
		*				CL_INVALID_BUILD_OPTIONS	if the build pcOptions specified by options are
		*											invalid
		*				CL_INVALID_OPERATION		if the build of a program executable for any of
		*											the devices listed in pclDeviceList by a 
		*											previous call to clBuildProgram for program has
		*											not completed or if there are kernel objects 
		*											attached to program
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the
		*											host
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int BuildProgram(cl_program           IN clProgram,
									cl_uint              IN uiNumDevices,
									const cl_device_id * IN pclDeviceList,
									const char *         IN pcOptions, 
									void (*pfn_notify)(cl_program program, void * user_data),
									void *               IN pUserData ) = 0;


		/******************************************************************************************
		* Function: 	GetProgramInfo    
		* Description:	returns information about the program object
		* Arguments:	clProgram [in]				specifies the program object being queried	
		*				clParamName [in]			specifies the information to query
		*				szParamValueSize [in]		is used to specify the size in bytes of memory
		*											pointed to by param_value. This size must be 
		*											>= size of return type
		*				pParamValue [in]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If 
		*											pParamValue is NULL, it is ignored
		*				pszParamValueSizeRet [in]	returns the actual size in bytes of data copied
		*											to param_value. If pszParamValueSizeRet is 
		*											NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_VALUE		if clParamName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL
		*				CL_INVALID_PROGRAM		if clProgram is a not a valid program object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int GetProgramInfo(	cl_program		IN  clProgram,
										cl_program_info	IN  clParamName,
										size_t			IN  szParamValueSize,
										void *			OUT pParamValue, 
										size_t *        OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		* Function: 	GetProgramBuildInfo    
		* Description:	returns build information for each device in the program object
		* Arguments:	clProgram [in]				specifies the program object being queried	
		*				clDevice [in]				specifies the device for which build 
		*											information is being queried. clDevice must be
		*											a valid device associated with clProgram
		*				clParamName [in]			specifies the information to query
		*				szParamValueSize [in]		is used to specify the size in bytes of memory
		*											pointed to by param_value. This size must be 
		*											>= size of return type
		*				pParamValue [in]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If 
		*											pParamValue is NULL, it is ignored
		*				pszParamValueSizeRet [in]	returns the actual size in bytes of data copied
		*											to param_value. If pszParamValueSizeRet is 
		*											NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_DEVICE		if clDevice is not in the list of devices 
		*										associated with clProgram
		*				CL_INVALID_VALUE		if clParamName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL
		*				CL_INVALID_PROGRAM		if clProgram is a not a valid program object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int GetProgramBuildInfo(	cl_program				IN  clProgram,
											cl_device_id			IN  clDevice,
											cl_program_build_info	IN  clParamName,
											size_t					IN  szParamValueSize,
											void *					OUT pParamValue, 
											size_t *				OUT pszParamValueSizeRet ) = 0;


		/******************************************************************************************
		* Function: 	CreateKernel    
		* Description:	create a kernel object
		* Arguments:	clProgram [in]		is a program object with a successfully built
		*									executable
		*				pscKernelName [in]	is a function name in the program declared with the 
		*									__kernel qualifer
		*				piErr [out]			will return an appropriate error code. If errcode_ret 
		*									is NULL, no error code is returned
		* Return value:	valid non-zero kernel object. piErr is set to:
		*				CL_SUCCESS				if the kernel object is created successfully
		*
		*				NULL value with one of the following error values returned in piErr:
		*				CL_INVALID_PROGRAM				clProgram is not a valid program object 
		*				CL_INVALID_PROGRAM_EXECUTABLE	if there is no successfully built 
		*												executable for clProgram
		*				CL_INVALID_KERNEL_NAME			if pscKernelName is not found in clProgram
		*				CL_INVALID_KERNEL_DEFINITION	if the function definition for __kernel
		*												function given by kernel_name such as the 
		*												number of arguments, the argument types are
		*												not the same for all devices for which the
		*												program executable has been built
		*				CL_INVALID_VALUE				if pscKernelName is NULL
		*				CL_OUT_OF_HOST_MEMORY			if there is a failure to allocate resources
		*												required by the OpenCL implementation on 
		*												the host
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_kernel CreateKernel(	cl_program		IN  clProgram,
										const char *	IN  pscKernelName,
										cl_int *		OUT piErr ) = 0;

		/******************************************************************************************
		* Function: 	CreateKernelsInProgram    
		* Description:	creates kernel objects for all kernel functions in program. Kernel objects 
		*				are not created for any __kernel functions in program that do not have the 
		*				same function definition across all devices for which a program executable 
		*				has been successfully built
		* Arguments:	clProgram [in]			is a program object with a successfully built
		*										executable
		*				uiNumKernels [in]		is the size of memory pointed to by kernels 
		*										specified as the number of cl_kernel entries
		*				pclKernels [out]		is the buffer where the kernel objects for kernels
		*										in program will be returned. If kernels is NULL, it 
		*										is ignored. If kernels is not NULL, num_kernels
		*										must be greater than or equal to the number of   
		*										kernels in program
		*				puiNumKernelsRet [out]	is the number of kernels in program. If 
		*										puiNumKernelsRet is NULL, it is ignored 
		* Return value:	CL_SUCCESS				if the kernel objects were successfully allocated
		*				CL_INVALID_PROGRAM				clProgram is not a valid program object 
		*				CL_INVALID_PROGRAM_EXECUTABLE	if there is no successfully built 
		*												executable for any device in program
		*				CL_INVALID_VALUE				if pclKernels is not NULL and uiNumKernels
		*												is less than the number of kernels in
		*												clProgram 
		*				CL_OUT_OF_HOST_MEMORY			if there is a failure to allocate resources
		*												required by the OpenCL implementation on 
		*												the host
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int CreateKernelsInProgram(	cl_program	IN  clProgram,
												cl_uint		IN  uiNumKernels,
												cl_kernel *	OUT pclKernels,
												cl_uint *	OUT puiNumKernelsRet ) = 0;

		/******************************************************************************************
		* Function: 	RetainKernel    
		* Description:	increments the kernel reference count
		* Arguments:	clKernel [in] -	specifies the OpenCL kernel being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_KERNEL		if clKernel is not a valid kernel object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/	
		virtual cl_int RetainKernel( cl_kernel IN clKernel ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseKernel    
		* Description:	decrements the kernel reference count. 
		* Arguments:	clKernel [in] -	specifies the OpenCL kernel being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_KERNEL		if clKernel is not a valid kernel object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int ReleaseKernel( cl_kernel IN clKernel ) = 0;

		/******************************************************************************************
		* Function: 	SetKernelArg    
		* Description:	used to set the argument value for a specific argument of a kernel
		* Arguments:	clKernel [in]		is a valid kernel object
		*				uiArgIndex [in]		is the argument index. Arguments to the kernel are 
		*									referred by indices that go from 0 for the leftmost 
		*									argument to n - 1, where n is the total number of 
		*									arguments declared by a kernel
		*									For example, consider the following kernel:
		*
		*									__kernel void
		*									image_filter (int n, int m,
		*											__constant float *filter_weights,
		*											__read_only image2d_t src_image,
		*											__write_only image2d_t dst_image)
		*									{
		*										...
		*									}
		*
		*									Argument index values for image_filter will be 0 for 
		*									n, 1 for m, 2 for filter_weights, 3 for src_image and 
		*									4 for dst_image
		*				szArgSize [in]		specifies the size of the argument value. If the 
		*									argument is a memory object, the size is the size of 
		*									the buffer or image object type. For arguments declared
		*									with the __local qualifier, the size specified will be 
		*									the size in bytes of the buffer that must be allocated 
		*									for the __local argument. If the argument is of type 
		*									sampler_t, the arg_size value must be equal to 
		*									sizeof(cl_sampler). For all other arguments, the size 
		*									will be the size of argument type
		*				pszArgValue [out]	is a pointer to data that should be used as the argument
		*									value for argument specified by uiArgIndex. The argument
		*									data pointed to by pszArgValue is copied and the 
		*									pszArgValue pointer can therefore be reused by the 
		*									application after clSetKernelArg returns. The argument
		*									value specified is the value used by all API calls that
		*									enqueue kernel (clEnqueueNDRangeKernel and clEnqueueTask)
		*									until the argument value is changed by a call to 
		*									clSetKernelArg for kernel
		* Return value:	CL_SUCCESS				if the clKernel object is created successfully
		*				CL_INVALID_KERNEL		if clKernel is not a valid kernel object
		*				CL_INVALID_ARG_INDEX	if uiArgIndex is not a valid argument index.
		*				CL_INVALID_ARG_VALUE	if pszArgValue specified is NULL for an argument that
		*										is not declared with the __local qualifier or vice-ver
		*				CL_INVALID_MEM_OBJECT	for an argument declared to be a memory object when 
		*										the specified arg_value is not a valid memory object
		*				CL_INVALID_SAMPLER		for an argument declared to be of type sampler_t when
		*										the specified arg_value is not a valid sampler object
		*				CL_INVALID_ARG_SIZE		if szArgSize does not match the size of the data type 
		*										for an argument that is not a memory object or if the 
		*										argument is a memory object and szArgSize != 
		*										sizeof(cl_mem) or if szArgSize is zero and the 
		*										argument is declared with the __local qualifier or if
		*										the argument is a sampler and szArgSize != 
		*										sizeof(cl_sampler).
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int SetKernelArg(cl_kernel		IN  clKernel,
									cl_uint			IN  uiArgIndex,
									size_t			IN  szArgSize,
									const void *	OUT pszArgValue ) = 0;


		/******************************************************************************************
		* Function: 	ReleaseKernel    
		* Description:	returns information about the kernel object 
		* Arguments:	clKernel [in]				specifies the kernel object being queried
		*				clParamName [in]			specifies the information to query
		*				pParamValue [out]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If
		*											pParamValue is NULL, it is ignored
		*				szParamValueSize [in]		is used to specify the size in bytes of memory 
		*											pointed to by pParamValue. This size must be 
		*											>= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data 
		*											copied to param_value. If pszParamValueSizeRet
		*											is NULL, it is ignored.
		* Return value:	CL_SUCCESS			if the function is executed successfully
		*				CL_INVALID_VALUE	if clParamName is not valid, or if size in bytes 
		*									specified by szParamValueSize is < size of return type 
		*									and pParamValue is not NULL
		*				CL_INVALID_KERNEL	if clKernel is not a valid kernel object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int GetKernelInfo(	cl_kernel		IN  clKernel,
										cl_kernel_info	IN  clParamName,
										size_t			IN  szParamValueSize,
										void *			OUT pParamValue,
										size_t *		OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		* Function: 	GetKernelWorkGroupInfo    
		* Description:	returns information about the kernel object that may be specific to a 
		*				device 
		* Arguments:	clKernel [in]				specifies the kernel object being queried
		*				clDevice [in]				identifies a specific device in the list of 
		*											devices associated with clKernel. The list of 
		*											devices is the list of devices in the OpenCL 
		*											context that is associated with clKernel. If 
		*											the list of devices associated with kernel is 
		*											a single device, device can be a NULL value
		*				clParamName [in]			specifies the information to query
		*				pParamValue [out]			is a pointer to memory where the appropriate 
		*											result being queried is returned. If
		*											pParamValue is NULL, it is ignored
		*				szParamValueSize [in]		is used to specify the size in bytes of memory 
		*											pointed to by pParamValue. This size must be 
		*											>= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data 
		*											copied to param_value. If pszParamValueSizeRet
		*											is NULL, it is ignored.
		* Return value:	CL_SUCCESS			if the function is executed successfully
		*				CL_INVALID_DEVICE	if clDevice is not in the list of devices associated 
		*									with clKernel or if clDevice is NULL but there is more
		*									than one device associated with clKernel
		*				CL_INVALID_VALUE	if clParamName is not valid, or if size in bytes 
		*									specified by szParamValueSize is < size of return type 
		*									and pParamValue is not NULL
		*				CL_INVALID_KERNEL	if clKernel is not a valid kernel object
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int GetKernelWorkGroupInfo(	cl_kernel					IN  clKernel,
												cl_device_id				IN  clDevice,
												cl_kernel_work_group_info	IN  clParamName,
												size_t						IN  szParamValueSize,
												void *						OUT pParamValue,
												size_t *					OUT pszParamValueSizeRet ) = 0;


	};

}}};
#endif // !defined(OCL_ICONTEXT_H_)
