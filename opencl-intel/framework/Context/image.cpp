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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  image.cpp
//  Implementation of the Image2D and Image3D Classes
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "image.h"
#include "cl_sys_defines.h"
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Image2D::Image2D(Context *         pContext, 
				 cl_mem_flags      clMemFlags,
				 cl_image_format * pclImageFormat,
				 void *            pHostPtr,
				 size_t            szImageWidth,
				 size_t            szImageHeight,
				 size_t            szImageRowPitch,
				 ocl_entry_points * pOclEntryPoints,
				 cl_err_code *     pErrCode): MemoryObject(pContext, clMemFlags, pOclEntryPoints, pErrCode)
{
#ifdef _DEBUG
	assert ( NULL != pErrCode );
#endif
	m_pclImageFormat = NULL;
	m_szImageWidth = 0;
	m_szImageHeight = 0;
	m_szImageRowPitch = 0;
	m_pHostPtr = pHostPtr;

	if (CL_FAILED(*pErrCode))
	{
		return;
	}
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D;

	if ( (szImageWidth < 1) || (szImageHeight < 1) )
	{
		*pErrCode = CL_INVALID_VALUE;
		return;
	}

	// check image format
	*pErrCode = CheckImageFormat(pclImageFormat, m_clFlags);
	if (CL_FAILED(*pErrCode))
	{
		return;
	}

	if ( (NULL == pHostPtr) && (0 != szImageRowPitch) )
	{
		*pErrCode = CL_INVALID_IMAGE_SIZE;
		return;
	}


	size_t szMinRowPitchSize = szImageWidth * GetPixelBytesCount(pclImageFormat);
	if ( (NULL != pHostPtr) && (0 != szImageRowPitch) && (szImageRowPitch<szMinRowPitchSize) )
	{
		*pErrCode = CL_INVALID_IMAGE_SIZE;
		return;
	}

	m_szImageWidth = szImageWidth;
	m_szImageHeight = szImageHeight;

	// set image format (copy data);
	m_pclImageFormat = new cl_image_format();
	*m_pclImageFormat = *pclImageFormat;

	m_szImageRowPitch = (szImageRowPitch != 0) ? szImageRowPitch : (m_szImageWidth * GetPixelBytesCount(m_pclImageFormat));

	// create buffer for image data
	m_szMemObjSize = CalcImageSize();

	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Image2D::~Image2D()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter MemoryObject D'tor"));
	if (NULL != m_pclImageFormat)
	{
		delete m_pclImageFormat;
        m_pclImageFormat = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(TEXT("Enter ::::GetImageInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t  szSize = 0;
	void * pValue = NULL;
	size_t szValue = 0;
	switch (clParamName)
	{
	case CL_IMAGE_FORMAT:
		szSize = sizeof(cl_image_format);
		pValue = m_pclImageFormat;
		break;
	case CL_IMAGE_ELEMENT_SIZE:
		szSize = sizeof(size_t);
		szValue = GetPixelBytesCount(m_pclImageFormat);
		pValue = &szValue;
		break;
	case CL_IMAGE_ROW_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_szImageRowPitch;
		break;
	case CL_IMAGE_SLICE_PITCH:
		szSize = sizeof(size_t);
		pValue = &szValue;
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
		szSize = sizeof(size_t);
		pValue = &szValue;
		break;
	default:
		return CL_INVALID_VALUE;
		break;
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
// Image2D::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::CreateDeviceResource(cl_device_id clDeviceId)
{
	LOG_DEBUG(TEXT("Enter CreateDeviceResource (clDeviceId=%d)"), clDeviceId);

	DeviceMemoryObject * pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		LOG_ERROR(TEXT("Can't find device %d"), clDeviceId);
		return CL_INVALID_DEVICE;
	}

	//return pDevMemObj->AllocateImage2D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageRowPitch, m_pHostPtr);
	return pDevMemObj->AllocateImage2D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageRowPitch, m_pMemObjData);
}
struct SMemCpyParams
{
	cl_uint			uiDimCount;
	cl_char*		pSrc;
	size_t			vSrcPitch[MAX_WORK_DIM-1];
	cl_char*		pDst;
	size_t			vDstPitch[MAX_WORK_DIM-1];
	size_t			vRegion[MAX_WORK_DIM];
};

void CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
	// Copy 1D array only
	if ( 1 == pCopyCmd->uiDimCount )
	{
		memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
		return;
	}

	SMemCpyParams sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pCopyCmd, sizeof(SMemCpyParams));
	sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
	// Make recursion
	for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
	{
		CopyMemoryBuffer(&sRecParam);
		sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
		sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(TEXT("Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)"), szRowPitch, pData, szSlicePitch);

	SMemCpyParams			sCpyParam;
	
	// Region
	sCpyParam.uiDimCount = 2;
	memcpy(sCpyParam.vRegion, pszRegion, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = pszRegion[0] * GetPixelBytesCount(m_pclImageFormat);
	
	// set row Pitche for src and dst
	size_t srcPitch[2] = { m_szImageRowPitch , 0 };
	size_t dstPitch[2] = { szRowPitch , 0 };
	
	sCpyParam.pSrc = (cl_char*)m_pMemObjData;
	memcpy(sCpyParam.vSrcPitch, srcPitch, sizeof(sCpyParam.vSrcPitch));
	sCpyParam.pDst = (cl_char*)pData;
	memcpy(sCpyParam.vDstPitch, dstPitch , sizeof(sCpyParam.vDstPitch));
	
	// origin
	sCpyParam.pSrc += pszOrigin[0] * GetPixelBytesCount(m_pclImageFormat); //Origin is in Pixels
	sCpyParam.pSrc += pszOrigin[1] * m_szImageRowPitch; //y * image width pitch 		
	
	CopyMemoryBuffer(&sCpyParam);

	return CL_SUCCESS;	
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::WriteData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(TEXT("Enter WriteData (szRowPitch=%d, pData=%d, szSlicePitch=%d)"), szRowPitch, pData, szSlicePitch);

	return CL_ERR_NOT_IMPLEMENTED;
}
size_t Image2D::GetSize() const
{
	return m_szMemObjSize;
}
void * Image2D::GetData( const size_t * pszOrigin ) const
{
    // TODO: Add support for pszOrigin != NULL;
	return m_pMemObjData;
}
void Image2D::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
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
bool Image2D::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
    if( (pszOrigin[0] + pszRegion[0]) > m_szImageWidth )
    {
        return false;
    }
    if( (pszOrigin[1] + pszRegion[1]) > m_szImageHeight )
    {
        return false;
    }
    // In bounds
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::CheckImageFormat
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::CheckImageFormat(cl_image_format * pclImageFormat, cl_mem_flags clMemFlags)
{
	if (NULL == pclImageFormat)
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}
	
	// validity checking
	if ((pclImageFormat->image_channel_order == CL_INTENSITY)	&&
		( (pclImageFormat->image_channel_data_type != CL_UNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_UNORM_INT16) &&
		  (pclImageFormat->image_channel_data_type != CL_SNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_SNORM_INT16) &&
		  (pclImageFormat->image_channel_data_type != CL_HALF_FLOAT) &&
		  (pclImageFormat->image_channel_data_type != CL_FLOAT)))
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	if ((pclImageFormat->image_channel_order == CL_LUMINANCE)	&&
		( (pclImageFormat->image_channel_data_type != CL_UNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_UNORM_INT16) &&
		  (pclImageFormat->image_channel_data_type != CL_SNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_SNORM_INT16) &&
		  (pclImageFormat->image_channel_data_type != CL_HALF_FLOAT) &&
		  (pclImageFormat->image_channel_data_type != CL_FLOAT)))
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	if ((pclImageFormat->image_channel_order == CL_RGB)	&&
		( (pclImageFormat->image_channel_data_type != CL_UNORM_SHORT_565) &&
		  (pclImageFormat->image_channel_data_type != CL_UNORM_SHORT_555) &&
		  (pclImageFormat->image_channel_data_type != CL_UNORM_INT_101010) ))
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	if ( ((pclImageFormat->image_channel_order == CL_ARGB) || (pclImageFormat->image_channel_order == CL_BGRA)) &&	
		( (pclImageFormat->image_channel_data_type != CL_UNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_SNORM_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_SIGNED_INT8) &&
		  (pclImageFormat->image_channel_data_type != CL_UNSIGNED_INT8) ))
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	// support checking
	if ((clMemFlags & CL_MEM_READ_WRITE) | (clMemFlags & CL_MEM_READ_ONLY) | (clMemFlags & CL_MEM_WRITE_ONLY))
	{
		cl_uint uiImagesFormatsCount = 0;
		cl_image_format * pclImageFormats = NULL;
		cl_err_code clErr = m_pContext->GetSupportedImageFormats(clMemFlags, m_clMemObjectType, 0, NULL, &uiImagesFormatsCount);
		if (CL_FAILED(clErr) || (0 == uiImagesFormatsCount))
		{
			return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		}
		pclImageFormats = new cl_image_format[uiImagesFormatsCount];
		if (NULL == pclImageFormats)
		{
			return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		}
		clErr = m_pContext->GetSupportedImageFormats(clMemFlags, m_clMemObjectType, uiImagesFormatsCount, pclImageFormats, NULL);
		if (CL_FAILED(clErr))
		{
			delete[] pclImageFormats;
			return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
		}

		for (cl_uint ui=0; ui<uiImagesFormatsCount; ++ui)
		{
			if ( (pclImageFormat->image_channel_order == pclImageFormat[ui].image_channel_order) &&
				(pclImageFormat->image_channel_data_type == pclImageFormat[ui].image_channel_data_type) )
			{
				delete[] pclImageFormats;
				return CL_SUCCESS;
			}
		}
		delete[] pclImageFormats;
	}

	return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
}
size_t Image2D::GetPixelBytesCount(cl_image_format * pclImageFormat)
{
	if (NULL == pclImageFormat)
	{
		return 0;
	}
	size_t szBytesCount = 0, szElementsCount = 0;
	
	// get size of element in bytes
	switch(pclImageFormat->image_channel_data_type)
	{
	case CL_SNORM_INT8:
	case CL_SIGNED_INT8:
		szBytesCount = sizeof(signed char);
		break;
	case CL_SNORM_INT16:
	case CL_SIGNED_INT16:
	case CL_UNORM_SHORT_565:
	case CL_UNORM_SHORT_555:
		szBytesCount = sizeof(signed short);
		break;
	case CL_UNORM_INT8:
	case CL_UNSIGNED_INT8:
		szBytesCount = sizeof(unsigned char);
		break;
	case CL_UNORM_INT16:
	case CL_UNSIGNED_INT16:
		szBytesCount = sizeof(unsigned short);
		break;
	case CL_SIGNED_INT32:
	case CL_UNORM_INT_101010:
		szBytesCount = sizeof(signed int);
		break;
	case CL_UNSIGNED_INT32:
		szBytesCount = sizeof(unsigned int);
		break;
	case CL_HALF_FLOAT:
		szBytesCount = sizeof(cl_half);
		break;
	case CL_FLOAT:
		szBytesCount = sizeof(cl_float);
		break;
	default:
		assert(0);
	}

	// get number of elements in pixel
	switch(pclImageFormat->image_channel_order)
	{
	case CL_R:
	case CL_A:
		szElementsCount = 1;
		break;
	case CL_RG:
	case CL_RA:
		szElementsCount = 2;
		break;
	case CL_RGB:
		szElementsCount = 1;
		break;
	case CL_RGBA:
	case CL_BGRA:
	case CL_ARGB:
		szElementsCount = 4;
		break;
	case CL_LUMINANCE:
	case CL_INTENSITY:
		szElementsCount = 1;
		break;
	default:
		assert(0);
	}

	return szBytesCount * szElementsCount;
}
size_t Image2D::CalcImageSize()
{
	if (NULL != m_pHostPtr && 0 != m_szImageRowPitch)
	{
		return m_szImageRowPitch * m_szImageHeight;
	}
	return m_szImageWidth * m_szImageHeight * GetPixelBytesCount(m_pclImageFormat);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Image3D::Image3D(Context *         pContext, 
				cl_mem_flags      clMemFlags,
				cl_image_format * pclImageFormat,
				void *            pHostPtr,
				size_t            szImageWidth,
				size_t            szImageHeight,
				size_t            szImageDepth,
				size_t            szImageRowPitch,
				size_t            szImageSlicePitch,
				ocl_entry_points * pOclEntryPoints,
				cl_err_code *     pErrCode): Image2D(pContext, clMemFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, pOclEntryPoints, pErrCode)
{
#ifdef _DEBUG
	assert ( NULL != pErrCode );
#endif
	m_szImageDepth = 0;
	m_szImageSlicePitch = 0;

	if (CL_FAILED(*pErrCode))
	{
		return;
	}
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE3D;

	if ( (NULL == pHostPtr) && (0 != szImageSlicePitch) )
	{
		*pErrCode = CL_INVALID_IMAGE_SIZE;
		return;
	}

	// check image slice pitch
	size_t szMaxSlicePitchSize = szImageRowPitch * szImageHeight;
	if ( (NULL != pHostPtr) && (szImageSlicePitch>szMaxSlicePitchSize))
	{
		*pErrCode = CL_INVALID_IMAGE_SIZE;
		return;
	}

	m_szImageDepth = szImageDepth;
	m_szImageSlicePitch = (m_szImageSlicePitch != 0) ? m_szImageSlicePitch : m_szImageRowPitch * m_szImageHeight;

	// create buffer for image data
	m_szMemObjSize = Image3D::CalcImageSize();

	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Image3D::~Image3D()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter MemoryObject D'tor"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(TEXT("Enter ::::GetImageInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t  szSize = 0;
	void * pValue = NULL;
	size_t szValue = 0;
	switch (clParamName)
	{
	case CL_IMAGE_FORMAT:
		szSize = sizeof(cl_image_format);
		pValue = m_pclImageFormat;
		break;
	case CL_IMAGE_ELEMENT_SIZE:
		szSize = sizeof(size_t);
		szValue = GetPixelBytesCount(m_pclImageFormat);
		pValue = &szValue;
		break;
	case CL_IMAGE_ROW_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_szImageRowPitch;
		break;
	case CL_IMAGE_SLICE_PITCH:
		szSize = sizeof(size_t);
		pValue = &m_szImageSlicePitch;
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
		szSize = sizeof(size_t);
		pValue = &m_szImageDepth;
		break;
	default:
		return CL_INVALID_VALUE;
		break;
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
// Image3D::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::CreateDeviceResource(cl_device_id clDeviceId)
{
	LOG_DEBUG(TEXT("Enter CreateDeviceResource (clDeviceId=%d)"), clDeviceId);

	DeviceMemoryObject * pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		LOG_ERROR(TEXT("Can't find device %d"), clDeviceId);
		return CL_INVALID_DEVICE;
	}
	//return pDevMemObj->AllocateImage3D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageDepth, m_szImageRowPitch, m_szImageSlicePitch, m_pHostPtr);
	return pDevMemObj->AllocateImage3D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageDepth, m_szImageRowPitch, m_szImageSlicePitch, m_pMemObjData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(TEXT("Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)"), szRowPitch, pData, szSlicePitch);

	SMemCpyParams			sCpyParam;

	// Region
	sCpyParam.uiDimCount = 3;
	memcpy(sCpyParam.vRegion, pszRegion, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = pszRegion[0] * GetPixelBytesCount(m_pclImageFormat);
	
	// set row Pitche for src and dst
	size_t srcPitch[3] = { m_szImageRowPitch , m_szImageSlicePitch };
	size_t dstPitch[3] = { szRowPitch , szSlicePitch };
	
	sCpyParam.pSrc = (cl_char*)m_pMemObjData;
	memcpy(sCpyParam.vSrcPitch, srcPitch, sizeof(sCpyParam.vSrcPitch));
	sCpyParam.pDst = (cl_char*)pData;
	memcpy(sCpyParam.vDstPitch, dstPitch , sizeof(sCpyParam.vDstPitch));
	
	// origin
	sCpyParam.pSrc += pszOrigin[0] * GetPixelBytesCount(m_pclImageFormat); //Origin is in Pixels
	sCpyParam.pSrc += pszOrigin[1] * m_szImageRowPitch; //y * image width pitch 		
	sCpyParam.pSrc += pszOrigin[2] * m_szImageSlicePitch; //y * image width pitch		
	
	CopyMemoryBuffer(&sCpyParam);

	return CL_SUCCESS;
}
size_t Image3D::GetSize() const
{
	return m_szMemObjSize;
}
void Image3D::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
{
	if (dimensions)
	{
		dimensions[0] = m_szImageWidth;
		dimensions[1] = m_szImageHeight;
		dimensions[2] = m_szImageDepth;		
	}
	*rowPitch = GetRowPitchSize();
	*slicePitch = GetSlicePitchSize();
}
bool Image3D::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
    if( (pszOrigin[0] + pszRegion[0]) > m_szImageWidth )
    {
        return false;
    }
    if( (pszOrigin[1] + pszRegion[1]) > m_szImageHeight )
    {
        return false;
    }
    if( (pszOrigin[2] + pszRegion[2]) > m_szImageDepth )
    {
        return false;
    }
    // In bounds
    return true;
}

void * Image3D::GetData( const size_t * pszOrigin ) const
{
    // TODO: Add support for pszOrigin != NULL;
	return m_pMemObjData;
}
size_t Image3D::CalcImageSize()
{
	if (NULL != m_pHostPtr && 0 != m_szImageSlicePitch)
	{
		return m_szImageSlicePitch * m_szImageDepth;
	}
	return m_szImageWidth * m_szImageHeight * m_szImageDepth * GetPixelBytesCount(m_pclImageFormat);
}
