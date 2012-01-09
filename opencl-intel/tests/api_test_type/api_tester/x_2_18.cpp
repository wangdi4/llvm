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
#include "Chapter_2_18.h"
#include "CL\cl.h"

Chapter_2_18::Chapter_2_18(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_18::~Chapter_2_18()
{
}

int Chapter_2_18::Run(const char* test, int rc)
{
    rc |= membuff_2_18_1(test);
    rc |= empty_2_18_2(test);

    return rc;
}

int Chapter_2_18::membuff_2_18_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.18.1.membuff";
    const int       BUF_SIZE = 128;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
    cl_mem          buffer;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    char                host_buf[BUF_SIZE];
    buffer = clCreateBuffer(cntxt, CL_MEM_USE_HOST_PTR, BUF_SIZE, host_buf, &rc);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int              ra;
    ra = clRetainMemObject(buffer);
    cl_int              r1 = clReleaseMemObject(buffer);
    cl_int              r2 = clReleaseMemObject(buffer);
    cl_int              r3 = clReleaseMemObject(buffer);

    // Check correctness
    if (CL_SUCCESS != ra || CL_SUCCESS != r1 ||
        CL_SUCCESS != r2 || CL_INVALID_MEM_OBJECT != r3)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_18::empty_2_18_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.18.2.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;

    // Execute action
    cl_int              ra;
    ra = clRetainMemObject((cl_mem)NULL);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
