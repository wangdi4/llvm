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

#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "test_utils.h"
#include <iostream>

extern "C" {
extern CL_API_ENTRY cl_mem CL_API_CALL clCreatePipeINTEL(
    cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size,
    cl_uint pipe_max_packets, const cl_pipe_properties *attributes,
    void *host_ptr, size_t *size_ret, cl_int *errcode_ret);
}

using namespace std;

extern cl_device_type gDeviceType;

bool clPipes() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;
  cl_context context = NULL;

  std::cout << "============================================================="
            << std::endl;
  std::cout << "clPipes" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  try {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
    CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

    const cl_uint uiPacketSize = sizeof(int), uiMaxPackets = 1024;
    clMemWrapper pipe =
        clCreatePipe(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                     uiPacketSize, uiMaxPackets, NULL, &iRet);
    CheckException("clCreatePipe", CL_SUCCESS, iRet);

    cl_uint uiInfoPacketSize, uiInfoMaxPackets;
    size_t szInfoSize;
    iRet = clGetPipeInfo(pipe, CL_PIPE_PACKET_SIZE, sizeof(uiInfoPacketSize),
                         &uiInfoPacketSize, &szInfoSize);
    CheckException("uiInfoPacketSize != uiPacketSize",
                   uiInfoPacketSize == uiPacketSize, true);
    CheckException("szInfoSize != sizeof(cl_uint)", szInfoSize,
                   sizeof(cl_uint));
    CheckException("clGetPipeInfo", CL_SUCCESS, iRet);

    iRet = clGetPipeInfo(pipe, CL_PIPE_MAX_PACKETS, sizeof(uiInfoMaxPackets),
                         &uiInfoMaxPackets, &szInfoSize);
    CheckException("uiInfoMaxPackets != uiMaxPackets",
                   uiInfoMaxPackets == uiMaxPackets, true);
    CheckException("szInfoSize != sizeof(cl_uint)", szInfoSize,
                   sizeof(cl_uint));
    CheckException("clGetPipeInfo", CL_SUCCESS, iRet);

    // negative cases:
    clCreatePipe((cl_context)platform,
                 CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, uiPacketSize,
                 uiMaxPackets, NULL, &iRet);
    CheckException("clCreatePipe", CL_INVALID_CONTEXT, iRet);

    cl_pipe_properties props;
    clCreatePipe(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                 uiPacketSize, uiMaxPackets, &props, &iRet);
    CheckException("clCreatePipe", CL_INVALID_VALUE, iRet);

    clCreatePipe(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, 0, 0, NULL,
                 &iRet);
    CheckException("clCreatePipe", CL_INVALID_PIPE_SIZE, iRet);

    iRet = clGetPipeInfo(pipe, CL_PIPE_PACKET_SIZE,
                         sizeof(uiInfoPacketSize) - 1, &uiInfoPacketSize, NULL);
    CheckException("clGetPipeInfo", CL_INVALID_VALUE, iRet);

    iRet = clGetPipeInfo(pipe, CL_PIPE_PROPERTIES, 0, nullptr, &szInfoSize);
    CheckException("szInfoSize != 0", size_t(0), szInfoSize);

    // Intel extension
    vector<char> pipeBuf;
    size_t szPipeBufSize = 0;
    clCreatePipeINTEL(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                      uiPacketSize, uiMaxPackets, NULL, NULL, &szPipeBufSize,
                      &iRet);
    CheckException("clCreatePipeINTEL", CL_SUCCESS, iRet);

    pipeBuf.resize(szPipeBufSize);
    clMemWrapper pipeIntel = clCreatePipeINTEL(
        context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, uiPacketSize,
        uiMaxPackets, NULL, &pipeBuf[0], &szPipeBufSize, &iRet);
    CheckException("clCreatePipeINTEL", CL_SUCCESS, iRet);

    clCreatePipeINTEL(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                      uiPacketSize, uiMaxPackets, NULL, NULL, &szPipeBufSize,
                      &iRet);
    CheckException("clCreatePipeINTEL", szPipeBufSize == pipeBuf.size(), true);

    szPipeBufSize--;
    clCreatePipeINTEL(context, CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
                      uiPacketSize, uiMaxPackets, NULL, &pipeBuf[0],
                      &szPipeBufSize, &iRet);
    CheckException("clCreatePipeINTEL", CL_OUT_OF_RESOURCES, iRet);

  } catch (const std::exception &) {
    bResult = false;
  }
  if (context) {
    clReleaseContext(context);
  }
  return bResult;
}
