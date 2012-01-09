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
#include "Chapter_2_36.h"
#include "Arrangement.h"

Chapter_2_36::Chapter_2_36(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_36::~Chapter_2_36()
{
}

int Chapter_2_36::Run(const char* test, int rc)
{
    rc |= Sampler_2_36_1(test);
    rc |= Sampler_2_36_2(test);
    rc |= Sampler_2_36_3(test);
    rc |= Sampler_2_36_4(test);
    rc |= Sampler_2_36_5(test);
    rc |= Sampler_2_36_6(test);
    rc |= Sampler_2_36_7(test);
    rc |= Sampler_2_36_8(test);
    rc |= empty_2_36_9(test);
    rc |= Sampler_2_36_10(test);
    rc |= Sampler_2_36_11(test);

    return rc;
}

int Chapter_2_36::Sampler_2_36_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.1.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_uint             n;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), &n, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > n)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.2.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_context          cntxt;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_CONTEXT, sizeof(cl_context), &cntxt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != cntxt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.3.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_addressing_mode  a_mode;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_ADDRESSING_MODE, sizeof(a_mode), &a_mode, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.4.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_filter_mode      f_mode;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_FILTER_MODE, sizeof(f_mode), &f_mode, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_5(const char* test)
{
    // constant
    const char      TEST_NAME[] = "2.36.5.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_uint             n;
    size_t              rsize;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), &n, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > n)
        rc |= PROCESSED_FAIL;
    if (sizeof(n) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.6.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_context          cntxt;
    size_t              rsize;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_CONTEXT, sizeof(cl_context), &cntxt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != cntxt)
        rc |= PROCESSED_FAIL;
    if (sizeof(cntxt) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.7.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_addressing_mode  a_mode;
    size_t              rsize;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_ADDRESSING_MODE, sizeof(a_mode), &a_mode, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(a_mode) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.8.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_filter_mode      f_mode;
    size_t              rsize;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_FILTER_MODE, sizeof(f_mode), &f_mode, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(f_mode) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::empty_2_36_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.9.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action
    cl_sampler          sampler = NULL;
    cl_int              ra;
    cl_int              n;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), &n, NULL);

    // Check correctness
    if (CL_INVALID_SAMPLER != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.10.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_uint             n;
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint) - 1, &n, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_36::Sampler_2_36_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.36.11.Sampler";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.Sampler();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_sampler          sampler = ar.GetSampler();
    cl_int              ra;
    ra = clGetSamplerInfo(sampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
