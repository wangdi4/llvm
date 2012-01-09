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
#include "Chapter_2_31.h"
#include "Arrangement.h"

Chapter_2_31::Chapter_2_31(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_31::~Chapter_2_31()
{
}

int Chapter_2_31::Run(const char* test, int rc)
{
    rc |= several_2_31_1(test);
    rc |= several_2_31_2(test);
    rc |= several_2_31_3(test);
    rc |= several_2_31_4(test);
    rc |= several_2_31_5(test);
    rc |= several_2_31_6(test);
    rc |= several_2_31_7(test);
    rc |= several_2_31_8(test);
    rc |= several_2_31_9(test);
    rc |= several_2_31_10(test);
    rc |= several_2_31_11(test);
    rc |= empty_2_31_12(test);
    rc |= several_2_31_13(test);
    rc |= several_2_31_14(test);
    rc |= several_2_31_15(test);

    return rc;
}

int Chapter_2_31::several_2_31_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.1.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_TYPE, sizeof(mt), &mt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mt) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_MEM_OBJECT_BUFFER != mt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.2.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              image = ar.GetImage();
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(image, CL_MEM_TYPE, sizeof(mt), &mt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mt) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_MEM_OBJECT_IMAGE2D != mt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.3.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              image = ar.GetImage();
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(image, CL_MEM_TYPE, sizeof(mt), &mt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mt) != rsize)
        rc |= PROCESSED_FAIL;
    if (CL_MEM_OBJECT_IMAGE3D != mt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.4.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_mem_flags        mf;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_FLAGS, sizeof(mf), &mf, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mf) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.5.several";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Image2D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              image = ar.GetImage();
    cl_int              ra;
    cl_mem_flags        mf;
    size_t              rsize;
    ra = clGetMemObjectInfo(image, CL_MEM_FLAGS, sizeof(mf), &mf, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mf) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.6.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Image3D())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              image = ar.GetImage();
    cl_int              ra;
    cl_mem_flags        mf;
    size_t              rsize;
    ra = clGetMemObjectInfo(image, CL_MEM_FLAGS, sizeof(mf), &mf, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(mf) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.7.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    size_t              ms;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(ms), &ms, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(ms) != rsize)
        rc |= PROCESSED_FAIL;
    if (BUFFER_SIZE != ms)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.8.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    void*               ptr;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_HOST_PTR, sizeof(ptr), &ptr, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(ptr) != rsize)
        rc |= PROCESSED_FAIL;
    if (NULL != ptr)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.9.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_uint             cnt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_MAP_COUNT, sizeof(cnt), &cnt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cnt) != rsize)
        rc |= PROCESSED_FAIL;
    if (0 != cnt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.10.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_uint             cnt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_REFERENCE_COUNT, sizeof(cnt), &cnt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cnt) != rsize)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.11.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_context          cntxt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_CONTEXT, sizeof(cntxt), &cntxt, &rsize);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
    if (sizeof(cntxt) != rsize)
        rc |= PROCESSED_FAIL;
    if (ar.GetContext() != cntxt)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::empty_2_31_12(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.12.empty";

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
    // Execute action
    cl_mem              buffer = NULL;
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_TYPE, sizeof(mt), &mt, &rsize);

    // Check correctness
    if (CL_INVALID_MEM_OBJECT != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_13(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.13.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, NULL, sizeof(mt), &mt, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_14(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.14.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    cl_mem_object_type  mt;
    size_t              rsize;
    ra = clGetMemObjectInfo(buffer, CL_MEM_TYPE, 0, &mt, &rsize);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_31::several_2_31_15(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.31.15.several";
    const int       BUFFER_SIZE = 1024;

    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int                 rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_mem              buffer = ar.GetBuffer();
    cl_int              ra;
    ra = clGetMemObjectInfo(buffer, CL_MEM_TYPE, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;

    return Finish(&status, TEST_NAME, rc);
}
