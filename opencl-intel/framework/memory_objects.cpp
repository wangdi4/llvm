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

#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/GenericMemObj.h"

#if defined (_WIN32)
#include "gl_sharing/gl_array_buffer.h"
#include "gl_sharing/gl_texture_buffer.h"
#include "gl_sharing/gl_texture1D.h"
#include "gl_sharing/gl_texture2D.h"
#include "gl_sharing/gl_texture3D.h"
#include "gl_sharing/gl_render_buffer.h"
#include "gl_sharing/gl_texture_array.h"

#if defined (DX_MEDIA_SHARING)
#include "d3d9_sharing/d3d9_resource.h"
#include "d3d9_context.hpp"
#endif
#endif
#include "GenericImageArray.h"
#include "pipe.h"
#include "cl_shared_ptr.hpp"

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::Framework;

typedef GenericImageArray<1> Image1DArray;
typedef GenericImageArray<2> Image2DArray;

// CL_DEVICE_TYPE_ALL matches any device/device group, and may be overridden by specific setting 

REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_BUFFER, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE1D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE1D_BUFFER, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE1D_BUFFER, 1, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D, 1, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE3D, 0, GenericMemObject)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE1D_ARRAY, 0, Image1DArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D_ARRAY, 0, Image2DArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_ALL, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_PIPE, 0, Pipe)

#if defined (_WIN32)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_BUFFER, 0, GLArrayBuffer)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE1D, 0, GLTexture1D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE_BUFFER, 0, GLTextureBuffer)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D, 0, GLTexture2D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE3D, 0, GLTexture3D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE1D_ARRAY, 0, GLTextureArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D_ARRAY, 0, GLTextureArray)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_RENDERBUFFER, 0, GLRenderBuffer)

#if defined (DX_MEDIA_SHARING)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_DX9, CL_DX9_OBJECT_SURFACE, 0, D3D9Surface);
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_DX11, CL_D3D11_OBJECT_BUFFER, 0, D3D11Buffer);
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_DX11, CL_D3D11_OBJECT_TEXTURE2D, 0, D3D11Texture2D);
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_DX11, CL_D3D11_OBJECT_TEXTURE3D, 0, D3D11Texture3D);
#endif
#endif
