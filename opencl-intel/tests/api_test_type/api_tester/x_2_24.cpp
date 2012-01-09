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
#include "Chapter_2_24.h"
#include "Arrangement.h"

Chapter_2_24::Chapter_2_24(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_24::~Chapter_2_24()
{
}

int Chapter_2_24::Run(const char* test, int rc)
{
    rc |= several_2_24_1(test);
    rc |= several_2_24_2(test);
    rc |= several_2_24_3(test);
    rc |= several_2_24_4(test);
    rc |= several_2_24_5(test);
    rc |= several_2_24_6(test);
    rc |= several_2_24_7(test);
    rc |= several_2_24_8(test);
    rc |= several_2_24_9(test);
    rc |= several_2_24_10(test);
    rc |= several_2_24_11(test);
    rc |= several_2_24_12(test);
    rc |= several_2_24_13(test);
    rc |= several_2_24_14(test);
    rc |= several_2_24_15(test);
    rc |= several_2_24_16(test);
    rc |= several_2_24_17(test);
    rc |= several_2_24_18(test);

    return rc;
}

int Chapter_2_24::several_2_24_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.1.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.2.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_FALSE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}


int Chapter_2_24::several_2_24_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.3.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_event            ev;
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        &ev);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.4.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_event            ev;
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        &ev);
    if (CL_SUCCESS != ra)
    {
        ra = clEnqueueWriteImage(
            q, 
            image, 
            CL_TRUE, 
            origin, 
            region, 
            0, 
            0, 
            (void *)host, 
            1, 
            &ev,
            NULL);
    }
    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.5.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        32, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.6.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        32, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.7.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    Arrangement         img;
    if (PROCESSED_OK != img.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = img.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.8.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {2, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.9.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 2, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.10.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 1};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.11.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {3, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.12.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 3, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.13.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 0};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.14.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        1, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.15.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        1, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_16(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.16.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        NULL, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_17(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.17.several";

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

    // Execute action
    cl_command_queue    q = NULL;
    cl_mem              image = ar.GetImage();
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_24::several_2_24_18(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.24.18.several";

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

    // Execute action
    cl_command_queue    q = ar.GetQueue();
    cl_mem              image = NULL;
    size_t              origin[3] = {0, 0, 0};
    size_t              region[3] = {1, 1, 1};
    char                host[32];
    cl_int              ra;
    ra = clEnqueueWriteImage(
        q, 
        image, 
        CL_TRUE, 
        origin, 
        region, 
        0, 
        0, 
        (void *)host, 
        0, 
        NULL,
        NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

