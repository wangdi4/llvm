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
#include "Chapter_2_9.h"
#include "Arrangement.h"

Chapter_2_9::Chapter_2_9(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_9::~Chapter_2_9()
{
}

int Chapter_2_9::Run(const char* test, int rc)
{
    rc |= ctxt_dev_2_9_1(test);
    rc |= ctxt_dev_2_9_2(test);
    rc |= ctxt_dev_2_9_3(test);
    rc |= empty_2_9_4(test);
    rc |= ctxt_dev_2_9_5(test);
    rc |= ctxt_dev_2_9_6(test);
    rc |= ctxt_dev_2_9_7(test);
    rc |= ctxt_dev_2_9_8(test);

    return rc;
}

int Chapter_2_9::ctxt_dev_2_9_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.1.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, dev, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (queue == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.2.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, dev, CL_QUEUE_PROFILING_ENABLE, NULL);

    // Check correctness
    if (queue == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.3.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, dev, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ra);

    // Check correctness
    if (queue == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::empty_2_9_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Execute action
    cl_command_queue    queue;
    queue = clCreateCommandQueue((cl_context)-1, (cl_device_id)-1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, NULL);

    // Check correctness
    if (queue != NULL)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.5.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, dev, -1, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (queue != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.6.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue((cl_context)-1, dev, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
    if (queue != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.7.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_device_id    dev;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetContextInfo(cntxt, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &dev, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, (cl_device_id)-1, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ra);

    // Check correctness
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;
    if (queue != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (queue)
        clReleaseCommandQueue(queue);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_9::ctxt_dev_2_9_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.9.8.cntxt_dev";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt;
    cl_device_id        dev;
     // Arrangement phase
    rc = cntxt_dev(&cntxt, &dev);
    if (PROCESSED_OK != rc)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    cl_command_queue    queue;
    queue = clCreateCommandQueue(cntxt, dev, 0, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (queue == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    queue_cleanup(cntxt, queue);

    return Finish(&status, TEST_NAME, rc);
}
