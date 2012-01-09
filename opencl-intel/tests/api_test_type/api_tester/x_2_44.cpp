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
#include "Chapter_2_44.h"
#include "Arrangement.h"

Chapter_2_44::Chapter_2_44(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_44::~Chapter_2_44()
{
}

int Chapter_2_44::Run(const char* test, int rc)
{
	rc |= program_2_44_1(test);
	rc |= program_2_44_2(test);
	rc |= program_2_44_3(test);
	rc |= program_2_44_4(test);
	rc |= program_2_44_5(test);
	rc |= program_2_44_6(test);
	rc |= program_2_44_7(test);
	rc |= program_2_44_8(test);
	rc |= program_2_44_9(test);

	return rc;
}

int Chapter_2_44::program_2_44_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.1.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	cl_build_status	bs;
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, sizeof(bs), &bs, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(bs) != rsize)
		rc |= PROCESSED_FAIL;
	if (CL_BUILD_SUCCESS != bs)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.2.program";
	const int       BUFFER_SIZE = 1024;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	char			co[BUFFER_SIZE];
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_OPTIONS, sizeof(co), &co, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.3.program";
	const int       BUFFER_SIZE = 16384;

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	char			bl[BUFFER_SIZE];
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, sizeof(bl), &bl, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.4.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, 0, NULL, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(cl_build_status) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.5.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	cl_build_status	bs;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, sizeof(bs), &bs, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (CL_BUILD_SUCCESS != bs)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.6.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = NULL;
	cl_device_id	dev = ar.GetDevice();
	cl_build_status	bs;
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, sizeof(bs), &bs, &rsize);

	// Check correctness
	if (CL_INVALID_PROGRAM != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.7.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = NULL;
	cl_build_status	bs;
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, sizeof(bs), &bs, &rsize);

	// Check correctness
	if (CL_INVALID_DEVICE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.8.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	cl_build_status	bs;
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, NULL, sizeof(bs), &bs, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_44::program_2_44_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.44.9.program";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.cntxt_dev())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.BuildProgram())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_int          ra;
	cl_program      prog = ar.GetProgram();
	cl_device_id	dev = ar.GetDevice();
	cl_build_status	bs;
	size_t          rsize;
	ra = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_STATUS, sizeof(bs) - 1, &bs, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
