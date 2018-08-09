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

#include "cl_types.h"

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\gl.h>

#include <cl\cl.h>

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

cl_int ParseGLContextOptions(const cl_context_properties * properties, cl_context_properties *hGL,  cl_context_properties *hDC,
                             bool* pbGLSharingSupported = nullptr);


}}}
