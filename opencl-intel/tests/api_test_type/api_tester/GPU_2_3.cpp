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
#include "Chapter_2_3.h"

int Chapter_2_3::GPU_2_3_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.1.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_device_type  type;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cl_device_type) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_DEVICE_TYPE_GPU != type)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.2.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         vendor;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_VENDOR_ID, sizeof(vendor), &vendor, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(vendor) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.3.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         units;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(units) != rsize)
        rc |= PROCESSED_FAIL;
    if (1 < units)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.4.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         dim;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dim), &dim, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(dim) != rsize)
        rc |= PROCESSED_FAIL;
    if (3 < dim)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.5.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_uint         dim;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(dim), &dim, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t*         w_items;
    w_items = new size_t[dim];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * dim, w_items, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(size_t) * dim != rsize)
        rc |= PROCESSED_FAIL;
    unsigned int             i;
    for (i = 0; i < dim; i++)
    {
        if (0 <= w_items[i])
        {
            rc |= PROCESSED_FAIL;
            break;
        }
    }
    delete w_items;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.6.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          g_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(g_size), &g_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(g_size) != rsize)
        rc |= PROCESSED_FAIL;
    if (1 < g_size)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.7.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_char;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(w_char), &w_char, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_char) != rsize)
        rc |= PROCESSED_FAIL;
    if (1 < w_char)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.8.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_short;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(w_short), &w_short, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_short) != rsize)
        rc |= PROCESSED_FAIL;
    if (2 < w_short)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.9.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_int;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(w_int), &w_int, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_int) != rsize)
        rc |= PROCESSED_FAIL;
    if (4 < w_int)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.10.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_long;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(w_long), &w_long, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_long) != rsize)
        rc |= PROCESSED_FAIL;
    if (4 < w_long)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.11.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_float;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(w_float), &w_float, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_float) != rsize)
        rc |= PROCESSED_FAIL;
    if (4 < w_float)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.12.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         w_double;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(w_double), &w_double, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(w_double) != rsize)
        rc |= PROCESSED_FAIL;
    if (4 < w_double)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.13.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         freq;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(freq), &freq, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(freq) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.14.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bitfield     addr;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_ADDRESS_BITS, sizeof(addr), &addr, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(addr) != rsize)
        rc |= PROCESSED_FAIL;
     if ((addr != 32) && (addr != 64))
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.15.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_ulong        alloc_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(alloc_size), &alloc_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(alloc_size) != rsize)
        rc |= PROCESSED_FAIL;
    // rqst - add checking value has returned

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_16(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.16.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bool         image;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(image) != rsize)
        rc |= PROCESSED_FAIL;
    if (image != CL_TRUE && image != CL_FALSE)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_17(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.17.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         rd_img;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(rd_img), &rd_img, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(rd_img) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 128 > rd_img)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_18(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.18.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         wr_img;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(wr_img), &wr_img, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(wr_img) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 8 > wr_img)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_19(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.19.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          max_2d_w;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(max_2d_w), &max_2d_w, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(max_2d_w) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 8192 > max_2d_w)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_20(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.20.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          max_2d_h;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(max_2d_h), &max_2d_h, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(max_2d_h) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 8192 > max_2d_h)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_21(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.21.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          max_3d_w;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(max_3d_w), &max_3d_w, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(max_3d_w) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 2048 > max_3d_w)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_22(const char* test)
{
    // constants

    const char      TEST_NAME[] = "2.3.22.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          max_3d_h;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(max_3d_h), &max_3d_h, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(max_3d_h) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 2048 > max_3d_h)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_23(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.23.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          max_3d_d;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(max_3d_d), &max_3d_d, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(max_3d_d) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 2048 > max_3d_d)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_24(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.24.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    cl_bool         image;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    r = clGetDeviceInfo(dev, CL_DEVICE_IMAGE_SUPPORT, sizeof(image), &image, NULL);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         samples;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_SAMPLERS, sizeof(samples), &samples, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(samples) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_TRUE == image && 16 > samples)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_25(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.25.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          p_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(p_size), &p_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(p_size) != rsize)
        rc |= PROCESSED_FAIL;
    if (256 > p_size)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_26(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.26.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         align;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(align), &align, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(align) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 == align)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_27(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.27.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         min_align;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(min_align), &min_align, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(min_align) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 == min_align)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_28(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.28.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_device_fp_config         fp_cfg;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(fp_cfg), &fp_cfg, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(fp_cfg) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_FP_DENORM != fp_cfg && CL_FP_INF_NAN != fp_cfg && CL_FP_ROUND_TO_NEAREST != fp_cfg &&
        CL_FP_ROUND_TO_ZERO != fp_cfg && CL_FP_ROUND_TO_INF != fp_cfg && CL_FP_FMA != fp_cfg)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_29(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.29.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_device_mem_cache_type         cache;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cache), &cache, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cache) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_NONE != cache && CL_READ_ONLY_CACHE != cache && CL_READ_WRITE_CACHE != cache)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_30(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.30.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         cache_line;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cache_line), &cache_line, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cache_line) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_31(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.31.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_ulong        cache_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cache_size), &cache_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cache_size) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_32(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.32.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_ulong        mem_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mem_size) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_33(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.33.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_ulong        buf_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(buf_size), &buf_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(buf_size) != rsize)
        rc |= PROCESSED_FAIL;
    if (64 * 1024 > buf_size)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_34(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.34.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         const_arg;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(const_arg), &const_arg, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(const_arg) != rsize)
        rc |= PROCESSED_FAIL;
    if (8 > const_arg)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_35(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.35.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_device_local_mem_type        mem_type;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(mem_type), &mem_type, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mem_type) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_36(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.36.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_ulong        mem_size;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mem_size) != rsize)
        rc |= PROCESSED_FAIL;
    if (16 * 1024 < mem_size)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_37(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.37.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bool         err_corr;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(err_corr), &err_corr, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(err_corr) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_38(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.38.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          timer;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(timer), &timer, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(timer) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_39(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.39.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bool         l_e;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_ENDIAN_LITTLE, sizeof(l_e), &l_e, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(l_e) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_40(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.40.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bool         d_a;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_AVAILABLE, sizeof(d_a), &d_a, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(d_a) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_41(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.41.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_bool         c_a;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_COMPILER_AVAILABLE, sizeof(c_a), &c_a, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(c_a) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_42(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.42.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_device_exec_capabilities         e_c;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(e_c), &e_c, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(e_c) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_EXEC_KERNEL != e_c && CL_EXEC_NATIVE_KERNEL)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_43(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.43.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_command_queue_properties         q_p;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_QUEUE_PROPERTIES, sizeof(q_p), &q_p, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(q_p) != rsize)
        rc |= PROCESSED_FAIL;
    if (!(q_p & CL_QUEUE_PROFILING_ENABLE))
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_44(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.44.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            dev_name[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(dev_name), &dev_name, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_45(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.45.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            vendor_name[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_VENDOR, sizeof(vendor_name), &vendor_name, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_46(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.46.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            drv_ver[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DRIVER_VERSION, sizeof(drv_ver), &drv_ver, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    // Add check drv_ver to be major_number.minor_number

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_47(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.47.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            profile[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_PROFILE, sizeof(profile), &profile, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (0 != strcmp(profile, "FULL_PROFILE") && 0 != strcmp(profile, "EMBEDDED_PROFILE"))
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_48(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.48.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            dev_ver[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_VERSION, sizeof(dev_ver), &dev_ver, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    // Add check dev_ver starts with OpenCL<space>1,0<space>

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_49(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.49.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    char            dev_ver[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_EXTENSIONS, sizeof(dev_ver), &dev_ver, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    // Add check dev_ver starts with OpenCL<space>1,0<space>

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_50(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.50.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    // Execute action
    char            dev_ver[128];
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo((cl_device_id)-1, CL_DEVICE_EXTENSIONS, sizeof(dev_ver), &dev_ver, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_DEVICE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_51(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.51.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         dummy;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 1, &dummy, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_52(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.52.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 0, NULL, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cl_uint) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_3::GPU_2_3_53(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.3.53.GPU";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    cl_device_id    dev;
    // Arrangement phase
    cl_int          r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if (CL_DEVICE_NOT_FOUND == r)
        return Finish(&status, TEST_NAME, PROCESSED_OK);
    if (CL_SUCCESS != r)
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
    
    // Execute action
    cl_uint         dummy;
    size_t          rsize;
    cl_int          ra = clGetDeviceInfo(dev, -1, sizeof(dummy), &dummy, &rsize);

    // Check correctness
    int             rc = PROCESSED_OK;
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
