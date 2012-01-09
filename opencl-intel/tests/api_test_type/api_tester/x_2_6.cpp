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
#include "Chapter_2_6.h"
#include "CL\cl.h"

Chapter_2_6::Chapter_2_6(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_6::~Chapter_2_6()
{
}

int Chapter_2_6::Run(const char* test, int rc)
{
    rc |= context_2_6_1(test);
    rc |= empty_2_6_2(test);

    return rc;
}

int Chapter_2_6::context_2_6_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.6.1.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_context      cntxt;
     // Arrangement phase
    cl_int          r;
    cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_CPU, NULL, NULL, &r);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int           ra;
    ra = clRetainContext(cntxt);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    if (cntxt)
    {
        while(CL_SUCCESS == clReleaseContext(cntxt))
            ;
    }

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_6::empty_2_6_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.6.2.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    ra = clRetainContext(NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;


    return Finish(&status, TEST_NAME, rc);
}
