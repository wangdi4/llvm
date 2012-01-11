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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  image.cpp
//  Implementation of the SingleUnifiedImage2D and Image3D Classes
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SingleUnifiedImage2D.h"

#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D, SingleUnifiedImage2D)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
SingleUnifiedImage2D::SingleUnifiedImage2D(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType):
SingleUnifiedMemObject(pContext, pOclEntryPoints, clObjType)
{
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedBuffer::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedImage2D::Initialize(
	cl_mem_flags			clMemFlags,
	const cl_image_format*	pclImageFormat,
	unsigned int			dim_count,
	const size_t*			dimension,
	const size_t*			pitches,
	void*					pHostPtr,
	cl_rt_memobj_creation_flags	creation_flags
	)
{
	assert(2 == dim_count);
	assert(NULL != dimension);
	assert( (dimension[0] >= 1) && (dimension[1] >= 1));
	assert(NULL != pitches);

	m_uiNumDim = 2;
	m_szImageWidth = dimension[0];
	m_szImageHeight = dimension[1];

	m_clImageFormat = *pclImageFormat;
	m_szElementSize = m_pContext->GetPixelBytesCount(pclImageFormat);
	m_szImageRowPitch = (pitches[0] != 0) ? pitches[0] : (m_szImageWidth * m_szElementSize);

	// create buffer for image data
	m_stMemObjSize = CalcImageSize();

	return SingleUnifiedMemObject::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, &m_szImageRowPitch, pHostPtr, creation_flags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
SingleUnifiedImage2D::~SingleUnifiedImage2D()
{
	LOG_DEBUG(TEXT("%s"), TEXT("Enter MemoryObject D'tor"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedImage2D::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
	LOG_DEBUG(TEXT("%s"), TEXT("Enter:(clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t  szSize = 0;
	const void * pValue = NULL;
	size_t	stZero = 0;
	switch (clParamName)
	{
	case CL_IMAGE_FORMAT:
		szSize = sizeof(cl_image_format);
		pValue = &m_clImageFormat;
		break;
	case CL_IMAGE_ELEMENT_SIZE:
		szSize = sizeof(size_t);
		pValue = &m_szElementSize;
		break;
	case CL_IMAGE_ROW_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_szImageRowPitch;
		break;
	case CL_IMAGE_WIDTH:
		szSize = sizeof(size_t);
		pValue = &m_szImageWidth;
		break;
	case CL_IMAGE_HEIGHT:
		szSize = sizeof(size_t);
		pValue = &m_szImageHeight;
		break;
	case CL_IMAGE_DEPTH:
	case CL_IMAGE_SLICE_PITCH:
		szSize = sizeof(size_t);
		pValue = &stZero;
		break;

	default:
        return CL_INVALID_VALUE;
	}

	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		return CL_INVALID_VALUE;
	}

	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (NULL != pParamValue && szSize > 0)
	{
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
	}

	return CL_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedImage2D::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

	SMemCpyParams			sCpyParam;

	// Region
	sCpyParam.uiDimCount = 2;
	memcpy(sCpyParam.vRegion, pszRegion, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = pszRegion[0] * m_szElementSize;

	// set row Pitch for src and dst
	size_t srcPitch[2] = { m_szImageRowPitch , 0 };
	size_t dstPitch[2] = { szRowPitch , 0 };

	sCpyParam.pSrc = (cl_char*)m_pMemObjData;
	memcpy(sCpyParam.vSrcPitch, srcPitch, sizeof(sCpyParam.vSrcPitch));
	sCpyParam.pDst = (cl_char*)pData;
	memcpy(sCpyParam.vDstPitch, dstPitch , sizeof(sCpyParam.vDstPitch));

	// origin
	sCpyParam.pSrc += pszOrigin[0] * m_szElementSize; //Origin is in Pixels
	sCpyParam.pSrc += pszOrigin[1] * m_szImageRowPitch; //y * image width pitch

	clCopyMemoryRegion(&sCpyParam);

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D::WriteData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedImage2D::WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter WriteData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

	return CL_ERR_NOT_IMPLEMENTED;
}

void * SingleUnifiedImage2D::GetBackingStoreData( const size_t * pszOrigin ) const
{
    // TODO: Add support for pszOrigin != NULL;
	assert(pszOrigin == NULL && "Add support for pszOrigin != NULL");
	return m_pMemObjData;
}
void SingleUnifiedImage2D::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
{
	if (dimensions)
	{
		dimensions[0] = m_szImageWidth;
		dimensions[1] = m_szImageHeight;
		for (int i = 2; i < MAX_WORK_DIM; i++)
		{
			dimensions[i] = 1;
		}
	}
	*rowPitch = GetRowPitchSize();
	*slicePitch = 0;
}
cl_err_code SingleUnifiedImage2D::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
    if( (pszOrigin[0] + pszRegion[0]) > m_szImageWidth )
    {
		return CL_INVALID_VALUE;
    }
    if( (pszOrigin[1] + pszRegion[1]) > m_szImageHeight )
    {
        return CL_INVALID_VALUE;
    }
    // In bounds
	return CL_SUCCESS;
}

cl_err_code SingleUnifiedImage2D::CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{
	return CL_INVALID_VALUE;
}

size_t SingleUnifiedImage2D::CalcImageSize()
{
	if (NULL != m_pHostPtr && 0 != m_szImageRowPitch)
	{
		return m_szImageRowPitch * m_szImageWidth;
	}
	return m_szImageWidth * m_szImageHeight * m_szElementSize;
}

size_t SingleUnifiedImage2D::GetPixelSize() const
{
	return m_szElementSize;
}

size_t SingleUnifiedImage2D::GetSlicePitchSize(void) const
{
	return 0;
}
