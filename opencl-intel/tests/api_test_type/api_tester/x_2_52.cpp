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
#include "Chapter_2_52.h"
#include "Arrangement.h"

Chapter_2_52::Chapter_2_52(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_52::~Chapter_2_52()
{
}

int Chapter_2_52::Run(const char* test, int rc)
{
	rc |= several_2_52_1(test);
	rc |= several_2_52_2(test);
	rc |= several_2_52_3(test);
	rc |= several_2_52_4(test);
	rc |= several_2_52_5(test);
	rc |= several_2_52_6(test);
	rc |= several_2_52_7(test);
	rc |= several_2_52_8(test);
	rc |= several_2_52_9(test);
	rc |= several_2_52_10(test);
	rc |= several_2_52_11(test);
	rc |= several_2_52_12(test);
	rc |= several_2_52_13(test);

    return rc;
}

int Chapter_2_52::several_2_52_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.1.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.2.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_event			ev;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &ev);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.3.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_event			ev;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &ev);
	if (CL_SUCCESS != ra)
		ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 1, &ev, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.4.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = NULL;
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_COMMAND_QUEUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.5.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = NULL;
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_KERNEL != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.6.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Kernel())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_KERNEL_ARGS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.7.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	Arrangement         ar1;
	if (PROCESSED_OK != ar1.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	cl_kernel			kernel = ar1.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_CONTEXT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.8.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 0, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_WORK_DIMENSION != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.9.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 4, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_WORK_DIMENSION != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_10(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.10.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_int              ra;
	size_t				offset = NULL;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, &offset, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_GLOBAL_OFFSET != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_11(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.11.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, NULL, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_GLOBAL_WORK_SIZE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_12(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.12.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 0;
	size_t				local_work_size = 1;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_GLOBAL_WORK_SIZE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_52::several_2_52_13(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.52.13.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.KernelReady())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_kernel			kernel = ar.GetKernel();
	size_t				global_work_size = 1;
	size_t				local_work_size = 1;
	cl_event			ev = NULL;
	cl_int              ra;
	ra = clEnqueueNDRangeKernel(q, kernel, 1, NULL, &global_work_size, &local_work_size, 1, &ev, NULL);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
