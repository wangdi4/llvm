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
#include "Chapter_2_54.h"
#include "Arrangement.h"

static void native(void* ptr)
{
}

Chapter_2_54::Chapter_2_54(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_54::~Chapter_2_54()
{
}

int Chapter_2_54::Run(const char* test, int rc)
{
    //rc |= several_2_54_1(test);
    //rc |= several_2_54_2(test);
    rc |= several_2_54_3(test);
	//rc |= several_2_54_4(test);
	//rc |= several_2_54_5(test);
	rc |= several_2_54_6(test);
	rc |= several_2_54_7(test);
	rc |= several_2_54_8(test);
	rc |= several_2_54_9(test);
	rc |= several_2_54_10(test);
	rc |= several_2_54_11(test);
	//rc |= several_2_54_12(test);
	//rc |= several_2_54_13(test);
	//rc |= several_2_54_14(test);
	//rc |= several_2_54_15(test);

    return rc;
}

int Chapter_2_54::several_2_54_1(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.1.several";


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
    cl_int              ra;
	int					nothing;
    ra = clEnqueueNativeKernel(q, &native, &nothing, sizeof(nothing), 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_2(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.2.several";


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
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, NULL, 0, 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_3(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.3.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(sizeof(cl_mem)))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
	cl_mem				buffer = ar.GetBuffer();
	int					arg = NULL;
	void*				place = &arg;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 1, &buffer, (const void **)&place, 0, NULL, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_4(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.4.several";


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
	int					arg = 0;
	cl_event			ev;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 0, NULL, &ev);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_5(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.5.several";


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
	int					arg = 0;
	cl_event			ev;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 0, NULL, &ev);
    if (CL_SUCCESS != ra)
        ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 1, &ev, NULL);

    // Check correctness
    if (CL_SUCCESS != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_6(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.6.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = NULL;
	int					arg = 0;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_COMMAND_QUEUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_7(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.7.several";


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
	int					arg = 0;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, NULL, &arg, sizeof(arg), 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_8(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.8.several";


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
    cl_int              ra;
	int					nothing;
    ra = clEnqueueNativeKernel(q, &native, NULL, sizeof(nothing), 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_9(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.9.several";


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
    cl_int              ra;
	int					nothing;
    ra = clEnqueueNativeKernel(q, &native, &nothing, 0, 0, NULL, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_10(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.10.several";


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
	int					arg = 0;
	void*				place = &arg;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 1, NULL, (const void **)&place, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_11(const char* test)
{
    // constants
    const char      TEST_NAME[] = "2.54.11.several";


    static  tTestStatus status = TEST_UNTOUCHED;
    if (Start(&status, TEST_NAME))
        return PROCESSED_OK;

    int             rc = PROCESSED_OK;
     // Arrangement phase
    Arrangement         ar;
    if (PROCESSED_OK != ar.Queue())
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(sizeof(cl_mem)))
        return Finish(&status, TEST_NAME, PROCESSED_FAIL);

    // Execute action
    cl_command_queue    q = ar.GetQueue();
	cl_mem				buffer = ar.GetBuffer();
	int					arg = 0;
    cl_int              ra;
    ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 1, &buffer, NULL, 0, NULL, NULL);

    // Check correctness
    if (CL_INVALID_VALUE != ra)
        rc |= PROCESSED_FAIL;
 
    return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_12(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.54.12.several";


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
	cl_mem				buffer = NULL;
	int					arg = 0;
	void*				place = &arg;
	cl_int              ra;
	ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 1, &buffer, (const void **)&place, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_13(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.54.13.several";


	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int             rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Queue())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);
	if (PROCESSED_OK != ar.Buffer(sizeof(cl_mem)))
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_command_queue    q = ar.GetQueue();
	cl_mem				buffer = ar.GetBuffer();
	void*				place = NULL;
	int					arg = 0;
	cl_int              ra;
	ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 1, &buffer, (const void **)&place, 0, NULL, NULL);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_14(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.54.14.several";


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
	int					arg = 0;
	cl_int              ra;
	ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 1, NULL, NULL);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_54::several_2_54_15(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.54.15.several";


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
	int					arg = 0;
	cl_event			ev = NULL;
	cl_int              ra;
	ra = clEnqueueNativeKernel(q, &native, &arg, sizeof(arg), 0, NULL, NULL, 1, &ev, NULL);

	// Check correctness
	if (CL_INVALID_EVENT_WAIT_LIST != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
