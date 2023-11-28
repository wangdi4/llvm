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
// map_test.cpp
///////////////////////////////////////////////////////////

#include "CL/cl.h"
#include "cl_device_api.h"
#include "cpu_dev_test.h"
#include "image_test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

extern IOCLDeviceAgent *dev_entry;
extern RTMemObjService localRTMemService;

#define IMAGE_WIDTH 10
#define IMAGE_HEIGHT 5
#define IMAGE_DEPTH 2
#define ELEMENT_SIZE 4
#define BUFFER_SIZE 1000

/*******************************************************************************
 MapMemObj
        Create command list
        Enqueue a map memory object
        Release command list
*******************************************************************************/
bool CmdMapMemObj(cl_dev_cmd_param_map *pMapParams) {
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

  cmds.type = CL_DEV_CMD_MAP;
  cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_MAP;
  cmds.params = pMapParams;
  cmds.param_size = sizeof(cl_dev_cmd_param_map);
  cmds.profiling = false;

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
 UNmapMemObj
        Create command list
        Enqueue a map memory object
        Release command list
*******************************************************************************/
bool CmdUnmapMemObj(cl_dev_cmd_param_map *pMapParams) {
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

  cmds.type = CL_DEV_CMD_UNMAP;
  cmds.id = (cl_dev_cmd_id)CL_DEV_CMD_UNMAP;
  cmds.params = pMapParams;
  cmds.param_size = sizeof(cl_dev_cmd_param_map);
  cmds.profiling = false;

  gExecDone = false;

  cl_dev_cmd_desc *cmdsBuff = &cmds;
  iRes = dev_entry->clDevCommandListExecute(list, &cmdsBuff, 1);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevCommandListExecute failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // release commans list
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

bool clMapBuffer_Test() {
  cl_mem_flags memFlags = CL_MEM_READ_WRITE;
  cl_map_flags mapFlags = CL_MAP_READ;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {BUFFER_SIZE, 0};
  IOCLDevMemoryObject *memObj;
  unsigned int dim_count = 1;
  char buffer[BUFFER_SIZE];
  char *pMapPtr;
  size_t origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t region[MAX_WORK_DIM] = {BUFFER_SIZE, 1, 1};

  // Create Buffer Memory Object
  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_BUFFER);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, memFlags, &format, dim_count, dim, &localRTMemService, &memObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Fill buffer with data to write
  for (unsigned int i = 0; i < BUFFER_SIZE; ++i) {
    buffer[i] = i;
  }

  // Write Host buffer into the created memory image
  if (!writeImage(false, memObj, buffer, dim_count, BUFFER_SIZE, 1, 1, true)) {
    return false;
  }

  // Create Mapped region
  cl_dev_cmd_param_map mapParams;
  memset(&mapParams, 0, sizeof(cl_dev_cmd_param_map));
  mapParams.ptr = NULL;
  mapParams.flags = mapFlags;
  mapParams.dim_count = dim_count;
  memcpy(mapParams.region, region, sizeof(region));
  memcpy(mapParams.origin, origin, sizeof(origin));

  iRes = memObj->clDevMemObjCreateMappedRegion(&mapParams);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevCreateMappedRegion failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  pMapPtr = (char *)mapParams.ptr;
  if (NULL == pMapPtr) {
    return false;
  }
  // Map the buffer into host ptr
  if (!CmdMapMemObj(&mapParams)) {
    return false;
  }

  // Compare the host buffer to the map ptr

  if (0 != memcmp(buffer, pMapPtr, BUFFER_SIZE)) {
    printf("Read buffer is different from Mapped ptr\n");
    return false;
  }

  // Try to access the memory object
  if (readImage(false, memObj, buffer, dim_count, BUFFER_SIZE, 1, 1, true)) {
    printf("Read of Map region was succeed\n");
  }

  // Unamp the region
  if (!CmdUnmapMemObj(&mapParams)) {
    printf("Failed to unmap memory object\n");
  }
  // The read should be OK now
  if (!readImage(false, memObj, buffer, dim_count, BUFFER_SIZE, 1, 1, true)) {
    printf("Read of region which was Mapped and Unmapped failed\n");
  }
  iRes = memObj->clDevMemObjReleaseMappedRegion(&mapParams);
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevReleaseMappedRegion failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = memObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  return true;
}

bool clMapImage_Test() {
  cl_mem_flags memFlags = CL_MEM_READ_WRITE;
  cl_map_flags mapFlags = CL_MAP_WRITE;
  const cl_image_format format = {CL_RGBA, CL_UNORM_INT8};
  size_t dim[] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  IOCLDevMemoryObject *memObj;
  unsigned int dim_count = 3;
  char *image;
  char *pMapPtr;
  size_t origin[MAX_WORK_DIM] = {0, 0, 0};
  size_t region[MAX_WORK_DIM] = {IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH};
  unsigned int sliceSize = IMAGE_WIDTH * ELEMENT_SIZE * IMAGE_HEIGHT;

  image = (char *)malloc(sliceSize * IMAGE_DEPTH);
  if (NULL == image) {
    return false;
  }

  // Create Image Memory Object
  localRTMemService.SetupState(&format, dim_count, dim, NULL,
                               CL_MEM_OBJECT_IMAGE3D);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(
      0, memFlags, &format, dim_count, dim, &localRTMemService, &memObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Fill Image with data to write
  unsigned int tmp;
  // Fill image data to write
  for (unsigned int k = 0; k < IMAGE_DEPTH; k++)
    for (unsigned int j = 0; j < IMAGE_HEIGHT; j++)
      for (unsigned int i = 0; i < IMAGE_WIDTH; ++i) {
        tmp = (j * IMAGE_WIDTH + i) * ELEMENT_SIZE + k * sliceSize;
        image[tmp] = tmp;
        image[tmp + 1] = tmp + 1;
        image[tmp + 2] = tmp + 2;
        image[tmp + 3] = tmp + 3;
      }

  // Write Host buffer into the created memory image
  if (!writeImage(false, memObj, image, dim_count, IMAGE_WIDTH, IMAGE_HEIGHT,
                  IMAGE_DEPTH, false)) {
    free(image);
    return false;
  }

  // Map the image into host ptr
  // Create Mapped region
  cl_dev_cmd_param_map mapParams;
  memset(&mapParams, 0, sizeof(cl_dev_cmd_param_map));
  mapParams.ptr = NULL;
  mapParams.flags = mapFlags;
  mapParams.dim_count = dim_count;
  memcpy(mapParams.region, region, sizeof(region));
  memcpy(mapParams.origin, origin, sizeof(origin));

  iRes = memObj->clDevMemObjCreateMappedRegion(&mapParams);
  if (CL_DEV_FAILED(iRes)) {
    free(image);
    printf("clDevCreateMappedRegion failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }
  pMapPtr = (char *)mapParams.ptr;
  if (NULL == pMapPtr) {
    free(image);
    return false;
  }
  // Map the buffer into host ptr
  if (!CmdMapMemObj(&mapParams)) {
    free(image);
    return false;
  }

  // Compare the host buffer to the map ptr
  if (0 != memcmp(image, pMapPtr, sliceSize * IMAGE_DEPTH)) {
    free(image);
    printf("Read buffer is different from Mapped ptr\n");
    return false;
  }

  iRes = memObj->clDevMemObjReleaseMappedRegion(&mapParams);
  if (CL_DEV_FAILED(iRes)) {
    free(image);
    printf("clDevReleaseMappedRegion failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  iRes = memObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    free(image);
    printf("clDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  free(image);
  return true;
}

bool mapTest() {

  /*****************************************
  Map Buffer Test
  *****************************************/
  if (!clMapBuffer_Test()) {
    return false;
  }

  /*****************************************
  Map Image Test
  *****************************************/
  if (!clMapImage_Test()) {
    return false;
  }

  return true;
}
