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
#include "Chapter_2_29.h"
#include "Arrangement.h"

Chapter_2_29::Chapter_2_29(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_29::~Chapter_2_29()
{
}

int Chapter_2_29::Run(const char* test, int rc)
{
	rc |= several_2_29_1(test);
	rc |= several_2_29_2(test);
	rc |= several_2_29_3(test);
	rc |= several_2_29_4(test);
	rc |= several_2_29_5(test);
	rc |= several_2_29_6(test);
	rc |= several_2_29_7(test);
	rc |= several_2_29_8(test);
	rc |= several_2_29_9(test);
	rc |= several_2_29_10(test);
	rc |= several_2_29_11(test);
	rc |= several_2_29_12(test);
	rc |= several_2_29_13(test);
	rc |= several_2_29_14(test);
	rc |= several_2_29_15(test);
	rc |= several_2_29_16(test);
	rc |= several_2_29_17(test);
	rc |= several_2_29_18(test);
	rc |= several_2_29_19(test);

    return rc;
}

int Chapter_2_29::several_2_29_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.1.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.2.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_FALSE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.3.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_WRITE, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.4.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_FALSE, 
		CL_MAP_WRITE, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.5.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image3D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.6.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image3D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_FALSE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.7.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image3D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_WRITE, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.8.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_FALSE, 
		CL_MAP_WRITE, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.9.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_event			ev;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		&ev, 
		&ra);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_10(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.10.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_event			ev;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		&ev, 
		&ra);
	if (CL_SUCCESS != ra)
		ptr = clEnqueueMapImage(
			q, 
			image, 
			CL_TRUE, 
			CL_MAP_READ, 
			origin, 
			region, 
			&image_row_pitch, 
			&image_slice_pitch, 
			1, 
			&ev, 
			NULL, 
			&ra);


	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_11(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.11.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		NULL);

	// Check correctness
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_12(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.12.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = NULL;
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_COMMAND_QUEUE != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_13(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.13.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = NULL;
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_MEM_OBJECT != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_14(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.14.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	Arrangement         ar1;
	if (PROCESSED_OK != ar1.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	cl_mem				image = ar1.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_15(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.15.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {1, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_16(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.16.several";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {3, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_17(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.17.several";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 0};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		0, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_29::several_2_29_18(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.18.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		1, 
		NULL, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
int Chapter_2_29::several_2_29_19(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.29.19.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue	q = ar.GetQueue();
	cl_mem				image = ar.GetImage();
	size_t				origin[3] = {0, 0, 0};
	size_t				region[3] = {2, 2, 1};
	size_t				image_row_pitch;
	size_t				image_slice_pitch;
	cl_event			ev = NULL;
	cl_int              ra;
	void*				ptr;
	ptr = clEnqueueMapImage(
		q, 
		image, 
		CL_TRUE, 
		CL_MAP_READ, 
		origin, 
		region, 
		&image_row_pitch, 
		&image_slice_pitch, 
		1, 
		&ev, 
		NULL, 
		&ra);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;
	if (NULL != ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

