// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "GenericImageArray.h"
#include "MemoryAllocator/GenericMemObj.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "cl_shared_ptr.hpp"
#include "pipe.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::Framework;

typedef GenericImageArray<1> Image1DArray;
typedef GenericImageArray<2> Image2DArray;

// CL_DEVICE_TYPE_ALL matches any device/device group, and may be overridden by
// specific setting

REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_BUFFER, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE1D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE1D_BUFFER, 0,
                               GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE1D_BUFFER, 1,
                               GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE2D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE2D, 1, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE3D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE1D_ARRAY, 0, Image1DArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_IMAGE2D_ARRAY, 0, Image2DArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE,
                               CL_MEM_OBJECT_PIPE, 0, Pipe)
