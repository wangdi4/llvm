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
#include "Chapter_2_17.h"
#include "Arrangement.h"

Chapter_2_17::Chapter_2_17(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_17::~Chapter_2_17()
{
}

int Chapter_2_17::Run(const char* test, int rc)
{
    rc |= queue_2_17_1(test);
    rc |= queue_2_17_2(test);
    rc |= queue_2_17_3(test);
    rc |= queue_2_17_4(test);
    rc |= queue_2_17_5(test);
    rc |= queue_2_17_6(test);
    rc |= queue_2_17_7(test);
    rc |= queue_2_17_8(test);
    rc |= queue_2_17_9(test);
    rc |= queue_2_17_10(test);
    rc |= queue_2_17_11(test);
    rc |= queue_2_17_12(test);
    rc |= queue_2_17_13(test);
    rc |= queue_2_17_14(test);
    rc |= queue_2_17_15(test);
    rc |= queue_2_17_16(test);
    rc |= queue_2_17_17(test);
    rc |= queue_2_17_18(test);
    rc |= queue_2_17_19(test);
    rc |= queue_2_17_20(test);

    return rc;
}

int Chapter_2_17::queue_2_17_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.1.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.2.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.3.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.4.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.5.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    dst_buf = clCreateBuffer(cntxt, CL_MEM_ALLOC_HOST_PTR, BUF_SIZE, NULL, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.6.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_int              ra;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.7.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_event            ev1;
    ra = clEnqueueWriteBuffer(q, dst_buf, CL_TRUE, 0, BUF_SIZE, dst_mem, 0, NULL, &ev1);
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 1, &ev1, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.8.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_event            ev1;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, &ev1);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.9.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[1];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.10.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, src_buf, 0, BUF_SIZE / 2, BUF_SIZE / 2, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.11.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, src_buf, 0, 0, BUF_SIZE / 2, 0, NULL, NULL);

    // Check correctness
    if (CL_MEM_COPY_OVERLAP != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.12.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.13.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(NULL, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.14.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, NULL, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.15.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, NULL, dst_buf, 0, 0, BUF_SIZE, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_16(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.16.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 1, NULL, NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_17(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.17.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_event            ev = NULL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 1, &ev, NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_18(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.18.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    cl_event            ev = (cl_event)INVALID_VALUE;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, BUF_SIZE, 0, &ev, NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_19(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.19.queue";
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
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, 0, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt, q);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_17::queue_2_17_20(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.17.20.queue";
    const int       BUF_SIZE = 8193;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_command_queue    q;
    cl_context          cntxt1;
    cl_device_id        dev;
     // Arrangement phase
    int                 r = queue(&cntxt1, &dev, &q);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_mem              src_buf;
    cl_int              src_r_mem;
    char                src_mem[BUF_SIZE];
    src_buf = clCreateBuffer(cntxt1, CL_MEM_USE_HOST_PTR, BUF_SIZE, src_mem, &src_r_mem);
    if (CL_SUCCESS != src_r_mem)
        rc |= PROCESSED_FAIL;
    cl_context          cntxt2;
    cntxt2 = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &ra);
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    cl_mem              dst_buf;
    cl_int              dst_r_mem;
    char                dst_mem[BUF_SIZE];
    dst_buf = clCreateBuffer(cntxt2, CL_MEM_COPY_HOST_PTR, BUF_SIZE, dst_mem, &dst_r_mem);
    if (CL_SUCCESS != dst_r_mem)
        rc |= PROCESSED_FAIL;
    ra = clEnqueueCopyBuffer(q, src_buf, dst_buf, 0, 0, 0, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    clReleaseContext(cntxt2);
    clReleaseMemObject(dst_buf);
    clReleaseMemObject(src_buf);
    queue_cleanup(cntxt1, q);

    return Finish(&status, TEST_NAME, rc);
}
