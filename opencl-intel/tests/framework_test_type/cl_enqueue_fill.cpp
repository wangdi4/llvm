#include <stdio.h>

#include "CL/cl.h"
#include "cl_types.h"

#define PROVISIONAL_MALLOC_SIZE 100
#include "cl_provisional.h"

#include "TestsHelpClasses.h"

#include "FrameworkTest.h"

#define IMG_W 128
#define IMG_H 128
#define IMG_D 128
#define BUFFER_CL_ALLOC_IMG (IMG_W * IMG_H * IMG_D * 1)

#define BUFFER_LEN (30)
#define UINT_BUFFER_LEN ( BUFFER_LEN * sizeof(cl_uint))

#define PATTERN_SIZE 132

#define IMAGE1D

extern cl_device_type gDeviceType;

class OCLEnvTest : public ::testing::Test
{
protected:
	cl_platform_id        m_platform;
	cl_device_id          m_clDefaultDeviceId;
	cl_context_properties m_prop[3];
	cl_context            m_context;
	cl_command_queue      m_queue;
	_PROVISONAL_MallocArray_t _mallocArr_;

	OCLEnvTest()
	: m_platform(0), m_setupOK(false)
	{}

    ~OCLEnvTest()
    {
        clFinish(m_queue);
        clReleaseCommandQueue(m_queue);
        clReleaseContext(m_context);
    }

	/**
	 * Check to see if the test fixture (environment) is ready.
	 * @return true if setup is OK, false otherwise.
	 */
	bool IsOCLReady()
	{
		return m_setupOK;
	}

#undef PROV_ARRAY_NAME
#define PROV_ARRAY_NAME m_provisional_array
	virtual void SetUp()
	{
		PROV_INIT_MALLOCARRAY_T(PROV_ARRAY_NAME, PROVISIONAL_MALLOC_SIZE)

		bool bResult = true;
		cl_int iRet;

		iRet = clGetPlatformIDs(1, &m_platform, NULL);
		bResult &= Check("", CL_SUCCESS, iRet);

		if (!bResult)
		{
			return;
		}

		m_prop[0] = CL_CONTEXT_PLATFORM;
		m_prop[1] = (cl_context_properties)m_platform;
		m_prop[2] = 0;

		m_context = PROV_OBJ( clCreateContextFromType(m_prop, gDeviceType, NULL, NULL, &iRet) );
		if (CL_SUCCESS != iRet)
		{
			printf("clCreateContextFromType = %s\n",ClErrTxt(iRet));
		    PROV_RETURN_AND_ABANDON();
		}
		//printf("context = %p\n", m_context);

		iRet = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_clDefaultDeviceId, NULL);
		if (CL_SUCCESS != iRet)
		{
			printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
		    PROV_RETURN_AND_ABANDON();
		}
		//printf("device = %p\n", m_clDefaultDeviceId);

		m_queue = PROV_OBJ( clCreateCommandQueue (m_context, m_clDefaultDeviceId, 0 /*no properties*/, &iRet) );
		if (CL_SUCCESS != iRet)
		{
			printf("clCreateCommandQueue = %s\n",ClErrTxt(iRet));
		    PROV_RETURN_AND_ABANDON();
		}

		m_setupOK = true;
	}

	virtual void TearDown()
	{
        clReleaseCommandQueue(m_queue);
        clReleaseContext(m_context);
		PROV_RETURN_AND_ABANDON();
		m_setupOK = false;
	}
protected:
	_PROVISONAL_MallocArray_t PROV_ARRAY_NAME;

	void TestFillWithLargePattern(cl_mem buffer1, size_t bufferSize, const char* pPattern, size_t szPatternSize);

private:
	bool                      m_setupOK;	

#undef PROV_ARRAY_NAME
#define PROV_ARRAY_NAME _mallocArr_
};



/**************************************************************************************************
* clEnqueueFillBuffer
**************************************************************************************************/

// We want our tests to use the same provisional framework of OCLEnvTest.
#undef PROV_ARRAY_NAME
#define PROV_ARRAY_NAME m_provisional_array

const char LENGTH_FOUR_PATTERN[4] = { 'L', 'I', 'V', 'E'};
const char LENGTH_TWELVE_PATTERN[12] = { 'L', 'I', 'V', 'E', 'D', 'E', 'A', 'D', 'B', 'E', 'E', 'F'};

typedef OCLEnvTest EnqueueFillTest;

void OCLEnvTest::TestFillWithLargePattern(cl_mem buffer1, size_t bufferSize, const char* pPattern, size_t szPatternSize)
{
	size_t countFillErrors = 0;
	cl_int iRet = clEnqueueFillBuffer(m_queue, buffer1, pPattern, szPatternSize, (PATTERN_SIZE * 15), (PATTERN_SIZE * 5), 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer (partial fill with 2) = " << ClErrTxt(iRet);
	char* pBuf = (char*)clEnqueueMapBuffer(m_queue, buffer1, CL_TRUE, CL_MAP_READ, 0, bufferSize, 0, NULL, NULL, &iRet);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueMapBuffer = " << ClErrTxt(iRet);
	for (size_t pos=0 ; pos < bufferSize ; ++pos)
	{
		if (pos < (PATTERN_SIZE * 5) )
		{
			if (pBuf[pos] != 2) ++countFillErrors;
		}
		else if (pos >= (PATTERN_SIZE * 5) && pos < (PATTERN_SIZE * (5+10)) )
		{
			if (pBuf[pos] != 1) ++countFillErrors;
		}
		else if (pos >= (PATTERN_SIZE * 15) && pos < (PATTERN_SIZE * (15+5)))
		{
			if (0 != strncmp(pBuf+pos, pPattern, szPatternSize)) ++countFillErrors;
			pos += szPatternSize - 1; // for loop will advance one more.
		}
		else
		{
			if (pBuf[pos] != 0) ++countFillErrors;
		}
	}
	clEnqueueUnmapMemObject(m_queue, buffer1, pBuf, 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueUnmapMemObject = " << ClErrTxt(iRet);
}

TEST_F(EnqueueFillTest, Buffer)
{
	ASSERT_EQ(true, IsOCLReady()) << "Failed to setup OCL environment.";

	cl_int iRet = 0;

	// SETUP WAS OK

	char    pattern[PATTERN_SIZE];
	size_t  bufferSize =  PATTERN_SIZE * 20;
	char    *pBuf; // pointer to buffer.
	size_t  countFillErrors;

	cl_mem buffer1 = PROV_OBJ( clCreateBuffer(m_context, CL_MEM_READ_WRITE , bufferSize, NULL, &iRet) );
	ASSERT_EQ(ClErrTxt(CL_SUCCESS), ClErrTxt(iRet)) << "clCreateBuffer = " << ClErrTxt(iRet);

	// Fill with 0.
	countFillErrors = 0;
	pattern[0] = 0;
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 1, 0, bufferSize, 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer (fill with 0) = " << ClErrTxt(iRet);
	pBuf = (char*)clEnqueueMapBuffer(m_queue, buffer1, CL_TRUE, CL_MAP_READ, 0, bufferSize, 0, NULL, NULL, &iRet);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueMapBuffer = " << ClErrTxt(iRet);
	for (size_t pos=0 ; pos < bufferSize ; ++pos)
	{
		if (pBuf[pos] != 0) ++countFillErrors;
	}
	clEnqueueUnmapMemObject(m_queue, buffer1, pBuf, 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueUnmapMemObject = " << ClErrTxt(iRet);

	EXPECT_EQ((size_t)0, countFillErrors) << "fill with 0 "<<bufferSize<<" bytes has "<<countFillErrors<<" bad bytes.";

	// Fill the pattern with 1's
	countFillErrors = 0;
	pattern[0] = 1;
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 1, (PATTERN_SIZE * 5), (PATTERN_SIZE * 10), 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer (partial fill with 1) = " << ClErrTxt(iRet);
	pBuf = (char*)clEnqueueMapBuffer(m_queue, buffer1, CL_TRUE, CL_MAP_READ, 0, bufferSize, 0, NULL, NULL, &iRet);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueMapBuffer = " << ClErrTxt(iRet);
	for (size_t pos=0 ; pos < bufferSize ; ++pos)
	{
		if (pos >= (PATTERN_SIZE * 5) && pos < (PATTERN_SIZE * (5+10)) )
		{
			if (pBuf[pos] != 1) ++countFillErrors;
		} else {
			if (pBuf[pos] != 0) ++countFillErrors;
		}
	}
	clEnqueueUnmapMemObject(m_queue, buffer1, pBuf, 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueUnmapMemObject = " << ClErrTxt(iRet);

	EXPECT_EQ((size_t)0, countFillErrors) << "partial fill with 1's "<<bufferSize<<" bytes has "<<countFillErrors<<" bad bytes.";

	// Fill the pattern with 2's
	countFillErrors = 0;
	pattern[0] = 2;
	memset(pattern, 2, PATTERN_SIZE);
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 1, 0, (PATTERN_SIZE * 5), 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer (partial fill with 2) = " << ClErrTxt(iRet);
	pBuf = (char*)clEnqueueMapBuffer(m_queue, buffer1, CL_TRUE, CL_MAP_READ, 0, bufferSize, 0, NULL, NULL, &iRet);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueMapBuffer = " << ClErrTxt(iRet);
	for (size_t pos=0 ; pos < bufferSize ; ++pos)
	{
		if (pos < (PATTERN_SIZE * 5) )
		{
			if (pBuf[pos] != 2) ++countFillErrors;
		}
		else if (pos >= (PATTERN_SIZE * 5) && pos < (PATTERN_SIZE * (5+10)) )
		{
			if (pBuf[pos] != 1) ++countFillErrors;
		}
		else
		{
			if (pBuf[pos] != 0) ++countFillErrors;
		}
	}
	clEnqueueUnmapMemObject(m_queue, buffer1, pBuf, 0, NULL, NULL);
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueUnmapMemObject = " << ClErrTxt(iRet);

	EXPECT_EQ((size_t)0, countFillErrors) << "partial fill with 2's and 1's "<<bufferSize<<" bytes has "<<countFillErrors<<" bad bytes.";

	// check error cases:
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 5, 0, PATTERN_SIZE, 0, NULL, NULL);	// size of pattern isn't power of 2
	EXPECT_EQ(CL_INVALID_VALUE, iRet) << "clEnqueueFillBuffer = " << ClErrTxt(iRet);
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 48, 0, PATTERN_SIZE, 0, NULL, NULL);	// size of pattern is divided by 3, but greater than larger vector of size 3
	EXPECT_EQ(CL_INVALID_VALUE, iRet) << "clEnqueueFillBuffer = " << ClErrTxt(iRet);
	iRet = clEnqueueFillBuffer(m_queue, buffer1, pattern, 9, 0, PATTERN_SIZE, 0, NULL, NULL);	// size of pattern is divided by 3, but isn't the size of any vector of size 3
	EXPECT_EQ(CL_INVALID_VALUE, iRet) << "clEnqueueFillBuffer = " << ClErrTxt(iRet);

	TestFillWithLargePattern(buffer1, bufferSize, LENGTH_FOUR_PATTERN, sizeof(LENGTH_FOUR_PATTERN));
	TestFillWithLargePattern(buffer1, bufferSize, LENGTH_TWELVE_PATTERN, sizeof(LENGTH_TWELVE_PATTERN));
        clFinish(m_queue);
	EXPECT_EQ((size_t)0, countFillErrors) << "pattern fill with LENGTH_FOUR_PATTERN "<<bufferSize<<" bytes has "<<countFillErrors<<" bad bytes.";
    clReleaseMemObject(buffer1);
}




/**
 * Get size of buffer required to read an image.
 * @param img
 * @param size returned size.
 * @param element returned element size.
 * @return CL_SUCCESS if OK, or error code.
 */
static cl_int getImgBufferSize(cl_mem img, size_t &size, size_t &element)
{
	size_t w, h, d, a;
	cl_int iRet;
	size = 0;

	iRet = clGetImageInfo(img, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &element, NULL);
	if (CL_SUCCESS != iRet) return iRet;
	iRet = clGetImageInfo(img, CL_IMAGE_WIDTH, sizeof(size_t), &w, NULL);
	if (CL_SUCCESS != iRet) return iRet;
	iRet = clGetImageInfo(img, CL_IMAGE_HEIGHT, sizeof(size_t), &h, NULL);
	if (CL_SUCCESS != iRet) return iRet;
	iRet = clGetImageInfo(img, CL_IMAGE_DEPTH, sizeof(size_t), &d, NULL);
	if (CL_SUCCESS != iRet) return iRet;
    iRet = clGetImageInfo(img, CL_IMAGE_ARRAY_SIZE, sizeof(size_t), &a, NULL);
    if (CL_SUCCESS != iRet) return iRet;

	size = w * (h?h:1) * (d?d:1) * (a?a:1) * element;
	return CL_SUCCESS;
}


template <typename InputColorDataType>
class ImageFillTest : public EnqueueFillTest
{
protected:
    cl_image_format m_imgFormat;
    cl_image_desc   m_imgDesc;

    ImageFillTest()
    {
    	/* values need to set by final test:
        m_imgFormat.image_channel_order = CL_RGBA;
        m_imgFormat.image_channel_data_type = CL_UNORM_INT8;

        m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
        m_imgDesc.image_width  = IMG_W;
        m_imgDesc.image_height = IMG_H;
        m_imgDesc.image_depth  = IMG_D;
        */
        m_imgDesc.image_array_size = 1;
        m_imgDesc.image_row_pitch = 0;
        m_imgDesc.image_slice_pitch = 0;
        m_imgDesc.num_mip_levels = 0;
        m_imgDesc.num_samples = 0;
        m_imgDesc.mem_object = NULL;
    }

	virtual ~ImageFillTest()
	{}

    template <typename ImgColorDataType>
    void printBadPixel(const size_t &w, const size_t &h, const size_t &d, const ImgColorDataType &expected, const ImgColorDataType &actual, size_t indx)
    {
        std::cout << "Color " << indx << " at " << w << "," << h << "," << d << " mismatch";
        std::cout << "  Expected: " << (int64_t)expected.s0 << "," << (int64_t)expected.s1 << "," << (int64_t)expected.s2 << "," << (int64_t)expected.s3;
        std::cout << "  Actual: " << (int64_t)actual.s0 << "," << (int64_t)actual.s1 << "," << (int64_t)actual.s2 << "," << (int64_t)actual.s3 << std::endl;
    }

/**
 * PRINT_BAD_PIXEL MACRO is used for debugging, please leave it here.
 */
//#define PRINT_BAD_PIXEL(colorNum)  if (w<5 && h<5 && d<5) printBadPixel<ImgColorDataType>(w, h, d, listOfExpectedColors[colorNum], *theColor, colorNum)
#define PRINT_BAD_PIXEL(colorNum)

#define LOOP_AND_CHECK(_test_) \
	clFlush(m_queue); \
	iRet = clEnqueueReadImage(m_queue, img, CL_BLOCKING, origin_base, region_full, 0, 0, img_buf, 0, NULL, NULL); \
	EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueReadImage = " << ClErrTxt(iRet); \
	countFillErrors = 0; \
	/*fprintf(stderr, "elementSize %lu, rowPitch %lu, slicePitch %lu\n", elementSize, rowPitch, slicePitch );*/ \
	for (size_t d = 0 ; d < depth ; ++d) \
	{ for (size_t h = 0 ; h < height ; ++h) \
		{ for (size_t w = 0 ; w < width ; ++w) \
			{ \
				pos = (elementSize*w) + (rowPitch*h) + (slicePitch*d); \
				/*if (w<5 && h<5 && d<5) fprintf(stderr, "pos %lu = %lu, %lu, %lu\n", pos, w, h, d);*/ \
				theColor = (ImgColorDataType *)(size_t(img_buf) + pos); \
				_test_; \
			} \
		} \
	}

static inline size_t AT_LEAST1(const size_t val) { return val ? val : 1; }

	template <typename ImgColorDataType>
	cl_int runSingleImageTest(InputColorDataType *listOfInputColors, ImgColorDataType *listOfExpectedColors)
	{
		cl_int iRet;
		size_t countFillErrors;
		size_t pos;
		ImgColorDataType *theColor;

		cl_mem img = clCreateImage(m_context, CL_MEM_READ_WRITE, &m_imgFormat, &m_imgDesc, NULL, &iRet);
		if (gDeviceType != CL_DEVICE_TYPE_ACCELERATOR)
		{
			EXPECT_EQ(CL_SUCCESS, iRet) << "clCreateImage img = " << ClErrTxt(iRet);
		}
		else
		{
			EXPECT_EQ(CL_INVALID_OPERATION, iRet) << "clCreateImage img = " << ClErrTxt(iRet);
		}

		if (CL_SUCCESS != iRet)
		{
			return iRet;
		}

		// allocate buffer for reading the image.
		size_t imgSize;
		size_t elementSize;
		size_t width  = m_imgDesc.image_width;
        size_t rowPitch;
        clGetImageInfo(img, CL_IMAGE_ROW_PITCH, sizeof(rowPitch), &rowPitch, NULL);
		size_t height = 1;
        size_t slicePitch;
        clGetImageInfo(img, CL_IMAGE_SLICE_PITCH, sizeof(slicePitch), &slicePitch, NULL);
		size_t depth = 1;

		if (m_imgDesc.image_type == CL_MEM_OBJECT_IMAGE2D || m_imgDesc.image_type == CL_MEM_OBJECT_IMAGE3D)
			height = m_imgDesc.image_height;

		if (m_imgDesc.image_type == CL_MEM_OBJECT_IMAGE3D)
			depth = m_imgDesc.image_depth;

		iRet = getImgBufferSize(img, imgSize, elementSize);
		EXPECT_EQ(CL_SUCCESS, iRet) << "getImgBufferSize img1" << ClErrTxt(iRet);

		void *img_buf = ALIGNED_MALLOC(imgSize, 128);

		size_t origin_base[3] = {0, 0, 0};
		size_t origin_mid[3] = {width/2, height/2, depth/2};
		size_t origin_qrtr[3] = {width/4, height/4, depth/4};

		size_t region_full[3] = {width, height, depth};
		size_t region_half[3] = {width/2, height/2, depth/2};

		// fix origins and regions
		switch (m_imgDesc.image_type)
		{
			case CL_MEM_OBJECT_IMAGE1D:
				origin_base[1] = 0;
				origin_mid[1]  = 0;
				origin_qrtr[1] = 0;
				region_full[1] = 1;
				region_half[1] = 1;
			case CL_MEM_OBJECT_IMAGE2D:
			case CL_MEM_OBJECT_IMAGE1D_ARRAY:
				origin_base[2] = 0;
				origin_mid[2]  = 0;
				origin_qrtr[2] = 0;
				region_full[2] = 1;
				region_half[2] = 1;
				break;
			default:
				break;
		}

		//fprintf(stderr, "testing clEnqueueFillImage - fill image with color 0\n");
		iRet = clEnqueueFillImage(m_queue, img, &listOfInputColors[0], origin_base, region_full, 0, NULL, NULL);
		EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillImage = " << ClErrTxt(iRet);

		LOOP_AND_CHECK(
			if (0 != memcmp(theColor, &listOfExpectedColors[0], elementSize)) {
				++countFillErrors;
                PRINT_BAD_PIXEL(0);
            }
		);
		EXPECT_EQ((size_t)0, countFillErrors) << "fill everything with color 0 had " << countFillErrors << " errors.";


		//fprintf(stderr, "testing clEnqueueFillImage - fill Upper left corner of image with color 1\n");
		iRet = clEnqueueFillImage(m_queue, img, &listOfInputColors[1], origin_base, region_half, 0, NULL, NULL);
		EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillImage = " << ClErrTxt(iRet);

		LOOP_AND_CHECK(
			if ((w < AT_LEAST1(width/2)) && (h < AT_LEAST1(height/2)) && (d < AT_LEAST1(depth/2)) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[1], elementSize) ) {
					++countFillErrors;
                    PRINT_BAD_PIXEL(1);
				}
			} else {
				if (0 != memcmp(theColor, &listOfExpectedColors[0], elementSize) ) {
					++countFillErrors;
                    PRINT_BAD_PIXEL(0);
				}
			}
		);
		EXPECT_EQ((size_t)0, countFillErrors) << "fill upper left back corner with color 1 had " << countFillErrors << " errors.";

		//fprintf(stderr, "testing clEnqueueFillImage - fill lower right corner of image with color 2\n");
		iRet = clEnqueueFillImage(m_queue, img, &listOfInputColors[2], origin_mid, region_half, 0, NULL, NULL);
		EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillImage = " << ClErrTxt(iRet);

		LOOP_AND_CHECK(
			if ((w >= width/2) && (h >= height/2) && (d >= depth/2) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[2], elementSize) ) {
					++countFillErrors;
                    PRINT_BAD_PIXEL(2);
                }
			} else if ((w < AT_LEAST1(width/2)) && (h < AT_LEAST1(height/2)) && (d < AT_LEAST1(depth/2)) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[1], elementSize) )
                {
					++countFillErrors;
                    PRINT_BAD_PIXEL(1);
                }
			} else
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[0], elementSize) )
                {
					++countFillErrors;
                    PRINT_BAD_PIXEL(0);
                }
			}
		);
		EXPECT_EQ((size_t)0, countFillErrors) << "fill lower right front corner with color 2 had " << countFillErrors << " errors.";

		//fprintf(stderr, "testing clEnqueueFillImage - fill middle of image with color 3\n");
		iRet = clEnqueueFillImage(m_queue, img, &listOfInputColors[3], origin_qrtr, region_half, 0, NULL, NULL);
		EXPECT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillImage = " << ClErrTxt(iRet);

		LOOP_AND_CHECK(
			if ( (w >= width/4) && (w < AT_LEAST1(3*width/4)) &&
				 (h >= height/4) && (h < AT_LEAST1(3*height/4)) &&
				 (d >= depth/4) && (d < AT_LEAST1(3*depth/4)) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[3], elementSize) )
					++countFillErrors;
			} else if ( (w >= width/2) && (h >= height/2) && (d >= depth/2) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[2], elementSize) )
					++countFillErrors;
			} else if ( (w < AT_LEAST1(width/2)) && (h < AT_LEAST1(height/2)) && (d < AT_LEAST1(depth/2)) )
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[1], elementSize) )
					++countFillErrors;
			} else
			{
				if (0 != memcmp(theColor, &listOfExpectedColors[0], elementSize) )
					++countFillErrors;
			}
		);
		EXPECT_EQ((size_t)0, countFillErrors) << "fill middle with color 3 had " << countFillErrors << " errors.";

		ALIGNED_FREE(img_buf);
		clReleaseMemObject(img);

		return CL_SUCCESS;
	}
};


/**************************************************************************************************
* clEnqueueFillImage
**************************************************************************************************/
typedef ImageFillTest<cl_float4> ImageFillTestNorm;
TEST_F(ImageFillTestNorm, Image)
{
	ASSERT_EQ(true, IsOCLReady()) << "Failed to setup OCL environment.";

	cl_int iRet = 0;

	// SETUP WAS OK
    m_imgFormat.image_channel_order = CL_RGBA;
    m_imgFormat.image_channel_data_type = CL_UNORM_INT8;

    m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    m_imgDesc.image_width  = IMG_W;
    m_imgDesc.image_height = IMG_H;
    m_imgDesc.image_depth  = IMG_D;


    cl_float4 inputFormatColors[] = { {{0,0,0,0}}, {{1,0,0,0}}, {{0,1,0,0}}, {{0,0,1,0}}, {{1,1,1,0}} };

    cl_uchar4 imgFormatColors_UNORM_INT8[] = { {{0,0,0,0}}, {{255,0,0,0}}, {{0,255,0,0}}, {{0,0,255,0}}, {{255,255,255,0}} };

    {
        SCOPED_TRACE("CL_RGBA, CL_FLOAT, CL_MEM_OBJECT_IMAGE3D");
        m_imgFormat.image_channel_order = CL_RGBA;
        m_imgFormat.image_channel_data_type = CL_FLOAT;
        m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
        EXPECT_NO_FATAL_FAILURE(
            iRet = runSingleImageTest<cl_float4>(inputFormatColors, inputFormatColors);
        );
    }

    {
        SCOPED_TRACE("CL_RGBA, CL_FLOAT, CL_MEM_OBJECT_IMAGE2D");
        m_imgFormat.image_channel_order = CL_RGBA;
        m_imgFormat.image_channel_data_type = CL_FLOAT;
        m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
        EXPECT_NO_FATAL_FAILURE(
            iRet = runSingleImageTest<cl_float4>(inputFormatColors, inputFormatColors);
        );
    }

    {
        SCOPED_TRACE("CL_RGBA, CL_FLOAT, CL_MEM_OBJECT_IMAGE1D");
        m_imgFormat.image_channel_order = CL_RGBA;
        m_imgFormat.image_channel_data_type = CL_FLOAT;
        m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
        EXPECT_NO_FATAL_FAILURE(
            iRet = runSingleImageTest<cl_float4>(inputFormatColors, inputFormatColors);
        );
    }

    {
		SCOPED_TRACE("CL_RGBA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE3D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8);
		);
    }

    {
		SCOPED_TRACE("CL_RGBA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE2D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8);
		);
    }

#ifdef IMAGE1D
    {
		SCOPED_TRACE("CL_RGBA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE1D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8);
		);
    }
#endif

    cl_uchar4 imgFormatColors_UNORM_INT8_BGRA[] = { {{0,0,0,0}}, {{0,0,255,0}}, {{0,255,0,0}}, {{255,0,0,0}}, {{255,255,255,0}} };

	{
		SCOPED_TRACE("CL_BGRA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE3D");
		m_imgFormat.image_channel_order = CL_BGRA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8_BGRA);
		);
    }

    {
		SCOPED_TRACE("CL_BGRA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE2D");
		m_imgFormat.image_channel_order = CL_BGRA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8_BGRA);
		);
    }

#ifdef IMAGE1D
    {
		SCOPED_TRACE("CL_BGRA, CL_UNORM_INT8, CL_MEM_OBJECT_IMAGE1D");
		m_imgFormat.image_channel_order = CL_BGRA;
		m_imgFormat.image_channel_data_type = CL_UNORM_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_uchar4>(inputFormatColors, imgFormatColors_UNORM_INT8_BGRA);
		);
    }
#endif
}




typedef ImageFillTest<cl_int4> ImageFillTestNonNormInt;
TEST_F(ImageFillTestNonNormInt, Image)
{
	ASSERT_EQ(true, IsOCLReady()) << "Failed to setup OCL environment.";

	cl_int iRet = 0;

	// SETUP WAS OK
    m_imgFormat.image_channel_order = CL_RGBA;
    m_imgFormat.image_channel_data_type = CL_SIGNED_INT32;

    m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    m_imgDesc.image_width  = IMG_W;
    m_imgDesc.image_height = IMG_H;
    m_imgDesc.image_depth  = IMG_D;


    cl_int4 inputFormatColors[] = { {{0,0,0,0}}, {{255,0,0,0}}, {{0,255,0,0}}, {{0,0,255,0}}, {{255,255,255,0}} };


    cl_int4 *imgFormatColors_UNORM_SIGNED_INT32 = inputFormatColors;

    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT32, CL_MEM_OBJECT_IMAGE3D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT32;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_int4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT32);
		);
    }

    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT32, CL_MEM_OBJECT_IMAGE2D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT32;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_int4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT32);
		);
    }

#ifdef IMAGE1D
    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT32, CL_MEM_OBJECT_IMAGE1D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT32;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_int4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT32);
		);
    }
#endif

	cl_char4 imgFormatColors_UNORM_SIGNED_INT8[] = { {{0,0,0,0}}, {{127,0,0,0}}, {{0,127,0,0}}, {{0,0,127,0}}, {{127,127,127,0}} };

    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT8, CL_MEM_OBJECT_IMAGE3D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_char4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT8);
		);
    }

    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT8, CL_MEM_OBJECT_IMAGE2D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_char4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT8);
		);
    }

#ifdef IMAGE1D
    {
		SCOPED_TRACE("CL_RGBA, CL_SIGNED_INT8, CL_MEM_OBJECT_IMAGE1D");
		m_imgFormat.image_channel_order = CL_RGBA;
		m_imgFormat.image_channel_data_type = CL_SIGNED_INT8;
		m_imgDesc.image_type = CL_MEM_OBJECT_IMAGE1D;
		EXPECT_NO_FATAL_FAILURE(
				iRet = runSingleImageTest<cl_char4>(inputFormatColors, imgFormatColors_UNORM_SIGNED_INT8);
		);
    }
#endif
}
