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
extern RTMemObjService localRTMemService;

#define HOST_BUFFER_SIZE 10000
char hostWriteBuffer[HOST_BUFFER_SIZE];
char hostReadBuffer[HOST_BUFFER_SIZE];

/*******************************************************************************
 writeMemory
        Create command list
        Enqueue a write buffer
        Release command list
*******************************************************************************/
bool writeMemory(bool profiling, IOCLDevMemoryObject *memObj, void *pBuffer,
                 size_t stSize) {
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
  rwParams.ptr = pBuffer;
  rwParams.dim_count = 1;
  rwParams.region[0] = stSize;
  rwParams.region[1] = 1;
  rwParams.region[2] = 1;
  rwParams.origin[0] = 0;
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
 readMemory
        Create command list
        Enqueue a read buffer
        Release command list
*******************************************************************************/
bool readMemory(bool profiling, IOCLDevMemoryObject *memObj, void *pBuffer,
                size_t stSize) {
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
  ;
  rwParams.dim_count = 1;
  rwParams.region[0] = stSize;
  rwParams.region[1] = 1;
  rwParams.region[2] = 1;
  rwParams.origin[0] = 0;
  rwParams.ptr = pBuffer;

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
bool memoryTest(bool profiling) {
  cl_mem_flags flags = CL_MEM_READ_WRITE;
  cl_image_format *format = NULL;
  size_t objSize[] = {HOST_BUFFER_SIZE, 1, 1};
  IOCLDevMemoryObject *memObj;

  // Create Memory Object
  localRTMemService.SetupState(format, 1, objSize, NULL, CL_MEM_OBJECT_BUFFER);
  cl_int iRes = dev_entry->clDevCreateMemoryObject(0, flags, format, 1, objSize,
                                                   &localRTMemService, &memObj);

  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevCreateMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  // Fill with Data
  for (unsigned int i = 0; i < HOST_BUFFER_SIZE; ++i) {
    hostWriteBuffer[i] = i % 256;
  }

  // Enqueue write memory
  if (!writeMemory(profiling, memObj, hostWriteBuffer, HOST_BUFFER_SIZE)) {
    memObj->clDevMemObjRelease();
    return false;
  }

  // Enqueue read memory
  if (!readMemory(profiling, memObj, hostReadBuffer, HOST_BUFFER_SIZE)) {
    memObj->clDevMemObjRelease();
    return false;
  }

  // Compare data
  // Fill with Data
  for (unsigned int i = 0; i < HOST_BUFFER_SIZE; ++i) {
    if (hostReadBuffer[i] != hostWriteBuffer[i]) {
      memObj->clDevMemObjRelease();
      return false;
    }
  }

  // Delete Memory Object
  iRes = memObj->clDevMemObjRelease();
  if (CL_DEV_FAILED(iRes)) {
    printf("pclDevDeleteMemoryObject failed: %s\n",
           clDevErr2Txt((cl_dev_err_code)iRes));
    return false;
  }

  return true;
}
