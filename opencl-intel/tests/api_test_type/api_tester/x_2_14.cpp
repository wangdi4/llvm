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
#include "Chapter_2_14.h"
#include "CL\cl.h"

Chapter_2_14::Chapter_2_14(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_14::~Chapter_2_14()
{
}

int Chapter_2_14::Run(const char* test, int rc)
{
    rc |= context_2_14_1(test);
    rc |= context_2_14_2(test);
    rc |= context_2_14_3(test);
    rc |= context_2_14_4(test);
    rc |= context_2_14_5(test);
    rc |= context_2_14_6(test);
    rc |= context_2_14_7(test);
    rc |= context_2_14_8(test);
    rc |= context_2_14_9(test);
    rc |= context_2_14_10(test);
    rc |= context_2_14_11(test);
    rc |= context_2_14_12(test);
    rc |= context_2_14_13(test);
    rc |= context_2_14_14(test);
    rc |= context_2_14_15(test);
    rc |= context_2_14_16(test);
    rc |= context_2_14_17(test);
    rc |= empty_2_14_18(test);

    return rc;
}

int Chapter_2_14::context_2_14_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.1.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.2.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.3.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.4.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.5.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.6.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.7.context";
    const int       BUF_SIZE = 128;

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
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_WRITE, BUF_SIZE, NULL, NULL);

    // Check correctness
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.8.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_WRITE_ONLY, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.9.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_ONLY, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.10.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_INVALID_HOST_PTR != ra)
        rc |= PROCESSED_FAIL;
    if (buffer != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.11.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.12.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, NULL, &ra);

    // Check correctness
    if (CL_INVALID_HOST_PTR != ra)
        rc |= PROCESSED_FAIL;
    if (buffer != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.13.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (buffer != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.14.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.15.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_ONLY, 0, host_buf, &ra);

    // Check correctness
    if (CL_INVALID_BUFFER_SIZE != ra)
        rc |= PROCESSED_FAIL;
    if (buffer != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_16(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.16.context";
    const int       BUF_SIZE = 128;

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
    char                host_buf[BUF_SIZE];
    cl_mem              buffer;
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, NULL);

    // Check correctness
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::context_2_14_17(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.17.context";
    const int       BUF_SIZE = 128;

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
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_READ_WRITE, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (buffer == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_14::empty_2_14_18(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.14.18.context";
    const int       BUF_SIZE = 128;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Execute action
    cl_int              ra;
    cl_mem              buffer;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer((cl_context)NULL, CL_MEM_COPY_HOST_PTR, BUF_SIZE, host_buf, &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
    if (buffer != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (buffer)
        clReleaseMemObject(buffer);

    return Finish(&status, TEST_NAME, rc);
}
