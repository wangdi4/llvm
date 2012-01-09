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
#include "Chapter_2_62.h"
#include "Arrangement.h"

Chapter_2_62::Chapter_2_62(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_62::~Chapter_2_62()
{
}

int Chapter_2_62::Run(const char* test, int rc)
{
	rc |= program_2_62_1(test);
	rc |= program_2_62_2(test);
	rc |= program_2_62_3(test);
	rc |= program_2_62_4(test);
	rc |= program_2_62_5(test);
	rc |= program_2_62_6(test);
	rc |= program_2_62_7(test);
	rc |= program_2_62_8(test);
	rc |= program_2_62_9(test);

	return rc;
}

int Chapter_2_62::program_2_62_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.1.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		queued;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_QUEUED, sizeof(queued), &queued, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;
	if (CL_SUCCESS == ra && sizeof(queued) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.2.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		submit;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_SUBMIT, sizeof(submit), &submit, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;
	if (CL_SUCCESS == ra && sizeof(submit) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.3.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		start;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(start), &start, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;
	if (CL_SUCCESS == ra && sizeof(start) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.4.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		end;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(end), &end, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;
	if (CL_SUCCESS == ra && sizeof(end) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.5.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		queued;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_QUEUED, 0, NULL, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;
	if (CL_SUCCESS == ra && sizeof(queued) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.6.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		queued;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_QUEUED, sizeof(queued), &queued, NULL);

	// Check correctness
	if (CL_SUCCESS != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.7.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Execute action
	cl_int          ra;
	cl_event	    ev = NULL;
	cl_ulong		queued;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_QUEUED, sizeof(queued), &queued, NULL);

	// Check correctness
	if (CL_INVALID_EVENT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.8.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		queued;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, NULL, sizeof(queued), &queued, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_62::program_2_62_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.62.9.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Event())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_event	    ev = ar.GetEvent();
	cl_ulong		queued;
	size_t          rsize;
	ra = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_QUEUED, sizeof(queued) - 1, &queued, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra && CL_PROFILING_INFO_NOT_AVAILABLE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
