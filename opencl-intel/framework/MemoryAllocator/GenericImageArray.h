// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "GenericMemObj.h"
#include "MemoryObject.h"

#if defined(_MSC_VER)
/* 'Intel::OpenCL::Framework::GenericImageArray<ELEM_IMAGE_NUM_DIM>' : inherits
    'Intel::OpenCL::Framework::GenericMemObject::...' via dominance - don't
   warn, it's intended */
#pragma warning(disable : 4250)
#endif

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class represents an array of images of any number of dimensions.
 * @param ELEM_IMAGE_NUM_DIM number of dimensions of each image element in the
 * array
 *
 * GenericImageArray inherits from GenericMemObject. This sub-object is an image
 * object whose number of dimensions one higher than the number of dimensions of
 * each image element in the array plus 1. This image handles most of the work
 * of the GenericImageArray, since the memory layout in the back-store of image
 * arrays is a sequential array of images, which is equivalent to a single image
 * from one higher number of dimensions.
 */
template <size_t ELEM_IMAGE_NUM_DIM>
class GenericImageArray : public GenericMemObject, public IMemoryObjectArray {
public:
  PREPARE_SHARED_PTR(GenericImageArray)

  static SharedPtr<GenericImageArray> Allocate(SharedPtr<Context> pContext,
                                               cl_mem_object_type clObjType) {
    return SharedPtr<GenericImageArray>(
        new GenericImageArray(pContext, clObjType));
  }

  // overridden methods:

  size_t GetNumObjects() const override;

private:
  /**
   * Constructor
   */
  GenericImageArray(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
      : GenericMemObject(pContext, GetEquivalentSingleImageType(clObjType)) {
    m_clMemObjectType = clObjType;
  }

  static cl_mem_object_type
  GetEquivalentSingleImageType(cl_mem_object_type clMemObjType);
};

/**
 * @fn GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetNumObjects() const
 */
template <size_t ELEM_IMAGE_NUM_DIM>
size_t GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetNumObjects() const {
  const IOCLDevBackingStore *pBackingStore;
  const cl_dev_err_code clErr =
      GetBackingStore(CL_DEV_BS_GET_IF_AVAILABLE, &pBackingStore);
  assert(CL_SUCCEEDED(clErr));
  return (CL_SUCCEEDED(clErr)
              ? pBackingStore->GetDimentions()[ELEM_IMAGE_NUM_DIM]
              : 0);
}

/**
 * @fn
 * GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetEquivalentSingleImageType(cl_mem_object_type
 * clMemObjType)
 */
template <size_t ELEM_IMAGE_NUM_DIM>
cl_mem_object_type
GenericImageArray<ELEM_IMAGE_NUM_DIM>::GetEquivalentSingleImageType(
    cl_mem_object_type clMemObjType) {
  switch (clMemObjType) {
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    return CL_MEM_OBJECT_IMAGE2D;
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    return CL_MEM_OBJECT_IMAGE3D;
  default:
    assert(false);
    return 0;
  }
}

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
