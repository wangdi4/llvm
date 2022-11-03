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

#include "common_runtime_tests.h"
#include <sstream>

class VR12 : public CommonRuntime {};

//|  TEST: VR12.CallbackGPUError (TC-72)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to receive notification with no argument using a
// callback function |  that an application can register and which will be
// called when the program executable |  has been built successfully for CPU
// device(s) and unsuccessfully for GPU device(s)
//|
//|  Method
//|  ------
//|
//| 1.  Create a program object and load the source code specified by the text
// strings |     into a program object. The source code is the same for both
// devices (i.e. no ifdef) |     and include an erroneous statement for the GPU
// device | 2.  Try to build (compiles & links) a program executable from the
// program source for both |     the CPU and the GPU devices using a function
// pointer to a notification routine.ll
//|
//|  Pass criteria
//|  -------------
//|
//|  The notification routine should be called when the program executable
// has been built (unsuccessfully).
//|

static int callbackTriggerCnt = 0;

// Our callback function
void CL_CALLBACK notify_callback(cl_program program, void *userData) {
  ++callbackTriggerCnt;
}

TEST_F(VR12, CallbackGPUError) {
  callbackTriggerCnt = 0;

  // get pltfrom and device ids
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  // cpu is at index 0, gpu is at index 1

  // create shared context
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  bool cl_khr_fp64_on_gpu, cl_khr_fp16_on_gpu, cl_khr_fp64_on_cpu,
      cl_khr_fp16_on_cpu;

  isExtensionSupportedOnDevice("cl_khr_fp64", 0, &cl_khr_fp64_on_cpu);
  isExtensionSupportedOnDevice("cl_khr_fp16", 0, &cl_khr_fp16_on_cpu);
  isExtensionSupportedOnDevice("cl_khr_fp64", 1, &cl_khr_fp64_on_gpu);
  isExtensionSupportedOnDevice("cl_khr_fp16", 1, &cl_khr_fp16_on_gpu);

  if (cl_khr_fp64_on_cpu && !cl_khr_fp64_on_gpu) {
    // create and build program for CPU and GPU
    ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(
        &ocl_descriptor.program, ocl_descriptor.context,
        "gpu_error_kernel.cl"));

    cl_int errcode_ret =
        clBuildProgram(ocl_descriptor.program, 2, ocl_descriptor.devices, NULL,
                       notify_callback, 0);

    // active waiting for CPU
    cl_build_status build_status = CL_BUILD_IN_PROGRESS;
    while (build_status == CL_BUILD_IN_PROGRESS) {
      sleepMS(100);
      ASSERT_NO_FATAL_FAILURE(
          getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[0],
                              CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
                              &build_status, NULL));
      ASSERT_NE(CL_BUILD_ERROR, build_status);
    }

    // active waiting for GPU
    build_status = CL_BUILD_IN_PROGRESS;
    while (build_status == CL_BUILD_IN_PROGRESS) {
      sleepMS(100);
      ASSERT_NO_FATAL_FAILURE(
          getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[1],
                              CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
                              &build_status, NULL));
      if (isAccelerator()) { // not suported on MIC
        ASSERT_NE(CL_BUILD_ERROR, build_status);
      } else {
        ASSERT_NE(CL_BUILD_SUCCESS, build_status);
      }
    }
  } else if (cl_khr_fp16_on_gpu && !cl_khr_fp16_on_cpu) {
    // create and build program for CPU and GPU
    ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(
        &ocl_descriptor.program, ocl_descriptor.context,
        "cpu_error_kernel.cl"));

    cl_int errcode_ret =
        clBuildProgram(ocl_descriptor.program, 2, ocl_descriptor.devices, NULL,
                       notify_callback, 0);

    // active waiting for GPU
    cl_build_status build_status = CL_BUILD_IN_PROGRESS;
    while (build_status == CL_BUILD_IN_PROGRESS) {
      sleepMS(100);
      ASSERT_NO_FATAL_FAILURE(
          getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[1],
                              CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
                              &build_status, NULL));
      ASSERT_NE(CL_BUILD_ERROR, build_status);
    }

    // active waiting for CPU
    build_status = CL_BUILD_IN_PROGRESS;
    while (build_status == CL_BUILD_IN_PROGRESS) {
      sleepMS(100);
      ASSERT_NO_FATAL_FAILURE(
          getProgramBuildInfo(ocl_descriptor.program, ocl_descriptor.devices[0],
                              CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
                              &build_status, NULL));
      if (isAccelerator()) { // not suported on MIC
        ASSERT_NE(CL_BUILD_ERROR, build_status);
      } else {
        ASSERT_NE(CL_BUILD_SUCCESS, build_status);
      }
    }
  } else {
    return;
  }

  // check how many times callback has been triggered
  size_t iterationLimit = 100;
  size_t iterationCnt = 0;
  while (callbackTriggerCnt < 1) {
    if (iterationCnt >= iterationLimit)
      break;

    ++iterationCnt;
    sleepMS(100);
  }

  ASSERT_EQ(1, callbackTriggerCnt) << "Callback has not been triggered";
}

//|  TEST: VR12.CallbackMICError (TC-73)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to receive notification with an argument using a
// callback function |  that an application can register and which will be
// called when the program executable |  has been built successfully for CPU or
// GPU device(s) and unsuccessfully for MIC device(s)
//|
//|  Method
//|  ------
//|
//|  1.  Create a program object and load the source code specified by
// the text strings into |      a program object. The source code is the
// same for both devices (i.e. no ifdef) and |      include an erroneous
// statement for the MIC device
//|  2.  Try to build (compiles & links) a program executable from the
// program source for both |      the all devices using a function pointer to
// a notification routine.
//|
//|  Pass criteria
//|  -------------
//|
//|  The notification routine should be called when the program executable
// has been built (unsuccessfully).
//|

TEST_F(VR12, CallbackMICError) {
  callbackTriggerCnt = 0;

  // get pltfrom and device ids
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  // cpu is at index 0, gpu is at index 1

  // create shared context
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  // create and build program for CPU,  GPU, or MIC
  ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(
      &ocl_descriptor.program, ocl_descriptor.context, "mic_error_kernel.cl"));

  cl_int errcode_ret =
      clBuildProgram(ocl_descriptor.program, 2, ocl_descriptor.devices, NULL,
                     notify_callback, 0);

  // active waiting for CPU
  cl_build_status build_status = CL_BUILD_IN_PROGRESS;
  while (build_status == CL_BUILD_IN_PROGRESS) {
    sleepMS(100);
    ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(
        ocl_descriptor.program, ocl_descriptor.devices[0],
        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));
    ASSERT_NE(CL_BUILD_ERROR, build_status);
  }

  // active waiting for GPU or MIC
  build_status = CL_BUILD_IN_PROGRESS;
  while (build_status == CL_BUILD_IN_PROGRESS) {
    sleepMS(100);
    ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(
        ocl_descriptor.program, ocl_descriptor.devices[1],
        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));
    if (isAccelerator()) {
      ASSERT_NE(CL_BUILD_SUCCESS, build_status);
    } else {
      ASSERT_NE(CL_BUILD_ERROR, build_status);
    }
  }

  // check how many times callback has been triggered
  size_t iterationLimit = 100;
  size_t iterationCnt = 0;
  while (callbackTriggerCnt < 1) {
    if (iterationCnt >= iterationLimit)
      break;

    ++iterationCnt;
    sleepMS(100);
  }

  ASSERT_EQ(1, callbackTriggerCnt) << "Callback has not been triggered";
}

//|  TEST: VR12.CallbackCPUGPUError (TC-74)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to receive notification with no argument using a
// callback function |  that an application can register and which will be
// called when the program executable |  has been built unsuccessfully for all
// listed devices
//|
//|  Method
//|  ------
//|
//|  1.  Create a program object and load the source code specified by
// the text strings into |      a program object. The source code includes
// syntax error
//|  2.  Try to build (compiles & links) a program executable from the
// program source for both |      the CPU and the GPU devices using a function
// pointer to a notification routine.
//|
//|  Pass criteria
//|  -------------
//|
//|  The notification routine should be called when the program executable
// has been built unsuccessfully.
//|

TEST_F(VR12, CallbackCPUGPUError) {
  callbackTriggerCnt = 0;

  // get pltfrom and device ids
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  // cpu is at index 0, gpu is at index 1

  // create shared context
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  // create and build program for CPU and GPU
  ASSERT_NO_FATAL_FAILURE(createProgramWithSourceFromKernelName(
      &ocl_descriptor.program, ocl_descriptor.context,
      "cpu_gpu_error_kernel.cl"));

  cl_int errcode_ret =
      clBuildProgram(ocl_descriptor.program, 2, ocl_descriptor.devices, NULL,
                     notify_callback, 0);

  // active waiting for CPU
  cl_build_status build_status = CL_BUILD_IN_PROGRESS;
  while (build_status == CL_BUILD_IN_PROGRESS) {
    sleepMS(100);
    ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(
        ocl_descriptor.program, ocl_descriptor.devices[0],
        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));
    ASSERT_NE(CL_BUILD_SUCCESS, build_status);
  }

  // active waiting for GPU
  build_status = CL_BUILD_IN_PROGRESS;
  while (build_status == CL_BUILD_IN_PROGRESS) {
    sleepMS(100);
    ASSERT_NO_FATAL_FAILURE(getProgramBuildInfo(
        ocl_descriptor.program, ocl_descriptor.devices[1],
        CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL));
    ASSERT_NE(CL_BUILD_SUCCESS, build_status);
  }

  // check how many times callback has been triggered
  size_t iterationLimit = 100;
  size_t iterationCnt = 0;
  while (callbackTriggerCnt < 1) {
    if (iterationCnt >= iterationLimit)
      break;

    ++iterationCnt;
    sleepMS(100);
  }

  ASSERT_EQ(1, callbackTriggerCnt) << "Callback has not been triggered";
}
