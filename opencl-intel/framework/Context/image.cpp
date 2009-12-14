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
				 cl_err_code *     pErrCode): MemoryObject(pContext, clMemFlags, pHostPtr, pErrCode)
{
#ifdef _DEBUG
	assert ( NULL != pErrCode );
#endif
	m_pclImageFormat = NULL;
	m_szImageWidth = 0;
	m_szImageHeight = 0;
	m_szImageRowPitch = 0;
	m_lDataOnHost = 0;
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
	LOG_DEBUG(L"Enter MemoryObject D'tor");
	if (NULL != m_pclImageFormat)
	{
		delete m_pclImageFormat;
        m_pclImageFormat = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::Release()
{
	return MemoryObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter ::::GetImageInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)",
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
		memcpy_s(pParamValue, szParamValueSize, pValue, szSize);
	}
	
	return CL_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::CreateDeviceResource(cl_device_id clDeviceId)
{
	LOG_DEBUG(L"Enter CreateDeviceResource (clDeviceId=%d)", clDeviceId);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		LOG_ERROR(L"Can't find device %d", clDeviceId);
		return CL_INVALID_DEVICE;
	}

	DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
	assert ( pDevMemObj != NULL );
#endif

	//return pDevMemObj->AllocateImage2D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageRowPitch, m_pHostPtr);
	return pDevMemObj->AllocateImage2D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageRowPitch, m_pMemObjData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image2D::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image2D::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", 
		szRowPitch, pData, szSlicePitch);

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

void* Image2D::CreateMappedRegion(cl_device_id clDeviceId, 
								 cl_map_flags clMapFlags, 
								 const size_t * szOrigins, 
								 const size_t * szRegions, 
								 size_t * pszImageRowPitch, 
								 size_t * pszImageSlicePitch)
{
	LOG_DEBUG(L"Enter CreateMappedRegion (clDeviceId=%d, cl_map_flags=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d)", 
		clDeviceId, clMapFlags, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
	
	// get device memory object
	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj || false == pDevMemObj->IsAllocated())
	{
		return NULL;
	}
	return pDevMemObj->CreateMappedRegion(clMapFlags, 2 /*Image2D*/, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
}
cl_err_code Image2D::ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter ReleaseMappedRegion (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return CL_DEVICE_NOT_FOUND;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	return pDevMemObj->ReleaseMappedRegion(mappedPtr);
}
void* Image2D::GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter GetMappedRegionInfo (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return NULL;
	}
	return pDevMemObj->GetMappedRegionInfo(mappedPtr);
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
		szBytesCount = sizeof(signed __int8);
		break;
	case CL_SNORM_INT16:
	case CL_SIGNED_INT16:
	case CL_UNORM_SHORT_565:
	case CL_UNORM_SHORT_555:
		szBytesCount = sizeof(signed __int16);
		break;
	case CL_UNORM_INT8:
	case CL_UNSIGNED_INT8:
		szBytesCount = sizeof(unsigned __int8);
		break;
	case CL_UNORM_INT16:
	case CL_UNSIGNED_INT16:
		szBytesCount = sizeof(unsigned __int16);
		break;
	case CL_SIGNED_INT32:
	case CL_UNORM_INT_101010:
		szBytesCount = sizeof(signed __int32);
		break;
	case CL_UNSIGNED_INT32:
		szBytesCount = sizeof(unsigned __int32);
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
		return m_szImageRowPitch * m_szImageWidth;
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
				cl_err_code *     pErrCode): Image2D(pContext, clMemFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, pErrCode)
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
	if ( (NULL != pHostPtr) && ((szImageSlicePitch<0) || (szImageSlicePitch>szMaxSlicePitchSize)) )
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
	LOG_DEBUG(L"Enter MemoryObject D'tor");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::Release()
{
	return MemoryObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter ::::GetImageInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)",
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
		memcpy_s(pParamValue, szParamValueSize, pValue, szSize);
	}
	
	return CL_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::CreateDeviceResource(cl_device_id clDeviceId)
{
	LOG_DEBUG(L"Enter CreateDeviceResource (clDeviceId=%d)", clDeviceId);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		LOG_ERROR(L"Can't find device %d", clDeviceId);
		return CL_INVALID_DEVICE;
	}

	DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
	assert ( pDevMemObj != NULL );
#endif

	//return pDevMemObj->AllocateImage3D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageDepth, m_szImageRowPitch, m_szImageSlicePitch, m_pHostPtr);
	return pDevMemObj->AllocateImage3D(m_clFlags, m_pclImageFormat, m_szImageWidth, m_szImageHeight, m_szImageDepth, m_szImageRowPitch, m_szImageSlicePitch, m_pMemObjData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Image3D::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Image3D::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

	return CL_ERR_NOT_IMPLEMENTED;
}
size_t Image3D::GetSize() const
{
	return m_szMemObjSize;
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
void* Image3D::CreateMappedRegion(cl_device_id clDeviceId, 
								 cl_map_flags clMapFlags, 
								 const size_t * szOrigins, 
								 const size_t * szRegions, 
								 size_t * pszImageRowPitch, 
								 size_t * pszImageSlicePitch)
{
	LOG_DEBUG(L"Enter CreateMappedRegion (clDeviceId=%d, cl_map_flags=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d)", 
		clDeviceId, clMapFlags, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
	
	// get device memory object
	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj || false == pDevMemObj->IsAllocated())
	{
		return NULL;
	}
	return pDevMemObj->CreateMappedRegion(clMapFlags, 3 /*Image3D*/,szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
}
cl_err_code Image3D::ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter ReleaseMappedRegion (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return CL_DEVICE_NOT_FOUND;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	return pDevMemObj->ReleaseMappedRegion(mappedPtr);
}

void* Image3D::GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter GetMappedRegionInfo (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return NULL;
	}
	return pDevMemObj->GetMappedRegionInfo(mappedPtr);
}
size_t Image3D::CalcImageSize()
{
	if (NULL != m_pHostPtr && 0 != m_szImageSlicePitch)
	{
		return m_szImageSlicePitch * m_szImageDepth;
	}
	return m_szImageWidth * m_szImageHeight * m_szImageDepth * GetPixelBytesCount(m_pclImageFormat);
}
