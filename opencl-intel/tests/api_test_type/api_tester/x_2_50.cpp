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

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include "stdafx.h"
#include "Chapter_2_50.h"
#include "Arrangement.h"

Chapter_2_50::Chapter_2_50(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_50::~Chapter_2_50()
{
}

int Chapter_2_50::Run(const char* test, int rc)
{
    rc |= kernel_2_50_1(test);
    rc |= kernel_2_50_2(test);
    rc |= kernel_2_50_3(test);
    rc |= kernel_2_50_4(test);
    rc |= kernel_2_50_5(test);
    rc |= empty_2_50_6(test);
    rc |= kernel_2_50_7(test);
    rc |= kernel_2_50_8(test);
    rc |= kernel_2_50_9(test);

    return rc;
}

int Chapter_2_50::kernel_2_50_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.1.kernel";
    const int       NAME_LENGHT = 64;


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    char            fName[NAME_LENGHT];
    size_t          rsize;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_FUNCTION_NAME, NAME_LENGHT, fName, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (strlen(fName) + 1 != rsize)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.2.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_uint         nArgs;
    size_t          rsize;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_NUM_ARGS, sizeof(nArgs), &nArgs, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cl_uint) != rsize)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.3.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_uint         rcount;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_REFERENCE_COUNT, sizeof(rcount), &rcount, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.4.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_context      c;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_CONTEXT, sizeof(c), &c, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.5.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_program      p;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_PROGRAM, sizeof(p), &p, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::empty_2_50_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.6.empty";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_program      prog;
    ra = clGetKernelInfo(NULL, CL_KERNEL_PROGRAM, sizeof(prog), &prog, NULL);

    // Check correctness
    if (CL_INVALID_KERNEL != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.7.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    cl_program      p;
    ra = clGetKernelInfo(ar.GetKernel(), 0, sizeof(p), &p, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.8.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_CONTEXT, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_50::kernel_2_50_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.50.9.kernel";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.Kernel();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    size_t          rsize;
    ra = clGetKernelInfo(ar.GetKernel(), CL_KERNEL_CONTEXT, sizeof(cl_context), NULL, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cl_context) != rsize)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}
