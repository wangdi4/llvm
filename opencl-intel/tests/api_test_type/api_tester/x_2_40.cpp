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
#include "Chapter_2_40.h"
#include "Arrangement.h"

Chapter_2_40::Chapter_2_40(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_40::~Chapter_2_40()
{
}

int Chapter_2_40::Run(const char* test, int rc)
{
    rc |= BuildProgram_2_40_1(test);
    rc |= CreateProgramWithSource_2_40_2(test);
    rc |= empty_2_40_3(test);

    return rc;
}

int Chapter_2_40::BuildProgram_2_40_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.40.1.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program          prog = ar.GetProgram();
    cl_int              ra;
    ra = clReleaseProgram(prog);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_40::CreateProgramWithSource_2_40_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.40.2.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program          prog = ar.GetProgram();
    cl_int              ra;
    ra = clReleaseProgram(prog);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_40::empty_2_40_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.40.3.empty";

        static  tTestStatus status = TEST_UNTOUCHED;if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    cl_int          ra;
    ra = clReleaseProgram(NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_PROGRAM != ra)
        rc |= PROCESSED_FAIL;


    return Finish(&status, TEST_NAME, rc);
}
