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

#include "MemoryAllocator/MemoryObject.h"
#include "gl_context.h"

#include <cl_synch_objects.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	struct cl_image_format_ext
	{
		cl_image_format clType;	// Original CL format
		bool			isGLExt; // is true if GL extended format
	};

	cl_image_format_ext ImageFrmtConvertGL2CL(GLuint glFrmt);
	GLuint ImageFrmtConvertCL2GL(cl_image_format clFrmt);
	GLenum GetTargetBinding( GLenum target );
	GLenum GetBaseTarget( GLenum target );
	GLenum GetGLType(cl_channel_type clType);
	GLenum GetGLFormat(cl_channel_type clType, bool isExt);

	class GLMemoryObject : public MemoryObject
	{
	public:
		virtual cl_err_code AcquireGLObject() = 0;
		virtual cl_err_code ReleaseGLObject() = 0;
		virtual cl_gl_object_type GetObjectType() = 0;
		cl_err_code GetGLObjectInfo(cl_gl_object_type * pglObjectType, GLuint * pglObjectName);
		
		cl_err_code SetAcquireCmdEvent(OclEvent* pEvent); // Set Event of Acquire command that belongs to the object.

		// Memory Object interface
		cl_err_code			UpdateLocation(FissionableDevice* pDevice);
		cl_err_code			CreateDeviceResource(FissionableDevice* pDevice);
		cl_err_code			GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent);
		cl_err_code			RelinquishDeviceHandle(FissionableDevice* pDevice, cl_dev_memobj_handle handle);
		bool				IsSharedWith(FissionableDevice* pDevice);

		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
		cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void *GetBackingStore( const size_t * pszOrigin = NULL ) const;

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

		bool		IsSupportedByDevice(FissionableDevice* pDevice) {return true;}

		size_t GetSize() const {return m_stMemObjSize;}

		// Update the host pointer that is used for the memory object
		cl_err_code UpdateHostPtr(cl_mem_flags		clMemFlags,	void* pHostPtr) {return CL_SUCCESS;}

		// Low level mapped region creation function
		cl_err_code	MemObjCreateDevMappedRegion(const FissionableDevice*,
							cl_dev_cmd_param_map*	cmd_param_map);
		cl_err_code	MemObjReleaseDevMappedRegion(const FissionableDevice*,
				cl_dev_cmd_param_map*	cmd_param_map);

		// IDeviceFissionObserver interface
		cl_err_code NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children);

	protected:
		GLMemoryObject(Context * pContext, ocl_entry_points * pOclEntryPoints);
		virtual ~GLMemoryObject();

		// This function is reposiable for creating a supporting child object
		virtual cl_err_code CreateChildObject() = 0;

		cl_err_code	SetGLMemFlags();
		
		GLuint	m_glObjHandle;
		GLuint	m_glMemFlags;

		Intel::OpenCL::Utils::AtomicPointer<OclEvent>		m_pAcquireEvent;
		Intel::OpenCL::Utils::AtomicPointer<MemoryObject>	m_pChildObject;
		// This object is the actual object that handle interaction with devices
		Intel::OpenCL::Utils::OclMutex		m_muAcquireRelease;
		Intel::OpenCL::Utils::AtomicCounter					m_clAcqurieState;
	};

	class GLTexture : public GLMemoryObject
	{
	public:
		  cl_err_code GetGLTextureInfo(cl_gl_texture_info glTextInfo, size_t valSize, void* pVal, size_t* pRetSize);

		  // Texture desription structure. This structure is used to pass parameters of the user texture.
		  // A pointer to this structure is passed to Initialize() method via pHostPtr parameter
		  struct GLTextureDescriptor
		  {
			  GLenum	glTextureTarget;
			  GLint		glMipLevel;
			  GLuint	glTexture;
		  };

		size_t GetPixelSize() const { return m_szElementSize;}
		// Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
		size_t GetRowPitchSize() const { return m_szImageRowPitch; }

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer) {return CL_INVALID_OPERATION;}

        cl_err_code	GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

	protected:
		GLTexture(Context * pContext, ocl_entry_points * pOclEntryPoints):
		GLMemoryObject(pContext, pOclEntryPoints),  m_glFramebuffer(0), m_glPBO(0) {}
		~GLTexture();

		cl_err_code CreateChildObject();

		GLTextureDescriptor	m_txtDescriptor;
		cl_image_format_ext m_clFormat;
		size_t				m_szElementSize;

		size_t	m_szImageWidth;
		size_t	m_szImageHeight;
		size_t	m_szImageRowPitch;

		GLint	m_glInternalFormat;
		GLint	m_glBorder;
		GLuint	m_glFramebuffer;
		GLuint	m_glPBO;

	};

}}}