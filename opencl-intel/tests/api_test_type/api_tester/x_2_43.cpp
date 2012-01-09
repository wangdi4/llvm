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

#include "Chapter_2_43.h"
#include "Arrangement.h"


Chapter_2_43::Chapter_2_43(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_43::~Chapter_2_43()
{
}

int Chapter_2_43::Run(const char* test, int rc)
{
    rc |= CreateProgramWithSource_2_43_1(test);
    rc |= CreateProgramWithSource_2_43_2(test);
    rc |= CreateProgramWithSource_2_43_3(test);
    rc |= CreateProgramWithSource_2_43_4(test);
    rc |= CreateProgramWithSource_2_43_5(test);
    rc |= BuildProgram_2_43_6(test);
    rc |= BuildProgram_2_43_7(test);
    rc |= empty_2_43_8(test);
    rc |= BuildProgram_2_43_9(test);

    return rc;
}

int Chapter_2_43::CreateProgramWithSource_2_43_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.1.CreateProgramWithSource";

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
    cl_uint         count;
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_REFERENCE_COUNT, sizeof(count), &count, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(count) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 >= count)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::CreateProgramWithSource_2_43_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.2.CreateProgramWithSource";

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
    cl_context      c;
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_CONTEXT, sizeof(c), &c, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(c) != rsize)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != c)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::CreateProgramWithSource_2_43_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.3.CreateProgramWithSource";

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
    cl_uint         n;
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_NUM_DEVICES, sizeof(n), &n, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(n) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 == n)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::CreateProgramWithSource_2_43_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.4.CreateProgramWithSource";

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
    cl_device_id    dev;
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_DEVICES, sizeof(dev), &dev, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(dev) != rsize)
        rc |= PROCESSED_FAIL;
    if (NULL == dev)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::CreateProgramWithSource_2_43_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.5.CreateProgramWithSource";
    const int       SRC_SIZE = 0x1000;

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
    char            src[SRC_SIZE];
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_SOURCE, SRC_SIZE, src, &rsize);

	ar.finish();

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::BuildProgram_2_43_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.6.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_int          ra;
    size_t          bin_size;
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES, sizeof(bin_size), &bin_size, &rsize);

	ar.finish();

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 == bin_size)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::BuildProgram_2_43_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.7.BuildProgram";
    const int       BIN_SIZE = 0x40000000;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_int          ra;
    char*           bin;
    bin = new char[BIN_SIZE];
    size_t          rsize;
    ra = clGetProgramInfo(prog, CL_PROGRAM_BINARIES, BIN_SIZE, &bin, &rsize);
    delete bin;

	ar.finish();

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::empty_2_43_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.8.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;

    // Execute action
    cl_int          ra;
    cl_uint         n;
    size_t          rsize;
    ra = clGetProgramInfo(NULL, CL_PROGRAM_NUM_DEVICES, sizeof(n), &n, &rsize);

    // Check correctness
    if (CL_INVALID_PROGRAM != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_43::BuildProgram_2_43_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.43.9.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
    // Arrangement phase
    Arrangement     ar;
    int             r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_program      prog = ar.GetProgram();
    cl_int          ra;
    cl_context      c;
    size_t          rsize;
    ra = clGetProgramInfo(prog, -1, sizeof(c), &c, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
