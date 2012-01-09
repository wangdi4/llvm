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
#include "Chapter_2_51.h"
#include "Arrangement.h"

Chapter_2_51::Chapter_2_51(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_51::~Chapter_2_51()
{
}

int Chapter_2_51::Run(const char* test, int rc)
{
    rc |= several_2_51_1(test);
    rc |= several_2_51_2(test);
    rc |= several_2_51_3(test);
    rc |= several_2_51_4(test);
    rc |= several_2_51_5(test);
    rc |= several_2_51_6(test);
    rc |= several_2_51_7(test);
    rc |= several_2_51_8(test);
    rc |= several_2_51_9(test);

    return rc;
}

int Chapter_2_51::several_2_51_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.1.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ws), &ws, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(ws) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 == ws)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.2.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              gs[3];
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(gs), &gs, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (3 * sizeof(size_t) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.3.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    cl_ulong            ms;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(ms), &ms, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(ms) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 == ms)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.4.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ws), &ws, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 == ws)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.5.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = NULL;
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ws), &ws, &rsize);

    // Check correctness
    if (CL_INVALID_KERNEL != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.6.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = NULL;
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ws), &ws, &rsize);

    // Check correctness
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.7.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, NULL, sizeof(ws), &ws, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.8.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, 0, &ws, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_51::several_2_51_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.51.9.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.cntxt_dev())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    if (PROCESSED_OK != ar.Kernel())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_kernel           kernel = ar.GetKernel();
    cl_device_id        dev = ar.GetDevice();
    cl_int              ra;
    size_t              ws;
    size_t              rsize;
    ra = clGetKernelWorkGroupInfo(kernel, dev, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ws), NULL, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
