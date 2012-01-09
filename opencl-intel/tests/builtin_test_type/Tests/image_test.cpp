#include "image_test.h"
#include "cl_image_declaration.h"
#include "cl_types.h"
#include <math.h>

// Registers the fixture into the 'registry'
//CPPUNIT_TEST_SUITE_REGISTRATION( ImageTest );

#define WIDTH	10
#define HEIGHT	5
#define DEPTH	3

void ImageTest::setUp()
{
}


void ImageTest::tearDown()
{
}

void ImageTest::__read_2d_uii_1u8_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) unsigned char	imData[HEIGHT][WIDTH];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j] = (rVal % 256) - 128;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH;
	image.format.image_channel_order = CLK_LUMINANCE;
	image.format.image_channel_data_type = CLK_UNSIGNED_INT8;
	image.pData = imData;
	image.uiElementSize = 1;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			_4i32 res = read_2d_uii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8)[0]",
				imData[i][j], (unsigned char)_mm_cvtsi128_si32(res));
			res = _mm_srli_si128(res, 4);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8)[1]",
				imData[i][j], (unsigned char)_mm_cvtsi128_si32(res));
			res = _mm_srli_si128(res, 4);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8)[2]",
				imData[i][j], (unsigned char)_mm_cvtsi128_si32(res));
			res = _mm_srli_si128(res, 4);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8)[3]",
				(unsigned char)1, (unsigned char)_mm_cvtsi128_si32(res));

		}
	}

	// Test clamp to edge
	_2i32 coor = _mm_set_pi32(-3, 4);
	_4i32 res = read_2d_uii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8,edge)",
				imData[0][4], (unsigned char)_mm_cvtsi128_si32(res));

	// Test clamp to border
	coor = _mm_set_pi32(-3, 4);
	res = read_2d_uii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8,border)",
				(unsigned char)0.0f, (unsigned char)_mm_cvtsi128_si32(res));
	res = _mm_srli_si128(res, 12);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8,border,alpha)",
				(unsigned char)1, (unsigned char)_mm_cvtsi128_si32(res));

}

void ImageTest::__read_2d_ii_2u16_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) short	imData[HEIGHT][WIDTH][2];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j][0] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
			rVal = rand();
			imData[i][j][1] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4;
	image.format.image_channel_order = CLK_RA;
	image.format.image_channel_data_type = CLK_SIGNED_INT16;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			_4i32 res = read_2d_ii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ii_2u16_test(sign16)[0]",
				imData[i][j][0], (short)_mm_cvtsi128_si32(res));
			res = _mm_srli_si128(res, 12);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ii_2u16_test(sign16)[1]",
				imData[i][j][1], (short)_mm_cvtsi128_si32(res));
		}
	}

	// Test clamp to edge
	_2i32 coor = _mm_set_pi32(-3, 4);
	_4i32 res = read_2d_ii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(sign16,edge)[0]",
				imData[0][4][0], (short)_mm_cvtsi128_si32(res));
	res = _mm_srli_si128(res, 12);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(sign16,edge)[1]",
				imData[0][4][1], (short)_mm_cvtsi128_si32(res));

	// Test clamp to border
	coor = _mm_set_pi32(-3, 4);
	res = read_2d_ii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(sign16,border)",
				(short)0.0f, (short)_mm_cvtsi128_si32(res));
	res = _mm_srli_si128(res, 12);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(sign16,border)",
				(short)0.0f, (short)_mm_cvtsi128_si32(res));
}

void ImageTest::__read_2d_fi_3s32_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) int	imData[HEIGHT][WIDTH];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j] = (((rVal & 0x3FF)  - 0x1FF) & 0x3FF);
			rVal = rand();
			imData[i][j] |= (((rVal & 0x3FF)  - 0x1FF) & 0x3FF) << 10;
			rVal = rand();
			imData[i][j] |= (((rVal & 0x3FF)  - 0x1FF) & 0x3FF) << 20;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4;
	image.format.image_channel_order = CLK_RGB;
	image.format.image_channel_data_type = CLK_UNORM_INT_101010;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			float fVal = (float)(imData[i][j]>>20 & 0x3FF) / (float)(0x3FF);
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010)[R]",
				fVal, _mm_cvtss_f32(res), 0.001f);
			fVal = (float)((imData[i][j]>>10) & 0x3FF) / (float)(0x3FF);
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010)[G]",
				fVal, _mm_cvtss_f32(res), 0.001f);
			fVal = (float)((imData[i][j]) & 0x3FF) / (float)(0x3FF);
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010)[B]",
				fVal, _mm_cvtss_f32(res), 0.001f);
		}
	}

	// Test clamp to edge
	_2i32 coor = _mm_set_pi32(-3, 4);
	float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	float fVal = (float)(imData[0][4] >> 20 & 0x3FF) / (float)(0x3FF);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[R]",
		fVal, _mm_cvtss_f32(res), 0.001f);
	fVal = (float)((imData[0][4]>>10) & 0x3FF) / (float)(0x3FF);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[G]",
		fVal, _mm_cvtss_f32(res), 0.001f);
	fVal = (float)((imData[0][4]) & 0x3FF) / (float)(0x3FF);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[B]",
		fVal, _mm_cvtss_f32(res), 0.001f);

	// Test clamp to border
	coor = _mm_set_pi32(-3, 4);
	res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[R]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[G]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[B]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_3s32_test(int101010, edge)[A]",
		1.f, _mm_cvtss_f32(res), 0.001f);
}

void ImageTest::__read_2d_fi_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int cVal = rand();
			int rVal = rand();
			imData[i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
			rVal = rand();
			imData[i][j][3] = (float)rVal / (float)RAND_MAX;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[R]",
				imData[i][j][0], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[G]",
				imData[i][j][1], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[B]",
				imData[i][j][2], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[A]",
				imData[i][j][3], _mm_cvtss_f32(res));
		}
	}

	// Test clamp to edge
	_2i32 coor = _mm_set_pi32(3, -4);
	float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,edge)[R]",
		imData[3][0][0], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,edge)[G]",
		imData[3][0][1], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,edge)[B]",
		imData[3][0][2], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,edge)[A]",
		imData[3][0][3], _mm_cvtss_f32(res));

	// Test clamp to border
	coor = _mm_set_pi32(-3, 4);
	res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[R]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[G]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[B]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[A]",
		0.f, _mm_cvtss_f32(res));
}

void ImageTest::__read_3d_fi_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) short	imData[DEPTH][HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned k=0;k<DEPTH;++k)
	{
		for(unsigned i=0;i<HEIGHT;++i)
		{
			for(unsigned j=0;j<WIDTH;++j)
			{
				int rVal = rand();
				imData[k][i][j][0] = rVal/2 - RAND_MAX;
				rVal = rand();
				imData[k][i][j][1] = rVal/2 - RAND_MAX;
				rVal = rand();
				imData[k][i][j][2] = rVal/2 - RAND_MAX;
				rVal = rand();
				imData[k][i][j][3] = rVal/2 - RAND_MAX;;
			}
		}
	}
	// Construct image header
	image.dim_count = 3;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.dim[2] = DEPTH;
	image.pitch[0] = WIDTH*4*sizeof(short);
	image.pitch[1] = HEIGHT*image.pitch[0];
	image.format.image_channel_order = CLK_BGRA;
	image.format.image_channel_data_type = CLK_SNORM_INT16;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(short);

	// Test inside the image
	for(unsigned k=0;k<DEPTH;++k)
	{
		for(unsigned i=0; i<HEIGHT; ++i)
		{
			for(unsigned j=0; j<WIDTH; ++j)
			{
				_4i32 coor = _mm_set_epi32(0, k, i, j);
 				float4 res = read_3d_fi(&image,
					(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
					CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
					coor);

				float fVal = (float)imData[k][i][j][2] / (float)0x7FFF;
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16)[R]",
					fVal, _mm_cvtss_f32(res), 0.001f);
				fVal = (float)imData[k][i][j][1] / (float)0x7FFF;
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16)[G]",
					fVal, _mm_cvtss_f32(res), 0.001f);
				fVal = (float)imData[k][i][j][0] / (float)0x7FFF;
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16)[B]",
					fVal, _mm_cvtss_f32(res), 0.001f);
				fVal = (float)imData[k][i][j][3] / (float)0x7FFF;
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16)[A]",
					fVal, _mm_cvtss_f32(res), 0.001f);
			}
		}
	}

	// Test clamp to edge
	_4i32 coor = _mm_set_epi32(0, 1, -4, 2);
	float4 res = read_3d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	float fVal = (float)imData[1][0][2][2] / (float)0x7FFF;
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16,edge)[R]",
			fVal, _mm_cvtss_f32(res), 0.001f);
	fVal = (float)imData[1][0][2][1] / (float)0x7FFF;
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16,edge)[G]",
			fVal, _mm_cvtss_f32(res), 0.001f);
	fVal = (float)imData[1][0][2][0] / (float)0x7FFF;
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16,edge)[B]",
			fVal, _mm_cvtss_f32(res), 0.001f);
	fVal = (float)imData[1][0][2][3] / (float)0x7FFF;
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_fi_4f_test(snorm16,edge)[A]",
			fVal, _mm_cvtss_f32(res), 0.001f);

	// Test clamp to border
	coor = _mm_set_epi32(0, 1, -4, 2);
	res = read_3d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[R]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[G]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[B]",
		0.f, _mm_cvtss_f32(res), 0.001f);
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_fi_4f_test(float,border)[A]",
		0.f, _mm_cvtss_f32(res), 0.001f);
}

void ImageTest::__read_2d_ff_nearest_norm_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int cVal = rand();
			int rVal = rand();
			imData[i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
			rVal = rand();
			imData[i][j][3] = (float)rVal / (float)RAND_MAX;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	// Test randomly 100 times
	for(int i=0; i<100; ++i)
	{
		float fX = ((float)rand() / (float)RAND_MAX);
		float fY = ((float)rand() / (float)RAND_MAX);

		float2 coor = _mm_movepi64_pi64(_mm_castps_si128(_mm_set_ps(0, 0, fY, fX)));
		float4 res = read_2d_ff(&image,
			(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
			CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE),
			coor);

		fX *= (float)WIDTH;
		fY *= (float)HEIGHT;
		int iY = floorf(fY);
		// WA: strange behaviour of the CPU, need twice call to floor.
		iY = floorf(fY);
		int iX = floorf(fX);
		// Test
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float)[R]",
			imData[iY][iX][0], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float)[G]",
			imData[iY][iX][1], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float)[B]",
			imData[iY][iX][2], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float)[A]",
			imData[iY][iX][3], _mm_cvtss_f32(res));
	}

	// Test clamp to edge
	float2 coor = _mm_movepi64_pi64(_mm_castps_si128(_mm_set_ps(0, 0, 3.f, -0.1f)));

	float4 res = read_2d_ff(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,edge)[R]",
		imData[3][0][0], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,edge)[G]",
		imData[3][0][1], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,edge)[B]",
		imData[3][0][2], _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,edge)[A]",
		imData[3][0][3], _mm_cvtss_f32(res));

	// Test clamp to border
	coor = _mm_movepi64_pi64(_mm_castps_si128(_mm_set_ps(0, 0, 3.f, -0.1f)));
	res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP|
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,border)[R]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,border)[G]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,border)[B]",
		0.f, _mm_cvtss_f32(res));
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ff_nearest_norm_4f_test(float,border)[A]",
		0.f, _mm_cvtss_f32(res));
}

void ImageTest::__read_3d_ff_nearest_norm_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[DEPTH][HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned k=0;k<DEPTH;++k)
	{
		for(unsigned i=0;i<HEIGHT;++i)
		{
			for(unsigned j=0;j<WIDTH;++j)
			{
				int cVal = rand();
				int rVal = rand();
				imData[k][i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
				cVal = rand();
				rVal = rand();
				imData[k][i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
				cVal = rand();
				rVal = rand();
				imData[k][i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
				rVal = rand();
				imData[k][i][j][3] = (float)rVal / (float)RAND_MAX;
			}
		}
	}

	// Construct image header
	image.dim_count = 3;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.dim[2] = DEPTH;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.pitch[1] = HEIGHT*image.pitch[0];
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	// Test randomly 100 times
	for(int i=0; i<100; ++i)
	{
		float fX = ((float)rand() / (float)RAND_MAX);
		float fY = ((float)rand() / (float)RAND_MAX);
		float fZ = ((float)rand() / (float)RAND_MAX);

		float4 coor = _mm_set_ps(0, fZ, fY, fX);
		float4 res = read_3d_ff(&image,
			(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
			CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE),
			coor);

		fX *= (float)WIDTH;
		fY *= (float)HEIGHT;
		fZ *= (float)DEPTH;
		int iY = floorf(fY);
		// WA: strange behaviour of the CPU, need twice call to floor.
		iY = floorf(fY);
		int iX = floorf(fX);
		int iZ = floorf(fZ);
		// Test
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_3d_ff_nearest_norm_4f_test(float)[R]",
			imData[iZ][iY][iX][0], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_3d_ff_nearest_norm_4f_test(float)[G]",
			imData[iZ][iY][iX][1], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_3d_ff_nearest_norm_4f_test(float)[B]",
			imData[iZ][iY][iX][2], _mm_cvtss_f32(res));
		res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_3d_ff_nearest_norm_4f_test(float)[A]",
			imData[iZ][iY][iX][3], _mm_cvtss_f32(res));
	}
}

void ImageTest::__read_2d_ff_linear_unorm_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int cVal = rand();
			int rVal = rand();
			imData[i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
			rVal = rand();
			imData[i][j][3] = (float)rVal / (float)RAND_MAX;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	for(unsigned i=1; i<HEIGHT-1; ++i)
	{
		for(unsigned j=1; j<WIDTH-1; ++j)
		{
			float fdX = ((float)rand() / (float)RAND_MAX);
			float fdY = ((float)rand() / (float)RAND_MAX);

			float2 coor = _mm_movepi64_pi64(_mm_castps_si128(_mm_set_ps(0, 0, i+fdY, j+fdX)));

			float4 res = read_2d_ff(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
				CL_DEV_SAMPLER_FILTER_LINEAR | CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Calculate offsets
			int iX, iY;
			iX = (int)floorf(j+fdX-0.5);
			iX = (int)floorf(j+fdX-0.5);
			iY = (int)floorf(i+fdY-0.5);
			// Update linear factors
			fdX = (j+fdX-0.5)-iX;;
			fdY = (i+fdY-0.5)-iY;
			// Test
			// R
			float fRes = (1-fdY)*(1-fdX)*imData[iY][iX][0] +
							fdY*(1-fdX)*imData[iY+1][iX][0] +
							(1-fdY)*fdX*imData[iY][iX+1][0] +
							fdY*fdX*imData[iY+1][iX+1][0];
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_ff_linear_unorm_4f_test(float)[R]",
				fRes, _mm_cvtss_f32(res), 0.005);
			// G
			fRes = (1-fdY)*(1-fdX)*imData[iY][iX][1] +
							fdY*(1-fdX)*imData[iY+1][iX][1] +
							(1-fdY)*fdX*imData[iY][iX+1][1] +
							fdY*fdX*imData[iY+1][iX+1][1];
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_ff_linear_unorm_4f_test(float)[G]",
				fRes, _mm_cvtss_f32(res), 0.005);
			// B
			fRes = (1-fdY)*(1-fdX)*imData[iY][iX][2] +
							fdY*(1-fdX)*imData[iY+1][iX][2] +
							(1-fdY)*fdX*imData[iY][iX+1][2] +
							fdY*fdX*imData[iY+1][iX+1][2];
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_ff_linear_unorm_4f_test(float)[B]",
				fRes, _mm_cvtss_f32(res), 0.005);
			// A
			fRes = (1-fdY)*(1-fdX)*imData[iY][iX][3] +
							fdY*(1-fdX)*imData[iY+1][iX][3] +
							(1-fdY)*fdX*imData[iY][iX+1][3] +
							fdY*fdX*imData[iY+1][iX+1][3];
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_ff_linear_unorm_4f_test(float)[A]",
				fRes, _mm_cvtss_f32(res), 0.005);
		}
	}
}

void ImageTest::__read_3d_ff_linear_unorm_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[DEPTH][HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned k=0;k<DEPTH;++k)
	{
		for(unsigned i=0;i<HEIGHT;++i)
		{
			for(unsigned j=0;j<WIDTH;++j)
			{
				int cVal = rand();
				int rVal = rand();
				imData[k][i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
				cVal = rand();
				rVal = rand();
				imData[k][i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
				cVal = rand();
				rVal = rand();
				imData[k][i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
				rVal = rand();
				imData[k][i][j][3] = (float)rVal / (float)RAND_MAX;
			}
		}
	}

	// Construct image header
	image.dim_count = 3;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.dim[2] = DEPTH;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.pitch[1] = image.pitch[0]*HEIGHT;
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	for(unsigned k=1; k<DEPTH-1;++k)
	{
		for(unsigned i=1; i<HEIGHT-1; ++i)
		{
			for(unsigned j=1; j<WIDTH-1; ++j)
			{
				float fdX = ((float)rand() / (float)RAND_MAX);
				float fdY = ((float)rand() / (float)RAND_MAX);
				float fdZ = ((float)rand() / (float)RAND_MAX);

				float4 coor = _mm_set_ps(0, k+fdZ, i+fdY, j+fdX);

				float4 res = read_3d_ff(&image,
					(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
					CL_DEV_SAMPLER_FILTER_LINEAR | CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
					coor);

				// Calculate offsets
				int iX, iY, iZ;
				iX = (int)floorf(j+fdX-0.5);
				iX = (int)floorf(j+fdX-0.5);
				iY = (int)floorf(i+fdY-0.5);
				iZ = (int)floorf(k+fdZ-0.5);
				// Update linear factors
				fdX = (j+fdX-0.5)-iX;;
				fdY = (i+fdY-0.5)-iY;
				fdZ = (k+fdZ-0.5)-iZ;
				// Test
				// R
				float fRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][0] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][0] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][0] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][0] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][0] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][0] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][0] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][0];
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_ff_linear_unorm_4f_test(float)[R]",
					fRes, _mm_cvtss_f32(res), 0.005);
				// G
				fRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][1] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][1] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][1] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][1] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][1] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][1] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][1] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][1];
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_ff_linear_unorm_4f_test(float)[G]",
					fRes, _mm_cvtss_f32(res), 0.005);
				// B
				fRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][2] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][2] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][2] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][2] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][2] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][2] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][2] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][2];
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_ff_linear_unorm_4f_test(float)[B]",
					fRes, _mm_cvtss_f32(res), 0.005);
				// A
				fRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][3] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][3] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][3] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][3] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][3] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][3] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][3] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][3];
				res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_ff_linear_unorm_4f_test(float)[A]",
					fRes, _mm_cvtss_f32(res), 0.005);
			}
		}
	}
}

void ImageTest::__read_2d_if_linear_unnorm_2s16_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) short	imData[HEIGHT][WIDTH][2];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j][0] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
			rVal = rand();
			imData[i][j][1] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4;
	image.format.image_channel_order = CLK_RA;
	image.format.image_channel_data_type = CLK_SIGNED_INT16;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned i=1; i<HEIGHT-1; ++i)
	{
		for(unsigned j=1; j<WIDTH-1; ++j)
		{
			float fdX = ((float)rand() / (float)RAND_MAX);
			float fdY = ((float)rand() / (float)RAND_MAX);

			float2 coor = _mm_movepi64_pi64(_mm_castps_si128(_mm_set_ps(0, 0, i+fdY, j+fdX)));

			_4i32 res = read_2d_if(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
				CL_DEV_SAMPLER_FILTER_LINEAR |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Calculate offsets
			int iX, iY;
			iX = (int)floorf(j+fdX-0.5);
			iX = (int)floorf(j+fdX-0.5);
			iY = (int)floorf(i+fdY-0.5);
			// Update linear factors
			fdX = (j+fdX-0.5)-iX;;
			fdY = (i+fdY-0.5)-iY;

			short sRes = (1-fdY)*(1-fdX)*imData[iY][iX][0] +
							fdY*(1-fdX)*imData[iY+1][iX][0] +
							(1-fdY)*fdX*imData[iY][iX+1][0] +
							fdY*fdX*imData[iY+1][iX+1][0];
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_linear_unnorm_if_2s16_test(sign16)[0]",
				(float)sRes, (float)((short)_mm_cvtsi128_si32(res)), 2);

			sRes = (1-fdY)*(1-fdX)*imData[iY][iX][1] +
							fdY*(1-fdX)*imData[iY+1][iX][1] +
							(1-fdY)*fdX*imData[iY][iX+1][1] +
							fdY*fdX*imData[iY+1][iX+1][1];
			res = _mm_srli_si128(res, 12);
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_2d_linear_unnorm_if_2s16_test(sign16)[1]",
				sRes, (float)((short)_mm_cvtsi128_si32(res)), 2);
		}
	}
}

void ImageTest::__read_3d_if_linear_unnorm_2s16_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) short	imData[DEPTH][HEIGHT][WIDTH][2];

	// Fill image data
	srand(0);
	for(unsigned k=0;k<DEPTH;++k)
	{
		for(unsigned i=0;i<HEIGHT;++i)
		{
			for(unsigned j=0;j<WIDTH;++j)
			{
				int rVal = rand();
				imData[k][i][j][0] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
				rVal = rand();
				imData[k][i][j][1] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
			}
		}
	}

	// Construct image header
	image.dim_count = 3;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.dim[2] = DEPTH;
	image.pitch[0] = WIDTH*4;
	image.pitch[1] = image.pitch[0]*HEIGHT;
	image.format.image_channel_order = CLK_RA;
	image.format.image_channel_data_type = CLK_SIGNED_INT16;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned k=1;k<DEPTH-1;++k)
	{
		for(unsigned i=1; i<HEIGHT-1; ++i)
		{
			for(unsigned j=1; j<WIDTH-1; ++j)
			{
				float fdX = ((float)rand() / (float)RAND_MAX);
				float fdY = ((float)rand() / (float)RAND_MAX);
				float fdZ = ((float)rand() / (float)RAND_MAX);

				float4 coor = _mm_set_ps(0, k+fdZ, i+fdY, j+fdX);

				_4i32 res = read_3d_if(&image,
					(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_CLAMP |
					CL_DEV_SAMPLER_FILTER_LINEAR |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
					coor);

				// Calculate offsets
				int iX, iY, iZ;
				iX = (int)floorf(j+fdX-0.5);
				iX = (int)floorf(j+fdX-0.5);
				iY = (int)floorf(i+fdY-0.5);
				iZ = (int)floorf(k+fdZ-0.5);
				// Update linear factors
				fdX = (j+fdX-0.5)-iX;;
				fdY = (i+fdY-0.5)-iY;
				fdZ = (k+fdZ-0.5)-iZ;

				short sRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][0] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][0] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][0] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][0] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][0] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][0] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][0] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][0];
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_linear_unnorm_if_2s16_test(sign16)[0]",
					(float)sRes, (float)((short)_mm_cvtsi128_si32(res)), 2);

				sRes = (1-fdZ)*(1-fdY)*(1-fdX)*imData[iZ][iY][iX][1] +
							 (1-fdZ)*fdY*(1-fdX)*imData[iZ][iY+1][iX][1] +
							 (1-fdZ)*(1-fdY)*fdX*imData[iZ][iY][iX+1][1] +
							 (1-fdZ)*fdY*fdX*imData[iZ][iY+1][iX+1][1] +
								fdZ*(1-fdY)*(1-fdX)*imData[iZ+1][iY][iX][1] +
								fdZ*fdY*(1-fdX)*imData[iZ+1][iY+1][iX][1] +
								fdZ*(1-fdY)*fdX*imData[iZ+1][iY][iX+1][1] +
								fdZ*fdY*fdX*imData[iZ+1][iY+1][iX+1][1];
				res = _mm_srli_si128(res, 12);
				CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__read_3d_linear_unnorm_if_2s16_test(sign16)[1]",
					sRes, (float)((short)_mm_cvtsi128_si32(res)), 2);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Write test
void ImageTest::__write_2d_uii_1u8_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) unsigned char	imData[HEIGHT][WIDTH];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j] = (rVal % 256) - 128;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH;
	image.format.image_channel_order = CLK_LUMINANCE;
	image.format.image_channel_data_type = CLK_UNSIGNED_INT8;
	image.pData = imData;
	image.uiElementSize = 1;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			_4i32 res = read_2d_uii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Write Back the data
			write_imageui(&image, coor, res);

			// Test
			CPPUNIT_ASSERT_EQUAL_MESSAGE("read_2d_uii(unsign8)[0]",
				imData[i][j], (unsigned char)_mm_cvtsi128_si32(res));
		}
	}
}

void ImageTest::__write_2d_ii_2u16_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) short	imData[HEIGHT][WIDTH][2];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j][0] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
			rVal = rand();
			imData[i][j][1] = (rVal & RAND_MAX)  - (RAND_MAX / 2);
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4;
	image.format.image_channel_order = CLK_RA;
	image.format.image_channel_data_type = CLK_SIGNED_INT16;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			_4i32 res = read_2d_ii(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Write Back the data
			write_imagei(&image, coor, res);

			// Test
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ii_2u16_test(sign16)[0]",
				imData[i][j][0], (short)_mm_cvtsi128_si32(res));
			res = _mm_srli_si128(res, 12);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_ii_2u16_test(sign16)[1]",
				imData[i][j][1], (short)_mm_cvtsi128_si32(res));
		}
	}
}

void ImageTest::__write_2d_fi_3s32_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) int	imData[HEIGHT][WIDTH];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int rVal = rand();
			imData[i][j] = (((rVal & 0x3FF)  - 0x1FF) & 0x3FF);
			rVal = rand();
			imData[i][j] |= (((rVal & 0x3FF)  - 0x1FF) & 0x3FF) << 10;
			rVal = rand();
			imData[i][j] |= (((rVal & 0x3FF)  - 0x1FF) & 0x3FF) << 20;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4;
	image.format.image_channel_order = CLK_RGB;
	image.format.image_channel_data_type = CLK_UNORM_INT_101010;
	image.pData = imData;
	image.uiElementSize = 4;

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Write Back the data
			write_imagef(&image, coor, res);

			// Test
			float fVal = (float)(imData[i][j]>>20 & 0x3FF) / (float)(0x3FF);
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__write_2d_fi_3s32_test(int101010)[R]",
				fVal, _mm_cvtss_f32(res), 0.001f);
			fVal = (float)((imData[i][j]>>10) & 0x3FF) / (float)(0x3FF);
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__write_2d_fi_3s32_test(int101010)[G]",
				fVal, _mm_cvtss_f32(res), 0.001f);
			fVal = (float)((imData[i][j]) & 0x3FF) / (float)(0x3FF);
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("__write_2d_fi_3s32_test(int101010)[B]",
				fVal, _mm_cvtss_f32(res), 0.001f);
		}
	}
}

void ImageTest::__write_2d_fi_4f_test()
{
	cl_mem_obj_descriptor image = {0};
	__declspec(align(16)) float	imData[HEIGHT][WIDTH][4];

	// Fill image data
	srand(0);
	for(unsigned i=0;i<HEIGHT;++i)
	{
		for(unsigned j=0;j<WIDTH;++j)
		{
			int cVal = rand();
			int rVal = rand();
			imData[i][j][0] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][1] = ((float)rVal / (float)RAND_MAX)  + cVal;
			cVal = rand();
			rVal = rand();
			imData[i][j][2] = ((float)rVal / (float)RAND_MAX)  + cVal;
			rVal = rand();
			imData[i][j][3] = (float)rVal / (float)RAND_MAX;
		}
	}

	// Construct image header
	image.dim_count = 2;
	image.dim[0] = WIDTH;
	image.dim[1] = HEIGHT;
	image.pitch[0] = WIDTH*4*sizeof(float);
	image.format.image_channel_order = CLK_RGBA;
	image.format.image_channel_data_type = CLK_FLOAT;
	image.pData = imData;
	image.uiElementSize = 4*sizeof(float);

	// Test inside the image
	for(unsigned i=0; i<HEIGHT; ++i)
	{
		for(unsigned j=0; j<WIDTH; ++j)
		{
			_2i32 coor = _mm_set_pi32(i, j);
			float4 res = read_2d_fi(&image,
				(cl_dev_sampler_prop)(CL_DEV_SAMPLER_ADDRESS_NONE |
				CL_DEV_SAMPLER_FILTER_NEAREST |	CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE),
				coor);

			// Write Back the data
			write_imagef(&image, coor, res);

			// Test
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[R]",
				imData[i][j][0], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[G]",
				imData[i][j][1], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[B]",
				imData[i][j][2], _mm_cvtss_f32(res));
			res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res), 4));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__read_2d_fi_4f_test(float)[A]",
				imData[i][j][3], _mm_cvtss_f32(res));
		}
	}
}
