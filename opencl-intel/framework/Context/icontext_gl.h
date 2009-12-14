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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  icontext_gl.h
//  Implementation of the IContextGL interface
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_ICONTEXT_GL_H_)
#define OCL_ICONTEXT_GL_H_

#include <cl_types.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\GLU.h>
#include <CL\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	IContextGL
	*
	* Description:	IContextGl iterface - outlines the context OpneCL Sharing Memory Objects with 
	*				OpenGL / OpenGL ES Buffer, Texture and Renderbuffer Object srelated functions
	* Author:		Uri Levy
	* Date:			June 2009
	**********************************************************************************************/
	class IContextGL
	{
	public:
        virtual ~IContextGL(){};

		/******************************************************************************************
		* Function: 	CreateFromGLBuffer    
		* Description:	creates an OpenCL buffer object from an OpenGL buffer object
		* Arguments:	clContext [in] -	a valid OpenCL context created from an OpenGL context
		*				clMemFlags [in] -	is a bit-field that is used to specify usage 
		*									information. Only CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY 
		*									and CL_MEM_READ_WRITE values can be used
		*				glBufObj [in] -		is the name of a GL buffer object. The data store of 
		*									the GL buffer object must have have been previously 
		*									created by calling glBufferData, although its contents 
		*									need not be initialized. The size of the data store 
		*									will be used to determine the size of the CL buffer 
		*									object
		*				pErrcodeRet [in] -	will return an appropriate error code as described 
		*									below. If errcode_ret is NULL, no error code is 
		*									returned
		* Return value:	valid non-zero OpenCL buffer object and pErrcodeRet is set to CL_SUCCESS 
		*				if the buffer object is created successfully. Otherwise, it returns a 
		*				NULL value with one of the following error values:
		*				CL_INVALID_CONTEXT		if context is not a valid context or was not 
		*										created from a GL context
		*				CL_INVALID_VALUE		if values specified in clMemFlags are not valid
		*				CL_INVALID_GL_OBJECT	if glBufObj is not a GL buffer object or is a GL 
		*										buffer object but does not have an existing data 
		*										store
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_mem CreateFromGLBuffer(	cl_context    IN clContext,
											cl_mem_flags  IN clMemFlags,
											GLuint        IN glBufObj,
											int *        OUT pErrcodeRet) = 0;

		/******************************************************************************************
		* Function: 	CreateFromGLTexture2D    
		* Description:	creates an OpenCL 2D image object from an OpenGL 2D texture object, or a 
		*				single face of an OpenGL cubemap texture object..
		* Arguments:	clContext [in] -	a valid OpenCL context created from an OpenGL context
		*				clMemFlags [in] -	is a bit-field that is used to specify usage 
		*									information. Only CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY 
		*									and CL_MEM_READ_WRITE values can be used
		*				glTextureTarget [in] -	must be one of GL_TEXTURE_2D, 
		*									GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
		*									GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
		*									GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		*									GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
		*									GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		*									GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, or 
		*									GL_TEXTURE_RECTANGLE47. texture_target is used only to 
		*									define the image type of texture. No reference to a 
		*									bound GL texture object is made or implied by this 
		*									parameter
		*				glMipLevel [in] -	is the mipmap level to be used
		*				glTexture [in] -	is the name of a GL 2D, cubemap or rectangle texture 
		*									object. The texture object must be a complete texture 
		*									as per OpenGL rules on texture completeness. The 
		*									texture format and dimensions defined by OpenGL for 
		*									the specified miplevel of the texture will be used to 
		*									create the 2D image object. Only GL texture objects 
		*									with an internal format that maps to appropriate image
		*									channel order and data type specified in tables 5.4 
		*									and 5.5 may be used to create a 2D image object.
		*				pErrcodeRet [in] -	will return an appropriate error code as described 
		*									below. If errcode_ret is NULL, no error code is 
		*									returned
		* Return value:	a valid non-zero OpenCL image object and pErrcodeRet is set to CL_SUCCESS
		*				if the image object is created successfully. Otherwise, it returns a NULL
		*				value with one of the following error values
		*				CL_INVALID_CONTEXT		if context is not a valid context or was not 
		*										created from a GL context
		*				CL_INVALID_VALUE		if values specified in clMemFlags are not valid 
		*										or if value specified in glTarget is not one of 
		*										the values specified in the description of 
		*										glTexture
		*				CL_INVALID_MIPLEVEL		if glMipLevel is less than the value of levelbase
		*										(for OpenGL implementations) or zero (for OpenGL
		*										ES implementations); or greater than the value of
		*										q (for both OpenGL and OpenGL ES). levelbase and q
		*										are defined for the texture in section 3.8.10 
		*										(Texture Completeness) of the OpenGL 2.1 
		*										specification and section 3.7.10 of the OpenGL ES 
		*										2.0
		*				CL_INVALID_MIPLEVEL		if glMipLevel is greather than zero and the OpenGL
		*										implementation does not support creating from 
		*										non-zero mipmap levels
		*				CL_INVALID_GL_OBJECT	if glTexture is not a GL texture object whose type
		*										matches texture_target, if the specified miplevel 
		*										of texture is not defined, or if the width or 
		*										height of the specified miplevel is zero
		*				CL_INVALID_IMAGE_FORMAT_DESCRIPTOR	if the OpenGL texture internal format
		*													does not map to a supported OpenCL 
		*													image format
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_mem CreateFromGLTexture2D(	cl_context    IN clContext,
												cl_mem_flags  IN clMemFlags,
												GLenum        IN glTextureTarget,
												GLint         IN glMipLevel,
												GLuint        IN glTexture,
												cl_int *     OUT pErrcodeRet) = 0;

		/******************************************************************************************
		* Function: 	CreateFromGLTexture3D    
		* Description:	creates an OpenCL 3D image object from an OpenGL 3D texture object
		* Arguments:	clContext [in] -	a valid OpenCL context created from an OpenGL context
		*				clMemFlags [in] -	is a bit-field that is used to specify usage 
		*									information. Only CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY 
		*									and CL_MEM_READ_WRITE values can be used
		*				glTextureTarget [in] -	must be GL_TEXTURE_3D. glTextureTarget is used 
		*									only to define the image type of texture. No reference
		*									to a bound GL texture object is made or implied by this
		*									parameter
		*				glMipLevel [in] -	is the mipmap level to be used
		*				glTexture [in] -	is the name of a GL 3D texture object. The texture 
		*									object must be a complete texture as per OpenGL rules
		*									on texture completeness. The texture format and 
		*									dimensions defined by OpenGL for the specified miplevel
		*									of the texture will be used to create the 3D image 
		*									object. Only GL texture objects with an internal format
		*									that maps to appropriate image channel order and data 
		*									type specified in tables 5.4 and 5.5 can be used to 
		*									create the 3D image object
		*				pErrcodeRet [in] -	will return an appropriate error code as described 
		*									below. If errcode_ret is NULL, no error code is 
		*									returned
		* Return value:	a valid non-zero OpenCL image object and pErrcodeRet is set to CL_SUCCESS
		*				if the image object is created successfully. Otherwise, it returns a NULL
		*				value with one of the following error values
		*				CL_INVALID_CONTEXT		if context is not a valid context or was not 
		*										created from a GL context
		*				CL_INVALID_VALUE		if values specified in clMemFlags are not valid 
		*										or if value specified in glTarget is not one of 
		*										the values specified in the description of 
		*										glTexture
		*				CL_INVALID_MIPLEVEL		if glMipLevel is less than the value of levelbase
		*										(for OpenGL implementations) or zero (for OpenGL
		*										ES implementations); or greater than the value of
		*										q (for both OpenGL and OpenGL ES). levelbase and q
		*										are defined for the texture in section 3.8.10 
		*										(Texture Completeness) of the OpenGL 2.1 
		*										specification and section 3.7.10 of the OpenGL ES 
		*										2.0
		*				CL_INVALID_MIPLEVEL		if glMipLevel is greather than zero and the OpenGL
		*										implementation does not support creating from 
		*										non-zero mipmap levels
		*				CL_INVALID_GL_OBJECT	if texture is not a GL texture object whose type 
		*										matches texture_target, if the specified miplevel 
		*										of texture is not defined, or if the width, height
		*										or depth of the specified miplevel is zero
		*				CL_INVALID_IMAGE_FORMAT_DESCRIPTOR	if the OpenGL texture internal format
		*													does not map to a supported OpenCL 
		*													image format
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_mem CreateFromGLTexture3D(	cl_context    IN clContext,
												cl_mem_flags  IN clMemFlags,
												GLenum        IN glTextureTarget,
												GLint         IN glMipLevel,
												GLuint        IN glTexture,
												cl_int *     OUT pErrcodeRet) = 0;

		/******************************************************************************************
		* Function: 	CreateFromGLRenderbuffer    
		* Description:	creates an OpenCL 2D image object from an OpenGL renderbuffer object
		* Arguments:	clContext [in] -	a valid OpenCL context created from an OpenGL context
		*				clMemFlags [in] -	is a bit-field that is used to specify usage 
		*									information. Only CL_MEM_READ_ONLY, CL_MEM_WRITE_ONLY 
		*									and CL_MEM_READ_WRITE values can be used
		*				glRenderBuffer [in] -	is the name of a GL renderbuffer object. The 
		*									renderbuffer storage must be specified before the 
		*									image object can be created. The renderbuffer format 
		*									and dimensions defined by OpenGL will be used to create
		*									the 2D image object. Only GL renderbuffers with 
		*									internal formats that maps to appropriate image channel
		*									order and data type  can be used to create the 2D image
		*									object
		*				pErrcodeRet [in] -	will return an appropriate error code as described 
		*									below. If errcode_ret is NULL, no error code is 
		*									returned
		* Return value:	a valid non-zero OpenCL image object and pErrcodeRet is set to CL_SUCCESS
		*				if the image object is created successfully. Otherwise, it returns a NULL
		*				value with one of the following error values:
		*				CL_INVALID_CONTEXT		if context is not a valid context or was not 
		*										created from a GL context
		*				CL_INVALID_VALUE		if values specified in clMemFlags are not valid
		*				CL_INVALID_GL_OBJECT	if glRenderBuffer is not a GL renderbuffer object 
		*										or if the width or height of glRenderBuffer is zero
		*				CL_INVALID_IMAGE_FORMAT_DESCRIPTOR if the OpenGL renderbuffer internal 
		*													format does not map to a supported 
		*													OpenCL image format
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_mem CreateFromGLRenderbuffer(cl_context    IN clContext,
												cl_mem_flags  IN clMemFlags,
												GLuint        IN glRenderBuffer,
												cl_int *     OUT pErrcodeRet) = 0;

		/******************************************************************************************
		* Function: 	GetGLObjectInfo    
		* Description:	The OpenGL object used to create the OpenCL memory object and information 
		*				about the object type i.e. whether it is a texture, renderbuffer or buffer 
		*				object can be queried using the following function
		* Arguments:	clMemObj [in] -		a valid OpenCL mem object
		*				pglObjectType [in]-	returns the type of GL object attached to memobj and 
		*									can be CL_GL_OBJECT_BUFFER, CL_GL_OBJECT_TEXTURE2D, 
		*									CL_GL_OBJECT_TEXTURE3D, or CL_GL_OBJECT_RENDERBUFFER.
		*									If pglObjectType is NULL, it is ignored
		*				pglObjectName [in]-	returns the GL object name used to create memobj. 
		*									If pglObjectName is NULL, it is ignored
		* Return value: CL_SUCCESS				if the call was executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is not a valid OpenCL memory object
		*				CL_INVALID_GL_OBJECT	if there is no GL object associated with clMemObj
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_int GetGLObjectInfo(	cl_mem               IN clMemObj,
										cl_gl_object_type * OUT pglObjectType,
										GLuint *            OUT pglObjectName) = 0;

		/******************************************************************************************
		* Function: 	GetGLTextureInfo    
		* Description:	returns additional information about the GL texture object associated with 
		*				clMemObj
		* Arguments:	clMemObj [in] -				a valid OpenCL mem object
		*				clglPramName [in]-			pecifies what additional information about the 
		*											GL texture object associated with clMemObj to 
		*											query
		*				pParamValue [in] -			is a pointer to memory where the result being 
		*											queried is returned. If pParamValue is NULL, 
		*											it is ignored
		*				szParamValueSize [out] -	is used to specify the size in bytes of memory 
		*											pointed to by pParamValue. This size must be >= 
		*											size of return type			
		*				pszParamValueSizeRet [out]-	returns the actual size in bytes of data copied
		*											to pParamValue. If pszParamValueSizeRet is NULL, 
		*											it is ignored
		* Return value: CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_MEM_OBJECT	if clMemObj is not a valid OpenCL memory object
		*				CL_INVALID_GL_OBJECT	if there is no GL object associated with clMemObj
		*				CL_INVALID_VALUE		if clglPramName is not valid, or if size in bytes 
		*										specified by szParamValueSize is < size of return 
		*										type and pParamValue is not NULL, or if pParamValue
		*										and pszParamValueSizeRet are NULL
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_int GetGLTextureInfo(cl_mem              IN clMemObj,
										cl_gl_texture_info  IN clglPramName,
										size_t              IN szParamValueSize,
										void *             OUT pParamValue,
										size_t *           OUT pszParamValueSizeRet) = 0;

	};

}}};
#endif // !defined(OCL_ICONTEXT_GL_H_)
