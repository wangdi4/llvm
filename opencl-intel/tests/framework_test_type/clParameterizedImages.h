//|
//| TESTSUITE: ParameterizedImagesTest
//|
//| tests for Image2DArray and for 3DImage with can be executed on both of them
//| for tests that can run only on one of this images please refer to it proper
// suite.
//|
#pragma once
#include "CL/cl_ext.h"
#include "TestsHelpClasses.h"

// NOTE: most of this classes are generic so it can only be in the header file,
// an other reason is that you can find all the tests in the .cpp so the
// internal implementation is separated from it

// Default parameters (for now....)
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_NUM_IMAGES 50
#define DEFAULT_ELEMENT_SIZE 4

#define DISABLE_IMAGE_TEST(builtinType, message)                               \
  do {                                                                         \
    TypeParam ImageType;                                                       \
    if (string(ImageType.getBuiltinType()).compare(builtinType) == 0) {        \
      cout << (message) << endl                                                \
           << " in file: " << __FILE__ << " line(" << __LINE__ << ")" << endl; \
      return;                                                                  \
    }                                                                          \
  } while (0)
#define DISABLE_PARTLY_LOOP_IMAGE_TEST(builtinType, index)                     \
  TypeParam ImageType;                                                         \
  if (string(ImageType.getBuiltinType()).compare(builtinType) == 0) {          \
    if (index == 0) {                                                          \
      cout << ("THIS TEST IS PARTLY DISABLED") << endl                         \
           << " in file: " << __FILE__ << " line(" << __LINE__ << ")" << endl; \
    }                                                                          \
  } else
#define DISABLE_FULLY_IMAGE_TEST(builtinType)                                  \
  DISABLE_IMAGE_TEST(builtinType, "THIS TEST IS FULLY DISABLED")

class CreateImage {
public:
  virtual inline cl_mem_object_type getMemType() = 0;
  virtual inline cl_device_info getMaxWidth() = 0;
  virtual inline cl_device_info getMaxHeight() = 0;
  virtual inline cl_device_info getMaxSize() = 0;
  virtual inline char *getBuiltinType() = 0;
  virtual inline char *getBuiltinSizeFunction() = 0;
};
class CreateImage3D : public CreateImage {
public:
  inline cl_mem_object_type getMemType() { return CL_MEM_OBJECT_IMAGE3D; }
  inline cl_device_info getMaxWidth() { return CL_DEVICE_IMAGE3D_MAX_WIDTH; }
  inline cl_device_info getMaxHeight() { return CL_DEVICE_IMAGE3D_MAX_HEIGHT; }
  inline cl_device_info getMaxSize() { return CL_DEVICE_IMAGE3D_MAX_DEPTH; }
  inline char *getBuiltinType() { return "image3d_t"; }
  inline char *getBuiltinSizeFunction() { return "get_image_depth"; }
};
class CreateImage2DArray : public CreateImage {
public:
  inline cl_mem_object_type getMemType() { return CL_MEM_OBJECT_IMAGE2D_ARRAY; }
  inline cl_device_info getMaxWidth() { return CL_DEVICE_IMAGE2D_MAX_WIDTH; }
  inline cl_device_info getMaxHeight() { return CL_DEVICE_IMAGE2D_MAX_HEIGHT; }
  inline cl_device_info getMaxSize() { return CL_DEVICE_IMAGE_ARRAY_MAX_SIZE; }
  inline char *getBuiltinType() { return "image2d_array_t"; }
  inline char *getBuiltinSizeFunction() { return "get_array_size"; }
};

template <typename TypeParam> class ParameterizedImagesTest : public baseOcl {
public:
  ParameterizedImagesTest(void)
      : ImagesWidth(region[0]), ImagesHeight(region[1]) {
    srand((unsigned)time(NULL));
    Image = NULL;
  }

  ~ParameterizedImagesTest(void) {}
  //   static void SetUpTestCase(){
  //     baseOcl::SetUpTestCase();
  //   }
  //   static void TearDownTestCase(){
  //     baseOcl::TearDownTestCase();
  //   }
  void virtual SetUp() {
    baseOcl::SetUp();
    // default settings;
    ImagesWidth = DEFAULT_WIDTH;   // region[0] reference
    ImagesHeight = DEFAULT_HEIGHT; // region[1] reference
    NumImages = DEFAULT_NUM_IMAGES;
    ElementSize = DEFAULT_ELEMENT_SIZE; // RGBA
    region[2] = 1;
    clFormat.image_channel_order = CL_RGBA;
    clFormat.image_channel_data_type = CL_UNORM_INT8;
  }

  void virtual TearDown() { baseOcl::TearDown(); }
  cl_mem createTypedImage(cl_context context, cl_mem_flags flags,
                          const cl_image_format *image_format,
                          cl_image_array_type image_array_type,
                          const size_t *image_width, const size_t *image_height,
                          size_t num_images, size_t image_row_pitch,
                          size_t image_slice_pitch, void *host_ptr,
                          cl_int *errcode_ret);
  void readPImageAndCompare(const cl_mem &clImgArray, const Ocl2DImage &Img) {
    cl_err_code err = ERROR_RESET;
    Ocl2DImage ReadImg(getSize(), false);
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      err = clEnqueueReadImage(cmd_queue, clImgArray, CL_TRUE, origin, region,
                               0, 0, ReadImg, 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueReadImage") << ERR_IN_LOOP(index);

      ASSERT_TRUE(Img == ReadImg)
          << "The two Images are not the same" << ERR_IN_LOOP(index);
    }
  }
  void writeToPImage(const cl_mem &clImageArray, const Ocl2DImage &WriteImg) {
    cl_err_code err = ERROR_RESET;
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      // just writing without checking if what we wrote actually succeeded
      err = clEnqueueWriteImage(cmd_queue, clImageArray, CL_TRUE, origin,
                                region, 0, 0, WriteImg, 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueWriteImage") << ERR_IN_LOOP(index);
    }
  }
  void copyPImageToBuffer(const cl_mem &clImageArray, const cl_mem &DstBuffer) {
    cl_err_code err = ERROR_RESET;
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      err =
          clEnqueueCopyImageToBuffer(cmd_queue, Image, DstBuffer, origin,
                                     region, index * getSize(), 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueCopyImageToBuffer") << ERR_IN_LOOP(index);
    }
    clFinish(cmd_queue);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFinish");
  }
  void copyBufferToPImage(const cl_mem &clImageArray, const cl_mem &SrcBuffer) {
    cl_err_code err = ERROR_RESET;
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      err = clEnqueueCopyBufferToImage(cmd_queue, SrcBuffer, Image, 0, origin,
                                       region, 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueCopyBufferToImage") << ERR_IN_LOOP(index);
    }
    err = clFinish(cmd_queue);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFinish");
  }
  void copyImageToPImage(const cl_mem &clImageArray, const cl_mem &SrcImage) {
    cl_err_code err = ERROR_RESET;
    size_t srcOrigin[] = {0, 0, 0};
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      err = clEnqueueCopyImage(cmd_queue, SrcImage, Image, srcOrigin, origin,
                               region, 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueCopyImage") << ERR_IN_LOOP(index);
    }
    clFinish(cmd_queue);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFinish");
  }
  void copyPImageTo3DImage(const cl_mem &clImageArray, const cl_mem &DstImage) {
    cl_err_code err = ERROR_RESET;
    size_t srcOrigin[] = {0, 0, 0};
    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      err = clEnqueueCopyImage(cmd_queue, Image, DstImage, srcOrigin, origin,
                               region, 0, NULL, NULL);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueCopyImage") << ERR_IN_LOOP(index);
    }
    clFinish(cmd_queue);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFinish");
  }

  void mapPImageAndRead(const cl_mem &clImageArray, const Ocl2DImage &Img) {
    cl_err_code err = ERROR_RESET;

    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      size_t rowPitch = -1, slicePitch = -1;
      void *pMap = NULL;
      pMap = (cl_uchar *)clEnqueueMapImage(
          cmd_queue, clImageArray, CL_TRUE, CL_MAP_READ, origin, region,
          &rowPitch, &slicePitch, 0, NULL, NULL, &err);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);

      ASSERT_FALSE(NULL == pMap)
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);
      ASSERT_EQ(rowPitch, getRowPitch())
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);

      ASSERT_TRUE(memcmp((void *)Img, pMap, getSize()) == 0)
          << "The two Images are not the same" << ERR_IN_LOOP(index);

      DISABLE_PARTLY_LOOP_IMAGE_TEST("image2d_array_t",
                                     index) { // TODO: remove once bug is fixed
        err = clEnqueueUnmapMemObject(cmd_queue, clImageArray, pMap, 0, NULL,
                                      NULL);
        ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
            << ERR_FUNCTION("clEnqueueUnmapMemObject") << ERR_IN_LOOP(index);
      }
    }
  }

  void mapPImageAndWrite(const cl_mem &clImageArray,
                         const Ocl2DImage &WriteImg) {
    cl_err_code err = ERROR_RESET;

    for (int index = 0; index < NumImages; index++) {
      size_t origin[] = {0, 0, index};
      size_t rowPitch = -1, slicePitch = -1;
      void *pMap = NULL;
      pMap = (cl_uchar *)clEnqueueMapImage(
          cmd_queue, clImageArray, CL_TRUE, CL_MAP_WRITE, origin, region,
          &rowPitch, &slicePitch, 0, NULL, NULL, &err);
      ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);

      ASSERT_FALSE(NULL == pMap)
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);
      ASSERT_EQ(rowPitch, getRowPitch())
          << ERR_FUNCTION("clEnqueueMapImage") << ERR_IN_LOOP(index);

      memcpy(pMap, (void *)WriteImg, getSize());

      /* clEnqueueUnmapMemObject is not implemented for now for array of images,
       * since it is not clear whether to unmap all mapped 2D images, or there
       * is a bug in the spec and we need a special API call to unmap a specific
       * 2D image. */
      DISABLE_PARTLY_LOOP_IMAGE_TEST("image2d_array_t",
                                     index) { // TODO: remove once bug is fixed
        err = clEnqueueUnmapMemObject(cmd_queue, clImageArray, pMap, 0, NULL,
                                      NULL);
        ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
            << ERR_FUNCTION("clEnqueueUnmapMemObject") << ERR_IN_LOOP(index);
      }
    }
  }
  void copyPImageToBufferWithKernel(const cl_mem &clImageArray,
                                    Ocl2DImage &DstArray,
                                    const char **pString) {
    cl_err_code err = ERROR_RESET;

    ASSERT_NO_FATAL_FAILURE(simpleProgramCreation(pString));
    ASSERT_NO_FATAL_FAILURE(simpleProgramBuild());
    ASSERT_NO_FATAL_FAILURE(simpleKernelCreation());

    clMemWrapper DstBuffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE, DstArray.size(), NULL, &err);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
        << ERR_FUNCTION("clCreateBuffer");

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clImageArray);
    ASSERT_OCL_SUCCESS(err, clSetKernelArg);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &DstBuffer);
    ASSERT_OCL_SUCCESS(err, clSetKernelArg);

    size_t global_work_size[] = {ImagesWidth, ImagesHeight, NumImages};
    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 3, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    ASSERT_OCL_SUCCESS(err, clEnqueueNDRangeKernel);

    err = clFinish(cmd_queue);
    ASSERT_OCL_SUCCESS(err, clFinish);

    err = clEnqueueReadBuffer(cmd_queue, DstBuffer, CL_TRUE, 0, DstArray.size(),
                              DstArray, 0, NULL, NULL);
    ASSERT_OCL_SUCCESS(err, clEnqueueReadBuffer);
  }
  void copyBufferWithKernelToPImage(const cl_mem &clImageArray,
                                    const Ocl2DImage &SourceArray,
                                    const char **pString) {
    cl_err_code err = ERROR_RESET;

    ASSERT_NO_FATAL_FAILURE(simpleProgramCreation(pString));
    ASSERT_NO_FATAL_FAILURE(simpleProgramBuild());
    ASSERT_NO_FATAL_FAILURE(simpleKernelCreation());

    clMemWrapper DstBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                            SourceArray.size(), NULL, &err);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err))
        << ERR_FUNCTION("clCreateBuffer");

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clImageArray);
    ASSERT_OCL_SUCCESS(err, clSetKernelArg);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &DstBuffer);
    ASSERT_OCL_SUCCESS(err, clSetKernelArg);

    size_t global_work_size[] = {ImagesWidth, ImagesHeight, NumImages};
    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 3, NULL, global_work_size,
                                 NULL, 0, NULL, NULL);
    ASSERT_OCL_SUCCESS(err, clEnqueueNDRangeKernel);

    err = clFinish(cmd_queue);
    ASSERT_OCL_SUCCESS(err, clFinish);
  }
  size_t getRowPitch() const {
    return ImagesWidth * ElementSize; // num channels in CL_RGBA
  }
  size_t getSize() const { return getRowPitch() * ImagesHeight; }

protected:
  size_t &ImagesWidth;
  size_t &ImagesHeight;
  size_t region[3];
  int NumImages;
  size_t ElementSize;

  cl_image_format clFormat;
  clMemWrapper Image;
};
