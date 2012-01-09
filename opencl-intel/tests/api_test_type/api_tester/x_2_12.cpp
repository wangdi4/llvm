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
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <share.h> 
#include "Chapter_2_12.h"
#include "Arrangement.h"

Chapter_2_12::Chapter_2_12(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_12::~Chapter_2_12()
{
}

int Chapter_2_12::Run(const char* test, int rc)
{
    rc |= queue_2_12_1(test);
    rc |= queue_2_12_2(test);
    rc |= queue_2_12_3(test);
    rc |= queue_2_12_4(test);
    rc |= queue_2_12_5(test);
    rc |= queue_2_12_6(test);
    rc |= queue_2_12_7(test);
    rc |= queue_2_12_8(test);
    rc |= queue_2_12_9(test);

    return rc;
}

int Chapter_2_12::queue_2_12_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.1.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_CONTEXT, sizeof(cntxt), &cntxt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cntxt) != rsize)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != cntxt)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.2.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_device_id        dev;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_DEVICE, sizeof(dev), &dev, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(dev) != rsize)
        rc |= PROCESSED_FAIL;
    if (ar.GetDevice() != dev)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.3.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_uint             count;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_REFERENCE_COUNT, sizeof(count), &count, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(count) != rsize)
        rc |= PROCESSED_FAIL;
    if (1 > count)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.4.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_command_queue_properties prop;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_PROPERTIES, sizeof(prop), &prop, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(prop) != rsize)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.5.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = NULL;
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_CONTEXT, sizeof(cntxt), &cntxt, &rsize);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.6.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, 0, sizeof(cntxt), &cntxt, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.7.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_CONTEXT, 0, &cntxt, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.8.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_CONTEXT, sizeof(cntxt), NULL, &rsize);

    // Check correctness
	if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_12::queue_2_12_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.12.9.queue";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Queue();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_context          cntxt;
    cl_int              ra;
    ra = clGetCommandQueueInfo(q, CL_QUEUE_CONTEXT, sizeof(cntxt), &cntxt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != cntxt)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

