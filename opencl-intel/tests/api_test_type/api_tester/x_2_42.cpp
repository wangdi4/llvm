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
#include "Chapter_2_42.h"
#include "Arrangement.h"

static void nothing(cl_program, void *user_data)
{
}

Chapter_2_42::Chapter_2_42(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_42::~Chapter_2_42()
{
}

int Chapter_2_42::Run(const char* test, int rc)
{
    rc |= program_2_42_1(test);
    rc |= prog_dev_2_42_2(test);
    rc |= prog_dev_2_42_3(test);
    rc |= prog_dev_2_42_4(test);
    rc |= prog_dev_2_42_5(test);
    rc |= prog_dev_2_42_6(test);
    rc |= prog_dev_2_42_7(test);
    rc |= prog_dev_2_42_8(test);
    rc |= prog_dev_2_42_9(test);
    rc |= prog_dev_2_42_10(test);

    return rc;
}

int Chapter_2_42::program_2_42_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.1.program";


    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    cl_program      prog;
    cl_context      cntx;
     // Arrangement phase
    int             r = program(&cntx, &prog);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 0, NULL, sOptions, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.2.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.3.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "-w";
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.4.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "-w";
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.5.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "-w";
    char            user_data[128];
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, user_data);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.6.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(NULL, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_PROGRAM != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.7.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 0, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.8.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 1, NULL, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.9.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "";
    dev = NULL;
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}

int Chapter_2_42::prog_dev_2_42_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.42.10.prog_dev";

    if (Start(test, TEST_NAME))
        return PROCESSED_OK;

    cl_context      cntx;
    cl_program      prog;
    cl_device_id    dev;
     // Arrangement phase
    int             r = prog_dev(&cntx, &prog, &dev);
    if (PROCESSED_OK != r)
        return Finish(TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int          ra;
    char*           sOptions = "INVALID";
    dev = NULL;
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_BUILD_OPTIONS != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    program_cleanup(cntx, prog);

    return Finish(TEST_NAME, rc);
}
