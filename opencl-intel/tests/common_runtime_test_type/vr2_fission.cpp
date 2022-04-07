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

class VR2_Fission: public FissionWrapper{};


//|	TEST: VR2_Fission.CPUGPUSubdevices (TC-106)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that in a single platform (CPU+GPU), the platform returns all existing devices
//|
//|	Method
//|	------
//|
//|	1. Create CPU and GPU devices
//|	2. Create subdevices for CPU
//| 3. Call clGetDeviceIds() with CL_DEVICE_TYPE_ALL
//|	
//|	Pass criteria
//|	-------------
//|
//|	All OpenCL functions were executed successfully
//|
TEST_F(VR2_Fission, CPUGPUSubdevices)
{
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
}

//|	TEST: VR2_Fission.CPUOnlySubDevices (TC-107)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that in a single platform (CPU+GPU), 
//| the platform knows to return only CPU devices including subdevices
//|
//|	Method
//|	------
//|
//|	1. Create CPU and GPU devices
//|	2. Create array of subdevices
//| 3. Call clGetDeviceIds() with CL_DEVICE_TYPE_CPU
//|	
//|	Pass criteria
//|	-------------
//|
//|	All OpenCL functions were executed successfully
TEST_F(VR2_Fission, CPUOnlySubDevices)
{
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
	ASSERT_NO_FATAL_FAILURE(getCPUDevice(ocl_descriptor.platforms, ocl_descriptor.devices));
}

//|	TEST: VR2_Fission.GPUOnlySubDevices (TC-108)
//|
//|	Purpose
//|	-------
//|	
//|	Verify that in a single platform (CPU+GPU), 
//| the platform knows to return only CPU devices including subdevices
//|
//|	Method
//|	------
//|
//|	1. Create CPU and GPU devices
//|	2. Create array of subdevices
//| 3. Call clGetDeviceIds() with CL_DEVICE_TYPE_GPU
//|	
//|	Pass criteria
//|	-------------
//|
//|	All OpenCL functions were executed successfully
TEST_F(VR2_Fission, GPUOnlySubDevices)
{
	ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, ocl_descriptor.devices));
	ASSERT_NO_FATAL_FAILURE(partitionByCounts(ocl_descriptor.devices[0], 2));
	ASSERT_NO_FATAL_FAILURE(getGPUDevice(ocl_descriptor.platforms, ocl_descriptor.devices));
}

