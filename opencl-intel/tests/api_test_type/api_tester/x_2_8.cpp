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
#include "Chapter_2_8.h"
#include "CL\cl.h"

Chapter_2_8::Chapter_2_8(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_8::~Chapter_2_8()
{
}

int Chapter_2_8::Run(const char* test, int rc)
{
    rc |= context_2_8_1(test);
    rc |= context_2_8_2(test);
    rc |= context_2_8_3(test);
    rc |= empty_2_8_4(test);
    rc |= context_2_8_5(test);
    rc |= context_2_8_6(test);
    rc |= context_2_8_7(test);

    return rc;
}

int Chapter_2_8::context_2_8_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.1.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_uint         cnt;
    cl_int          ra;
    size_t          rsize;
    ra = clGetContextInfo(cntxt,
        CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &cnt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cl_uint) != rsize)
        rc |= PROCESSED_FAIL;
    if (1 != cnt)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::context_2_8_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.2.context";
    const int       devices = 64;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_device_id    dev[devices];
    cl_int          ra;
    size_t          rsize;
    ra = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, devices * sizeof(cl_device_id), &dev, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::context_2_8_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.3.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context_properties   prop;
    cl_int                  ra;
    size_t                  rsize;
    ra = clGetContextInfo(cntxt, CL_CONTEXT_PROPERTIES, sizeof(cl_context_properties), &prop, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 != rsize)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::empty_2_8_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Execute action
    cl_context_properties   prop;
    cl_int                  ra;
    size_t                  rsize;
    ra = clGetContextInfo((cl_context)-1, CL_CONTEXT_PROPERTIES, sizeof(cl_context_properties), &prop, &rsize);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::context_2_8_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.5.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context_properties   prop;
    cl_int                  ra;
    size_t                  rsize;
    ra = clGetContextInfo(cntxt, -1, sizeof(cl_context_properties), &prop, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::context_2_8_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.6.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context_properties   prop;
    cl_int                  ra;
    size_t                  rsize;
    ra = clGetContextInfo(cntxt, CL_CONTEXT_PROPERTIES, 1, &prop, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_8::context_2_8_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.8.7.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int                  ra;
    size_t                  rsize;
    ra = clGetContextInfo(cntxt, CL_CONTEXT_PROPERTIES, 0, NULL, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 != rsize)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}
