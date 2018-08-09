// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include "MemoryAllocator/GraphicsApiMemoryObject.h"
#include "gl_context.h"
#include "gl_shr_utils.h"
#include <cl_synch_objects.h>

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class GLContext;

	class GLMemoryObject : public GraphicsApiMemoryObject
	{
	public:

        PREPARE_SHARED_PTR(GLMemoryObject)

		virtual cl_err_code AcquireGLObject() = 0;
		virtual cl_err_code ReleaseGLObject() = 0;
		
		cl_gl_object_type GetObjectType() const { return m_clglObjectType;}

		cl_err_code GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName);
		virtual cl_err_code GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize);
		// Memory Object interface
		cl_err_code			RelinquishDeviceHandle(SharedPtr<FissionableDevice> pDevice, cl_dev_memobj_handle handle);

		cl_err_code ReadData(	void *          pOutData, 
			const size_t *  pszOrigin, 
			const size_t *  pszRegion,
			size_t          szRowPitch   = 0,
			size_t          szSlicePitch = 0);

		cl_err_code WriteData(	const void *    pOutData, 
			const size_t *  pszOrigin, 
			const size_t *  pszRegion,
			size_t          szRowPitch   = 0,
			size_t          szSlicePitch = 0);
		size_t GetSize() const {return m_stMemObjSize;}

		// Texture desription structure. This structure is used to pass parameters of the user texture.
		// A pointer to this structure is passed to Initialize() method via pHostPtr parameter
		struct GLTextureDescriptor
		{
			GLenum	glTextureTarget;
			GLint	glMipLevel;
			GLuint	glTexture;
		};

	protected:
        GLMemoryObject(SharedPtr<Context> pContext, cl_gl_object_type lglObjectType);

		cl_err_code	SetGLMemFlags();
		
		GLuint		m_glObjHandle;
		GLuint		m_glMemFlags;
		SharedPtr<GLContext>	m_pGLContext;

		cl_gl_object_type m_clglObjectType;
	};

	class GLTexture : public GLMemoryObject
	{
	public:

          PREPARE_SHARED_PTR(GLTexture)

          ~GLTexture();

		  cl_err_code GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize);

		// MemoryObject Interface
		cl_err_code Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
			const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags );

		cl_err_code AcquireGLObject();
		cl_err_code ReleaseGLObject();

		size_t GetPixelSize() const { return m_stElementSize;}

		// Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
		size_t GetRowPitchSize() const { return m_stPitches[0]; }
		size_t GetSlicePitchSize() const { return m_stPitches[1]; }

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer) {return CL_INVALID_OPERATION;}

        cl_err_code	GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

        cl_err_code GetDimensionSizes( size_t* pszRegion ) const;

		virtual GLint CalculateTextureDimensions() = 0;

        virtual void GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const;

	protected:
		GLTexture(SharedPtr<Context> pContext, cl_gl_object_type clglObjType):
		GLMemoryObject(pContext, clglObjType),  m_glFramebuffer(0), m_glPBO(0) {}

		// Virtual function required for appropriate handling of 1D and 2D textures
		virtual void BindFramebuffer2Texture() = 0;
		virtual void TexSubImage() = 0;

		cl_err_code CreateChildObject();

		GLTextureDescriptor	m_txtDescriptor;
		cl_image_format_ext m_clFormat;
		size_t				m_stElementSize;

		size_t				m_stDimensions[3];
		size_t				m_stPitches[2];

		GLenum	m_glReadBackFormat;
		GLenum	m_glReadBackType;

		GLint	m_glInternalFormat;
		GLint	m_glBorder;
		GLuint	m_glFramebuffer;
		GLuint	m_glPBO;

	};

}}}
