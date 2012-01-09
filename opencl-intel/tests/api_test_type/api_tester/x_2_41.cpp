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
#include "Chapter_2_41.h"
#include "Arrangement.h"

bool bBuildFinished = false;

static void CL_CALLBACK nothing(cl_program, void *user_data)
{
	bBuildFinished = true;
}

Chapter_2_41::Chapter_2_41(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_41::~Chapter_2_41()
{
}

int Chapter_2_41::Run(const char* test, int rc)
{
    rc |= CreateProgramWithSource_2_41_1(test);
    rc |= CreateProgramWithSource_2_41_2(test);
    //rc |= CreateProgramWithSource_2_41_3(test);
    //rc |= CreateProgramWithSource_2_41_4(test);
    //rc |= CreateProgramWithSource_2_41_5(test);
    rc |= CreateProgramWithSource_2_41_6(test);
    rc |= CreateProgramWithSource_2_41_7(test);
    rc |= CreateProgramWithSource_2_41_8(test);
    rc |= CreateProgramWithSource_2_41_9(test);
    //rc |= CreateProgramWithSource_2_41_10(test);

    return rc;
}

int Chapter_2_41::CreateProgramWithSource_2_41_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.1.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 0, NULL, sOptions, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.2.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.3.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "-w";
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.4.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "-w";
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, NULL);

	while( false == bBuildFinished );
	bBuildFinished = false;

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.5.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "-w";
    char            user_data[128];
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, user_data);

	while( false == bBuildFinished );
	bBuildFinished = false;

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.6.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(NULL, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_PROGRAM != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.7.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 0, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.8.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "";
    ra = clBuildProgram(prog, 1, NULL, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.9.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "";
    dev = NULL;
    ra = clBuildProgram(prog, 1, &dev, sOptions, NULL, NULL);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_41::CreateProgramWithSource_2_41_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.41.10.CreateProgramWithSource";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Arrangement phase
    Arrangement     ar;
    int             r = ar.CreateProgramWithSource();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	r = ar.cntxt_dev();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_device_id    dev = ar.GetDevice();
    cl_int          ra;
    char*           sOptions = "INVALID";
    ra = clBuildProgram(prog, 1, &dev, sOptions, nothing, NULL);

	while( false == bBuildFinished );
	bBuildFinished = false;

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_BUILD_OPTIONS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}
