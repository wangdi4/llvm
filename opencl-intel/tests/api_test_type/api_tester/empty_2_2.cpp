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
#include "Chapter_2_2.h"
#include "CL\cl.h"

#define MAX(a, b) (a > b ? a : b)

Chapter_2_2::Chapter_2_2(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_2::~Chapter_2_2()
{
}

int Chapter_2_2::Run(const char* test, int rc)
{
    rc |= empty_2_2_1(test);
    rc |= empty_2_2_2(test);
    rc |= empty_2_2_3(test);
    rc |= empty_2_2_4(test);
    rc |= empty_2_2_5(test);
    rc |= empty_2_2_6(test);
    rc |= empty_2_2_7(test);
    rc |= empty_2_2_8(test);
    rc |= empty_2_2_9(test);
    rc |= empty_2_2_10(test);
    rc |= empty_2_2_11(test);
    rc |= empty_2_2_12(test);
    rc |= empty_2_2_13(test);
    rc |= empty_2_2_14(test);
    rc |= empty_2_2_15(test);
    rc |= empty_2_2_16(test);
    rc |= empty_2_2_17(test);
    rc |= empty_2_2_18(test);

    return rc;
}

int Chapter_2_2::empty_2_2_1(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.1.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, 0, 0, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE_TYPE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_2(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.2.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 0, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_3(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.3.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_4(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_5(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.5.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_6(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.6.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_7(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.7.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int      ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, 1, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_8(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.8.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    device;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &device, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_9(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.9.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    device;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra && CL_DEVICE_NOT_FOUND != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_10(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.10.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    device;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_DEVICE_NOT_FOUND != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_11(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.11.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         rn;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 0, NULL, &rn);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > rn)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_12(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.12.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         rn;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, &rn);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_DEVICE_NOT_FOUND != ra)
        rc |= PROCESSED_FAIL;
    if (1 > rn)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_13(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.13.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         rn;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &rn);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_DEVICE_NOT_FOUND  != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_14(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.14.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         rn;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 0, NULL, &rn);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 != rn)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_15(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.15.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    dev;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 1, &dev, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_16(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.16.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         rn;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &rn);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > rn)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_17(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.17.empty";
    const int   DEVICES = 64;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    devs[DEVICES];
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, DEVICES, devs, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_2::empty_2_2_18(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.2.18.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_device_id    dev;
    cl_int          ra = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 1, &dev, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
