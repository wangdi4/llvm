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
#include "Chapter_2_16.h"
#include "Arrangement.h"

Chapter_2_16::Chapter_2_16(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_16::~Chapter_2_16()
{
}

int Chapter_2_16::Run(const char* test, int rc)
{
    rc |= queue_2_16_1(test);
    rc |= queue_2_16_2(test);
    rc |= queue_2_16_3(test);
    rc |= queue_2_16_4(test);
    rc |= queue_2_16_5(test);
    rc |= queue_2_16_6(test);
    rc |= queue_2_16_7(test);
    rc |= queue_2_16_8(test);
    rc |= queue_2_16_9(test);
    rc |= queue_2_16_10(test);
    rc |= queue_2_16_11(test);
    rc |= queue_2_16_12(test);
    rc |= queue_2_16_13(test);
    rc |= queue_2_16_14(test);

    return rc;
}

int Chapter_2_16::queue_2_16_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.1.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    if (CL_SUCCESS != r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.2.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    if (CL_SUCCESS != r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_FALSE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.3.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    if (CL_SUCCESS != r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    cl_event            ev;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, &ev);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseEvent(ev);
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.4.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              r1;
    cl_event            ev1;
    r1 = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, &ev1);
    cl_int              r2;
    cl_event            ev2;
    r2 = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 1, &ev1, &ev2);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != r1 || CL_SUCCESS != r2)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseEvent(ev1);
    clReleaseEvent(ev2);
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.5.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(NULL, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.6.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    char                host_buf[BUF_SIZE];
    ra = clEnqueueWriteBuffer(q, NULL, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.7.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE + 1, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.8.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, BUF_SIZE - 1, 2, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.9.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(NULL, buffer, CL_TRUE, 0, BUF_SIZE, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.10.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.11.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.12.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.13.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    buffer = clCreateBuffer(cntxt, CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, NULL, &r_mem);
    cl_int              ra;
    char                host_buf[BUF_SIZE];
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);
    
// Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_16::queue_2_16_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.16.14.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer;
    cl_int              r_mem;
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, host_buf, &r_mem);
    cl_int              ra;
    ra = clEnqueueWriteBuffer(q, buffer, CL_TRUE, 0, BUF_SIZE, host_buf, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != r_mem || CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(buffer);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}
