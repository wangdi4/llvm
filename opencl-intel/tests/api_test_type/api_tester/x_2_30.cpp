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
#include "Chapter_2_30.h"
#include "Arrangement.h"

Chapter_2_30::Chapter_2_30(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_30::~Chapter_2_30()
{
}

int Chapter_2_30::Run(const char* test, int rc)
{
	rc |= several_2_30_1(test);
	rc |= several_2_30_2(test);
	rc |= several_2_30_3(test);
	rc |= several_2_30_4(test);
	rc |= several_2_30_5(test);
	rc |= several_2_30_6(test);
	rc |= several_2_30_7(test);
	rc |= several_2_30_8(test);
	rc |= several_2_30_9(test);
	rc |= several_2_30_10(test);
	rc |= several_2_30_11(test);

    return rc;
}

int Chapter_2_30::several_2_30_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.1.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.2.several";

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
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              image = ar.GetImage();
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
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, image, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.3.several";
	const int       BUFFER_SIZE = 1024;

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
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              image = ar.GetImage();
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
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, image, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.4.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_event			ev;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, &ev);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.5.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	cl_event			ev;
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		&ev,
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 1, &ev, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.6.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	q = NULL;
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_COMMAND_QUEUE != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.7.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	buffer = NULL;
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_MEM_OBJECT != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.8.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	ptr = NULL;
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.9.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	if (CL_SUCCESS == ra)
	{
		Arrangement         ar;
		if (PROCESSED_OK != ar.Queue())
			return Finish(&status, TEST_NAME, PROCESSED_FAIL);
		cl_command_queue    q = ar.GetQueue();
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 0, NULL, NULL);
	}
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_10(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.10.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 1, NULL, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_30::several_2_30_11(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.30.11.several";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(BUFFER_SIZE))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int              ra;
	cl_command_queue    q = ar.GetQueue();
	cl_mem              buffer = ar.GetBuffer();
	void*               ptr;
	cl_event			ev = NULL;
	ptr = clEnqueueMapBuffer(
		q, 
		buffer, 
		CL_TRUE, 
		CL_MAP_READ, 
		0, 
		BUFFER_SIZE, 
		0, 
		NULL,
		NULL,
		&ra);
	if (CL_SUCCESS == ra)
		ra = clEnqueueUnmapMemObject(q, buffer, ptr, 1, &ev, NULL);
	else
		rc |= PROCESSED_FAIL;

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ptr)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
