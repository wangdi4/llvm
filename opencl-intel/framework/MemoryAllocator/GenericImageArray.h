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

#include "MemoryObject.h"
#include "GenericMemObj.h"

/* 'Intel::OpenCL::Framework::GenericImageArray<ELEM_IMAGE_NUM_DIM>' : inherits
    'Intel::OpenCL::Framework::GenericMemObject::...' via dominance - don't warn, it's intended */
#pragma warning(disable : 4250)

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class represents an array of images of any number of dimensions.
 * @param ELEM_IMAGE_NUM_DIM number of dimensions of each image element in the array
 * 							 
 * GenericImageArray inherits from GenericMemObject. This sub-object is an image object whose
 * number of dimensions one higher than the number of dimensions of each image element in the array
 * plus 1. This image handles most of the work of the GenericImageArray, since the memory layout in
 * the back-store of image arrays is a sequential array of images, which is equivalent to a single
 * image from one higher number of dimensions.
 */
template<size_t ELEM_IMAGE_NUM_DIM>
class GenericImageArray : public GenericMemObject, public IMemoryObjectArray
{
public:    

    /**
     * Constructor
     */
    GenericImageArray(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
      GenericMemObject(pContext, pOclEntryPoints, GetEquivalentSingleImageType(clObjType))
      {
          m_clMemObjectType = clObjType;
      }

    // overridden methods:

	size_t GetNumObjects() const;
    
private:

    static cl_mem_object_type GetEquivalentSingleImageType(cl_mem_object_type clMemObjType);

};

using namespace Intel::OpenCL::Framework;

/**
 * @fn GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetNumObjects() const
 */
template<size_t ELEM_IMAGE_NUM_DIM>
size_t GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetNumObjects() const
{
    const IOCLDevBackingStore* pBackingStore;    
    const cl_dev_err_code clErr = GetBackingStore(CL_DEV_BS_GET_IF_AVAILABLE, &pBackingStore);
    assert(CL_SUCCEEDED(clErr));
    return (CL_SUCCEEDED(clErr) ? pBackingStore->GetDimentions()[ELEM_IMAGE_NUM_DIM] : 0 );
}

/**
 * @fn GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetEquivalentSingleImageType(cl_mem_object_type clMemObjType)
 */
template<size_t ELEM_IMAGE_NUM_DIM>
cl_mem_object_type GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetEquivalentSingleImageType(cl_mem_object_type clMemObjType)
{
    switch (clMemObjType)
    {
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        return CL_MEM_OBJECT_IMAGE2D;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        return CL_MEM_OBJECT_IMAGE3D;
    default:
        assert(false);
        return 0;
    }
}

}}}
