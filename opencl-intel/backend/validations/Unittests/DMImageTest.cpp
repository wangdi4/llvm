// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "BufferContainerList.h"
#include "Image.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "gtest_wrapper.h"

#include <iostream>
#include <limits>
#include <list>

using namespace Validation;

class DMImageTest : public ::testing::Test {
protected:
  BufferContainerList inputList;

  void GenerateImage(ImageSizeDesc imSizes, ImageTypeVal imageType) {
    IBufferContainer *args = inputList.CreateBufferContainer();

    ImageChannelDataTypeVal dataType = OpenCL_FLOAT;
    ImageChannelOrderVal order = OpenCL_RGBA;

    size_t elemSize = sizeof(float) * 4 /*RGBA*/;
    imSizes.row = imSizes.width * elemSize;
    imSizes.slice = imSizes.row * imSizes.height;

    ImageDesc argDesc(imageType, imSizes, dataType, order, false);
    IMemoryObject *image = args->CreateImage(argDesc);

    // TODO: add image support to data generator.
    uint8_t *pData = reinterpret_cast<uint8_t *>(image->GetDataPtr());
    for (size_t i = 0; i < argDesc.GetSizeInBytes(); ++i) {
      pData[i] = i;
    }
  }

  void CheckValues(BufferContainerList &inp) {
    EXPECT_TRUE(inp.GetBufferContainerCount() ==
                inputList.GetBufferContainerCount());
    BufferContainer *expectedBC =
        static_cast<BufferContainer *>(inputList.GetBufferContainer(0));
    BufferContainer *readBC =
        static_cast<BufferContainer *>(inp.GetBufferContainer(0));
    EXPECT_TRUE(expectedBC->GetMemoryObjectCount() ==
                readBC->GetMemoryObjectCount());
    Image *expectedImage = static_cast<Image *>(expectedBC->GetMemoryObject(0));
    Image *readImage = static_cast<Image *>(readBC->GetMemoryObject(0));
    ASSERT_TRUE(expectedImage != NULL);
    ASSERT_TRUE(readImage != NULL);
    const ImageDesc *expectedDesc =
        static_cast<const ImageDesc *>(expectedImage->GetMemoryObjectDesc());
    const ImageDesc *readDesc =
        static_cast<const ImageDesc *>(readImage->GetMemoryObjectDesc());
    EXPECT_TRUE(expectedDesc->GetSizeInBytes() == readDesc->GetSizeInBytes());
    uint8_t *expectedData =
        reinterpret_cast<uint8_t *>(expectedImage->GetDataPtr());
    uint8_t *readData = reinterpret_cast<uint8_t *>(readImage->GetDataPtr());
    for (size_t i = 0; i < readDesc->GetSizeInBytes(); ++i) {
      EXPECT_TRUE(expectedData[i] == readData[i]);
    }
  }

  void WriteReadTest(const std::string &filename) {
    { // Write
      BinaryContainerListWriter saver(filename);
      saver.Write(&inputList);
    }

    { // Read
      BinaryContainerListReader loader(filename);
      BufferContainerList inp;
      loader.Read(&inp);

      CheckValues(inp);
    }
  }
};

TEST_F(DMImageTest, BinaryReadWrite) {
  printf("NOTE: The test DMImageTest.BinaryReadWrite has been manually "
         "disabled\n.");
  return;

  const char TEST_FILE_NAME[] = "ImageBinaryReadWriteCheck.bin";

  {
    // 1D image tests
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE1D;
    imSizes.width = 64;
    imSizes.height = 0;
    imSizes.depth = 0;
    imSizes.array_size = 0;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  {
    // 1D image buffer tests
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE1D_BUFFER;
    imSizes.width = 64;
    imSizes.height = 0;
    imSizes.depth = 0;
    imSizes.array_size = 0;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  {
    // 1D image array test
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE1D_ARRAY;
    imSizes.width = 64;
    imSizes.height = 0;
    imSizes.depth = 0;
    imSizes.array_size = 4;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  {
    // 2D image test
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE2D;
    imSizes.width = 64;
    imSizes.height = 48;
    imSizes.depth = 0;
    imSizes.array_size = 0;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  {
    // 2D image array test
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE2D_ARRAY;
    imSizes.width = 64;
    imSizes.height = 48;
    imSizes.depth = 0;
    imSizes.array_size = 4;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  {
    // 3D image test
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE3D;
    imSizes.width = 64;
    imSizes.height = 48;
    imSizes.depth = 4;
    imSizes.array_size = 0;
    GenerateImage(imSizes, imageType);

    WriteReadTest(TEST_FILE_NAME);
  }

  remove(TEST_FILE_NAME);
}

TEST_F(DMImageTest, XMLReadWrite) {

  printf(
      "NOTE: The test DMImageTest.XMLReadWrite has been manually disabled\n.");
  return;
  const char TEST_FILE_NAME[] = "ImageXMLReadWriteCheck.xml";

  {
    ImageSizeDesc imSizes;
    ImageTypeVal imageType = OpenCL_MEM_OBJECT_IMAGE2D;
    imSizes.width = 64;
    imSizes.height = 48;
    imSizes.depth = 0;
    imSizes.array_size = 0;
    GenerateImage(imSizes, imageType);

    XMLBufferContainerListWriter saver(TEST_FILE_NAME);
    saver.Write(&inputList);

    XMLBufferContainerListReader loader(TEST_FILE_NAME);
    BufferContainerList inp;
    loader.Read(&inp);

    CheckValues(inp);
  }

  remove(TEST_FILE_NAME);
}
