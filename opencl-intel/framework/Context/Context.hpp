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

#include "Context.h"

namespace Intel { namespace OpenCL { namespace Framework {

#if !defined (_WIN32)
    /* In the line:
    for (size_t i = 0; i < DIM - 1; i++)
    don't issue an error that i < DIM - 1 is always false when DIM is 1 - this is intentional */
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

template<size_t DIM, cl_mem_object_type OBJ_TYPE>
cl_err_code Context::CreateImage(cl_mem_flags	         clFlags,
                                 const cl_image_format * pclImageFormat,
                                 void *                  pHostPtr,
                                 const size_t*           szImageDims,
                                 const size_t*           szImagePitches,
                                 SharedPtr<MemoryObject>*         ppImage,
                                 bool                    bIsImageBuffer)
{
    assert ( NULL != ppImage );
    //check image sizes
    const size_t szImageDimsPerDim[3][3] = {
        { m_sz2dWidth },                            // DIM == 1
        { m_sz2dWidth, m_sz2dHeight, 0},            // DIM == 2
        { m_sz3dWidth, m_sz3dHeight, m_sz3dDepth }  // DIM == 3
    };
    // OpenCL 1.2 doesn't state that the dimensions shouldn't be 0, but I believe this is a mistake (Yariv should verify it)
    if (bIsImageBuffer)
    {        
        if (0 == szImageDims[0])
        {
            LOG_ERROR(TEXT("image width is 0") , "");
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        if (szImageDims[0] > m_sz1dImgBufSize)
        {
            LOG_ERROR(TEXT("For a 1D image buffer, the image width must be <= CL_DEVICE_IMAGE_MAX_BUFFER_SIZE"), "");
            return CL_INVALID_IMAGE_SIZE;
        }
    }
    else
    {
        for (size_t i = 0; i < DIM; i++)
        {
            if (0 == szImageDims[i])
            {
                LOG_ERROR(TEXT("0 == szImageDims[i]"), "");
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            if (szImageDims[i] > szImageDimsPerDim[DIM - 1][i])
            {
                LOG_ERROR(TEXT("image dimension is not allowed"), "");
                return CL_INVALID_IMAGE_SIZE;
            }
        }
    }    

    cl_err_code clErr = CheckSupportedImageFormatByMemFlags(clFlags, *pclImageFormat, OBJ_TYPE);
    if (CL_FAILED(clErr))
    {
        return clErr;
    }

    clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, OBJ_TYPE, CL_MEMOBJ_GFX_SHARE_NONE, this, ppImage);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new Image3D, returned: %s"), ClErrTxt(clErr));
        return clErr;
    }

    size_t dim[3] = {0}, pitch[2] = {0};
    for (size_t i = 0; i < DIM; i++)
    {
        dim[i] = szImageDims[i];
    }
    for (size_t i = 0; i < DIM - 1; i++)
    {
        pitch[i] = szImagePitches[i];
    }
    if (bIsImageBuffer)
    {
        clErr = (*ppImage)->Initialize(clFlags, pclImageFormat, DIM, dim, pitch, pHostPtr, CL_RT_MEMOBJ_FORCE_BS);
    }
    else
    {
        clErr = (*ppImage)->Initialize(clFlags, pclImageFormat, DIM, dim, pitch, pHostPtr, 0);
    }    
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error Initialize new buffer, returned: %s"), ClErrTxt(clErr));
        (*ppImage)->Release();
        return clErr;
    }

    m_mapMemObjects.AddObject(*ppImage);

    return clErr;
}

}}}
