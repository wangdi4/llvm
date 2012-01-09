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
#include "Chapter_2_53.h"
#include "Arrangement.h"

Chapter_2_53::Chapter_2_53(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_53::~Chapter_2_53()
{
}

int Chapter_2_53::Run(const char* test, int rc)
{
    rc |= several_2_53_1(test);
    rc |= several_2_53_2(test);
    rc |= several_2_53_3(test);
    rc |= several_2_53_4(test);
    rc |= several_2_53_5(test);
    rc |= several_2_53_6(test);
    rc |= several_2_53_7(test);
    rc |= several_2_53_8(test);
    rc |= several_2_53_9(test);

    return rc;
}

int Chapter_2_53::several_2_53_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.1.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.2.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    cl_event            ev;
    ra = clEnqueueTask(q, kernel, 0, NULL, &ev);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.3.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    cl_event            ev;
    ra = clEnqueueTask(q, kernel, 0, NULL, &ev);
    if (CL_SUCCESS == ra)
        ra = clEnqueueTask(q, kernel, 1, &ev, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.4.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = NULL;
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.5.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = NULL;
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_KERNEL != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.6.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_KERNEL_ARGS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.7.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    Arrangement         ar1;
    if (PROCESSED_OK != ar1.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_kernel           kernel = ar1.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.8.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    ra = clEnqueueTask(q, kernel, 1, NULL, NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_53::several_2_53_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.53.9.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.KernelReady())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_command_queue    q = ar.GetQueue();
    cl_int              ra;
    cl_event            ev = NULL;
    ra = clEnqueueTask(q, kernel, 1, &ev, NULL);

    // Check correctness
    if (CL_INVALID_EVENT_WAIT_LIST != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}
