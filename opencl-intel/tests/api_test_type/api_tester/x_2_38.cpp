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
#include "Chapter_2_38.h"
#include "Arrangement.h"

Chapter_2_38::Chapter_2_38(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_38::~Chapter_2_38()
{
}

int Chapter_2_38::Run(const char* test, int rc)
{
    rc |= BuildProgram_2_38_1(test);
    //rc |= BuildProgram_2_38_2(test);
    rc |= BuildProgram_2_38_3(test);
    rc |= BuildProgram_2_38_4(test);
    rc |= BuildProgram_2_38_5(test);
    rc |= BuildProgram_2_38_6(test);
    rc |= BuildProgram_2_38_7(test);
    rc |= BuildProgram_2_38_8(test);
    rc |= BuildProgram_2_38_9(test);

    return rc;
}


int Chapter_2_38::BuildProgram_2_38_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.1.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, &dev, &binarySize, (const unsigned char **)&binary, NULL, NULL);

    // Check correctness
    if (NULL == prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.2.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        devices[2];
    devices[0] = ar.GetDevice();
    devices[1] = ar.GetDevice();
    size_t              binarySizes[2];
    binarySizes[0] = ar.GetBinarySize();
    binarySizes[1] = ar.GetBinarySize();
    void*               binaries[2];
    binaries[0] = ar.GetBinary();
    binaries[1] = ar.GetBinary();
    cl_program          prog;
    cl_int              rd[2];
    memset(rd, -1, sizeof(rd));
    cl_int              ra;
    prog = clCreateProgramWithBinary(cntxt, 2, devices, binarySizes, (const unsigned char **)binaries, rd, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == prog)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != rd[0])
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != rd[1])
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.3.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    cl_int              rd;
    cl_int              ra;
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(NULL, 1, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.4.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    cl_int              rd;
    cl_int              ra;
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 0, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.5.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    cl_int              rd;
    cl_int              ra;
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, NULL, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.6.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = NULL;
    size_t              binarySize = ar.GetBinarySize();
    cl_int              rd;
    cl_int              ra;
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.7.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_device_id        dev = NULL;
    size_t              binarySize = 0;
    cl_int              rd;
    cl_int              ra;
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_38::BuildProgram_2_38_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.8.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_int              rd;
    cl_int              ra;
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    void*               binary = NULL;
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
    if (NULL != prog)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}



int Chapter_2_38::BuildProgram_2_38_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.38.9.BuildProgram";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    int                 r = ar.BuildProgram();
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_context          cntxt = ar.GetContext();
    cl_int              rd;
    cl_int              ra;
    cl_device_id        dev = ar.GetDevice();
    size_t              binarySize = ar.GetBinarySize();
    void*               binary = ar.GetBinary();
    cl_program          prog;
    prog = clCreateProgramWithBinary(cntxt, 1, &dev, &binarySize, (const unsigned char **)&binary, &rd, &ra);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (NULL == prog)
        rc |= PROCESSED_FAIL;
    if (CL_SUCCESS != rd)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

