// Copyright (c) 1997-2004 Intel Corporation
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

#include <stdlib.h>
#include "stdafx.h"
#include "Chapter_2_46.h"
#include "Arrangement.h"

Chapter_2_46::Chapter_2_46(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_46::~Chapter_2_46()
{
}

int Chapter_2_46::Run(const char* test, int rc)
{
    rc |= BuildProgram_2_46_1(test);
    rc |= BuildProgram_2_46_2(test);
    rc |= BuildProgram_2_46_3(test);
    rc |= BuildProgram_2_46_4(test);
    rc |= BuildProgram_2_46_5(test);
    rc |= CreateProgramWithSource_2_46_6(test);
    rc |= empty_2_46_7(test);
    rc |= BuildProgram_2_46_8(test);

    return rc;
}

int Chapter_2_46::BuildProgram_2_46_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.1.BuildProgram";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernel;
    cl_int          ra;
    ra = clCreateKernelsInProgram(ar.GetProgram(), 1, &kernel, NULL);

    // Check correctness
    if (CL_SUCCESS == ra && NULL == kernel)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != ra && CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseKernel(kernel);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::BuildProgram_2_46_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.2.BuildProgram";
    const int       KERNELS = 1024;


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_uint         nk;
    ra = clCreateKernelsInProgram(ar.GetProgram(), KERNELS, NULL, &nk);

    // Check correctness
    if (1 < nk)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::BuildProgram_2_46_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.3.BuildProgram";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernel;
    cl_int          ra;
    cl_uint         nk;
    ra = clCreateKernelsInProgram(ar.GetProgram(), 1, &kernel, &nk);

    // Check correctness
    if (CL_SUCCESS == ra && NULL == kernel)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != ra && CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (1 < nk)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    clReleaseKernel(kernel);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::BuildProgram_2_46_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.4.BuildProgram";
    const int       KERNELS = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    ra = clCreateKernelsInProgram(ar.GetProgram(), KERNELS, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}


int Chapter_2_46::BuildProgram_2_46_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.5.BuildProgram";
    const int       KERNELS = 1024;


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernels[KERNELS];
    memset(kernels, 0, KERNELS * sizeof(cl_kernel));
    cl_int          ra;
    ra = clCreateKernelsInProgram(ar.GetProgram(), KERNELS, kernels, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    int             i;
    for (i = 0; ; i++)
    {
        if ((cl_kernel)0 != kernels[i])
            clReleaseKernel(kernels[i]);
        else
            break;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::CreateProgramWithSource_2_46_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.6.CreateProgramWithSource";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernel;
    cl_int          ra;
    ra = clCreateKernelsInProgram(ar.GetProgram(), 1, &kernel, NULL);

    // Check correctness
    if (CL_INVALID_PROGRAM_EXECUTABLE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseKernel(kernel);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::empty_2_46_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.7.empty";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    int             rc = PROCESSED_OK;
    // Execute action
    cl_kernel       kernel;
    cl_uint         num_kernels = sizeof(cl_kernel);
    cl_int          ra;
    ra = clCreateKernelsInProgram(NULL, num_kernels, &kernel, NULL);

    // Check correctness
    if (CL_INVALID_PROGRAM != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_46::BuildProgram_2_46_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.46.8.BuildProgram";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel       kernel;
    cl_int          ra;
    ra = clCreateKernelsInProgram(ar.GetProgram(), 0, &kernel, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
