/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  TypeAlignment.cpp

\*****************************************************************************/

#include "TypeAlignment.h"

#include "cl_types.h"

#include <cassert>


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
	  return sizeof(void*);
	
	case CL_KRNL_ARG_PTR_GLOBAL:
	  return sizeof(void*);
	
	case CL_KRNL_ARG_PTR_CONST:
	  return sizeof(void*);
	
	case CL_KRNL_ARG_PTR_IMG_2D:
	  return sizeof(void*);
	
	case CL_KRNL_ARG_PTR_IMG_3D:
	  return sizeof(void*);
  
  case CL_KRNL_ARG_PTR_IMG_2D_ARR:
    return sizeof(void*);
  
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
    
  case CL_KRNL_ARG_COMPOSITE:
    {
      // No alignment for structures
	    alignment = 0;
	  }
	  
	default:
    alignment = getSize(arg);
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

char* TypeAlignment::align(size_t alignment, char* ptr) {
  return reinterpret_cast<char *>(align(alignment, reinterpret_cast<size_t>(ptr)));
}

}}}
