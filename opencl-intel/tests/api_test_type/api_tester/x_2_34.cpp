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
#include "Chapter_2_34.h"
#include "Arrangement.h"

Chapter_2_34::Chapter_2_34(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_34::~Chapter_2_34()
{
}

int Chapter_2_34::Run(const char* test, int rc)
{
    rc |= Sampler_2_34_1(test);
    rc |= empty_2_34_2(test);

    return rc;
}

int Chapter_2_34::Sampler_2_34_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.34.1.Sampler";

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
    ra = clRetainSampler(sampler);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    // Clean-up phase
    if (ar.GetSampler())
        clReleaseSampler(ar.GetSampler());
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_34::empty_2_34_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.34.2.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action
    cl_sampler          sampler = NULL;
    cl_int              ra;
    ra = clRetainSampler(sampler);

    // Check correctness
    if (CL_INVALID_SAMPLER != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
