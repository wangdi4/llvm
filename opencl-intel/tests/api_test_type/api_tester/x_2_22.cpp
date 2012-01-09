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
#include "Chapter_2_22.h"
#include "Arrangement.h"

Chapter_2_22::Chapter_2_22(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_22::~Chapter_2_22()
{
}

int Chapter_2_22::Run(const char* test, int rc)
{
    rc |= context_2_22_1(test);
    rc |= context_2_22_2(test);
    rc |= context_2_22_3(test);
    rc |= context_2_22_4(test);
    rc |= context_2_22_5(test);
    rc |= empty_2_22_6(test);
    rc |= context_2_22_7(test);
    rc |= context_2_22_8(test);
    rc |= context_2_22_9(test);

    return rc;
}

int Chapter_2_22::context_2_22_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.1.context";

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
    cl_image_format     fmt[1];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 1, fmt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.2.context";

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
    cl_image_format     fmt[128];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 128, fmt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.3.context";

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
    cl_image_format     fmt[1];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE3D, 1, fmt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.4.context";

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
    cl_image_format     fmt[128];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE3D, 128, fmt, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.5.context";

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
    cl_uint             n;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &n);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (1 > n)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::empty_2_22_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.6.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action
    cl_context          cntxt = NULL;
    cl_image_format     fmt[1];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 1, fmt, NULL);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.7.context";

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
    cl_image_format     fmt[1];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, 0, CL_MEM_OBJECT_IMAGE2D, 1, fmt, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.8.context";

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
    cl_image_format     fmt[1];
    cl_int              ra;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, 0, 1, fmt, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_22::context_2_22_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.22.9.context";

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
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 1, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}
