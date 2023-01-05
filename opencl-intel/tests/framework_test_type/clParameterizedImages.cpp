#if 0 // This test must be rewritten completely.

#include "TestsHelpClasses.h"
#include <time.h>

#include "clParameterizedImages.h"
#include <cstdio>
#include <string>
using testing::Types;

#define TEMPLATE_DECLERATION_FIX                                               \
  size_t &ImagesWidth = this->ImagesWidth;                                     \
  size_t &ImagesHeight = this->ImagesHeight;                                   \
  int &NumImages = this->NumImages;                                            \
  size_t &ElementSize = this->ElementSize;                                     \
  cl_image_format &clFormat = this->clFormat;                                  \
  clMemWrapper &Image = this->Image;                                           \
  cl_context &context = baseOcl::context;                                      \
  cl_device_id &device = baseOcl::device;                                      \
  cl_command_queue &cmd_queue = baseOcl::cmd_queue;                            \
  cl_kernel &kernel = this->kernel;




//--------------Internal Implementation------------------------

template<>
cl_mem ParameterizedImagesTest<CreateImage3D>::createTypedImage(cl_context context,
                     cl_mem_flags flags,
                     const cl_image_format *image_format,
                     cl_image_array_type image_array_type,
                     const size_t *image_width,
                     const size_t *image_height,
                     size_t num_images,
                     size_t image_row_pitch,
                     size_t image_slice_pitch,
                     void * host_ptr,
                     cl_int *errcode_ret){
                       return clCreateImage3D(context, flags, image_format, *image_width, *image_height, num_images, image_row_pitch,image_slice_pitch, host_ptr, errcode_ret );
}

template<> 
cl_mem ParameterizedImagesTest<CreateImage2DArray>::createTypedImage(cl_context context,
                      cl_mem_flags flags,
                      const cl_image_format *image_format,
                      cl_image_array_type image_array_type,
                      const size_t *image_width,
                      const size_t *image_height,
                      size_t num_images,
                      size_t image_row_pitch,
                      size_t image_slice_pitch,
                      void * host_ptr,
                      cl_int *errcode_ret){
                        return clCreateImage2DArrayINTEL(context, flags, image_format, image_array_type, image_width, image_height, num_images, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
}
//--------------Internal Implementation END------------------------


//--------------ACTUAL TESTS---------------------------------------

//this defines the Typed tests , meaning 0 = 3DImage AND 1 = 2DImageArray 
typedef Types<CreateImage3D, CreateImage2DArray> Implementations;
TYPED_TEST_CASE(ParameterizedImagesTest, Implementations);

TYPED_TEST(ParameterizedImagesTest, SimpleCreation){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;

  cl_err_code err = ERROR_RESET;

  //Creation
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY , &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0,NULL , &err );
  EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  cl_mem_object_type memObjType;
  err = clGetMemObjectInfo(Image, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memObjType, NULL);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetMemObjectInfo");
  EXPECT_EQ(ImageType.getMemType(), memObjType) << ERR_FUNCTION("clGetMemObjectInfo");
  Image.reset();
  

  //Creation
  Image = this->createTypedImage( context, CL_MEM_READ_WRITE , &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0,NULL , &err );
  EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  Image.reset();

  //Creation
  ImagesWidth = 800;
  ImagesHeight = 600;
  NumImages = 15;
  Image = this->createTypedImage( context, CL_MEM_WRITE_ONLY , &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0,NULL , &err );
  EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  Image.reset();

  //Creation
  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");


}

TYPED_TEST(ParameterizedImagesTest, CreationFailures){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;
  cl_err_code err = ERROR_RESET;

  //CL INVALID CONTEXT
  Image = this->createTypedImage( NULL, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_CONTEXT),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  //CL INVALID VALUE
  Image = this->createTypedImage( context, -1, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  //CL INVALID IMAGE FORMAT DESCRIPTOR
  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, NULL,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  cl_image_format badFormat;
  badFormat.image_channel_order = -1;
  badFormat.image_channel_data_type = -1;
  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &badFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  //CL INVALID IMAGE SIZE
  size_t zero = 0;
  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &zero, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &zero,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,1,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");


  size_t maxWidth, maxHeight, maxSize;
  err = clGetDeviceInfo(device, ImageType.getMaxWidth(), sizeof(size_t), &maxWidth, NULL);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetDeviceInfo");
  err = clGetDeviceInfo(device, ImageType.getMaxHeight(), sizeof(size_t), &maxHeight, NULL);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetDeviceInfo");
  err = clGetDeviceInfo(device, ImageType.getMaxSize(), sizeof(size_t), &maxSize, NULL);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetDeviceInfo");
  maxWidth++;
  maxHeight++;
  maxSize++;

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &maxWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &maxHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,maxSize,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");
  // TODO: image_row_pitch and image_slice_pitch checking missing.
  
  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch()-1,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch()+1,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,this->getSize()-1, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,this->getSize()+1, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_IMAGE_SIZE),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");



  //CL INVALID HOST PTR
  Image =this->createTypedImage( context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_HOST_PTR),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Image =this->createTypedImage( context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  EXPECT_EQ(oclErr(CL_INVALID_HOST_PTR),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY , &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  EXPECT_EQ(oclErr(CL_INVALID_HOST_PTR),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  EXPECT_EQ(NULL,Image) << ERR_FUNCTION("createTypedImage");


  //Creation failure
  cl_uint maxReadImageArgs, maxWriteImageArgs;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(maxReadImageArgs), &maxReadImageArgs, NULL);
  ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetDeviceInfo");
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(maxWriteImageArgs), &maxWriteImageArgs, NULL);
  ASSERT_EQ(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("clGetDeviceInfo");
  Image =this->createTypedImage( context, CL_MEM_READ_ONLY, &clFormat, CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight, maxReadImageArgs + 1, this->getRowPitch(), 0, NULL, &err);
  EXPECT_NE(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("createTypedImage");
  Image =this->createTypedImage( context, CL_MEM_WRITE_ONLY, &clFormat, CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight, maxWriteImageArgs + 1, this->getRowPitch(), 0, NULL, &err);
  EXPECT_NE(oclErr(CL_SUCCESS), oclErr(err)) << ERR_FUNCTION("createTypedImage");

  //CL IMAGE FORMAT NOT SUPPORTED TODO: what formats are not supported?

  //CL MEM OBJECT ALLOCATION FAILURE TODO: how to check?

  //CL INVALID OPERATION TODO: how to check?

  //CL OUT OF RESOURCES TODO: how to check?

  //CL OUT OF HOST MEMORY TODO: how to check?
}
TYPED_TEST(ParameterizedImagesTest, SimpleReading){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;
  ImagesWidth = 820;
  ImagesHeight = 600;
  NumImages = 20;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,Img)) << "READING FAILED";

}

TYPED_TEST(ParameterizedImagesTest, SimpleWriting) {
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  Ocl2DImage WriteImg(this->getSize(),true);
  EXPECT_NO_FATAL_FAILURE(this->writeToPImage(Image,WriteImg)) << "WRITING FAILED";

  //now checking
  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,WriteImg)) << "READING THE WRITTEN IMAGES FAILED";
}






TYPED_TEST(ParameterizedImagesTest, CopyImageToBuffer){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");

  clMemWrapper DstBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, ImgArray.size(), NULL, &err);
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateBuffer");
    clMemWrapper SrcBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, ImgArray.size(), NULL, &err);
    ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateBuffer");
    // failure
    size_t orig[3] = {0}, region[3] = {ImgArray.size() + 1, 1, 1};
    err = clEnqueueCopyBufferRect(cmd_queue, SrcBuffer, DstBuffer, orig, orig, region, 0, 0, 0, 0, 0, NULL, NULL);
    EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(err)) << ERR_FUNCTION("clEnqueueReadBuffer");

  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBuffer(Image, DstBuffer));

  //checks that the buffer is o.k. TODO: remove
  Ocl2DImage ReadImg(this->getSize(),false);
  err = clEnqueueReadBuffer(cmd_queue, DstBuffer, CL_TRUE, 0, Img.size(), ReadImg, 0, NULL, NULL);
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err));
  ASSERT_TRUE(Img == ReadImg);

  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  err = clEnqueueReadBuffer(cmd_queue,DstBuffer,CL_TRUE,0,ImgArray.size(),DstArray,0,NULL,NULL);
  ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clEnqueueReadBuffer");

  EXPECT_TRUE(ImgArray == DstArray);
}

TYPED_TEST(ParameterizedImagesTest, CopyBufferToImage){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  clMemWrapper SrcBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, Img.size(), Img, &err);
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateBuffer");

  //checks that the buffer is o.k TODO: remove
  Ocl2DImage ReadImg(this->getSize(),false);
  err = clEnqueueReadBuffer(cmd_queue, SrcBuffer, CL_TRUE, 0, Img.size(), ReadImg, 0, NULL, NULL);
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err));
  ASSERT_TRUE(Img == ReadImg);

  ASSERT_NO_FATAL_FAILURE(this->copyBufferToPImage(Image, SrcBuffer));

  //now checking
  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,Img)) << "READING THE WRITTEN IMAGES FAILED";
}


TYPED_TEST(ParameterizedImagesTest, CopyImageToPImage){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);


  clMemWrapper Image2D = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, &clFormat, ImagesWidth, ImagesHeight, this->getRowPitch(), Img, &err);

  ASSERT_NO_FATAL_FAILURE(this->copyImageToPImage(Image, Image2D));

  //now checking
  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,Img)) << "READING THE WRITTEN IMAGES FAILED";

}
TYPED_TEST(ParameterizedImagesTest, CopyPImageTo3DImage){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");

  clMemWrapper Image3D = clCreateImage3D(context, CL_MEM_READ_WRITE, &clFormat, ImagesWidth, ImagesHeight,NumImages, 0, 0, NULL, &err);
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("clCreateBuffer");

  ASSERT_NO_FATAL_FAILURE(this->copyPImageTo3DImage(Image, Image3D));

  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  size_t origin3D[3] = {0,0,0};
  size_t region3D[3] = {ImagesWidth,ImagesHeight,NumImages};
  err = clEnqueueReadImage(cmd_queue, Image3D, CL_TRUE, origin3D, region3D, 0, 0, DstArray, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS,err) << ERR_FUNCTION("clEnqueueReadBuffer");

  EXPECT_TRUE(ImgArray == DstArray);
}
TYPED_TEST(ParameterizedImagesTest, MapImageReading){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");

  ASSERT_NO_FATAL_FAILURE(this->mapPImageAndRead(Image, Img));
}

TYPED_TEST(ParameterizedImagesTest, MapImageWriting){
  TEMPLATE_DECLERATION_FIX;
  cl_err_code err = ERROR_RESET;

  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  Ocl2DImage WriteImg(this->getSize(),true);

  ASSERT_NO_FATAL_FAILURE(this->mapPImageAndWrite(Image, WriteImg)) << "WRITING WITH MAP FAILED";

  //now checking
  EXPECT_NO_FATAL_FAILURE(this->readPImageAndCompare(Image,WriteImg)) << "READING THE WRITTEN IMAGES FAILED";

}
//testing built-in functions

//simple kernel with hard-coded sampler
#define SIMPLE_READING_KERNEL                                                  \
  __kernel void imageRead(__read_only % s /*1 type of image */ srcImg,         \
                          __global uchar4 * pPixels) {                         \
    const sampler_t sampler = % s | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;     \
    size_t x = get_global_id(0);                                               \
    size_t y = get_global_id(1);                                               \
    size_t z = get_global_id(2);                                               \
    size_t pxlOff =                                                            \
        x + y * DEFAULT_WIDTH + z * DEFAULT_WIDTH * DEFAULT_HEIGHT;            \
    % s temp = % s(srcImg, sampler, (% s)(x, y, z, 0)); /*function name*/      \
    pPixels[pxlOff] = convert_uchar4(temp);                                    \
  }

TYPED_TEST(ParameterizedImagesTest, KernelReadImagef){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;
  DISABLE_FULLY_IMAGE_TEST("image2d_array_t"); //TODO: remove once 2d array of images builtin is implemented
  clFormat.image_channel_data_type = CL_UNORM_INT8;
  clFormat.image_channel_order = CL_RGBA;
  cl_err_code err = ERROR_RESET;
  
  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating
  
  char kernelSource[MAX_SOURCE_SIZE];
  char *pString = kernelSource;
  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "float4", "255.0f * read_imagef", "int4");
  
  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArray, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArray);

  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "float4", "255.0f * read_imagef", "float4");

  Ocl2DImage DstArrayFloat(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArrayFloat, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArrayFloat);
}
TYPED_TEST(ParameterizedImagesTest, KernelReadImagei){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;
  DISABLE_FULLY_IMAGE_TEST("image2d_array_t"); //TODO: remove once 2d array of images builtin is implemented
  clFormat.image_channel_data_type = CL_SIGNED_INT8;
  clFormat.image_channel_order = CL_RGBA;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  char kernelSource[MAX_SOURCE_SIZE];
  char *pString = kernelSource;
  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "int4", "read_imagei", "int4");

  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArray, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArray);

  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "int4", "read_imagei", "float4");

  Ocl2DImage DstArrayFloat(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArrayFloat, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArrayFloat);
}
TYPED_TEST(ParameterizedImagesTest, KernelReadImageui){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;
  DISABLE_FULLY_IMAGE_TEST("image2d_array_t"); //TODO: remove once 2d array of images builtin is implemented
  clFormat.image_channel_data_type = CL_UNSIGNED_INT8;
  clFormat.image_channel_order = CL_RGBA;
  cl_err_code err = ERROR_RESET;

  Ocl2DImage Img(this->getSize(),true);
  Ocl2DImage ImgArray(NumImages,Img);
  Image = this->createTypedImage( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,this->getRowPitch(),0, ImgArray , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  char kernelSource[MAX_SOURCE_SIZE];
  char *pString = kernelSource;
  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "uint4", "read_imageui", "int4");

  Ocl2DImage DstArray(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArray, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArray);

  sprintf(kernelSource,XSTR(SIMPLE_READING_KERNEL), ImageType.getBuiltinType(),"CLK_NORMALIZED_COORDS_FALSE", "uint4", "read_imageui", "float4");

  Ocl2DImage DstArrayFloat(this->getSize()*NumImages,false);
  ASSERT_NO_FATAL_FAILURE(this->copyPImageToBufferWithKernel(Image, DstArrayFloat, (const char**)&pString));

  EXPECT_TRUE(ImgArray == DstArrayFloat);
}

//simple kernel that checks size function
#define SIMPLE_GET_SIZE_KERNEL                                                 \
  __kernel void imageSize(__read_only % s /*1 type of image */ srcImg,         \
                          __global int *outputSize) {                          \
    *outputSize = % s(srcImg);                                                 \
  }

TYPED_TEST(ParameterizedImagesTest, KernelGetSize){
  TEMPLATE_DECLERATION_FIX;
  TypeParam ImageType;
  cl_err_code err = ERROR_RESET;
  DISABLE_FULLY_IMAGE_TEST("image2d_array_t"); //TODO: remove once 2d array of images builtin is implemented
  Image = this->createTypedImage( context, CL_MEM_READ_WRITE, &clFormat,CL_IMAGE_ARRAY_SAME_DIMENSIONS, &ImagesWidth, &ImagesHeight,NumImages,0,0, NULL , &err );
  ASSERT_EQ(oclErr(CL_SUCCESS),oclErr(err)) << ERR_FUNCTION("createTypedImage");
  // until here it was just creating

  char kernelSource[MAX_SOURCE_SIZE];
  char *pString = kernelSource;
  sprintf(kernelSource,XSTR(SIMPLE_GET_SIZE_KERNEL), ImageType.getBuiltinType(), ImageType.getBuiltinSizeFunction());
  ASSERT_NO_FATAL_FAILURE(this->simpleProgramCreation((const char**)&pString));
  ASSERT_NO_FATAL_FAILURE(this->simpleProgramBuild());
  ASSERT_NO_FATAL_FAILURE(this->simpleKernelCreation());


  size_t returnedSize = 0 ;
  clMemWrapper bufferSize=clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR ,sizeof(size_t), &returnedSize, &err);
  ASSERT_OCL_SUCCESS(err,clCreateBuffer);
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &Image);
  ASSERT_OCL_SUCCESS(err,clSetKernelArg);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferSize);
  ASSERT_OCL_SUCCESS(err,clSetKernelArg);

  size_t global_work_size[] = { ImagesWidth, ImagesHeight, NumImages };
  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 3, NULL, global_work_size, NULL, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, clEnqueueNDRangeKernel);

  err = clFinish(cmd_queue);
  ASSERT_OCL_SUCCESS(err,clFinish);
  
  err = clEnqueueReadBuffer(cmd_queue,bufferSize,CL_TRUE,0,sizeof(size_t),&returnedSize,0,NULL,NULL);
  ASSERT_OCL_SUCCESS(err, clEnqueueReadBuffer);

  ASSERT_EQ(NumImages, returnedSize);
}
#endif
