// Copyright (c) 2006-2010 Intel Corporation
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
//  Image1DBuffer.cpp
//  Implementation of the Image1DBufer Class
//  Created on:      22-Aug-2011
//  Original author: kdmitry
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Image1DBuffer.h"

using namespace Intel::OpenCL::Framework;

cl_err_code Image1DBuffer::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet) const
{
    if (NULL == pParamValue && NULL == pszParamValueSizeRet)
    {
        return CL_INVALID_VALUE;
    }    
    if (CL_IMAGE_BUFFER == clParamName)
    {
        if (NULL != pParamValue && szParamValueSize < sizeof(cl_mem))
        {
            return CL_INVALID_VALUE;
        }
        if (NULL != pParamValue)
        {
            *(cl_mem*)pParamValue = m_pBuffer->GetHandle();
        }
        if (NULL != pszParamValueSizeRet)
        {
            *pszParamValueSizeRet = sizeof(cl_mem);
        }
        return CL_SUCCESS;
    }
    return GenericMemObject::GetImageInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

void Image1DBuffer::SetBuffer(SharedPtr<GenericMemObject> pBuffer)
{
    m_pBuffer = pBuffer;
}
