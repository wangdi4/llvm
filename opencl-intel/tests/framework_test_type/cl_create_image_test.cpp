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

#include <iostream>
#include "CL/cl_platform.h"
#include "CL/cl.h"
#include "test_utils.h"

bool clCreateImageTest()
{
    cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_context context = NULL;
    cl_image_format clFormat = {0};
    cl_image_desc clImageDesc = {0};
    cl_mem clImg2D = NULL, clImg3D = NULL;

    std::cout << "=============================================================" << std::endl;
    std::cout << "clCreateImageTest" << std::endl;
    std::cout << "=============================================================" << std::endl;

    iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return bResult;
    }

    const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    
    context = clCreateContextFromType(prop, CL_DEVICE_TYPE_CPU, NULL, NULL, &iRet);    
    bResult &= Check(L"clCreateContextFromType", CL_SUCCESS, iRet);    
    if (!bResult)
    {
        goto end;
    }
    clFormat.image_channel_order = CL_RGBA;
    clFormat.image_channel_data_type = CL_UNSIGNED_INT8;
    clImageDesc.image_width = 100;
    clImageDesc.image_height = 100;
    clImageDesc.image_depth = 4;

    // 2D image
    clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    clImg2D = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);    
    bResult &= Check(L"clCreateImage", CL_SUCCESS, iRet);

    // 3D image 
    clImageDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    clImg3D = clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
    bResult &= Check(L"clCreateImage", CL_SUCCESS, iRet);

    // NULL image_desc
    clCreateImage(context, 0, &clFormat, NULL, NULL, &iRet);
    bResult &= Check(L"clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

    // invalid value in image_desc
    clImageDesc.image_width = 0;
    clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
    bResult &= Check(L"clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);
    clImageDesc.image_width = 100;

    // unsupported image type
    clImageDesc.image_type = 0xffff;
    clCreateImage(context, 0, &clFormat, &clImageDesc, NULL, &iRet);
    bResult &= Check(L"clCreateImage", CL_INVALID_IMAGE_DESCRIPTOR, iRet);

end:
    if (clImg2D)
    {
        clReleaseMemObject(clImg2D);
    }
    if (clImg3D)
    {
        clReleaseMemObject(clImg3D);
    }
    if (context)
    {
        clReleaseContext(context);
    }
    return bResult;
}
