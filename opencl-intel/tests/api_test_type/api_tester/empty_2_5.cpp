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
#include "Chapter_2_5.h"
#include "CL\cl.h"

Chapter_2_5::Chapter_2_5(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_5::~Chapter_2_5()
{
}

int Chapter_2_5::Run(const char* test, int rc)
{
    rc |= empty_2_5_1(test);
    rc |= empty_2_5_2(test);
    rc |= empty_2_5_3(test);
    rc |= empty_2_5_4(test);
    rc |= empty_2_5_5(test);
    rc |= empty_2_5_6(test);
    rc |= empty_2_5_7(test);
    rc |= empty_2_5_8(test);

    return rc;
}

int Chapter_2_5::empty_2_5_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.1.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.2.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra && CL_DEVICE_NOT_FOUND != ra && CL_DEVICE_NOT_AVAILABLE != ra)
        rc |= PROCESSED_FAIL;
    else if (CL_SUCCESS == ra && NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.3.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

     // Arrangement phase
    cl_uint         num_devices;
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &num_devices);
    if (CL_SUCCESS == r && 0 == num_devices)
    {
       return Finish(&status, TEST_NAME, PROCESSED_OK);
    }

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra && CL_DEVICE_NOT_FOUND != ra && CL_DEVICE_NOT_AVAILABLE != ra)
        rc |= PROCESSED_FAIL;
    else if (CL_SUCCESS == ra && NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.5.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_ALL, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.6.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, NULL, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE_TYPE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.7.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_context      cntxt;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (NULL == cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        cl_int          ra;
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_5::empty_2_5_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.5.8.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContextFromType((cl_context_properties *)-1, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != cntxt)
        rc |= PROCESSED_FAIL;


    // clean-up phase
    if (cntxt)
    {
        ra = clReleaseContext(cntxt);
        if (CL_SUCCESS != ra)
            rc |= PROCESSED_FAIL;
    }
    return Finish(&status, TEST_NAME, rc);
}
