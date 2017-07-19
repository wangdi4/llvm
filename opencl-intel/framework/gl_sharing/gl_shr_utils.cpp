// Copyright (c) 2006-2012 Intel Corporation
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

#include "gl_shr_utils.h"

#include <cl\cl.h>
#include <cl\cl_gl.h>
#include <gl\glext.h>

using namespace Intel::OpenCL::Framework;

cl_int Intel::OpenCL::Framework::ParseGLContextOptions(const cl_context_properties * properties, cl_context_properties *hGL,  cl_context_properties *hDC,
                                                       bool* pbGLSharingSupported)
{
    if (nullptr != pbGLSharingSupported)
    {
        *pbGLSharingSupported = false;
    }    
	*hGL = nullptr;
	*hDC = nullptr;
	if ( nullptr == properties)
	{
		return CL_SUCCESS;
	}
	while (nullptr != *properties)
	{
		switch (*properties)
		{
		case CL_GL_CONTEXT_KHR:
			*hGL = *(properties+1);
            if (nullptr != pbGLSharingSupported)
            {
                *pbGLSharingSupported = true;
            }            
			break;
		case  CL_WGL_HDC_KHR:
			*hDC = *(properties+1);
            if (nullptr != pbGLSharingSupported)
            {
                *pbGLSharingSupported = true;
            }            
			break;
		}
		properties+=2;
	}
	return CL_SUCCESS;
}

struct fmt_cvt
{
	cl_image_format_ext		clType;
	GLuint					glInternalType;
};

fmt_cvt formatConvert[] =
{
	{{{CL_RGBA, CL_UNORM_INT8}, false}, GL_RGBA},
	{{{CL_RGBA, CL_UNORM_INT8}, false}, GL_RGBA8},
	{{{CL_RGBA, CL_UNORM_INT16}, false}, GL_RGBA16},
	{{{CL_RGBA, CL_SIGNED_INT8}, true}, GL_RGBA8I},
	{{{CL_RGBA, CL_SIGNED_INT16}, true}, GL_RGBA16I},
	{{{CL_RGBA, CL_SIGNED_INT32}, true}, GL_RGBA32I},
	{{{CL_RGBA, CL_UNSIGNED_INT8}, true}, GL_RGBA8UI},
	{{{CL_RGBA, CL_UNSIGNED_INT16}, true}, GL_RGBA16UI},
	{{{CL_RGBA, CL_UNSIGNED_INT32}, true}, GL_RGBA32UI},
	{{{CL_RGBA, CL_HALF_FLOAT}, true}, GL_RGBA16F},
	{{{CL_RGBA, CL_FLOAT}, false}, GL_RGBA32F},
	{{{0,0},false}, 0}
};

cl_image_format_ext Intel::OpenCL::Framework::ImageFrmtConvertGL2CL(GLuint glFrmt)
{
	unsigned int i=0;
	while (formatConvert[i].glInternalType != 0)
	{
		if (glFrmt == formatConvert[i].glInternalType)
		{
			return formatConvert[i].clType;
		}
		++i;
	}
	return formatConvert[i].clType;
}

GLuint Intel::OpenCL::Framework::ImageFrmtConvertCL2GL(cl_image_format clFrmt)
{
	unsigned int i=0;
	while (formatConvert[i].glInternalType != 0)
	{
		if ( (clFrmt.image_channel_order == formatConvert[i].clType.clType.image_channel_order) &&
			 (clFrmt.image_channel_data_type == formatConvert[i].clType.clType.image_channel_data_type) )
		{
			return formatConvert[i].glInternalType;
		}
		++i;
	}
	return formatConvert[i].glInternalType;
}

GLenum Intel::OpenCL::Framework::GetTargetBinding( GLenum target )
{
	switch( target )
	{
	case GL_ARRAY_BUFFER:
		return GL_ARRAY_BUFFER_BINDING;
	case GL_TEXTURE_1D:
		return GL_TEXTURE_BINDING_1D;
	case GL_TEXTURE_BUFFER:
		return GL_TEXTURE_BINDING_BUFFER;
	case GL_TEXTURE_2D:
		return GL_TEXTURE_BINDING_2D;
	case GL_TEXTURE_1D_ARRAY:
		return GL_TEXTURE_BINDING_1D_ARRAY;
	case GL_TEXTURE_3D:
		return GL_TEXTURE_BINDING_3D;
	case GL_TEXTURE_2D_ARRAY:
		return GL_TEXTURE_BINDING_2D_ARRAY;
	case GL_TEXTURE_RECTANGLE:
		return GL_TEXTURE_BINDING_RECTANGLE;
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	
		return GL_TEXTURE_BINDING_CUBE_MAP;
	default:
		return target;
	}
}

GLenum Intel::OpenCL::Framework::GetBaseTarget( GLenum target )
{
	switch( target )
	{
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:	
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:	
		return GL_TEXTURE_CUBE_MAP;
	default:
		return target;
	}
}

GLenum Intel::OpenCL::Framework::GetGLType(cl_channel_type clType)
{
	switch (clType)
	{
	case CL_UNORM_INT8:
		return GL_UNSIGNED_BYTE;
	case CL_UNORM_INT16:
		return GL_UNSIGNED_SHORT;
	case CL_SIGNED_INT8:
		return GL_BYTE;
	case CL_SIGNED_INT16:
		return GL_SHORT;
	case CL_SIGNED_INT32:
		return GL_INT;
	case CL_UNSIGNED_INT8:
		return GL_UNSIGNED_BYTE;
	case CL_UNSIGNED_INT16:
		return GL_UNSIGNED_SHORT;
	case CL_UNSIGNED_INT32:
		return GL_UNSIGNED_INT;
	case CL_FLOAT:
		return GL_FLOAT;
	case CL_HALF_FLOAT:
		return GL_HALF_FLOAT;
	}
	return 0;
}

GLenum Intel::OpenCL::Framework::GetGLFormat(cl_channel_type clType, bool isExt)
{
	if ( isExt && (CL_HALF_FLOAT != clType) )
	{
		return  GL_RGBA_INTEGER_EXT;
	}
	else
	{
		return GL_RGBA;
	}

	return 0;
}
