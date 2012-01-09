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
#include "Chapter_2_59.h"
#include "Arrangement.h"

Chapter_2_59::Chapter_2_59(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_59::~Chapter_2_59()
{
}

int Chapter_2_59::Run(const char* test, int rc)
{
	rc |= several_2_59_1(test);
	rc |= several_2_59_2(test);
	rc |= several_2_59_3(test);

    return rc;
}

int Chapter_2_59::several_2_59_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.59.1.several";


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
	cl_event			ev;
	cl_int              ra;
	ra = clEnqueueMarker(q, &ev);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (NULL == ev)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_59::several_2_59_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.59.2.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Execute action
	cl_command_queue	q = NULL;
	cl_event			ev;
	cl_int              ra;
	ra = clEnqueueMarker(q, &ev);

	// Check correctness
	if (CL_INVALID_COMMAND_QUEUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_59::several_2_59_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.59.3.several";


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
	cl_int              ra;
	ra = clEnqueueMarker(q, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

