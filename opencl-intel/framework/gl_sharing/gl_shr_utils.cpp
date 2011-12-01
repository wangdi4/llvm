// Copyright (c) 2006-2010 Intel Corporation
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

cl_int Intel::OpenCL::Framework::ParseGLContextOptions(const cl_context_properties * properties, cl_context_properties *hGL,  cl_context_properties *hDC,
                                                       bool* pbGLSharingSupported)
{
    if (NULL != pbGLSharingSupported)
    {
        *pbGLSharingSupported = false;
    }    
	*hGL = NULL;
	*hDC = NULL;
	if ( NULL == properties)
	{
		return CL_SUCCESS;
	}
	while (NULL != *properties)
	{
		switch (*properties)
		{
		case CL_GL_CONTEXT_KHR:
			*hGL = *(properties+1);
            if (NULL != pbGLSharingSupported)
            {
                *pbGLSharingSupported = true;
            }            
			break;
		case  CL_WGL_HDC_KHR:
			*hDC = *(properties+1);
            if (NULL != pbGLSharingSupported)
            {
                *pbGLSharingSupported = true;
            }            
			break;
		}
		properties+=2;
	}
	return CL_SUCCESS;
}

