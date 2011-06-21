#include "stdafx.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/SingleUnifiedBuffer.h"
#include "MemoryAllocator/SingleUnifiedImage2D.h"
#include "MemoryAllocator/SingleUnifiedImage3D.h"
#include "MemoryAllocator/SingleUnifiedImage2DArray.h"
#if defined (_WIN32)
#include "gl_sharing/gl_buffer.h"
#include "gl_sharing/gl_texture2D.h"
#include "gl_sharing/gl_texture3D.h"
#include "gl_sharing/gl_render_buffer.h"
#endif

using namespace Intel::OpenCL::Framework;

REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_BUFFER, SingleUnifiedBuffer)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D, SingleUnifiedImage2D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE3D, SingleUnifiedImage3D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_NONE, CL_MEM_OBJECT_IMAGE2D_ARRAY, SingleUnifiedImage2DArray)

#if defined (_WIN32)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_BUFFER, GLBuffer)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE2D, GLTexture2D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_TEXTURE3D, GLTexture3D)
REGISTER_MEMORY_OBJECT_CREATOR(CL_DEVICE_TYPE_CPU, CL_MEMOBJ_GFX_SHARE_GL, CL_GL_OBJECT_RENDERBUFFER, GLRenderBuffer)
#endif