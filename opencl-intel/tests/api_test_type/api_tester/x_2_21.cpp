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
#include "Chapter_2_21.h"
#include "Arrangement.h"

Chapter_2_21::Chapter_2_21(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_21::~Chapter_2_21()
{
}

int Chapter_2_21::Run(const char* test, int rc)
{
    rc |= context_2_21_1(test);
    rc |= context_2_21_2(test);
    rc |= context_2_21_3(test);
    rc |= empty_2_21_4(test);
    rc |= context_2_21_5(test);
    rc |= context_2_21_6(test);
    rc |= context_2_21_7(test);
    rc |= context_2_21_8(test);
    rc |= context_2_21_9(test);
    rc |= context_2_21_10(test);
    rc |= context_2_21_11(test);

    return rc;
}

int Chapter_2_21::context_2_21_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.1.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int              ra;
    cl_uint             n;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &n);
    if (CL_SUCCESS != ra)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_image_format*    fmt;
    fmt = new cl_image_format[n];
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, n, fmt, &n);
    if (CL_SUCCESS != ra)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_mem              mem;
    char                image[32];      // Should be enouph for all formats
    unsigned            i;
    for (i = 0; i < n; i++)
    {
        mem = clCreateImage3D(cntxt, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, fmt, 1, 1, 2, 0, 0, image, NULL);
        // Check correctness
        if (NULL == mem)
        {
            rc |= PROCESSED_FAIL;
            break;
        }
        clReleaseMemObject(mem);
    }
    delete fmt;
 
    // clean-up phase
    context_cleanup(cntxt);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.2.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_SNORM_INT16;
    fmt.image_channel_order = CL_BGRA;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 2, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_FORMAT_DESCRIPTOR != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.3.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_SNORM_INT16;
    fmt.image_channel_order = CL_RGB;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 2, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_FORMAT_DESCRIPTOR != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::empty_2_21_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.4.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_BGRA;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(NULL, CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 2, 0, 0, image, &ra);

    // Check correctness
    if (CL_INVALID_CONTEXT != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.5.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RA;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 0, 1, 2, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_SIZE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.6.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RA;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 1, 0, 2, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_SIZE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.7.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RA;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 1, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_SIZE != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.8.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RA;
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 2, 0, 0, NULL, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_HOST_PTR != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.9.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_INT8;
    fmt.image_channel_order = CL_RA;
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_COPY_HOST_PTR, &fmt, 1, 1, 2, 0, 0, NULL, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_HOST_PTR != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.10.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_mem              mem;
    cl_image_format     fmt;
    fmt.image_channel_data_type = CL_UNORM_SHORT_565;
    fmt.image_channel_order = CL_INTENSITY;
    char                image[4 * 3 *  2];
    cl_int              ra;
    mem = clCreateImage3D(cntxt, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &fmt, 1, 1, 2, 0, 0, image, &ra);

    // Check correctness
    if (NULL != mem || CL_INVALID_IMAGE_FORMAT_DESCRIPTOR != ra)
        rc |= PROCESSED_FAIL;
 
    // clean-up phase
    cntxt_mem_cleanup(cntxt, mem);

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_21::context_2_21_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.21.11.context";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    cl_context          cntxt = (cl_context)INVALID_VALUE;
     // Arrangement phase
    int                 r = context(&cntxt);
    if (PROCESSED_OK != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);


    // Execute action
    cl_int              ra;
    cl_uint             n;
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, 0, NULL, &n);
    if (CL_SUCCESS != ra)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_image_format*    fmt;
    fmt = new cl_image_format[n];
    ra = clGetSupportedImageFormats(cntxt, CL_MEM_USE_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, n, fmt, &n);
    if (CL_SUCCESS != ra)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    cl_mem              mem;
    char                image[32];      // Should be enough for all formats
    mem = clCreateImage3D(cntxt, CL_MEM_READ_WRITE, &fmt[0], 1, 1, 2, 0, 0, image, &ra);
    // Check correctness
    if (NULL != mem)
        rc = PROCESSED_FAIL;
    else
        clReleaseMemObject(mem);
    if (CL_INVALID_HOST_PTR != ra)
        rc = PROCESSED_FAIL;
    delete fmt;
 
    // clean-up phase
    context_cleanup(cntxt);

    return Finish(&status, TEST_NAME, rc);
}
