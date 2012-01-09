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
#include "Chapter_2_56.h"
#include "Arrangement.h"

static void native(void* ptr)
{
}

Chapter_2_56::Chapter_2_56(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_56::~Chapter_2_56()
{
}

int Chapter_2_56::Run(const char* test, int rc)
{
	rc |= several_2_56_1(test);
	rc |= several_2_56_2(test);
	rc |= several_2_56_3(test);
	rc |= several_2_56_4(test);
	rc |= several_2_56_5(test);
	rc |= several_2_56_6(test);
	rc |= several_2_56_7(test);
	rc |= several_2_56_8(test);
	rc |= several_2_56_9(test);

    return rc;
}

int Chapter_2_56::several_2_56_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.1.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_queue	q;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_QUEUE, sizeof(q), &q, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(q) != rsize)
		rc |= PROCESSED_FAIL;
	if (ar.GetQueue() != q)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.2.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_type		cmd;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_TYPE, sizeof(cmd), &cmd, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(cmd) != rsize)
		rc |= PROCESSED_FAIL;
	if (CL_COMMAND_TASK != cmd)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.3.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_int				es;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(es), &es, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(es) != rsize)
		rc |= PROCESSED_FAIL;
	if (CL_QUEUED != es &&
		CL_SUBMITTED != es &&
		CL_RUNNING != es &&
		CL_COMPLETE != es)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.4.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_uint				cnt;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_REFERENCE_COUNT, sizeof(cnt), &cnt, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(cnt) != rsize)
		rc |= PROCESSED_FAIL;
	if (0 == cnt)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.5.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Execute action
	cl_event			ev = NULL;
	cl_command_queue	q;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_QUEUE, sizeof(q), &q, &rsize);

	// Check correctness
	if (CL_INVALID_EVENT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.6.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_queue	q;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, NULL, sizeof(q), &q, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.7.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_queue	q;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_QUEUE, 0, &q, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.8.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_queue	q;
	size_t				rsize;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_QUEUE, sizeof(q), NULL, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_56::several_2_56_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.56.9.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_event			ev = ar.GetEvent();
	cl_command_queue	q;
	cl_int              ra;
	ra = clGetEventInfo(ev, CL_EVENT_COMMAND_QUEUE, sizeof(q), &q, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
