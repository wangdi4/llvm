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
#include "Chapter_2_33.h"
#include "Arrangement.h"

Chapter_2_33::Chapter_2_33(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_33::~Chapter_2_33()
{
}

int Chapter_2_33::Run(const char* test, int rc)
{
    rc |= context_2_33_1(test);
    rc |= context_2_33_2(test);
    rc |= context_2_33_3(test);
    rc |= context_2_33_4(test);
    rc |= context_2_33_5(test);
    rc |= context_2_33_6(test);
    rc |= context_2_33_7(test);
    rc |= context_2_33_8(test);
    rc |= context_2_33_9(test);
    rc |= context_2_33_10(test);
    rc |= context_2_33_11(test);
    rc |= context_2_33_12(test);
    rc |= context_2_33_12(test);
    rc |= context_2_33_14(test);
    rc |= context_2_33_15(test);
    rc |= context_2_33_16(test);
    rc |= context_2_33_17(test);
    rc |= empty_2_33_18(test);
    rc |= context_2_33_19(test);
    rc |= context_2_33_20(test);

    return rc;
}

int Chapter_2_33::context_2_33_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.1.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.2.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.3.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.4.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.5.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.6.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_CLAMP, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.7.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_NONE, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.8.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_NONE, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.9.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.10.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_REPEAT, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.11.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.12.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.13.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.14.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_CLAMP, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.15.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_NONE, CL_FILTER_NEAREST, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_16(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.16.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_FALSE, CL_ADDRESS_NONE, CL_FILTER_LINEAR, NULL);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_17(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.17.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_int              ra;
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, &ra);

    // Check correctness
    if (NULL == sampler)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::empty_2_33_18(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.18.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action
    cl_context          cntxt = NULL;
    cl_int              ra;
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, &ra);

    // Check correctness
    if (NULL != sampler)
        rc |= PROCESSED_FAIL;
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_19(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.19.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_int              ra;
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, CL_ADDRESS_REPEAT, -1, &ra);

    // Check correctness
    if (NULL != sampler)
        rc |= PROCESSED_FAIL;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_33::context_2_33_20(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.33.20.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.context();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_int              ra;
    cl_sampler          sampler;
    sampler = clCreateSampler(cntxt, CL_TRUE, -1, CL_FILTER_NEAREST, &ra);

    // Check correctness
    if (NULL != sampler)
        rc |= PROCESSED_FAIL;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (sampler)
        clReleaseSampler(sampler);
 
    return Finish(&status, TEST_NAME, rc);
}

