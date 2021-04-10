// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "TypeAlignment.h"
#include "CL/cl.h"

#include <assert.h>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

size_t TypeAlignment::getSize(const KernelArgument& arg) {
  switch(arg.Ty)
  {
  case KRNL_ARG_INT:
    return arg.SizeInBytes;

  case KRNL_ARG_UINT:
    return arg.SizeInBytes;

  case KRNL_ARG_FLOAT:
    return arg.SizeInBytes;

  case KRNL_ARG_DOUBLE:
    return arg.SizeInBytes;

  case KRNL_ARG_VECTOR:
  case KRNL_ARG_VECTOR_BY_REF:
    {
      // Extract the vector element size and the number of vector elements
      unsigned int elemSize = arg.SizeInBytes >> 16;
      unsigned int numElements = (arg.SizeInBytes) & 0xFFFF;
      return elemSize * numElements;
    }

  case KRNL_ARG_SAMPLER:
    return sizeof(cl_int);

  case KRNL_ARG_COMPOSITE:
    return arg.SizeInBytes;

  case KRNL_ARG_PTR_LOCAL:
  case KRNL_ARG_PTR_GLOBAL:
  case KRNL_ARG_PTR_CONST:
  case KRNL_ARG_PTR_IMG_1D:
  case KRNL_ARG_PTR_IMG_1D_ARR:
  case KRNL_ARG_PTR_IMG_1D_BUF:
  case KRNL_ARG_PTR_IMG_2D:
  case KRNL_ARG_PTR_IMG_2D_DEPTH:
  case KRNL_ARG_PTR_IMG_3D:
  case KRNL_ARG_PTR_IMG_2D_ARR:
  case KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
  case KRNL_ARG_PTR_BLOCK_LITERAL:
  case KRNL_ARG_PTR_QUEUE_T:
  case KRNL_ARG_PTR_SAMPLER_T:
  case KRNL_ARG_PTR_PIPE_T:
  case KRNL_ARG_PTR_CLK_EVENT_T:
    return arg.SizeInBytes;
  }

  // TODO : exception? assert?
  assert(false && "Unknown KernelArgument type");
  return 0;
}

size_t TypeAlignment::getAlignment(const KernelArgument& arg) {
  
  size_t alignment;
  switch(arg.Ty)
  {
  case KRNL_ARG_VECTOR:
  case KRNL_ARG_VECTOR_BY_REF:
    {
      size_t vectorAlignment = getSize(arg);
    
      unsigned int numElements = (arg.SizeInBytes) & 0xFFFF;
    
      // Vectors of 3 elements need to be aligned to a 4-elements vector
      if (numElements == 3) {
        // Align num elems to 4 elements by adding elemSize
        unsigned int elemSize = arg.SizeInBytes >> 16;
        vectorAlignment += elemSize;
      }
    
      alignment = vectorAlignment;
      // Adding assert to check we are following the OpenCL spec:
      // A built-in data type that is not a power of two bytes in size must be
      // aligned to the next larger power of two
      assert((0 == (alignment & (alignment - 1))) && "Alignment is not power of 2!");
    }
    break;
  case KRNL_ARG_PTR_BLOCK_LITERAL:
  case KRNL_ARG_COMPOSITE:
    // No alignment for structures
    alignment = 0;
    break;
  default:
    alignment = getSize(arg);
    break;
  }
  
  return alignment;
}

size_t TypeAlignment::align(size_t alignment, size_t offset) {
  
  // Check if there is a need to align
  if (alignment == 0) {
    return offset;
  }
  
  return (offset + alignment - 1) & ~(alignment - 1);
}

char* TypeAlignment::align(size_t alignment, const char* ptr) {
  return reinterpret_cast<char *>(align(alignment, reinterpret_cast<size_t>(ptr)));
}

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

