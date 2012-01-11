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

#include "GenericMemObj.h"
#include "CL/cl.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * Represents a 1D image buffer object
 */
class Image1DBuffer : public GenericMemObject
{

public:

    /**
     * Constructor
     */
    Image1DBuffer(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) : GenericMemObject(pContext, pOclEntryPoints, clObjType) { }

    /**
     * Destructor
     */
    ~Image1DBuffer();

    /**
     * Sets the buffer object from which the Image1DBuffer is created and also increment its dependency
     * @param pBuffer the buffer object
     */
    void SetBuffer(GenericMemObject* pBuffer);

    // overloaded functions

    cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet) const;

private:

    GenericMemObject* m_pBuffer;

};

}}}
