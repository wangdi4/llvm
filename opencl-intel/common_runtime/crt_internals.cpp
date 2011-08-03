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
#include "crt_internals.h"
#include "crt_module.h"

#include <algorithm>

namespace OCLCRT
{
	extern CrtModule crt_ocl_module;
};



/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
	///  calculates the LCM (Least common multiple)
int CalculateLCM(int a, int b)
{
	int temp_a = a, temp_b = b;
    int temp = 0;
	for (;;)
    {
		if (temp_a == 0)
		{
			temp = temp_b;
			break;
		}
        temp_b %= temp_a;
        if (temp_b == 0)
		{
			temp = temp_a;
			break;
		}
        temp_a %= temp_b;
    }
    return temp ? (a / temp * b) : 0;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

	/// User event Callback, which releases userevent when called by all
	/// registered callbacks	
void __stdcall CrtEventCallBack(cl_event e, cl_int status, void* user_data) 
{	
	CrtEventCallBackData* evData = (CrtEventCallBackData*)user_data;
		
		/// Check if this is the last event to call this callback
	if (0 == atomic_decrement(&(evData->numReqCalls)))
	{
			/// Release the crt user event (bridge)
		cl_int error = evData->m_eventDEV->dispatch->clSetUserEventStatus(
			evData->m_eventDEV, CL_COMPLETE);		

		evData->m_eventDEV->dispatch->clReleaseEvent(evData->m_eventDEV);
		delete evData;
	}
}

void CL_CALLBACK CrtMemDestructorCallBack(cl_mem m, void* userData)
{	
	CrtMemDtorCallBackData *memData = (CrtMemDtorCallBackData*)userData;
	
	CrtMemObject* memObj = *(memData->m_clMemHandle);
		
		/// If no memory objects currently coupled in,
		/// destroy the memory object then
	if (0 == atomic_decrement(&(memData->m_count)))	
	{	
		delete *(memData->m_clMemHandle);
			/// We need this null, so we detect that this
			///	mem object is not longer valid.
		*(memData->m_clMemHandle) = NULL;
		delete memData;		
		
	}	
}


	/// Mem destructor callback and user data
struct CrtMemDtorForwarderData
{
		/// underlying caller context
	mem_dtor_fn		m_user_dtor_func;	
	void*			m_user_data;
	cl_uint			m_count;

};


void CL_CALLBACK CrtMemDtorForwarder(cl_mem m, void* userData)
{	
	CrtMemDtorForwarderData* memData = (CrtMemDtorForwarderData*)userData;	

	if (0 == atomic_decrement(&(memData->m_count)))
	{
		memData->m_user_dtor_func(m, memData->m_user_data);
		delete memData;	
	}	
}



/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------	
CrtContext::CrtContext(
	cl_context 						context_handle,	
	const cl_context_properties *	properties,
	cl_uint							num_devices,
	const cl_device_id *			devices,
	logging_fn						pfn_notify,
	void *							user_data,
	cl_int *						errcode_ret): m_context_handle(context_handle)
{
	cl_int errCode = CL_SUCCESS;

	
	for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
	{
		cl_device_id* outDevices = new cl_device_id[OCLCRT::crt_ocl_module.m_deviceInfoMap.size()];
		if (NULL == devices)
		{
			errCode = CL_OUT_OF_HOST_MEMORY;			
			break;
		}
		cl_uint matchDevices = 0;
		GetDevicesByPlatformId(num_devices, 
							devices, 
							OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV, 
							&matchDevices, 
							outDevices);
		
		if (matchDevices == 0)
			continue;
		
		CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMap[outDevices[0]];

		cl_context_properties* props;
		if (CRT_FAIL == OCLCRT::ReplacePlatformId(properties, devInfo->m_platformIdDEV, &props))
		{
			errCode = CL_OUT_OF_HOST_MEMORY;
			break;
		}
			/// It doesn't matter which device we pick from outDevices
		
		cl_context ctx = devInfo->m_origDispatchTable.clCreateContext(
																props, 
																matchDevices, 
																outDevices, 
																pfn_notify, 
																user_data, 
																&errCode);
		if (CL_SUCCESS != errCode)
			break;

		for (cl_uint j=0; j < matchDevices; j++)
		{
			m_DeviceToContext[outDevices[j]] = ctx;
		}		

		m_contexts[ctx] = &(OCLCRT::crt_ocl_module.m_deviceInfoMap[outDevices[0]]->m_origDispatchTable);
	}

	if (CL_SUCCESS == errCode)
		errCode = GetDevicesPreferredAlignment(num_devices, devices, &m_alignment);

	if (errcode_ret)
		*errcode_ret = errCode;	
}

void CrtContext::GetDevicesByPlatformId(
	const cl_uint			inNumDevices, 
	const cl_device_id*		inDevices, 
	const cl_platform_id&	pId, 
	cl_uint*				outNumDevices, 
	cl_device_id*			outDevices)
{
	cl_int devCount = 0;
	for (cl_uint i=0; i < inNumDevices; i++)
	{
		if (OCLCRT::crt_ocl_module.m_deviceInfoMap[inDevices[i]]->m_platformIdDEV == pId)
		{
			outDevices[devCount++] = inDevices[i];
		}
	}
	*outNumDevices = devCount;
}

cl_int CrtContext::GetDevicesPreferredAlignment(
	const cl_uint			numDevices, 
	const cl_device_id*		devices, 
	cl_uint*				alignment)
{			
	for (cl_uint i=0; i < numDevices; i++)
	{		
		cl_int errCode = CL_SUCCESS;
		cl_uint devAlignment = 0;
		errCode = OCLCRT::crt_ocl_module.m_deviceInfoMap[devices[i]]->m_origDispatchTable.clGetDeviceInfo(
						devices[i], 
						CL_DEVICE_MEM_BASE_ADDR_ALIGN, 
						sizeof(cl_uint), 
						(void*)(&devAlignment), 
						NULL);

		if (errCode != CL_SUCCESS)
			return CL_OUT_OF_RESOURCES;

		if (i==0)
			*alignment  = devAlignment;
		else
			*alignment  = CalculateLCM(*alignment , devAlignment);
	}

	return CL_SUCCESS;
}

cl_int CrtContext::Release()
{
	cl_int errCode = CL_SUCCESS;
				
	SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
	for(;itr != m_contexts.end(); itr++)		
	{		
		errCode = itr->second->clReleaseContext(itr->first);		
		if (CL_SUCCESS != errCode)
		{
			return errCode;
		}
	}
	return errCode;
}

cl_int CrtContext::GetReferenceCount(cl_uint* refCountParam)
{
	cl_int errCode = CL_SUCCESS;
	cl_uint refCount = 0;

	SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();	
	errCode = itr->second->clGetContextInfo(
		itr->first,
		CL_CONTEXT_REFERENCE_COUNT,
		sizeof(cl_uint),
		&refCount,
		NULL);
	
	*refCountParam = refCount; 
	return errCode;	
}

CrtContext::~CrtContext()
{	
	m_contexts.clear();
	m_commandQueues.clear();
}

cl_int CrtContext::FlushQueues()
{
	cl_uint errCode = CL_SUCCESS;
	for(std::list<cl_command_queue>::iterator itr = m_commandQueues.begin(); 
		itr != m_commandQueues.end();
		itr++)
	{
		const cl_command_queue q = *itr;
		errCode = q->dispatch->clFlush(q);
		if (CL_SUCCESS != errCode)
			return errCode;
	}
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtMemObject::CrtMemObject(cl_mem_flags flags, void* hostPtr, CrtContext* ctx)
{
	m_flags = flags;
	m_pUsrPtr = NULL;

	if (flags & CL_MEM_USE_HOST_PTR && hostPtr)
		m_pUsrPtr = hostPtr;
	
	m_pBstPtr = NULL;
	m_pContext = ctx;	
	m_refCount = 1;
}

CrtMemObject::~CrtMemObject()
{			
	m_ContextToMemObj.clear();
	
	/// there are 3 cases we need to deallocate m_pBstPtr:
	///		1. no user pointer provided so we allocated backing store
	///		2. user point provided but not aligned, so we kept m_pUsrPtr	
	if (m_pBstPtr && (!(m_flags & CL_MEM_USE_HOST_PTR) || m_pUsrPtr))
	{		
		_aligned_free(m_pBstPtr);
		m_pBstPtr = NULL;
	}
}


inline cl_mem CrtMemObject::getDeviceMemObj(cl_device_id deviceId)
{			
	cl_context devCtx = m_pContext->m_DeviceToContext[deviceId];
	return m_ContextToMemObj[devCtx];
}

cl_int CrtMemObject::Release()
{	
	CTX_MEM_MAP::iterator itr_first = m_ContextToMemObj.begin();
	if (itr_first == m_ContextToMemObj.end())
	{	
			/// Already released previously
		return CL_SUCCESS;
	}	
	itr_first->second->dispatch->clRetainMemObject(itr_first->second);

	for (CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();	
		itr != m_ContextToMemObj.end();
		itr++)
	{		
		itr->second->dispatch->clReleaseMemObject(itr->second);					
	}			

	itr_first->second->dispatch->clReleaseMemObject(itr_first->second);	
	return CL_SUCCESS;
}


cl_int CrtMemObject::RegisterDestructorCallback(mem_dtor_fn memDtorFunc, void* user_data)
{
	cl_int errCode = CL_SUCCESS;
		
		/// this memory is dealocated in the callback 'CrtMemDtorForwarder'
	CrtMemDtorForwarderData* memData = new CrtMemDtorForwarderData;
	if (memData == NULL)
	{
		errCode = CL_OUT_OF_HOST_MEMORY;
		return errCode;
	}
	memData->m_count = (cl_uint)m_ContextToMemObj.size();
	memData->m_user_dtor_func = memDtorFunc;
	memData->m_user_data = user_data;

	for (CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin(); 
		itr != m_ContextToMemObj.end();
		itr++)
	{
		errCode = itr->second->dispatch->clSetMemObjectDestructorCallback(
			itr->second,
			CrtMemDtorForwarder,
			(void*)memData);

		if (CL_SUCCESS != errCode)
			return errCode;
	}	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtBuffer::CrtBuffer(
	const size_t		size, 
	cl_mem_flags		flags, 
	void*				host_ptr, 
	CrtContext*			ctx): CrtMemObject(flags, host_ptr, ctx)
{
	m_parentBuffer = NULL;
	m_size = size;
}

CrtBuffer::CrtBuffer(
	CrtBuffer*		parent_buffer,
	cl_mem_flags	flags,		
	CrtContext*		ctx): CrtMemObject(flags, NULL, ctx)
{
	m_parentBuffer = parent_buffer;
}


CrtBuffer::~CrtBuffer()
{	
	// Do nothing
}

cl_int CrtBuffer::Create(CrtMemObject** bufObj)
{
			/// HOST pointer provided and memory is aligned
	if (m_pUsrPtr && (( (size_t)m_pUsrPtr & (m_pContext->getAlignment()-1)) == 0))
	{
		m_pBstPtr = m_pUsrPtr;
		m_pUsrPtr = NULL;
	}	
	if (m_pBstPtr == NULL && !m_pContext->memObjectAlwaysNotInSync(CrtObject::CL_BUFFER))
	{
			/// Convert bits to bytes
		m_pBstPtr = _aligned_malloc(m_size, m_pContext->getAlignment());	
		
		if (!m_pBstPtr)
			return CL_OUT_OF_HOST_MEMORY;

		if (m_flags & CL_MEM_COPY_HOST_PTR || m_flags & CL_MEM_USE_HOST_PTR)
		{
			memcpy(m_pBstPtr, m_pUsrPtr, m_size);
		}
	}	

	cl_int errCode = CL_SUCCESS;

	*bufObj = this;	
	
		/// This is being deallocated in callback 'CrtMemDtorCallBack'		
	CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;			
	if (NULL == memData)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
	for(;itr != m_pContext->m_contexts.end(); itr++)		
	{
		cl_context ctx = itr->first;
		cl_mem_flags crtCreateFlags = (m_flags | CL_MEM_USE_HOST_PTR ) & ~CL_MEM_ALLOC_HOST_PTR;
		cl_mem memObj = ctx->dispatch->clCreateBuffer(
			ctx, 
			crtCreateFlags, 
			m_size, 
			m_pBstPtr, 
			&errCode);

		if (CL_SUCCESS == errCode)
		{
			m_ContextToMemObj[ctx] = memObj;
						
			memData->m_count = (cl_uint)m_pContext->m_contexts.size();
			memData->m_clMemHandle = bufObj;
			errCode = memObj->dispatch->clSetMemObjectDestructorCallback(memObj, CrtMemDestructorCallBack, (void*)memData);
		}
		else
		{
			Release();			
		}						
	}
	if (CL_SUCCESS != errCode)
	{
		*bufObj = NULL;		
		delete memData;
	}

	return errCode;
}

cl_int CrtBuffer::Create(
	CrtMemObject**			bufObj,
	cl_buffer_create_type	buffer_create_type,
	const void *            buffer_create_info)
{
	cl_int errCode = CL_SUCCESS;

	SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
	for(;itr != m_pContext->m_contexts.end(); itr++)		
	{
		cl_context ctx = itr->first;		
		cl_mem parent_Buffer = m_parentBuffer->m_ContextToMemObj[ctx];
		cl_mem memObj = ctx->dispatch->clCreateSubBuffer(
			parent_Buffer, 
			m_flags, 
			buffer_create_type, 
			buffer_create_info, 
			&errCode);

		if (CL_SUCCESS == errCode)
		{
			m_ContextToMemObj[ctx] = memObj;

			CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;		
			if (NULL == memData)
			{
				Release();
				errCode = CL_OUT_OF_HOST_MEMORY;				
				break;
			}
			memData->m_count = (cl_uint)m_pContext->m_contexts.size();
			memData->m_clMemHandle = bufObj;
			errCode = memObj->dispatch->clSetMemObjectDestructorCallback(memObj, CrtMemDestructorCallBack, (void*)memData);
		}
		else
		{
			Release();			
		}
	}
	
	if (CL_SUCCESS != errCode)
		*bufObj = NULL;

	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtImage::CrtImage(   
	cl_mem_object_type		image_type,
	const cl_image_format * image_format,
	size_t                  image_width,
	size_t                  image_height,
	size_t                  image_depth,
	size_t                  image_row_pitch,
	size_t                  image_slice_pitch,
	cl_mem_flags			flags, 
	void*					host_ptr, 
	CrtContext*				ctx): CrtMemObject(flags, host_ptr, ctx),
	m_imageType(image_type),
	m_imageFormat(image_format),
	m_imageWidth(image_width),
	m_imageHeight(image_height),
	m_imageDepth(image_depth),
	m_imageRowPitch(image_row_pitch),
	m_imageSlicePitch(image_slice_pitch)
{
	if( m_imageRowPitch == 0 )
    {
        m_imageRowPitch = ( cl_uint )m_imageWidth * GetElementSize();
    }

    if( m_imageSlicePitch == 0 )
    {
        m_imageSlicePitch = m_imageRowPitch * ( cl_uint )m_imageHeight;
    }
	m_size = GetElementSize() * m_imageWidth * m_imageHeight * m_imageDepth;
}


CrtImage::~CrtImage()
{	
	// Do nothing
}

cl_int CrtImage::Create(CrtMemObject** bufObj)
{
		/// HOST pointer provided and memory is aligned
	if (m_pUsrPtr && (( (size_t)m_pUsrPtr & (CRT_PAGE_ALIGNMENT-1)) == 0))
	{
		m_pBstPtr = m_pUsrPtr;
		m_pUsrPtr = NULL;
	}	
	if (m_pBstPtr == NULL && !m_pContext->memObjectAlwaysNotInSync(CrtObject::CL_IMAGE))
	{
			/// Convert bits to bytes
		m_pBstPtr = _aligned_malloc(m_size, CRT_PAGE_ALIGNMENT /* stored in bytes */);	
		
		if (!m_pBstPtr)
			return CL_OUT_OF_HOST_MEMORY;

		if (m_flags & CL_MEM_COPY_HOST_PTR || m_flags & CL_MEM_USE_HOST_PTR)
		{
			memcpy(m_pBstPtr, m_pUsrPtr, m_size);
		}
	}	

	cl_int errCode = CL_SUCCESS;

	*bufObj = this;	
	
		/// This is being deallocated in callback 'CrtMemDtorCallBack'		
	CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;			
	if (NULL == memData)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
	for(;itr != m_pContext->m_contexts.end(); itr++)		
	{
		cl_context ctx = itr->first;
		cl_mem_flags crtCreateFlags = (m_flags | CL_MEM_USE_HOST_PTR ) & ~CL_MEM_ALLOC_HOST_PTR;
		cl_mem memObj  = NULL;
		if (m_imageType == CL_MEM_OBJECT_IMAGE2D)
		{
			memObj = ctx->dispatch->clCreateImage2D(
				ctx, 
				crtCreateFlags,
				m_imageFormat,
				m_imageWidth,
				m_imageHeight,
				m_imageRowPitch,
				m_pBstPtr, 
				&errCode);
		}
		else if (m_imageType == CL_MEM_OBJECT_IMAGE3D)
		{
			memObj = ctx->dispatch->clCreateImage3D(
				ctx, 
				crtCreateFlags,
				m_imageFormat,
				m_imageWidth,
				m_imageHeight,
				m_imageDepth,
				m_imageRowPitch,
				m_imageSlicePitch,				
				m_pBstPtr, 
				&errCode);
		}

		if (CL_SUCCESS == errCode)
		{
			m_ContextToMemObj[ctx] = memObj;
						
			memData->m_count = (cl_uint)m_pContext->m_contexts.size();
			memData->m_clMemHandle = bufObj;
			errCode = memObj->dispatch->clSetMemObjectDestructorCallback(memObj, CrtMemDestructorCallBack, (void*)memData);
		}
		else
		{
			Release();			
		}						
	}
	if (CL_SUCCESS != errCode)
	{
		*bufObj = NULL;		
		delete memData;
	}

	return errCode;
}

size_t CrtImage::GetElementSize() const
{
	size_t stChannels = 0;
	size_t stChSize = 0;
	switch (m_imageFormat->image_channel_order)
	{
	case CL_R:case CL_A:case CL_LUMINANCE:case CL_INTENSITY:
	case CL_RGB:	// Special case, must be used only with specific data type
		stChannels = 1;
		break;
	case CL_RG:case CL_RA:
		stChannels = 2;
		break;
	case CL_RGBA: case CL_ARGB: case CL_BGRA:
		stChannels = 4;
		break;
	default:
		assert(0);
	}
	switch (m_imageFormat->image_channel_data_type)
	{
		case (CL_SNORM_INT8):
		case (CL_UNORM_INT8):
		case (CL_SIGNED_INT8):
		case (CL_UNSIGNED_INT8):
				stChSize = 1;
				break;
		case (CL_SNORM_INT16):
		case (CL_UNORM_INT16):
		case (CL_UNSIGNED_INT16):
		case (CL_SIGNED_INT16):
		case (CL_HALF_FLOAT):
		case CL_UNORM_SHORT_555:
		case CL_UNORM_SHORT_565:
				stChSize = 2;
				break;
		case (CL_SIGNED_INT32):
		case (CL_UNSIGNED_INT32):
		case (CL_FLOAT):
		case CL_UNORM_INT_101010:
				stChSize = 4;
				break;
		default:
				assert(0);
	}

	return stChannels*stChSize;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateImage(
	cl_mem_object_type		image_type,
	cl_mem_flags            flags,
	const cl_image_format * image_format,
	size_t                  image_width,
	size_t                  image_height,
	size_t                  image_depth,
	size_t                  image_row_pitch,
	size_t                  image_slice_pitch,
	void *                  host_ptr,
	CrtMemObject**			imageObj)
{
	cl_int errCode = CL_SUCCESS;	
		
	if ((flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR) && host_ptr == NULL)
		return CL_INVALID_HOST_PTR;
	
	if (!(flags & CL_MEM_USE_HOST_PTR && flags & CL_MEM_COPY_HOST_PTR) && NULL != host_ptr)
		return CL_INVALID_HOST_PTR;
	
	CrtImage* image = new CrtImage(
		image_type,
		image_format, 
		image_width, 
		image_height,
		image_depth,
		image_row_pitch,
		image_slice_pitch,
		flags, 
		host_ptr, 
		this);

	if (!image)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}	
	errCode = image->Create(imageObj);				
	if (CL_SUCCESS != errCode)
	{
		image->Release();
		delete image;			
	}
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
	/// CreateBuffer 
cl_int CrtContext::CreateBuffer( 	
	cl_mem_flags			flags, 
	size_t					size, 
	void *					host_ptr, 	
	CrtMemObject**			bufObj)
{		
	cl_int errCode = CL_SUCCESS;	
	
	if (size == 0)
		return CL_INVALID_BUFFER_SIZE;
		
	if ((flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR) && host_ptr == NULL)
		return CL_INVALID_HOST_PTR;
	
	if (!(flags & CL_MEM_USE_HOST_PTR && flags & CL_MEM_COPY_HOST_PTR) && NULL != host_ptr)
		return CL_INVALID_HOST_PTR;
	
	CrtBuffer* buffer = new CrtBuffer(size, flags, host_ptr, this);
	if (!buffer)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}	
	errCode = buffer->Create(bufObj);				
	if (CL_SUCCESS != errCode)
	{
		buffer->Release();
		delete buffer;			
	}
	return errCode;
}

	/// CreateBuffer 
cl_int CrtContext::CreateSubBuffer(
	CrtBuffer*				parent_buffer,
	cl_mem_flags			flags, 	
	cl_buffer_create_type	buffer_create_type,
	const void *            buffer_create_info,
	CrtMemObject**			bufObj)
{		
	cl_int errCode = CL_SUCCESS;	
				
	if (flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR)
		return CL_INVALID_VALUE;
			
	CrtBuffer* buffer = new CrtBuffer(parent_buffer, flags, this);
	errCode = buffer->Create(bufObj, buffer_create_type, buffer_create_info);		
	if (CL_SUCCESS != errCode)
	{
		buffer->Release();
		delete buffer;			
	}
	else
	{
		*bufObj = buffer;
	}	
	return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int	CrtContext::CreateCommandQueue(	
	cl_device_id                device, 
	cl_command_queue_properties	properties, 
	CrtQueue**					crtQueue)
{
	cl_int errCode = CL_SUCCESS;
	CrtQueue* pCrtQueue = NULL;
	*crtQueue = NULL;

	DEV_CTX_MAP::iterator itr = m_DeviceToContext.find(device);
	if (itr == m_DeviceToContext.end())
		return CL_INVALID_DEVICE;
	
	cl_command_queue queueDEV = itr->second->dispatch->clCreateCommandQueue(itr->second, device, properties, &errCode);
	if (CL_SUCCESS == errCode)
	{
		pCrtQueue = new CrtQueue;
		pCrtQueue->m_cmdQueueDEV = queueDEV;
		pCrtQueue->m_contextCRT = this;
		pCrtQueue->m_device = device;
		*crtQueue = pCrtQueue;
	}
	m_commandQueues.push_back(queueDEV);
	return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

cl_int CrtContext::CreateProgramWithSource( cl_uint            count ,
                                            const char **      strings ,
                                            const size_t *     lengths ,
                                            CrtProgram **      crtProgram )
{

    cl_int errCode = CL_SUCCESS;
    CrtProgram *pCrtProgram = new CrtProgram;
    if( !pCrtProgram )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
	for(;itr != m_contexts.end(); itr++)		
	{
        cl_context ctx = itr->first;
        cl_program pgmObj = ctx->dispatch->clCreateProgramWithSource( ctx, count, strings, lengths, &errCode );
		if (CL_SUCCESS == errCode)
		{
            pCrtProgram->m_ContextToProgram[ctx] = pgmObj;
		}
        else
        {
            break;
        }
    }

	if (CL_SUCCESS == errCode)
	{
        pCrtProgram->m_contextCRT = this;
        pCrtProgram->m_refCount = 1;
        *crtProgram = pCrtProgram;
    }
    else
    {
            /// Release all previously allocated underlying program objects
        if( !pCrtProgram->m_ContextToProgram.empty() )
        {
            cl_int errCode2 = CL_SUCCESS;
            CrtProgram::CTX_PGM_MAP::iterator itr2 = pCrtProgram->m_ContextToProgram.begin();
            for(;itr2 != pCrtProgram->m_ContextToProgram.end(); itr2++)
            {
                errCode2 = itr2->first->dispatch->clReleaseProgram(itr2->second);
            }
        }
        delete pCrtProgram;
    }


	return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

SyncManager::SyncManager()
{
	m_callBackData = NULL;
	m_outEventArray = NULL;
}


cl_int SyncManager::PrepareToExecute(
	CrtQueue*		queue,
	cl_uint			NumEventsInWaitList,
	const cl_event*	inEventWaitList,
	cl_uint*		numOutEvents,
	cl_event**		OutEvents,
	const cl_uint	numMemObjects,
	CrtMemObject*	memObjects)
{	
	assert(numMemObjects == 0);
	assert(memObjects == NULL);
	
	cl_context dstContext = queue->m_contextCRT->m_DeviceToContext[queue->m_device];		
	
	cl_int errCode = CL_SUCCESS;
	
	if (NumEventsInWaitList != 0)
		m_outEventArray = new cl_event[NumEventsInWaitList];	

	m_eventRetained = false;
	cl_uint j = 0;
	for (cl_uint i=0; i < NumEventsInWaitList; i++)
	{
			/// Ohh... there are events belong to different undelying platforms			
		const CrtEvent* waitEvent = (CrtEvent*)(
			(reinterpret_cast<const _cl_event_crt*>(inEventWaitList[i]))->object
			);			
		
		cl_context evContext = NULL;
		cl_event eventDEV = NULL;

		if (waitEvent->m_isUserEvent)
		{
			eventDEV = ((CrtUserEvent*)waitEvent)->m_ContextToEvent[dstContext];
			evContext = dstContext;
		}
		else
		{
			eventDEV = waitEvent->m_eventDEV;
			evContext = waitEvent->m_queueCRT->m_contextCRT->m_DeviceToContext[waitEvent->m_queueCRT->m_device];
		}
					
			/// the target event (aiming for queue) is dependant on an event beloging to different 
			/// underlying context.
		if (!waitEvent->m_isUserEvent && dstContext != evContext)			
		{
				/// Initialization of callback and data is done only once					
			if (!m_callBackData)
			{
					/// Initialize crt callback data
				m_callBackData = new CrtEventCallBackData;
				if (!m_callBackData)						
				{
					errCode = CL_OUT_OF_HOST_MEMORY;
					goto FINISH;
				}
				m_callBackData->numReqCalls = 0;
				m_callBackData->m_eventDEV = queue->m_cmdQueueDEV->dispatch->clCreateUserEvent(dstContext, &errCode);				
				if (CL_SUCCESS != errCode)
				{						
					errCode = CL_OUT_OF_RESOURCES;
					goto FINISH;
				}				

				m_userEvent = m_callBackData->m_eventDEV;

					/// The corresponding release happens in 1) CrtEventCallBack 2) ~SyncManager
				errCode = m_callBackData->m_eventDEV->dispatch->clRetainEvent( 
								m_callBackData->m_eventDEV);
					
				if (CL_SUCCESS != errCode)
				{						
					errCode = CL_OUT_OF_RESOURCES;
					goto FINISH;
				}

				m_eventRetained = true;
				
				if (CL_SUCCESS != errCode)
				{
					errCode = CL_OUT_OF_RESOURCES;
					goto FINISH;
				}
					/// annex the user event to the destination queue events
				m_outEventArray[j++] = m_callBackData->m_eventDEV;
			}								
			
			m_callBackData->numReqCalls++;

			errCode = eventDEV->dispatch->clSetEventCallback(
				eventDEV, CL_COMPLETE, CrtEventCallBack, m_callBackData );				
			
			if (CL_SUCCESS != errCode)
				goto FINISH;				
		}
		else
		{
			m_outEventArray[j++] = eventDEV;
		}
	}
	*numOutEvents = j;
	if (0 == *numOutEvents)
	{
		*OutEvents = NULL;
	}
	else
		*OutEvents = m_outEventArray;

FINISH:
	if (CL_SUCCESS != errCode)
	{						
		if (m_eventRetained)
		{				
			m_callBackData->m_eventDEV->dispatch->clReleaseEvent(
								m_callBackData->m_eventDEV);				
		}			
		if (m_callBackData)
			delete m_callBackData;

		m_userEvent = NULL;

		return errCode;
	}		
	return errCode;
}

SyncManager::~SyncManager()
{			
	if (m_eventRetained && m_userEvent)
	{				
		m_userEvent->dispatch->clReleaseEvent(m_userEvent);				
	}		
	if (m_outEventArray)
	{
		delete[] m_outEventArray;
		m_outEventArray = NULL;
	}		
}


CrtEvent::~CrtEvent()
{
		/// If quiting gracefully
	cl_uint oldRef = atomic_decrement_ret_prev(&m_refCount);
	if (oldRef > 0)
	{
			/// Its enought to call Release once since we didn't
			/// forward Retains from CRT to the underlying platform
		m_eventDEV->dispatch->clReleaseEvent(m_eventDEV);
	}
}

CrtUserEvent::~CrtUserEvent()
{
		/// If quiting gracefully
	cl_uint oldRef = atomic_decrement_ret_prev(&m_refCount);
	if (oldRef > 0)
	{
		std::map<cl_context,cl_event>::iterator itr = m_ContextToEvent.begin();	
		for(;itr != m_ContextToEvent.end(); itr++)
		{
				/// Its enought to call Release once since we didn't
				/// forward Retains from CRT to the underlying platform
			itr->second->dispatch->clReleaseEvent(itr->second);
		}
	}
}
