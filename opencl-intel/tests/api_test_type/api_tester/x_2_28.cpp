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
#include "Chapter_2_28.h"
#include "Arrangement.h"

Chapter_2_28::Chapter_2_28(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_28::~Chapter_2_28()
{
}

int Chapter_2_28::Run(const char* test, int rc)
{
    rc |= several_2_28_1(test);
    rc |= several_2_28_2(test);
    rc |= several_2_28_3(test);
    rc |= several_2_28_4(test);
    rc |= several_2_28_5(test);
    //rc |= several_2_28_6(test);
    rc |= several_2_28_7(test);
    rc |= several_2_28_8(test);
    rc |= several_2_28_9(test);
    rc |= several_2_28_10(test);
    rc |= several_2_28_11(test);
    rc |= several_2_28_12(test);
    rc |= several_2_28_13(test);
    rc |= several_2_28_14(test);

    return rc;
}

int Chapter_2_28::several_2_28_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.1.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.2.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_FALSE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.3.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_WRITE, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.4.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_FALSE, 
        CL_MAP_WRITE, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.5.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    cl_event            ev;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        &ev,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (ev)
        clReleaseEvent(ev);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.6.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    cl_event            ev;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        &ev,
        &ra);
    if (CL_SUCCESS == ra && NULL != ptr)
    {
        ptr = clEnqueueMapBuffer(
            q, 
            buffer, 
            CL_TRUE, 
            CL_MAP_READ, 
            0, 
            BUFFER_SIZE, 
            1, 
            &ev,
            NULL,
            &ra);
    }

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (ev)
        clReleaseEvent(ev);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.7.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        NULL);

    // Check correctness
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.8.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        0, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.9.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        512, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.10.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        1, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.11.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    cl_event            ev = NULL;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        1, 
        &ev,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.12.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              buffer = NULL;
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.13.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = NULL;
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_28::several_2_28_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.28.14.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    Arrangement         ar1;
    if (PROCESSED_OK != ar1.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_command_queue    q = ar1.GetQueue();
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    ptr = clEnqueueMapBuffer(
        q, 
        buffer, 
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFFER_SIZE, 
        0, 
        NULL,
        NULL,
        &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
