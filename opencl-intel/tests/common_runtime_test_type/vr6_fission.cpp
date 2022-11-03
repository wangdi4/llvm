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

class VR6_Fission : public FissionWrapper {};

//|  TEST: VR6_Fission.CPURootGPURootSharedContext (TC-109)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create  common context with CPU and GPU devices
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create shared context for CPU and GPU root devices
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, CPURootGPURootSharedContext) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
  // set up context
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0, 2,
                                        ocl_descriptor.devices, NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(2, num_devices) << "Number of devices in context is " << num_devices
                            << " and it should be 2";
}

//|  TEST: VR6_Fission.CPUSubGPURootSharedContext (TC-110)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create common context with CPU subdevices and GPU
// device
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create shared context for CPU subdevices and GPU root device
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, CPUSubGPURootSharedContext) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  cl_uint desired_num_devices = 2;
  ASSERT_NO_FATAL_FAILURE(
      partitionByCounts(ocl_descriptor.devices[0], desired_num_devices));
  // set up context
  // merge GPU with CPU's subdevices and create their context
  cl_device_id outDevices[3];
  ASSERT_NO_FATAL_FAILURE(
      mergeAllSubdevicesWithGPU(ocl_descriptor.devices[1], outDevices));
  ASSERT_NO_FATAL_FAILURE(createContext(&ocl_descriptor.context, 0,
                                        1 + desired_num_devices, outDevices,
                                        NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(1 + desired_num_devices, num_devices)
      << "Number of devices in context is " << num_devices
      << " and it should be " << (1 + desired_num_devices);
}

//|  TEST: VR6_Fission.CPURootContext (TC-111)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create several contexts
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create context for CPU root device only
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, CPURootContext) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
  // set up context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, 1, &ocl_descriptor.devices[0], NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(1, num_devices) << "Number of devices in context is " << num_devices
                            << " and it should be " << 1;
}

//|  TEST: VR6_Fission.GPURootContext (TC-111)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create several contexts
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create context for GPU root device only
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, GPURootContext) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
  // set up context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, 1, &ocl_descriptor.devices[1], NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(1, num_devices) << "Number of devices in context is " << num_devices
                            << " and it should be " << 1;
}

//|  TEST: VR6_Fission.GPURootContext (TC-111)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create several contexts
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create context for CPU subdevices only
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, SubDevicesContext) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  cl_uint desired_num_devices = 2;
  ASSERT_NO_FATAL_FAILURE(
      partitionByCounts(ocl_descriptor.devices[0], desired_num_devices));
  // set up context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, desired_num_devices, subdevices, NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(desired_num_devices, num_devices)
      << "Number of devices in context is " << num_devices
      << " and it should be " << desired_num_devices;
}

//|  TEST: VR6_Fission.AllContexts (TC-111)
//|
//|  Purpose
//|  -------
//|
//|  Verify the ability to create several contexts
//|
//|  Method
//|  ------
//|
//|  1. Create array of subdevices
//|  2. Create context for CPU root device only
//|  3. Create context for GPU root device only
//|  4. Create context for CPU subdevices only
//|
//|  Pass criteria
//|  -------------
//|
//|  All objects were returned successfully
//|
TEST_F(VR6_Fission, AllContexts) {
  ASSERT_NO_FATAL_FAILURE(
      getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
  cl_uint desired_num_devices = 2;
  ASSERT_NO_FATAL_FAILURE(
      partitionByCounts(ocl_descriptor.devices[0], desired_num_devices));
  // set up contexts
  cl_context cpu_context = 0;
  cl_context gpu_context = 0;
  cl_context sub_devices_context = 0;

  // set up CPU context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, 1, &ocl_descriptor.devices[0], NULL, NULL));
  cl_uint num_devices = 0;
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(1, num_devices) << "Number of devices in context is " << num_devices
                            << " and it should be " << 1;

  // set up GPU context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, 1, &ocl_descriptor.devices[1], NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(1, num_devices) << "Number of devices in context is " << num_devices
                            << " and it should be " << 1;

  // set up CPU subdevices context
  ASSERT_NO_FATAL_FAILURE(createContext(
      &ocl_descriptor.context, 0, desired_num_devices, subdevices, NULL, NULL));
  ASSERT_NO_FATAL_FAILURE(getContextInfo(&ocl_descriptor.context,
                                         CL_CONTEXT_NUM_DEVICES,
                                         sizeof(cl_uint), &num_devices));
  ASSERT_EQ(desired_num_devices, num_devices)
      << "Number of devices in context is " << num_devices
      << " and it should be " << desired_num_devices;
}
