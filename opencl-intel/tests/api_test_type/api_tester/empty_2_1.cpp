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
#include "Chapter_2_1.h"
#include "CL\cl.h"

#define MAX(a, b) (a > b ? a : b)

Chapter_2_1::Chapter_2_1(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_1::~Chapter_2_1()
{
}

int Chapter_2_1::Run(const char* test, int rc)
{
    rc |= GetPlatformInfo_2_1_1(test);
    rc |= GetPlatformInfo_2_1_2(test);
    rc |= GetPlatformInfo_2_1_3(test);
    rc |= GetPlatformInfo_2_1_4(test);
    rc |= GetPlatformInfo_2_1_5(test);
#ifdef  PLATFORM
    rc |= GetPlatformInfo_2_1_6(test);
    rc |= GetPlatformIDs_2_1_1(test);
    rc |= GetPlatformIDs_2_1_2(test);
    rc |= GetPlatformIDs_2_1_3(test);
    rc |= GetPlatformIDs_2_1_4(test);
    rc |= GetPlatformIDs_2_1_5(test);
#endif

    return rc;
}

int Chapter_2_1::GetPlatformInfo_2_1_1(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.1.GetPlatformInfo";
    const char  FULL_TEXT[] = "FULL_PROFILE";
    const int   FULL_LENGTH = (sizeof(FULL_TEXT));
    const char  EMBEDDED_TEXT[] = "EMBEDDED_PROFILE";
    const int   EMBEDDED_LENGTH = (sizeof(EMBEDDED_TEXT));
    const int   BUF_LENGHT = (MAX(FULL_LENGTH, EMBEDDED_LENGTH));


    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    char        sProfile[BUF_LENGHT];
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(NULL, CL_PLATFORM_PROFILE, BUF_LENGHT, sProfile, &rsize);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (!(0 != strcmp(sProfile, FULL_TEXT) || (FULL_LENGTH + 1) != rsize)        ||
        !(0 != strcmp(sProfile, EMBEDDED_TEXT) || (EMBEDDED_LENGTH + 1) != rsize)
       )
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformInfo_2_1_2(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.2.GetPlatformInfo";
    const char  VER_TEXT[] = "OpenCL 1.0 ";
    const int   VER_LENGTH = sizeof(VER_TEXT) - 1;
    const int   BUF_LENGHT = 128;

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    char        sVer[BUF_LENGHT];
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(NULL, CL_PLATFORM_VERSION, BUF_LENGHT, sVer, &rsize);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 != strncmp(sVer, VER_TEXT, VER_LENGTH))
        rc |= PROCESSED_FAIL;
    if (sizeof(VER_TEXT) >= rsize || strlen(sVer) < sizeof(VER_TEXT))
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformInfo_2_1_3(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.3.GetPlatformInfo";
    const int   BUF_LENGHT = 128;

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    char*       sVer[BUF_LENGHT];
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(NULL, -1, BUF_LENGHT, sVer, &rsize);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformInfo_2_1_4(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.4.GetPlatformInfo";
    const int   BUF_LENGHT = 128;

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(NULL, CL_PLATFORM_VERSION, BUF_LENGHT, NULL, &rsize);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformInfo_2_1_5(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.5.GetPlatformInfo";
    const int   BUF_LENGHT = 128;

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    char*       sVer[BUF_LENGHT];
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(NULL, CL_PLATFORM_VERSION, 5, sVer, &rsize);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformInfo_2_1_6(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.6.GetPlatformInfo";
    const char  FULL_TEXT[] = "FULL_PROFILE ";
    const int   FULL_LENGTH = (sizeof(FULL_TEXT));
    const char  EMBEDDED_TEXT[] = "EMBEDDED_PROFILE";
    const int   EMBEDDED_LENGTH = (sizeof(EMBEDDED_TEXT));
    const int   BUF_LENGHT = (MAX(FULL_LENGTH, EMBEDDED_LENGTH));


    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    int         rc = PROCESSED_OK;
    // Arrangement phase
    cl_platform_id  platform[1];
    cl_int          r = clGetPlatformIDs(sizeof(platform) / sizeof(platform[0]), platform, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, r);

    // Execute action
    char        sProfile[BUF_LENGHT];
    size_t      rsize;
    cl_int      ra = clGetPlatformInfo(platform[0], CL_PLATFORM_PROFILE, BUF_LENGHT, sProfile, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (!(0 != strcmp(sProfile, FULL_TEXT) || (FULL_LENGTH + 1) != rsize)        ||
        !(0 != strcmp(sProfile, EMBEDDED_TEXT) || (EMBEDDED_LENGTH + 1) != rsize)
       )
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformIDs_2_1_1(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.1.GetPlatformIDs";

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_platform_id  platform[1];
    cl_int          ra = clGetPlatformIDs(sizeof(platform) / sizeof(platform[0]), platform, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformIDs_2_1_2(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.2.GetPlatformIDs";

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         n;
    cl_int          ra = clGetPlatformIDs(0, NULL, &n);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > n)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformIDs_2_1_3(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.3.GetPlatformIDs";

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_uint         n;
    cl_platform_id  platform[128];
    cl_int          ra = clGetPlatformIDs(sizeof(platform) / sizeof(platform[0]), platform, &n);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > n)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformIDs_2_1_4(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.4.GetPlatformIDs";

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_int          ra = clGetPlatformIDs(1, NULL, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_1::GetPlatformIDs_2_1_5(const char* test)
{
    // constants
    const char  TEST_NAME[] = "2.1.5.GetPlatformIDs";

    static tTestStatus    status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;
    
    // Execute action
    cl_platform_id  platform[1];
    cl_int          ra = clGetPlatformIDs(0, platform, NULL);

    // Check correctness
    int         rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}