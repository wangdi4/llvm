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

#include "SingleUnifiedImage2DArray.h"
#include "Device.h"

#include <cpu_dev_limits.h>
#include <assert.h>

//using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


//REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, TRUE, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_BUFFER, SingleUnifiedBuffer)

SingleUnifiedImage2DArray::SingleUnifiedImage2DArray(Context* pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType) :
	SingleUnifiedImage3D(pContext, pOclEntryPoints, clObjType)
{
	m_clMemObjectType = CL_MEM_OBJECT_IMAGE2D_ARRAY;
}

SingleUnifiedImage2DArray::~SingleUnifiedImage2DArray()
{
	for (size_t i = 0, e=m_img2DArray.size(); i < e; ++i)
	{
		m_img2DArray[i]->Release();
	}
    m_img2DArray.clear();
}

cl_err_code SingleUnifiedImage2DArray::Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			)
{
	// First intialize as 3D image
	cl_err_code ret = SingleUnifiedImage3D::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr);
	if ( CL_FAILED(ret) )
	{
		return ret;
	}

    for (size_t i = 0; i < m_szImageDepth; i++)
    {
		MemoryObject* pImage2D = new SingleUnifiedImage2D(m_pContext, (ocl_entry_points*)(m_pContext->GetHandle()->dispatch), CL_MEM_OBJECT_IMAGE2D);
		if ( NULL == pImage2D )
		{
			return CL_OUT_OF_HOST_MEMORY;
		}

		ret = pImage2D->Initialize(clMemFlags, pclImageFormat, 2, dimension, &m_szImageRowPitch, &((char*)m_pMemObjData)[m_szImageSlicePitch * i]);
        if (CL_FAILED(ret))
        {
			pImage2D->Release();
            return ret;
        }

		m_img2DArray.push_back(pImage2D);
    }

	return CL_SUCCESS;
}

cl_err_code SingleUnifiedImage2DArray::ReadData(void* pOutData, const size_t* pszOrigin,
                                   const size_t* pszRegion, size_t szRowPitch,
                                   size_t szSlicePitch)
{
    const size_t imageIndex = pszOrigin[2];
    MemoryObject* image = m_img2DArray[imageIndex];
    const size_t origin[3] = { pszOrigin[0], pszOrigin[1], 0};
    return image->ReadData(pOutData, origin, pszRegion, szRowPitch, szSlicePitch);
}

cl_err_code SingleUnifiedImage2DArray::WriteData(const void* pOutData, const size_t* pszOrigin,
                                    const size_t* pszRegion, size_t szRowPitch,
                                    size_t szSlicePitch)
{
    const size_t imageIndex = pszOrigin[2];
    MemoryObject* image = m_img2DArray[imageIndex];
    const size_t origin[3] = { pszOrigin[0], pszOrigin[1], 0};
    return image->WriteData(pOutData, origin, pszRegion, szRowPitch, szSlicePitch);
}

void SingleUnifiedImage2DArray::GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const
{
    m_img2DArray[0]->GetLayout(dimensions, rowPitch, slicePitch);
}

cl_err_code SingleUnifiedImage2DArray::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
	if (pszOrigin[2] >= m_szImageDepth)
    {
        return CL_INVALID_VALUE;
    }
    if (1 != pszRegion[2])
    {
        return CL_INVALID_VALUE;
    }
    const size_t origin[3] = { pszOrigin[0], pszOrigin[1], 0};
    return m_img2DArray[pszOrigin[2]]->CheckBounds(origin, pszRegion);
}

void* SingleUnifiedImage2DArray::GetBackingStoreData(const size_t* pszOrigin)
{
    if (pszOrigin[2] >= m_szImageDepth)
        return NULL;
    const size_t origin[3] = { pszOrigin[0], pszOrigin[1], 0};
    return m_img2DArray[pszOrigin[2]]->GetBackingStoreData(origin);
}

#if 0   // disabled until changes in the spec regarding 2D image arrays are made
cl_err_code SingleUnifiedImage2DArray::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
    LOG_DEBUG(TEXT("Enter(clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    if (NULL == pParamValue && NULL == pszParamValueSizeRet)
    {
        return CL_INVALID_VALUE;
    }
    size_t  szSize = 0;
    const void * pValue = NULL;
    const cl_image_array_type imageArrType = CL_IMAGE_ARRAY_SAME_DIMENSIONS;
    switch (clParamName)
    {
    case CL_IMAGE_ARRAY_TYPE:
        szSize = sizeof(imageArrType);
        pValue = &imageArrType;
        break;
    default:
		return SingleUnifiedImage2D::GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
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
#endif