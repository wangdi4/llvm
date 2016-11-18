// Copyright (c) 2006-2007 Intel Corporation
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
//
// ocl_fission.h

#ifndef OCL_FISSION_
#define OCL_FISSION_

#include "common_runtime_tests.h"

#pragma OPENCL EXTENSION cl_ext_device_fission : enable

class FissionTests: public CommonRuntime{
public:

	// releaseDevice - calls and validates clReleaseDevice
	void releaseDevice(cl_device_id device )
	{
		ASSERT_EQ(CL_SUCCESS, clReleaseDeviceEXT(device ));
	}

	// retainDevice - calls and validates clRetainDevice
	void retainDevice(cl_device_id device)
	{
		ASSERT_EQ(CL_SUCCESS, clRetainDeviceEXT(device));
	}

	// clCreateSubDevices - calls and validates clCreateSubDevices
	void createSubDevices(cl_device_id in_device, 
		const cl_device_partition_property_ext * properties,
		cl_uint num_entries, cl_device_id *out_devices, cl_uint *num_devices)
	{
		ASSERT_EQ(CL_SUCCESS, clCreateSubDevicesEXT(in_device, properties,
			num_entries, out_devices, num_devices));
	}

	void createNumByCountsSubdevices(cl_device_id in_device, cl_device_id* out_devices, cl_uint numSubDevices)
	{
		if(NULL==out_devices)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		ASSERT_TRUE(0<numSubDevices) << "numSubDevices must be a positive integer"; 
		// get sub-devices
		cl_uint actual_num_devices = 0;
		cl_device_partition_property_ext properties[3];
		properties[0] = CL_DEVICE_PARTITION_BY_COUNTS_EXT;
		properties[1] = 1;
		properties[2] = 1;
		properties[3] = CL_PARTITION_BY_COUNTS_LIST_END_EXT
		properties[4] = CL_PROPERTIES_LIST_END_EXT;

		// query the number of available sub devices
		ASSERT_NO_FATAL_FAILURE(createSubDevices(in_device, properties, numSubDevices, 
			NULL, &actual_num_devices));

		std::cout << "Device can be partitioned into " << actual_num_devices << std::endl;

		if (actual_num_devices < 1)
		{
			std::cout << "Number of sub-devices set in configuration is " << numSubDevices << " while device can be partitioned  to " << actual_num_devices << std::endl;
			return;
		}
		// create sub devices
		ASSERT_NO_FATAL_FAILURE(createSubDevices(in_device, properties, numSubDevices, 
			out_devices, &actual_num_devices));
	}

	// mergeDevices - merges in_devices1 and in_devices2 into out_devices
	// num_in_devices1 - size of in_devices1
	// num_in_devices2 - size of in_devices2
	// size of out_devices must be (num_in_devices1+num_in_devices2)
	void mergeDevices(cl_device_id* in_devices1, cl_device_id* in_devices2, cl_device_id* out_devices, 
		cl_uint num_in_devices1, cl_uint num_in_devices2)
	{
		if(NULL==in_devices1 || NULL==in_devices2 ||NULL==out_devices)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		for(int i=0; i<num_in_devices1; ++i)
		{
			out_devices[i] = in_devices1[i];
		}
		for(int i=0; i<num_in_devices2; ++i)
		{
			out_devices[i+num_in_devices1] = in_devices1[i];
		}
	}
};

#endif /* OCL_FISSION_ */