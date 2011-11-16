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

#include "MemoryObjectFactory.h"
#include "SingleUnifiedBuffer.h"
#include "Device.h"

#include <cpu_dev_limits.h>
#include <assert.h>

//using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, TRUE, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_BUFFER, SingleUnifiedBuffer)

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
SingleUnifiedBuffer::SingleUnifiedBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType):
	SingleUnifiedMemObject(pContext, pOclEntryPoints, clObjType)
{
	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;
}

SingleUnifiedBuffer::~SingleUnifiedBuffer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedBuffer::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedBuffer::Initialize(
		cl_mem_flags		clMemFlags,
		const cl_image_format*	pclImageFormat,
		unsigned int		dim_count,
		const size_t*		dimension,
		const size_t*       pitches,
		void*				pHostPtr
		)
{
	assert(1 == dim_count);
	assert(NULL != dimension);

	m_uiNumDim = 1;
	// The object size is given in first dimension
	m_stMemObjSize = dimension[0];

	return SingleUnifiedMemObject::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedBuffer::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedBuffer::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    size_t szOffset = pszOrigin[0];
    size_t szDataSize = pszRegion[0];

    LOG_DEBUG(L"Enter ReadData (szDataSize=%d, pData=%d, szOffset=%d)", szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_stMemObjSize ||  NULL == m_pMemObjData)
	{
		return CL_INVALID_VALUE;
	}

    MEMCPY_S(pData, szDataSize, (void*)((cl_uchar*)m_pMemObjData+szOffset), szDataSize);

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedBuffer::WriteData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedBuffer::WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    size_t szOffset = pszOrigin[0];
    size_t szDataSize = pszRegion[0];

    LOG_DEBUG(L"Enter WriteData (szDataSize=%d, pData=%d, szOffset=%d)", szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_stMemObjSize ||  NULL == m_pMemObjData)
	{
		LOG_ERROR(TEXT("%S"),TEXT("Invalid Buffer copy parameters"));
		return CL_INVALID_VALUE;
	}

    MEMCPY_S((void*)((cl_uchar*)m_pMemObjData+szOffset), szDataSize, pData, szDataSize);

	return CL_SUCCESS;
}

void SingleUnifiedBuffer::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
{
	if (dimensions)
	{
		dimensions[0] = GetSize();
		for (int i = 1; i < MAX_WORK_DIM; i++)
		{
			dimensions[i] = 1;
		}
	}
	*rowPitch = *slicePitch = 0;
}


cl_err_code SingleUnifiedBuffer::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
	// KW: the pointers are always not NULL. not an issue
	return ((*pszOrigin + *pszRegion) <= (m_stMemObjSize)) ? CL_SUCCESS : CL_INVALID_VALUE ;
}

cl_err_code SingleUnifiedBuffer::CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{

	size_t totalSize =
	// offset
		pszOrigin[2] * szSlicePitch + pszOrigin[1] * szRowPitch + pszOrigin[0] +
	// data size
		(pszRegion[2]-1)*szSlicePitch + (pszRegion[1]-1) * szRowPitch + pszRegion[0];
	return (totalSize <= m_stMemObjSize)? CL_SUCCESS : CL_INVALID_VALUE;
}

void * SingleUnifiedBuffer::GetBackingStoreData( const size_t * pszOrigin ) const
{
    void* pData = NULL;
    if( NULL == pszOrigin )
    {
        pData = m_pMemObjData;
    }
    else
    {
        // Get with offset
        pData = (void*)((cl_uchar*)m_pMemObjData+pszOrigin[0]);
    }
	return pData;
}

cl_err_code SingleUnifiedBuffer::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, MemoryObject** ppBuffer)
{
	const cl_buffer_region* region = reinterpret_cast<const cl_buffer_region*>(buffer_create_info);

	if ( 0 != (region->origin % CPU_DEV_MAXIMUM_ALIGN) )
	{
		return CL_MISALIGNED_SUB_BUFFER_OFFSET;
	}

	if ( 0 == clFlags )
	{
		clFlags = m_clFlags;
	}

	MemoryObject* pSubBuffer = new SingleUnifiedSubBuffer( m_pContext, (ocl_entry_points*)m_handle.dispatch);
	if ( NULL == pSubBuffer )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	cl_err_code err = pSubBuffer->Initialize(clFlags, NULL, 1, &(region->size), &(region->origin), this);
	if ( CL_FAILED(err) )
	{
		pSubBuffer->Release();
		LOG_ERROR(TEXT("SubBuffer creation error: %x"), err);
		return err;
	}

	assert(NULL != ppBuffer);
	*ppBuffer = pSubBuffer;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedSubBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////

SingleUnifiedSubBuffer::SingleUnifiedSubBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints)
	: SingleUnifiedBuffer(pContext, pOclEntryPoints, CL_MEM_OBJECT_BUFFER)
{
}

SingleUnifiedSubBuffer::~SingleUnifiedSubBuffer()
{
	if ( NULL != m_pParentObject )
	{
		m_pParentObject->RemovePendency();
	}
}

cl_err_code SingleUnifiedSubBuffer::Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			)
{
	SingleUnifiedBuffer* pParentObject = reinterpret_cast<SingleUnifiedBuffer*>(pHostPtr);
	cl_dev_err_code devErr = pParentObject->m_pDeviceObject->clDevMemObjCreateSubObject(clMemFlags, pitches, dimension, &m_pDeviceObject);
	if ( CL_DEV_FAILED(devErr) )
	{
		return CL_OUT_OF_RESOURCES;
	}

	m_uiNumDim = dim_count;
	m_stOrigin[0] = pitches[0];
	m_stMemObjSize = dimension[0];

	if ( NULL != pParentObject->m_pHostPtr )
	{
		m_pHostPtr = (char*)pParentObject->m_pHostPtr + m_stOrigin[0];
	}
	UpdateLocation(pParentObject->m_pLocation);

	m_pParentObject = pParentObject;
	m_pParentObject->AddPendency();

	return CL_SUCCESS;
}

bool SingleUnifiedSubBuffer::IsSupportedByDevice(FissionableDevice* pDevice)
{
	// Need to check only for sub-buffers
	cl_uint         align;
	size_t          rsize;

	pDevice->GetInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(align), &align, &rsize);

	return ((m_stOrigin[0] % align) == 0);

}
