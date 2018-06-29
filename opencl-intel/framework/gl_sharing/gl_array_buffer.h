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

#include "gl_mem_objects.h"
#include "gl_context.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\glext.h>
#include <cl\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

    class GLArrayBuffer : public GLMemoryObject
    {
    public:

        PREPARE_SHARED_PTR(GLArrayBuffer)

        static SharedPtr<GLArrayBuffer> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
        {
            return SharedPtr<GLArrayBuffer>(new GLArrayBuffer(pContext, clObjType));
        }

        cl_err_code AcquireGLObject();
        cl_err_code ReleaseGLObject();

        // MemoryObject Interface
        cl_err_code Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count,
            const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags creation_flags);

        cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
            const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer);

        size_t GetPixelSize() const {return 1;}
        // Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
        size_t GetRowPitchSize() const { return 0;};
        size_t GetSlicePitchSize() const  { return 0;};

        cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

        cl_err_code GetDimensionSizes(size_t* pszRegion) const;

        virtual void GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const;

    protected:

        GLArrayBuffer(SharedPtr<Context> pContext, cl_gl_object_type clglObjType);

        cl_err_code CreateChildObject() { assert(false && "shouldn't be called"); return CL_INVALID_OPERATION; }
        // do not implement
        GLArrayBuffer(const GLArrayBuffer&);
        GLArrayBuffer& operator=(const GLArrayBuffer&);

        GLenum m_glBufferTarget;
        GLenum m_glBufferTargetBinding;
    };

}}}
