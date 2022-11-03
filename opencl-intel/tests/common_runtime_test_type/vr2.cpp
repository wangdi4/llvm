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

class VR2 : public CommonRuntime {};

//|  TEST: VR2.VR2SinglePlatformCPUGPU (TC-2)
//|
//|  Purpose
//|  -------
//|
//|  Verify that in a SINGLE platfrom (CPU+GPU), the platfrom returns both
// devices
//|
//|  Method
//|  ------
//|
//|  1. Call clGetDeviceIDs() with device_type CL_DEVICE_TYPE_ALL and
// num_entries greater than 1 |  (other parameters are not NULL) |  2.
// Validate that 2 devices were returned |  3. For each device returned call
// clGetDeviceInfo |  4. Validate that at least one device is CPU and one is
// GPU (order of devices in device array is unknown)
//|
//|  Pass criteria
//|  -------------
//|
//|  The list of devices returned in clGetDeviceIDs contains exactly 2
// devices, one CPU and one GPU
//|
TEST_F(VR2, SinglePlatformCPUGPU) {
  // set up platform, expect return of a single platform
  cl_uint num_entries = 1;
  cl_uint num_platforms = 0;
  cl_uint num_devices = 0;
  cl_device_id devices[] = {0, 0, 0};
  ASSERT_NO_FATAL_FAILURE(
      getPlatformIDs(num_entries, ocl_descriptor.platforms, &num_platforms));

  // set up num_entries to value greater than 2
  // set up device, expect return of 2 devices - CPU and GPU
  num_entries = 3;
  cl_device_type device_type = CL_DEVICE_TYPE_ALL;
  ASSERT_NO_FATAL_FAILURE(getDeviceIDs(ocl_descriptor.platforms[0], device_type,
                                       num_entries, devices, &num_devices));
  // validate that CPU and GPU devices were returned
  size_t param_value_size = sizeof(cl_device_type);
  cl_device_type param_value[2];
  for (int i = 0; i < 2; ++i) {
    ASSERT_NO_FATAL_FAILURE(getDeviceInfo(devices[i], CL_DEVICE_TYPE,
                                          param_value_size, &param_value[i]));
  }
  // overall exactly 2 devices
  EXPECT_EQ(2, num_devices) << "num_devices is not 2";
  // one device is CPU, another is GPU
  EXPECT_TRUE((CL_DEVICE_TYPE_CPU == param_value[0] &&
               getSecondDeviceType() == param_value[1]) ||
              (CL_DEVICE_TYPE_CPU == param_value[1] &&
               getSecondDeviceType() == param_value[0]))
      << "Did not return CPU and GPU devices";
}

//| testSingleDevice - helper function to test that the platfrom returns device
// of type device_type
//|
//|  1. Call clGetDeviceIDs() with device_type and num_entries greater than 1
//|  (other parameters are not NULL)
//|  2. Validate that 1 device was returned
//|  3. Call clGetDeviceInfo on the returned device
//|  4. Validate that the returned device of type device_type
static void testSingleDevice(cl_device_type device_type,
                             cl_platform_id *platforms, cl_device_id *devices) {
  // set up platform, expect return of a single platform
  cl_uint num_entries = 1;
  cl_uint num_platforms = 0;
  cl_uint num_devices = 0;
  cl_device_type param_value;

  ASSERT_NO_FATAL_FAILURE(
      getPlatformIDs(num_entries, platforms, &num_platforms));

  // set up CPU device with num_entries greater than 1
  num_entries = 2;
  // set up device, expect return of 1 device
  ASSERT_NO_FATAL_FAILURE(getDeviceIDs(platforms[0], device_type, num_entries,
                                       devices, &num_devices));
  ASSERT_EQ(1, num_devices) << "Did not return single device";
  // validate that CPU was returned
  size_t param_value_size = sizeof(cl_device_type);
  // query device
  ASSERT_NO_FATAL_FAILURE(getDeviceInfo(devices[0], CL_DEVICE_TYPE,
                                        param_value_size, &param_value));
  EXPECT_EQ(device_type, param_value) << "Did not return requested device";
}

//|  TEST: VR2.SinglePlatformCPUTest (TC-3)
//|
//|  Purpose
//|  -------
//|
//|  Verify that SINGLE platfrom returns CPU device
//|
//|  Method
//|  ------
//|
//|  1. Call clGetDeviceIDs() with device_type CL_DEVICE_TYPE_CPU and
// num_entries greater than 1 |  (other parameters are not NULL) |  2.
// Validate that 1 device was returned |  3. Call clGetDeviceInfo on the
// returned device |  4. Validate that the returned device is CPU
//|
//|  Pass criteria
//|  -------------
//|
//|  Single platfrom returns CPU device
//|
TEST_F(VR2, SinglePlatformCPUTest) {
  ASSERT_NO_FATAL_FAILURE(testSingleDevice(
      CL_DEVICE_TYPE_CPU, ocl_descriptor.platforms, ocl_descriptor.devices));
}

//|  TEST: VR2.SinglePlatformGPUTest (TC-4)
//|
//|  Purpose
//|  -------
//|
//|  Verify that SINGLE platfrom returns GPU device
//|
//|  Method
//|  ------
//|
//|  1. Call clGetDeviceIDs() with device_type CL_DEVICE_TYPE_GPU and
// num_entries greater than 1 |  (other parameters are not NULL) |  2.
// Validate that 1 device was returned |  3. Call clGetDeviceInfo on the
// returned device |  4. Validate that the returned device is GPU
//|
//|  Pass criteria
//|  -------------
//|
//|  Single platfrom returns GPU device
//|
TEST_F(VR2, SinglePlatformGPUTest) {
  ASSERT_NO_FATAL_FAILURE(testSingleDevice(
      getSecondDeviceType(), ocl_descriptor.platforms, ocl_descriptor.devices));
}
