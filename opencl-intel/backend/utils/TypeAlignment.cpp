/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "TypeAlignment.h"
#include "CL/cl.h"

#include <assert.h>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

size_t TypeAlignment::getSize(const cl_kernel_argument& arg) {
  switch(arg.type)
  {
  case CL_KRNL_ARG_INT:
    return arg.size_in_bytes;

  case CL_KRNL_ARG_UINT:
    return arg.size_in_bytes;

  case CL_KRNL_ARG_FLOAT:
    return arg.size_in_bytes;

  case CL_KRNL_ARG_DOUBLE:
    return arg.size_in_bytes;

  case CL_KRNL_ARG_VECTOR:
  case CL_KRNL_ARG_VECTOR_BY_REF:
    {
      // Extract the vector element size and the number of vector elements
      unsigned int elemSize = arg.size_in_bytes >> 16;
      unsigned int numElements = (arg.size_in_bytes) & 0xFFFF;
      return elemSize * numElements;
    }

  case CL_KRNL_ARG_SAMPLER:
    return sizeof(cl_int);

  case CL_KRNL_ARG_COMPOSITE:
    return arg.size_in_bytes;

  case CL_KRNL_ARG_PTR_LOCAL:
  case CL_KRNL_ARG_PTR_GLOBAL:
  case CL_KRNL_ARG_PTR_CONST:
  case CL_KRNL_ARG_PTR_IMG_1D:
  case CL_KRNL_ARG_PTR_IMG_1D_ARR:
  case CL_KRNL_ARG_PTR_IMG_1D_BUF:
  case CL_KRNL_ARG_PTR_IMG_2D:
  case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
  case CL_KRNL_ARG_PTR_IMG_3D:
  case CL_KRNL_ARG_PTR_IMG_2D_ARR:
  case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
  case CL_KRNL_ARG_PTR_BLOCK_LITERAL:
  case CL_KRNL_ARG_PTR_QUEUE_T:
  case CL_KRNL_ARG_PTR_SAMPLER_T:
  case CL_KRNL_ARG_PTR_PIPE_T:
  case CL_KRNL_ARG_PTR_CLK_EVENT_T:
    return arg.size_in_bytes;
  
  default:
  // TODO : exception? assert?
    assert(0 && "Unknown cl_kernel_argument type");
    return 0;
  }
}

size_t TypeAlignment::getAlignment(const cl_kernel_argument& arg) {
  
  size_t alignment;
  switch(arg.type)
  {
  case CL_KRNL_ARG_VECTOR:
  case CL_KRNL_ARG_VECTOR_BY_REF:
    {
      size_t vectorAlignment = getSize(arg);
    
      unsigned int numElements = (arg.size_in_bytes) & 0xFFFF;
    
      // Vectors of 3 elements need to be aligned to a 4-elements vector
      if (numElements == 3) {
        // Align num elems to 4 elements by adding elemSize
        unsigned int elemSize = arg.size_in_bytes >> 16;
        vectorAlignment += elemSize;
      }
    
      alignment = vectorAlignment;
      // Adding assert to check we are following the OpenCL spec:
      // A built-in data type that is not a power of two bytes in size must be
      // aligned to the next larger power of two
      assert((0 == (alignment & (alignment - 1))) && "Alignment is not power of 2!");
    }
    break;
  case CL_KRNL_ARG_PTR_BLOCK_LITERAL:
  case CL_KRNL_ARG_COMPOSITE:
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

