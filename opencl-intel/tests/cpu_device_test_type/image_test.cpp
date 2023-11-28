// Copyright (c) 2006 Intel Corporation
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

///////////////////////////////////////////////////////////
// logger_test.cpp
///////////////////////////////////////////////////////////

#include "CL/cl.h"
#include "cl_device_api.h"
#include "cpu_dev_test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

extern IOCLDeviceAgent *dev_entry;
#define IMAGE_WIDTH 10
#define IMAGE_HEIGHT 5
#define IMAGE_DEPTH 2
#define ELEMENT_SIZE 4
#define BUFFER_SIZE 1000

extern RTMemObjService localRTMemService;

/*******************************************************************************
 GetSupportedImageFormats test

*******************************************************************************/
bool clGetSupportedImageFormats_Test() {

  cl_mem_flags flags;
  cl_mem_object_type image_type;
  cl_uint IN num_entries = 30;
  cl_image_format formats[30];
  cl_uint num_entries_ret;

  flags = CL_MEM_READ_ONLY;
  image_type = CL_MEM_OBJECT_IMAGE2D;

  cl_int iRes = dev_entry->clDevGetSupportedImageFormats(
      flags, image_type, num_entries, formats, &num_entries_ret);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevGetSupportedImageFormats failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  num_entries = 0;

  iRes = dev_entry->clDevGetSupportedImageFormats(
      flags, image_type, num_entries, formats, &num_entries_ret);

  if (!CL_DEV_FAILED(iRes)) {
    printf("pclDevGetSupportedImageFormats failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dev_entry->clDevGetSupportedImageFormats(flags, image_type,
                                                  num_entries, NULL, 0);
  if (CL_DEV_FAILED(iRes)) // test suppose to fail
  {
    printf("pclDevGetSupportedImageFormats failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dev_entry->clDevGetSupportedImageFormats(
      flags, image_type, num_entries, NULL, &num_entries_ret);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevGetSupportedImageFormats failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

/*******************************************************************************
 copyImage
        Create command list
        Enqueue a copy command from one memory object to another
        Release command list
*******************************************************************************/
bool copyImage(bool profiling, IOCLDevMemoryObject *srcMemObj,
               IOCLDevMemoryObject *dstMemObj, cl_uint src_dim_count,
               cl_uint dst_dim_count, size_t src_origin[MAX_WORK_DIM],
               size_t dst_origin[MAX_WORK_DIM], size_t region[MAX_WORK_DIM]) {
  cl_int iRes;

  // Create command list
  cl_dev_cmd_list_props props = CL_DEV_LIST_ENABLE_OOO;
  cl_dev_cmd_list list;

  iRes = dev_entry->clDevCreateCommandList(props, 0, &list);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  // Execute command
  cl_dev_cmd_desc cmds;
  cl_dev_cmd_param_copy cpyParams;

  memset(&cpyParams, 0, sizeof(cl_dev_cmd_param_copy));
  cpyParams.srcMemObj = srcMemObj;
  cpyParams.dstMemObj = dstMemObj;
  cpyParams.src_dim_count = src_dim_count;
  cpyParams.dst_dim_count = dst_dim_count;
  for (unsigned int i = 0; i < MAX_WORK_DIM; i++) {
    cpyParams.src_origin[i] = src_origin[i];
    cpyParams.dst_origin[i] = dst_origin[i];
    cpyParams.region[i] = region[i];
  }

  cmds.type = CL_DEV_CMD_COPY;
  cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_COPY;
  cmds.params = &cpyParams;
  cmds.param_size = sizeof(cl_dev_cmd_param_copy);
  cmds.profiling = profiling;
  gExecDone = false;

  cl_dev_cmd_desc *cmdsBuff = &cmds;
  iRes = dev_entry->clDevCommandListExecute(list, &cmdsBuff, 1);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevCommandListExecute failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dev_entry->clDevFlushCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevFlushCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  while (!gExecDone) {
    SLEEP(10);
  }

  // release command list
  iRes = dev_entry->clDevReleaseCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("second clDevReleaseCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}
/*******************************************************************************
 writeImage
        Create command list
        Enqueue a write buffer
        Release command list
*******************************************************************************/
bool writeImage(bool profiling, IOCLDevMemoryObject *memObj, void *pHostImage,
                unsigned int dim_count, size_t width, size_t height,
                size_t depth, bool bIsBuffer) {
  cl_int iRes;

  // Create command list
  cl_dev_cmd_list_props props = CL_DEV_LIST_ENABLE_OOO;
  cl_dev_cmd_list list;

  iRes = dev_entry->clDevCreateCommandList(props, 0, &list);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Execute command
  cl_dev_cmd_desc cmds;
  cl_dev_cmd_param_rw rwParams;

  memset(&rwParams, 0, sizeof(cl_dev_cmd_param_rw));
  rwParams.memObj = memObj;
  rwParams.ptr = pHostImage;
  rwParams.dim_count = dim_count;
  rwParams.region[0] = width;
  rwParams.region[1] = height;
  rwParams.region[2] = depth;

  for (unsigned int i = 0; i < MAX_WORK_DIM; i++) {
    rwParams.origin[i] = 0;
  }
  if (!bIsBuffer) {
    rwParams.pitch[0] = rwParams.region[0] * ELEMENT_SIZE;
    rwParams.pitch[1] = rwParams.pitch[0] * height;
  } else {
    rwParams.pitch[0] = rwParams.region[0];
    rwParams.pitch[1] = 0;
  }

  cmds.type = CL_DEV_CMD_WRITE;
  cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_WRITE;
  cmds.params = &rwParams;
  cmds.param_size = sizeof(cl_dev_cmd_param_rw);
  cmds.profiling = profiling;
  gExecDone = false;
  cl_dev_cmd_desc *cmdsBuff = &cmds;
  iRes = dev_entry->clDevCommandListExecute(list, &cmdsBuff, 1);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevCommandListExecute failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dev_entry->clDevFlushCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevFlushCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  while (!gExecDone) {
    SLEEP(10);
  }
  // release command list
  iRes = dev_entry->clDevReleaseCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("second clDevReleaseCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

/*******************************************************************************
 readImage
        Create command list
        Enqueue a read buffer
        Release command list
*******************************************************************************/
bool readImage(bool profiling, IOCLDevMemoryObject *memObj, void *pHostImage,
               unsigned int dim_count, size_t width, size_t height,
               size_t depth, bool bIsBuffer) {
  cl_int iRes;

  // Create command list
  cl_dev_cmd_list_props props = CL_DEV_LIST_ENABLE_OOO;
  cl_dev_cmd_list list;

  iRes = dev_entry->clDevCreateCommandList(props, 0, &list);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Execute command
  cl_dev_cmd_desc cmds;
  cl_dev_cmd_param_rw rwParams;

  memset(&rwParams, 0, sizeof(cl_dev_cmd_param_rw));
  rwParams.memObj = memObj;
  rwParams.ptr = pHostImage;
  rwParams.dim_count = dim_count;
  rwParams.region[0] = width;
  rwParams.region[1] = height;
  rwParams.region[2] = depth;

  for (unsigned int i = 0; i < MAX_WORK_DIM; i++) {
    rwParams.origin[i] = 0;
  }
  if (!bIsBuffer) {
    rwParams.pitch[0] = rwParams.region[0] * ELEMENT_SIZE;
    rwParams.pitch[1] = rwParams.pitch[0] * height;
  } else {
    rwParams.pitch[0] = rwParams.region[0];
    rwParams.pitch[1] = 0;
  }

  cmds.type = CL_DEV_CMD_READ;
  cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_READ;
  cmds.params = &rwParams;
  cmds.param_size = sizeof(cl_dev_cmd_param_rw);
  cmds.profiling = profiling;

  gExecDone = false;

  cl_dev_cmd_desc *cmdsBuff = &cmds;
  iRes = dev_entry->clDevCommandListExecute(list, &cmdsBuff, 1);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevCommandListExecute failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dev_entry->clDevFlushCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevFlushCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  while (!gExecDone) {
    SLEEP(10);
  }

  // release command list
  iRes = dev_entry->clDevReleaseCommandList(list);
  if (CL_DEV_FAILED(iRes)) {
    printf("second clDevReleaseCommandList failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}
bool clReadWrite2DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, 1};
  IOCLDevMemoryObject *memObj;
  unsigned int dim_count = 2;

  char *hostWriteImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);
  char *hostReadImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);

  if (NULL == hostWriteImage || NULL == hostReadImage) {
    return false;
  }
  // Create 2D Image Memory Object
  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &memObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  unsigned int tmp;
  // Fill image data to write
  for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
      tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE;
      hostWriteImage[tmp] = tmp;
      hostWriteImage[tmp + 1] = tmp + 1;
      hostWriteImage[tmp + 2] = tmp + 2;
      hostWriteImage[tmp + 3] = tmp + 3;
    }

  // Write Host Image into the created memory image

  if (!writeImage(profiling, memObj, hostWriteImage, 2, IMAGE_WIDTH,
                  IMAGE_HEIGHT, 0, false)) {
    return false;
  }

  // Read Image to the host pointer compare to original memory that was written

  if (!readImage(profiling, memObj, hostReadImage, 2, IMAGE_WIDTH, IMAGE_HEIGHT,
                 0, false)) {
    return false;
  }

  if (0 != memcmp(hostWriteImage, hostReadImage,
                  IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT)) {
    printf("Read image different from Written image\n");
    return false;
  }

  free(hostReadImage);
  free(hostWriteImage);

  iRes = memObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopy2DImageto2DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, 1};
  unsigned int dim_count = 2;
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  // Create two image objects
  char *hostWriteImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);
  char *hostReadImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);

  if (NULL == hostWriteImage || NULL == hostReadImage) {
    return false;
  }

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &dstMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, dim_count, dim,
                                            &localRTMemService, &srcMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Write data to the source image

  unsigned int tmp;
  // Fill image data to write
  for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
      tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE;
      hostWriteImage[tmp] = tmp;
      hostWriteImage[tmp + 1] = tmp + 1;
      hostWriteImage[tmp + 2] = tmp + 2;
      hostWriteImage[tmp + 3] = tmp + 3;
    }

  // Write Host Image into the created memory image
  if (!writeImage(profiling, srcMemObj, hostWriteImage, 2, IMAGE_WIDTH,
                  IMAGE_HEIGHT, 0, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy image to dst image
  size_t src_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t dst_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t region[MAX_WORK_DIM] = {IMAGE_WIDTH, IMAGE_HEIGHT, 1};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 2, 2, src_origin, dst_origin,
                 region)) {
    printf("Copy 2D to 2D image fail\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Read the new image and check that memory is what was originally written
  if (!readImage(profiling, dstMemObj, hostReadImage, 2, IMAGE_WIDTH,
                 IMAGE_HEIGHT, 0, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  if (0 != memcmp(hostWriteImage, hostReadImage,
                  IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT)) {
    printf("Read image different from Written image\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region in the image starting at origin not 0
  // Write 0 to destination image
  memset(hostReadImage, 0, IMAGE_HEIGHT * IMAGE_WIDTH * 2);
  // Write Host Image into the created memory image

  if (!writeImage(profiling, dstMemObj, hostReadImage, 2, IMAGE_WIDTH,
                  IMAGE_HEIGHT, 0, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region from memObj into dstMemObj starting at 2, 2
  src_origin[0] = 2;
  src_origin[1] = 2;
  dst_origin[0] = 2;
  dst_origin[1] = 2;
  // Region is in pixels
  region[0] = IMAGE_WIDTH - 2;
  region[1] = IMAGE_HEIGHT - 2;

  if (!copyImage(profiling, srcMemObj, dstMemObj, 2, 2, src_origin, dst_origin,
                 region)) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D to 2D image fail\n");
    return false;
  }

  // Read Image
  if (!readImage(profiling, dstMemObj, hostReadImage, 2, IMAGE_WIDTH,
                 IMAGE_HEIGHT, 0, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Expect 0 in image[0]
  if (hostReadImage[0] != 0) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D region to 2D region - wrong use of orgign\n");
    return false;
  }

  for (unsigned int j = 2; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 2; i < IMAGE_WIDTH; ++i) {
      tmp = j * IMAGE_WIDTH * ELEMENT_SIZE + i * ELEMENT_SIZE;
      if (hostWriteImage[tmp] != hostReadImage[tmp]) {
        free(hostReadImage);
        free(hostWriteImage);
        printf("Copy 2D region to 2D region - wrong use of orgign\n");
        return false;
      }
    }

  free(hostReadImage);
  free(hostWriteImage);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {

    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopy2DImageToBuffer_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, 1};
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  char *hostWriteImage, *hostReadBuffer;
  unsigned int dim_count = 2;

  // Create 2D Image Memory Object

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &srcMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Create Buffer Object
  dim[0] = IMAGE_WIDTH * IMAGE_HEIGHT * ELEMENT_SIZE;
  localRTMemService.SetupState(&format, 1, dim, NULL, CL_MEM_OBJECT_BUFFER);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, 1, dim,
                                            &localRTMemService, &dstMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Write Data to Image object
  hostReadBuffer = (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);
  hostWriteImage = (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT);
  if (NULL == hostWriteImage || NULL == hostReadBuffer) {
    printf("Memory alloc failed\n");
    return false;
  }

  // Fill image data to write
  unsigned int tmp;
  for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
      tmp = ELEMENT_SIZE * (j * IMAGE_WIDTH + i);
      hostWriteImage[tmp] = tmp;
      hostWriteImage[tmp + 1] = tmp + 1;
      hostWriteImage[tmp + 2] = tmp + 2;
      hostWriteImage[tmp + 3] = tmp + 3;
    }

  // Write Host Image into the created memory image

  if (!writeImage(profiling, srcMemObj, hostWriteImage, 2, IMAGE_WIDTH,
                  IMAGE_HEIGHT, 0, false)) {
    return false;
  }
  // Copy Image to Buffer
  size_t src_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t dst_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t region[MAX_WORK_DIM] = {IMAGE_WIDTH, IMAGE_HEIGHT, 1};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 2, 1, src_origin, dst_origin,
                 region)) {
    printf("Copy 2D to buffer fail\n");
    return false;
  }

  // Read Buffer
  if (!readImage(profiling, dstMemObj, hostReadBuffer, 1,
                 IMAGE_WIDTH * IMAGE_HEIGHT * ELEMENT_SIZE, 1, 1, true)) {
    return false;
  }
  // Compare Results
  if (0 != memcmp(hostWriteImage, hostReadBuffer,
                  IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT)) {
    printf("Read image different from Written image\n");
    free(hostReadBuffer);
    free(hostWriteImage);
    return false;
  }
  // Delete the memory objects
  free(hostWriteImage);
  free(hostReadBuffer);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopyBufferToBuffer_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {BUFFER_SIZE, 0, 0};
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  char *hostWriteBuffer, *hostReadBuffer;
  unsigned int dim_count = 1;

  // Create Source Buffer Memory Object

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_BUFFER);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &srcMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Create Destination Buffer Object

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_BUFFER);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, dim_count, dim,
                                            &localRTMemService, &dstMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Write Data to Buffer
  hostReadBuffer = (char *)malloc(BUFFER_SIZE);
  hostWriteBuffer = (char *)malloc(BUFFER_SIZE);
  if (NULL == hostWriteBuffer || NULL == hostReadBuffer) {
    printf("Memory alloc failed to allocate memory buffer\n");
    return false;
  }

  // Fill buffer with data to write
  for (unsigned int j = 0; j < BUFFER_SIZE; j++) {
    hostWriteBuffer[j] = j;
  }

  // Write Host Buffer into the created memory image
  if (!writeImage(profiling, srcMemObj, hostWriteBuffer, 1, BUFFER_SIZE, 1, 1,
                  true)) {
    return false;
  }
  // Copy Image to Buffer
  size_t src_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t dst_origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t region[MAX_WORK_DIM] = {BUFFER_SIZE, 1, 1};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 1, 1, src_origin, dst_origin,
                 region)) {

    free(hostReadBuffer);
    free(hostWriteBuffer);
    printf("Copy buffer to buffer fail\n");
    return false;
  }

  // Read Buffer
  if (!readImage(profiling, dstMemObj, hostReadBuffer, 1, BUFFER_SIZE, 1, 1,
                 true)) {
    free(hostReadBuffer);
    free(hostWriteBuffer);
    return false;
  }
  // Compare Results
  if (0 != memcmp(hostWriteBuffer, hostReadBuffer, BUFFER_SIZE)) {
    printf("Read buffer different from Written image\n");
    free(hostReadBuffer);
    free(hostWriteBuffer);
    return false;
  }
  // Delete the memory objects
  free(hostWriteBuffer);
  free(hostReadBuffer);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}
bool clCopy3DImageto3DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  unsigned int dim_count = 3;
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  // Create two image objects
  char *hostWriteImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT * IMAGE_DEPTH);
  char *hostReadImage =
      (char *)malloc(IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT * IMAGE_DEPTH);

  if (NULL == hostWriteImage || NULL == hostReadImage) {
    return false;
  }

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &dstMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, dim_count, dim,
                                            &localRTMemService, &srcMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Write data to the source image

  unsigned int tmp, sliceSize;
  // Fill image data to write
  sliceSize = IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT;
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
        tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE + k * sliceSize;
        hostWriteImage[tmp] = tmp;
        hostWriteImage[tmp + 1] = tmp + 1;
        hostWriteImage[tmp + 2] = tmp + 2;
        hostWriteImage[tmp + 3] = tmp + 3;
      }

  // Write Host Image into the created memory image

  if (!writeImage(profiling, srcMemObj, hostWriteImage, 3, IMAGE_WIDTH,
                  IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy image to dst image
  size_t src_origin[3] = {0, 0, 0};
  size_t dst_origin[3] = {0, 0, 0};
  size_t region[3] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 3, 3, src_origin, dst_origin,
                 region)) {
    printf("Copy 3D to 3D iamge fail\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Read the new image and check that memory is what was originally written
  if (!readImage(profiling, dstMemObj, hostReadImage, 3, IMAGE_WIDTH,
                 IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  if (0 != memcmp(hostWriteImage, hostReadImage, IMAGE_DEPTH * sliceSize)) {
    printf("Read image different from Written image\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region in the image starting at origin not 0
  // Write 0 to destination image
  memset(hostReadImage, 0, IMAGE_DEPTH * sliceSize);
  // Write Host Image into the created memory image

  if (!writeImage(profiling, dstMemObj, hostReadImage, 3, IMAGE_WIDTH,
                  IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region from memObj into dstMemObj starting at 2, 2, 0
  src_origin[0] = 2;
  src_origin[1] = 2;
  src_origin[2] = 0;
  dst_origin[0] = 2;
  dst_origin[1] = 2;
  dst_origin[2] = 0;
  // Region is in pixels
  region[0] = IMAGE_WIDTH - 2;
  region[1] = IMAGE_HEIGHT - 2;
  region[2] = IMAGE_DEPTH;
  if (!copyImage(profiling, srcMemObj, dstMemObj, 3, 3, src_origin, dst_origin,
                 region)) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D to 2D iamge fail\n");
    return false;
  }

  // Read Image
  if (!readImage(profiling, dstMemObj, hostReadImage, 3, IMAGE_WIDTH,
                 IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Expect 0 in image[0]
  if (hostReadImage[0] != 0) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D region to 2D region - wrong use of orgign\n");
    return false;
  }
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 2; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 2; i < IMAGE_WIDTH; ++i) {
        tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE + k * sliceSize;
        if (hostWriteImage[tmp] != hostReadImage[tmp]) {
          free(hostReadImage);
          free(hostWriteImage);
          printf("Copy 2D region to 2D region - wrong use of orgign\n");
          return false;
        }
      }

  free(hostReadImage);
  free(hostWriteImage);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {

    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopy3DImageToBuffer_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  char *hostWriteImage, *hostReadBuffer;
  unsigned int dim_count = 3;
  unsigned int uiImageSlizeSize = IMAGE_WIDTH * IMAGE_HEIGHT * ELEMENT_SIZE;
  unsigned int uiBufferSize;

  // Create 3D Image Memory Object

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &srcMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Create Buffer Object
  uiBufferSize = uiImageSlizeSize * IMAGE_DEPTH;
  dim[0] = uiBufferSize;

  localRTMemService.SetupState(&format, 1, dim, NULL, CL_MEM_OBJECT_BUFFER);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, 1, dim,
                                            &localRTMemService, &dstMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Write Data to Image object
  hostReadBuffer = (char *)malloc(uiBufferSize);
  hostWriteImage = (char *)malloc(uiBufferSize);
  if (NULL == hostWriteImage || NULL == hostReadBuffer) {
    printf("Memory alloc failed\n");
    return false;
  }

  // Fill image data to write
  unsigned int tmp;
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
        tmp = k * uiImageSlizeSize + ELEMENT_SIZE * (j * IMAGE_WIDTH + i);
        hostWriteImage[tmp] = tmp;
        hostWriteImage[tmp + 1] = tmp + 1;
        hostWriteImage[tmp + 2] = tmp + 2;
        hostWriteImage[tmp + 3] = tmp + 3;
      }

  // Write Host Image into the created memory image

  if (!writeImage(profiling, srcMemObj, hostWriteImage, 3, IMAGE_WIDTH,
                  IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    return false;
  }
  // Copy Image to Buffer
  size_t src_origin[3] = {0, 0, 0};
  size_t dst_origin[3] = {0, 0, 0};
  size_t region[3] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 3, 1, src_origin, dst_origin,
                 region)) {
    printf("Copy 2D to buffer fail\n");
    return false;
  }

  // Read Buffer
  if (!readImage(profiling, dstMemObj, hostReadBuffer, 1, uiBufferSize, 1, 1,
                 true)) {
    return false;
  }
  // Compare Results
  if (0 != memcmp(hostWriteImage, hostReadBuffer, uiBufferSize)) {
    printf("Read image different from Written image\n");
    free(hostReadBuffer);
    free(hostWriteImage);
    return false;
  }
  // Delete the memory objects
  free(hostWriteImage);
  free(hostReadBuffer);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}
bool clCopyBufferTo3DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  char *hostWriteBuffer, *hostReadImage;
  unsigned int dim_count = 3;
  unsigned int uiImageSlizeSize = IMAGE_WIDTH * IMAGE_HEIGHT * ELEMENT_SIZE;
  unsigned int uiBufferSize;

  // Create 3D Image Memory Object

  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dim_count, dim, &localRTMemService, &dstMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Create Buffer Object
  uiBufferSize = uiImageSlizeSize * IMAGE_DEPTH;
  dim[0] = uiBufferSize;

  localRTMemService.SetupState(&format, 1, dim, NULL, CL_MEM_OBJECT_BUFFER);
  iRes = dev_entry->clDevCreateMemoryObject(0, flags, &format, 1, dim,
                                            &localRTMemService, &srcMemObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Write Data to Buffer object
  hostWriteBuffer = (char *)malloc(uiBufferSize);
  hostReadImage = (char *)malloc(uiBufferSize);
  if (NULL == hostReadImage || NULL == hostWriteBuffer) {
    printf("Memory alloc failed\n");
    return false;
  }

  // Fill bufer data to write
  unsigned int tmp;
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
        tmp = k * uiImageSlizeSize + ELEMENT_SIZE * (j * IMAGE_WIDTH + i);
        hostWriteBuffer[tmp] = tmp;
        hostWriteBuffer[tmp + 1] = tmp + 1;
        hostWriteBuffer[tmp + 2] = tmp + 2;
        hostWriteBuffer[tmp + 3] = tmp + 3;
      }

  // Write Host Buffer into the created memory buffer

  if (!writeImage(profiling, srcMemObj, hostWriteBuffer, 1, uiBufferSize, 1, 1,
                  true)) {
    return false;
  }
  // Copy Buffer to Image
  size_t src_origin[3] = {0, 0, 0};
  size_t dst_origin[3] = {0, 0, 0};
  size_t region[3] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};

  if (!copyImage(profiling, srcMemObj, dstMemObj, 1, 3, src_origin, dst_origin,
                 region)) {
    printf("buffer to image fail\n");
    return false;
  }

  // Read Image
  if (!readImage(profiling, dstMemObj, hostReadImage, 3, IMAGE_WIDTH,
                 IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    return false;
  }
  // Compare Results
  if (0 != memcmp(hostWriteBuffer, hostReadImage, uiBufferSize)) {
    printf("Read image different from Written image\n");
    free(hostWriteBuffer);
    free(hostReadImage);
    return false;
  }
  // Delete the memory objects
  free(hostWriteBuffer);
  free(hostReadImage);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopy3DImageto2DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  unsigned int src_dim_count = 3;
  unsigned int dst_dim_count = 2;
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  unsigned int ui2DImageSlizeSize = IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT;
  // Create two image objects
  char *hostWriteImage = (char *)malloc(ui2DImageSlizeSize * IMAGE_DEPTH);
  char *hostReadImage = (char *)malloc(ui2DImageSlizeSize);

  if (NULL == hostWriteImage || NULL == hostReadImage) {
    return false;
  }

  localRTMemService.SetupState(&format, dst_dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dst_dim_count, dim, &localRTMemService, &dstMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  localRTMemService.SetupState(&format, src_dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, src_dim_count, dim, &localRTMemService, &srcMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Write data to the source image

  unsigned int tmp;
  // Fill image data to write
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
        tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE + k * ui2DImageSlizeSize;
        hostWriteImage[tmp] = tmp;
        hostWriteImage[tmp + 1] = tmp + 1;
        hostWriteImage[tmp + 2] = tmp + 2;
        hostWriteImage[tmp + 3] = tmp + 3;
      }

  // Write 3D Host Image into the created memory image

  if (!writeImage(profiling, srcMemObj, hostWriteImage, src_dim_count,
                  IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy image to dst image
  size_t src_origin[3] = {0, 0, 0};
  size_t dst_origin[3] = {0, 0, 0};
  size_t region[3] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};

  if (!copyImage(profiling, srcMemObj, dstMemObj, src_dim_count, dst_dim_count,
                 src_origin, dst_origin, region)) {
    printf("Copy 3D to 2D iamge fail\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Read the 2D image and check that memory is what was originally written
  if (!readImage(profiling, dstMemObj, hostReadImage, dst_dim_count,
                 IMAGE_WIDTH, IMAGE_HEIGHT, 1, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  if (0 != memcmp(hostWriteImage, hostReadImage, ui2DImageSlizeSize)) {
    printf("Read image different from Written image\n");
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region in the image starting at origin not 0
  // Write 0 to destination image
  memset(hostReadImage, 0, ui2DImageSlizeSize);
  // Write Host Image into the 2D image

  if (!writeImage(profiling, dstMemObj, hostReadImage, dst_dim_count,
                  IMAGE_WIDTH, IMAGE_HEIGHT, 1, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }

  // Copy a region from memObj into dstMemObj starting at 2, 2, 0
  src_origin[0] = 2;
  src_origin[1] = 2;
  src_origin[2] = 0;
  dst_origin[0] = 2;
  dst_origin[1] = 2;
  dst_origin[2] = 0;
  // Region is in pixels
  region[0] = IMAGE_WIDTH - 2;
  region[1] = IMAGE_HEIGHT - 2;
  region[2] = IMAGE_DEPTH;
  if (!copyImage(profiling, srcMemObj, dstMemObj, src_dim_count, dst_dim_count,
                 src_origin, dst_origin, region)) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D to 2D iamge fail\n");
    return false;
  }

  // Read Image
  if (!readImage(profiling, dstMemObj, hostReadImage, dst_dim_count,
                 IMAGE_WIDTH, IMAGE_HEIGHT, 1, false)) {
    free(hostReadImage);
    free(hostWriteImage);
    return false;
  }
  // Expect 0 in image[0]
  if (hostReadImage[1] != 0) {
    free(hostReadImage);
    free(hostWriteImage);
    printf("Copy 2D region to 2D region - wrong use of orgign\n");
    return false;
  }
  for (unsigned int j = 2; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 2; i < IMAGE_WIDTH; ++i) {
      tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE;
      if (hostWriteImage[tmp] != hostReadImage[tmp]) {
        free(hostReadImage);
        free(hostWriteImage);
        printf("Copy 3D region to 2D region - wrong use of orgign\n");
        return false;
      }
    }

  free(hostReadImage);
  free(hostWriteImage);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {

    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool clCopy2DImageto3DImage_Test(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  unsigned int dst_dim_count = 3;
  unsigned int src_dim_count = 2;
  IOCLDevMemoryObject *srcMemObj, *dstMemObj;
  unsigned int ui2DImageSlizeSize = IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT;
  // Create two image objects
  char *hostDstImage = (char *)malloc(ui2DImageSlizeSize * IMAGE_DEPTH);
  char *hostSourceImage = (char *)malloc(ui2DImageSlizeSize);

  if (NULL == hostDstImage || NULL == hostSourceImage) {
    return false;
  }

  localRTMemService.SetupState(&format, dst_dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, dst_dim_count, dim, &localRTMemService, &dstMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }

  localRTMemService.SetupState(&format, src_dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE2D);
  iRes = dev_entry->clDevCreateMemoryObject(
      0, flags, &format, src_dim_count, dim, &localRTMemService, &srcMemObj);
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }
  // Write data to the source image

  memset(hostDstImage, 0, ui2DImageSlizeSize * IMAGE_DEPTH);
  if (!writeImage(profiling, dstMemObj, hostDstImage, dst_dim_count,
                  IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }
  // Write data to the 3D image
  unsigned int tmp;
  // Fill 2D image data to write
  for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
      tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE;
      hostSourceImage[tmp] = tmp;
      hostSourceImage[tmp + 1] = tmp + 1;
      hostSourceImage[tmp + 2] = tmp + 2;
      hostSourceImage[tmp + 3] = tmp + 3;
    }

  // Write 2D Host Image into the created memory image
  if (!writeImage(profiling, srcMemObj, hostSourceImage, src_dim_count,
                  IMAGE_WIDTH, IMAGE_HEIGHT, 0, false)) {
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }

  // Copy image to dst image
  size_t src_origin[3] = {0, 0, 0};
  size_t dst_origin[3] = {0, 0, 0};
  size_t region[3] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};

  if (!copyImage(profiling, srcMemObj, dstMemObj, src_dim_count, dst_dim_count,
                 src_origin, dst_origin, region)) {
    printf("Copy 3D to 2D iamge fail\n");
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }

  // Read the 3D image and check that memory is what was originally written
  if (!readImage(profiling, dstMemObj, hostDstImage, dst_dim_count, IMAGE_WIDTH,
                 IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }
  if ((0 != memcmp(hostDstImage, hostSourceImage, ui2DImageSlizeSize)) ||
      (0 != hostDstImage[ui2DImageSlizeSize + 1])) {
    printf("Read image different from Written image\n");
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }

  // Copy a region in the image starting at origin not 0
  // Write 0 to destination image
  memset(hostDstImage, 0, ui2DImageSlizeSize * IMAGE_DEPTH);
  // Write Host Image into the 2D image

  if (!writeImage(profiling, dstMemObj, hostDstImage, dst_dim_count,
                  IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }

  // Copy a region from memObj into dstMemObj starting at 2, 2, 0
  src_origin[0] = 2;
  src_origin[1] = 2;
  src_origin[2] = 0;
  dst_origin[0] = 2;
  dst_origin[1] = 2;
  dst_origin[2] = 0;
  // Region is in pixels
  region[0] = IMAGE_WIDTH - 2;
  region[1] = IMAGE_HEIGHT - 2;
  region[2] = IMAGE_DEPTH;
  if (!copyImage(profiling, srcMemObj, dstMemObj, src_dim_count, dst_dim_count,
                 src_origin, dst_origin, region)) {
    free(hostDstImage);
    free(hostSourceImage);
    printf("Copy 2D to 3D iamge fail\n");
    return false;
  }

  // Read Image
  if (!readImage(profiling, dstMemObj, hostDstImage, dst_dim_count, IMAGE_WIDTH,
                 IMAGE_HEIGHT, IMAGE_DEPTH, false)) {
    free(hostDstImage);
    free(hostSourceImage);
    return false;
  }
  // Expect 0 in image[0]
  if (hostDstImage[1] != 0) {
    free(hostDstImage);
    free(hostSourceImage);
    printf("Copy 2D region to 2D region - wrong use of orgign\n");
    return false;
  }
  for (unsigned int j = 2; j < IMAGE_HEIGHT; j++)
    for (unsigned int i = 2; i < IMAGE_WIDTH; ++i) {
      tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE;
      if (hostDstImage[tmp] != hostSourceImage[tmp]) {
        free(hostDstImage);
        free(hostSourceImage);
        printf("Copy 3D region to 2D region - wrong use of orgign\n");
        return false;
      }
    }

  free(hostDstImage);
  free(hostSourceImage);
  iRes = srcMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {

    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = dstMemObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}

bool imageTest(bool profiling) {
  if (!clGetSupportedImageFormats_Test()) {
    return false;
  }
  /*****************************************
  Read Write image test
  *****************************************/

  if (!clReadWrite2DImage_Test(profiling)) {
    return false;
  }
  /*****************************************
  Copy 2D->2D image test
  *****************************************/

  if (!clCopy2DImageto2DImage_Test(profiling)) {
    return false;
  }

  /*****************************************
  Copy 2D->buffer image test
  *****************************************/

  if (!clCopy2DImageToBuffer_Test(profiling)) {
    return false;
  }
  /*****************************************
  Copy buffer->buffer image test
  *****************************************/

  if (!clCopyBufferToBuffer_Test(profiling)) {
    return false;
  }

  /*****************************************
  Copy 3D->3D image test
  *****************************************/

  if (!clCopy3DImageto3DImage_Test(profiling)) {
    return false;
  }

  /*****************************************
  Copy 3D->1D image test
  *****************************************/

  if (!clCopy3DImageToBuffer_Test(profiling)) {
    return false;
  }

  /*****************************************
  Copy 3D->2D image test
  *****************************************/

  if (!clCopy3DImageto2DImage_Test(profiling)) {
    return false;
  }

  /*****************************************
  Copy 2D->3D image test
  *****************************************/

  if (!clCopy2DImageto3DImage_Test(profiling)) {
    return false;
  }
  /*****************************************
  Copy 1D->3D image test
  *****************************************/

  if (!clCopyBufferTo3DImage_Test(profiling)) {
    return false;
  }

  return true;
}
