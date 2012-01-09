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
#include "Chapter_2_26.h"
#include "Arrangement.h"

Chapter_2_26::Chapter_2_26(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_26::~Chapter_2_26()
{
}

int Chapter_2_26::Run(const char* test, int rc)
{
    rc |= several_2_26_1(test);
    rc |= several_2_26_2(test);
    rc |= several_2_26_3(test);
    rc |= several_2_26_4(test);
    rc |= several_2_26_5(test);
    rc |= several_2_26_6(test);
    rc |= several_2_26_7(test);
    rc |= several_2_26_8(test);
    rc |= several_2_26_9(test);
    rc |= several_2_26_10(test);
    rc |= several_2_26_11(test);
    rc |= several_2_26_12(test);
    rc |= several_2_26_13(test);
    rc |= several_2_26_14(test);
    rc |= several_2_26_15(test);

    return rc;
}

int Chapter_2_26::several_2_26_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.1.several";
    const int       BUFFER_SIZE = 2 * 2 * 1 * 4 * sizeof(CL_UNORM_INT8) + 1;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        1, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.2.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8) + 1;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        1, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.3.several";
    const int       BUFFER_SIZE = 2 * 2 * 1 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 1};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.4.several";
    const int       BUFFER_SIZE = 2 * 2 * 1 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 2};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.5.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 3};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.6.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 3};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.7.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = NULL;
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.8.several";
    const int       BUFFER_SIZE = 2 * 2 * 1 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    Arrangement         ar1;
    if (PROCESSED_OK != ar1.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_command_queue    q = ar1.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.9.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

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
    Arrangement         ar1;
    if (PROCESSED_OK != ar1.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_mem              image = ar1.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.10.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    Arrangement         ar1;
    if (PROCESSED_OK != ar1.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_mem              buffer = ar1.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.11.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = NULL;
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.12.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = NULL;
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.13.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        1, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.14.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        1, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_26::several_2_26_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.26.15.several";
    const int       BUFFER_SIZE = 2 * 2 * 2 * 4 * sizeof(CL_UNORM_INT8);

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    cl_mem              buffer = ar.GetBuffer();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    cl_event            ev = NULL;
    ra = clEnqueueCopyImageToBuffer(
        q, 
        image, 
        buffer, 
        origin, 
        region, 
        0, 
        1, 
        &ev,
        NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (ev)
        clReleaseEvent(ev);

    return Finish(&status, TEST_NAME, rc);
}
