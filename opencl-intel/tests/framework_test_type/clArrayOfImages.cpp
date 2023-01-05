#if 0 // This test must be rewritten completely.
//|
//| TESTSUITE: ArrayOfImagesTest
//|
//| tests for Image2DArray which can are not qualify for 3DImage, 
//| i.e. special cases for Image2DArray
//|

#include "TestsHelpClasses.h"
#include "clParameterizedImages.h"
#include <cstdio>
#include <time.h>



class ArrayOfImagesTest :
  public ParameterizedImagesTest<CreateImage2DArray>
{

};
TEST_F(ArrayOfImagesTest, ImageInfo){
  cl_err_code err = ERROR_RESET;
  
  //Creation
    Image = clCreateImage2DArrayINTEL( context, CL_MEM_READ_ONLY , &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0,NULL , &err );
  EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateImage2DArrayINTEL");
    
  
//  cl_mem_object_type memObjType;
    size_t numImages = -1;

    err = clGetMemObjectInfo(Image, CL_MEM_ARRAY_SIZE, sizeof(size_t), &numImages, NULL);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetMemObjectInfo");
    EXPECT_EQ(NumImages, numImages) << ERR_FUNCTION("clGetMemObjectInfo");

    clMemWrapper image2d = clCreateImage2D(context, CL_MEM_READ_ONLY, &clFormat, ImagesWidth, ImagesHeight, 0, NULL, &err);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clCreateImage2D");

    err = clGetMemObjectInfo(image2d, CL_MEM_ARRAY_SIZE, sizeof(size_t), &numImages, NULL);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetMemObjectInfo");
    EXPECT_EQ(0, numImages) << ERR_FUNCTION("clGetMemObjectInfo");
    
    size_t memSize = -1;
    err = clGetMemObjectInfo(Image, CL_MEM_SIZE, sizeof(size_t), &memSize, NULL);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetMemObjectInfo");
    EXPECT_EQ(memSize, ImagesWidth * ImagesHeight * NumImages * 4) << ERR_FUNCTION("clGetMemObjectInfo");

    size_t size;
    cl_image_array_type val;
    err = clGetImageInfo(Image, CL_IMAGE_ARRAY_TYPE, sizeof(val), &val, &size);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetImageInfo");
    EXPECT_EQ(val, CL_IMAGE_ARRAY_SAME_DIMENSIONS);
    EXPECT_EQ(size, sizeof(val));

  Image.reset();
}

TEST_F(ArrayOfImagesTest, CreationFailures){
  cl_err_code err = ERROR_RESET;

  //CL INVALID VALUE
  Image = clCreateImage2DArrayINTEL(context, CL_MEM_READ_WRITE, &clFormat, (cl_image_array_type) 0, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(err)) << ERR_FUNCTION("clCreateImage2DArrayINTEL");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("clCreateImage2DArrayINTEL");

}

// disabled until BE adds support for image arrays in llvm_kernel.cpp
TEST_F(ArrayOfImagesTest, DISABLED_SetKernelArg)
{
    const char* src[] = {
        "__kernel void f(image2d_array_t image_array)"\
        "{"\
        "}"
    };
    cl_err_code err;

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &device, "", NULL, NULL);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clBuildProgram");
    cl_kernel kernel = clCreateKernel(program, "f", &err);
    ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clCreateKernel");
    Ocl2DImage Img(this->getSize(),false);
    Ocl2DImage ImgArray(NumImages,Img);

    Image = clCreateImage2DArrayINTEL( context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
    EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateImage2DArrayINTEL");
    
    cl_image_format imageFormats[10];
    cl_uint numImgFormats;
    err = clGetSupportedImageFormats(context, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D_ARRAY, 10, imageFormats, &numImgFormats);
    EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clGetSupportedImageFormats");

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &Image);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clSetKernelArg");
    const size_t workSize = 1;
    err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, &workSize, &workSize, 0, NULL, NULL);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clEnqueueNDRangeKernel");
    err = clFlush(cmd_queue);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFlush");
    err = clFinish(cmd_queue);
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clFinish");
}
//TODO: this test can be moved to clParameterizedImages.cpp once write_imagef will be implemented for 3D Image,
//you should uncomment the first line (Macro) and delete the second one(typedef)
//TODO: not tested due to no implementation , should check it is actully does it job....
#define SIMPLE_WRITING_KERNEL                                                  \
  __kernel void imageWrite(__write_only % s /*1 type of image */ srcImg,       \
                           __global % s * pPixels) {                           \
    size_t x = get_global_id(0);                                               \
    size_t y = get_global_id(1);                                               \
    size_t z = get_global_id(2);                                               \
    size_t pxlOff = x + y * DEFAULT_WIDTH;                                     \
    % s(srcImg, (int4)(x, y, z, 0), pPixels[pxlOff]); /*function name*/        \
  }

TEST_F(ArrayOfImagesTest, DISABLED_KernelWriteImagef){
//   TEMPLATE_DECLERATION_FIX;
  typedef CreateImage2DArray TypeParam;

  TypeParam ImageType;
  clFormat.image_channel_data_type = CL_UNORM_INT8;
  clFormat.image_channel_order = CL_RGBA;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage WriteImg(this->getSize(),true);
  //  Ocl2DImage ImgArray(NumImages,WriteImg);
  Image = createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  char kernelSource[MAX_SOURCE_SIZE];
  char *pString = kernelSource;
  sprintf(kernelSource,XSTR(SIMPLE_WRITING_KERNEL), ImageType.getBuiltinType(), "float4", "write_imagef");

  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(copyBufferWithKernelToPImage(Image, DstArray, (const char**)&pString));

  //now checking
  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,WriteImg)) << "READING THE WRITTEN IMAGES FAILED";
}
#endif
