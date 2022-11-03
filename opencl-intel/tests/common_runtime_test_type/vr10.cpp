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

#ifndef VR10_GTEST_
#define VR10_GTEST_

static void testCPUGPUbinariesBody(OpenCLDescriptor &ocl_descriptor) {
  // create context for CPU
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  // create and build program for CPU
  ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource(
      "simple_kernels.cl", &ocl_descriptor.program, ocl_descriptor.context, 2,
      ocl_descriptor.devices, NULL, NULL, NULL));

  size_t paramSize = 0;
  ASSERT_NO_FATAL_FAILURE(getProgramInfo(
      ocl_descriptor.program, CL_PROGRAM_DEVICES, 0, NULL, &paramSize));
  ASSERT_EQ(sizeof(cl_device_id) * 2, paramSize)
      << "Size of CL_PROGRAM_DEVICES param is invalid";

  std::vector<cl_device_id> deviceList(paramSize / sizeof(cl_device_id), 0);
  ASSERT_NO_FATAL_FAILURE(getProgramInfo(ocl_descriptor.program,
                                         CL_PROGRAM_DEVICES, paramSize,
                                         &deviceList[0], NULL));

  // get the size of the resulting binary
  ASSERT_NO_FATAL_FAILURE(getProgramInfo(
      ocl_descriptor.program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &paramSize));
  ASSERT_EQ(sizeof(size_t) * 2, paramSize)
      << "Size of CL_PROGRAM_BINARY_SIZES param is invalid";

  size_t binarySizes[2] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(getProgramInfo(ocl_descriptor.program,
                                         CL_PROGRAM_BINARY_SIZES, paramSize,
                                         binarySizes, NULL));

  // get binaries for CPU and GPU
  std::vector<std::vector<unsigned char>> programBinaries(2);
  programBinaries[0].resize(binarySizes[0] / sizeof(unsigned char));
  programBinaries[1].resize(binarySizes[1] / sizeof(unsigned char));

  const unsigned char *binaries[2] = {0, 0};
  binaries[0] = &programBinaries[0][0];
  binaries[1] = &programBinaries[1][0];

  ASSERT_NO_FATAL_FAILURE(getProgramInfo(ocl_descriptor.program,
                                         CL_PROGRAM_BINARIES, sizeof(binaries),
                                         binaries, NULL));

  // release previous program
  ASSERT_EQ(CL_SUCCESS, clReleaseProgram(ocl_descriptor.program))
      << "clReleaseProgram failed";

  // load program binaries
  cl_int binaryStatus[2] = {0, 0};
  ASSERT_NO_FATAL_FAILURE(createProgramWithBinary(
      &ocl_descriptor.program, ocl_descriptor.context, 2, &deviceList[0],
      binarySizes, binaries, binaryStatus));

  ASSERT_EQ(CL_SUCCESS, binaryStatus[0])
      << "Unable to load valid program binary for the first device";
  ASSERT_EQ(CL_SUCCESS, binaryStatus[1])
      << "Unable to load valid program binary for the second device";
}
#endif /* VR10_GTEST_ */
