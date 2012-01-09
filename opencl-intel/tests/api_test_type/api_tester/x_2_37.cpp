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
#include "Chapter_2_37.h"

static const char*     ssProgram[] =
{
    "int main(int argv, char* argv[])",
    "{",
    "    return 0;",
    "}"
};
static const cl_int    count = 4;
static const size_t    lines[] = {32, 1, 13, 1};


Chapter_2_37::Chapter_2_37(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_37::~Chapter_2_37()
{
}

int Chapter_2_37::Run(const char* test, int rc)
{
    rc |= context_2_37_1(test);
    rc |= context_2_37_2(test);
    rc |= context_2_37_3(test);
    rc |= empty_2_37_4(test);
    rc |= context_2_37_5(test);
    rc |= context_2_37_6(test);

    return rc;
}

int Chapter_2_37::context_2_37_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.1.context";

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
    cl_int              ra;
    cl_program          prog;
    prog = clCreateProgramWithSource(cntxt,
        count, ssProgram, lines, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (prog == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_37::context_2_37_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.2.context";

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
    cl_int              ra;
    cl_program          prog;
    prog = clCreateProgramWithSource(cntxt,
        count, ssProgram, NULL, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (prog == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_37::context_2_37_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.3.context";

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
    cl_int              ra;
    size_t              lines[count];

    int                 i;
    for (i = 0;
        i < count; i++)
        lines[i] = 0;
    cl_program          prog;
    prog = clCreateProgramWithSource(cntxt,
        count, ssProgram, lines, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (prog == NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_37::empty_2_37_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Execute action
    cl_int              ra;
    cl_program          prog;
    prog = clCreateProgramWithSource((cl_context)NULL, count, ssProgram, lines, &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
    if (prog != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_37::context_2_37_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.5.context";

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
    cl_int              ra;
    const char*         ssProgram[count];
    int                 i;
    for (i = 0; i < count; i++)
        ssProgram[i] = NULL;
    cl_program          prog;
    prog = clCreateProgramWithSource(cntxt, count, ssProgram, NULL, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (prog != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_37::context_2_37_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.37.6.context";

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
    cl_int              ra;
    cl_program          prog;
    prog = clCreateProgramWithSource(cntxt, 0, ssProgram, lines, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (prog != NULL)
        rc |= PROCESSED_FAIL;

    // clean-up phase
    if (cntxt)
        clReleaseContext(cntxt);
    if (prog)
        clReleaseProgram(prog);

    return Finish(&status, TEST_NAME, rc);
}

