// Copyright (c) 2008-2012 Intel Corporation
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
#include "test_utils.h"

extern cl_device_type gDeviceType;

bool clSampler()
{
	  cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;

    std::cout << "=============================================================" << std::endl;
    std::cout << "clSampler" << std::endl;
    std::cout << "=============================================================" << std::endl;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);        
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };    
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
        CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

        cl_sampler_properties samplerProps[7] = {CL_SAMPLER_NORMALIZED_COORDS, CL_FALSE,
                                                 CL_SAMPLER_ADDRESSING_MODE, CL_ADDRESS_MIRRORED_REPEAT,
                                                 CL_SAMPLER_FILTER_MODE, CL_FILTER_LINEAR, 0};

        cl_sampler sampler = clCreateSamplerWithProperties(context, samplerProps, &iRet);

        // FPGA emulator doesn't support sampler
        if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
        {
            CheckException("clCreateSamplerWithProperties", CL_INVALID_OPERATION, iRet);
        }
        else
        {
            CheckException("clCreateSamplerWithProperties", CL_SUCCESS, iRet);
            iRet = clReleaseSampler(sampler);
            CheckException("clReleaseSampler", CL_SUCCESS, iRet);

            // NULL properties
            sampler = clCreateSamplerWithProperties(context, NULL, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_SUCCESS, iRet);
            iRet = clReleaseSampler(sampler);
            CheckException("clReleaseSampler", CL_SUCCESS, iRet);

            // wrong API:

            // the same name appears twice
            samplerProps[2] = CL_SAMPLER_NORMALIZED_COORDS;
            clCreateSamplerWithProperties(context, samplerProps, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_INVALID_VALUE, iRet);

            // invalid value for CL_SAMPLER_NORMALIZED_COORDS
            samplerProps[2] = CL_SAMPLER_ADDRESSING_MODE;
            assert(CL_FALSE != 2 && CL_TRUE != 2);
            samplerProps[1] = 2;
            clCreateSamplerWithProperties(context, samplerProps, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_INVALID_VALUE, iRet);

            // invalid value for CL_SAMPLER_ADDRESSING_MODE
            samplerProps[1] = CL_FALSE;
            const cl_sampler_properties invalidAddrMode = CL_ADDRESS_MIRRORED_REPEAT + 1;
            assert(CL_ADDRESS_NONE != invalidAddrMode && CL_ADDRESS_CLAMP_TO_EDGE != invalidAddrMode && CL_ADDRESS_CLAMP != invalidAddrMode && CL_ADDRESS_REPEAT != invalidAddrMode);
            samplerProps[3] = invalidAddrMode;
            clCreateSamplerWithProperties(context, samplerProps, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_INVALID_VALUE, iRet);

            // invalid value for CL_SAMPLER_FILTER_MODE
            samplerProps[3] = CL_ADDRESS_MIRRORED_REPEAT;
            assert(CL_FILTER_LINEAR + 1 != CL_FILTER_NEAREST);
            samplerProps[5] = CL_FILTER_LINEAR + 1;
            clCreateSamplerWithProperties(context, samplerProps, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_INVALID_VALUE, iRet);

            // invalid name
            samplerProps[5] = CL_FILTER_LINEAR;
            assert(1 != CL_SAMPLER_NORMALIZED_COORDS && 1 != CL_SAMPLER_ADDRESSING_MODE && 1 != CL_SAMPLER_FILTER_MODE);
            samplerProps[0] = 1;
            clCreateSamplerWithProperties(context, samplerProps, &iRet);
            CheckException("clCreateSamplerWithProperties", CL_INVALID_VALUE, iRet);
        }
    }
    catch (const std::exception&)
    {
        bResult = false;
    }
    if (context)
    {
        clReleaseContext(context);
    }
    return bResult;
}
