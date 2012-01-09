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
#include "Chapter_2_32.h"
#include "Arrangement.h"

Chapter_2_32::Chapter_2_32(tDiscrete* pDiscrete):
    Helper(pDiscrete)
{
}

Chapter_2_32::~Chapter_2_32()
{
}

int Chapter_2_32::Run(const char* test, int rc)
{
	rc |= several_2_32_1(test);
	rc |= several_2_32_2(test);
	rc |= several_2_32_3(test);
	rc |= several_2_32_4(test);
	rc |= several_2_32_5(test);
	rc |= several_2_32_6(test);
	rc |= several_2_32_7(test);
	rc |= several_2_32_8(test);
	rc |= several_2_32_9(test);
	rc |= several_2_32_10(test);
	rc |= several_2_32_11(test);
	rc |= several_2_32_12(test);
	rc |= several_2_32_13(test);
	rc |= several_2_32_14(test);
	rc |= several_2_32_15(test);
	rc |= several_2_32_16(test);
	rc |= several_2_32_17(test);
	rc |= several_2_32_18(test);
	rc |= several_2_32_19(test);

    return rc;
}

int Chapter_2_32::several_2_32_1(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.1.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(fmt), &fmt, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(fmt) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_2(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.2.several";

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
	size_t				el_size;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_ELEMENT_SIZE, sizeof(el_size), &el_size, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(el_size) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_3(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.3.several";

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
	size_t				rp;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_ROW_PITCH, sizeof(rp), &rp, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(rp) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_4(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.4.several";

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
	size_t				sp;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_SLICE_PITCH, sizeof(sp), &sp, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(sp) != rsize)
		rc |= PROCESSED_FAIL;
	if (0 != sp)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_5(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.5.several";

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
	size_t				iw;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(iw), &iw, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(iw) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_6(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.6.several";

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
	size_t				ih;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(ih), &ih, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(ih) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_7(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.7.several";

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
	size_t				id;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_DEPTH, sizeof(id), &id, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(id) != rsize)
		rc |= PROCESSED_FAIL;
	if (0 != id)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_8(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.8.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(fmt), &fmt, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(fmt) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_9(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.9.several";

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
	size_t				el_size;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_ELEMENT_SIZE, sizeof(el_size), &el_size, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(el_size) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_10(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.10.several";

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
	size_t				rp;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_ROW_PITCH, sizeof(rp), &rp, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(rp) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_11(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.11.several";

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
	size_t				sp;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_SLICE_PITCH, sizeof(sp), &sp, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(sp) != rsize)
		rc |= PROCESSED_FAIL;
	if (0 == sp)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_12(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.12.several";

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
	size_t				iw;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(iw), &iw, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(iw) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_13(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.13.several";

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
	size_t				ih;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(ih), &ih, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(ih) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_14(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.14.several";

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
	size_t				id;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_DEPTH, sizeof(id), &id, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(id) != rsize)
		rc |= PROCESSED_FAIL;
	if (0 == id)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_15(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.15.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, 0, NULL, &rsize);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;
	if (sizeof(fmt) != rsize)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_16(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.16.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(fmt), &fmt, NULL);

	// Check correctness
	if (CL_SUCCESS != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_17(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.17.several";

	static  tTestStatus status = TEST_UNTOUCHED;
	if (Start(&status, TEST_NAME))
		return PROCESSED_OK;

	int                 rc = PROCESSED_OK;
	// Arrangement phase
	Arrangement         ar;
	if (PROCESSED_OK != ar.Image2D())
		return Finish(&status, TEST_NAME, PROCESSED_FAIL);

	// Execute action
	cl_mem              image = NULL;
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(fmt), &fmt, &rsize);

	// Check correctness
	if (CL_INVALID_MEM_OBJECT != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_18(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.18.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, NULL, sizeof(fmt), &fmt, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}

int Chapter_2_32::several_2_32_19(const char* test)
{
	// constants
	const char      TEST_NAME[] = "2.32.19.several";

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
	cl_image_format		fmt;
	cl_int              ra;
	size_t				rsize;
	ra = clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(fmt) - 1, &fmt, &rsize);

	// Check correctness
	if (CL_INVALID_VALUE != ra)
		rc |= PROCESSED_FAIL;

	return Finish(&status, TEST_NAME, rc);
}
