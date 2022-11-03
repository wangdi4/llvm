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

class VR13 : public CommonRuntime {};

//|  TEST: VR13.CPUOnlyBuild (TC-75)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to build to create a kernel from a shared program
// VR-13
//|
//|  Method
//|  ------
//|
//|  1.  Create program object from source and build it on CPU and GPU
// devices. |  2.  Create kernel for this program object.
//|
//|  Pass criteria
//|  -------------
//|
//|  Validate that kernel object was created successfully.
//|
TEST_F(VR13, KernelOnCPUGPU) {
  // get pltfrom and device id
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));

  // create context for CPU
  cl_context_properties properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)ocl_descriptor.platforms[0],
      0};
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, properties, 2,
                                        ocl_descriptor.devices, NULL, NULL));

  // create and build program for CPU
  ASSERT_NO_FATAL_FAILURE(createAndBuildProgramWithSource(
      "simple_kernels.cl", &ocl_descriptor.program, ocl_descriptor.context, 1,
      &ocl_descriptor.devices[0], NULL, NULL, NULL));

  // create kernel
  ASSERT_NO_FATAL_FAILURE(createKernel(&ocl_descriptor.kernels[0],
                                       ocl_descriptor.program, "kernel_0"));
}
