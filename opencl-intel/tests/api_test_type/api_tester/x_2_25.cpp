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
#include "Chapter_2_25.h"
#include "Arrangement.h"

Chapter_2_25::Chapter_2_25(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_25::~Chapter_2_25()
{
}

int Chapter_2_25::Run(const char* test, int rc)
{
	rc |= several_2_25_1(test);
	rc |= several_2_25_2(test);
	rc |= several_2_25_3(test);
	rc |= several_2_25_4(test);
	rc |= several_2_25_5(test);
	rc |= several_2_25_6(test);
	rc |= several_2_25_7(test);
	rc |= several_2_25_8(test);
	rc |= several_2_25_9(test);
	rc |= several_2_25_10(test);
	rc |= several_2_25_11(test);
	rc |= several_2_25_12(test);
	rc |= several_2_25_13(test);
	rc |= several_2_25_14(test);
	rc |= several_2_25_15(test);
	rc |= several_2_25_16(test);
	rc |= several_2_25_17(test);
	rc |= several_2_25_18(test);
	rc |= several_2_25_19(test);

    return rc;
}

int Chapter_2_25::several_2_25_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.1.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.2.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage3D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 2, 0, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.3.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage3D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 2, 0, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.4.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage3D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 2, 0, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage3D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 2, 0, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 2};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}


int Chapter_2_25::several_2_25_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.5.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = NULL;
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_COMMAND_QUEUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.6.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	Arrangement         ar1;
	if (PROCESSED_OK != ar1.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	cl_command_queue	q = ar1.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.7.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	Arrangement         ar1;
	if (PROCESSED_OK != ar1.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	cl_context			src_cntxt = ar1.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.8.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	Arrangement         ar1;
	if (PROCESSED_OK != ar1.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	cl_context			dst_cntxt = ar1.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.9.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_RGBA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_IMAGE_FORMAT_MISMATCH != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_10(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.10.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				src_image = NULL;
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_MEM_OBJECT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_11(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.11.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_mem				dst_image = NULL;
	cl_int              ra;
	if (src_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_MEM_OBJECT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_12(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.12.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {2, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_13(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.13.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 2, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_14(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.14.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {3, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_15(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.15.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 2};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_16(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.16.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 1};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_17(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.17.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	cl_event			ev;
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 0, NULL, &ev);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_18(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.18.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 1, NULL, NULL);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_25::several_2_25_19(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.25.19.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.context())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_context			src_cntxt = ar.GetContext();
	cl_mem				src_image;
	cl_image_format     src_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                src_picture[32];
	memset(src_picture, 0xFF, sizeof(src_picture));
	src_image = clCreateImage2D(src_cntxt, CL_MEM_USE_HOST_PTR, &src_fmt, 2, 2, 0, src_picture, NULL);
	cl_context			dst_cntxt = ar.GetContext();
	cl_mem				dst_image;
	cl_image_format     dst_fmt = {CL_BGRA, CL_UNORM_INT8};
	char                dst_picture[32];
	memset(dst_picture, 0x00, sizeof(dst_picture));
	dst_image = clCreateImage2D(dst_cntxt, CL_MEM_USE_HOST_PTR, &dst_fmt, 2, 2, 0, dst_picture, NULL);
	cl_int              ra;
	if (src_image == NULL || dst_image == NULL)
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	size_t				src_origin[3] = {0, 0, 0};
	size_t				dst_origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	cl_event			ev = NULL;
	ra = clEnqueueCopyImage(q, src_image, dst_image, src_origin, dst_origin, region, 1, &ev, NULL);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}