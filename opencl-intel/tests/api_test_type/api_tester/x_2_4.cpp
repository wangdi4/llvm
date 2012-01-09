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
#include "Chapter_2_4.h"
#include "CL\cl.h"

Chapter_2_4::Chapter_2_4(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_4::~Chapter_2_4()
{
}

int Chapter_2_4::Run(const char* test, int rc)
{
    rc |= empty_2_4_1(test);
    rc |= device_2_4_2(test);
    rc |= device_2_4_3(test);
    rc |= empty_2_4_4(test);
    rc |= device_2_4_5(test);
    //rc |= empty_2_4_6(test);

    return rc;
}

int Chapter_2_4::empty_2_4_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.1.device";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContext(0, 1, &dev, NULL, NULL, &ra);

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

int Chapter_2_4::device_2_4_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.2.device";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContext((cl_context_properties *)-1, 1, &dev, NULL, NULL, &ra);

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

int Chapter_2_4::device_2_4_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.3.device";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContext(0, 0, &dev, NULL, NULL, &ra);

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

int Chapter_2_4::empty_2_4_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContext(0, 1, NULL, NULL, NULL, &ra);

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

int Chapter_2_4::device_2_4_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.5.device";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_context      cntxt;
    cntxt = clCreateContext(0, 1, &dev, NULL, NULL, NULL);

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

int Chapter_2_4::empty_2_4_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.4.6.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev = NULL;
    // Execute action
    cl_int          ra;
    cl_context      cntxt;
    cntxt = clCreateContext(0, 1, &dev, NULL, NULL, &ra);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE != ra)
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
