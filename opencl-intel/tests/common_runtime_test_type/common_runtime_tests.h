// Copyright (c) 2006-2012 Intel Corporation
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
// common_runtime_tests.h


#ifndef COMMON_RUNTIME_GTEST_
#define COMMON_RUNTIME_GTEST_

#include <iostream>
#include <gtest/gtest.h>
#include "global_environment.h"
#include "ocl_wrapper.h"
#include "typename_getter.h"
#include "general_purpose_struct.h"
#include "dynamic_array.h"
#include "common_methods.h"


/*
 * CommonRuntime - fixture for all test cases, containins ocl_descriptor.
 * ocl_descriptor encapsulates all commonly needed OpenCL objects
 **/
class CommonRuntime : public ::testing::Test{
 protected:
	OpenCLDescriptor ocl_descriptor;

	// SetUp - called before each test is being run
	virtual void SetUp() 
	{		
	}

	//	TearDown - called after each test
	virtual void TearDown() 
	{		
	}
};

// FissionWrapper - encapsulates allocation and automatic dealocation of subdevices 
class FissionWrapper: public virtual CommonRuntime{
public:
	cl_device_id* subdevices;
	cl_uint subdevices_size;

	// SetUp - called before each test is being run
	virtual void SetUp() 
	{		
		subdevices = NULL;
		subdevices_size = 0;
	}

	//	TearDown - called after each test
	virtual void TearDown() 
	{		
		if(NULL!=subdevices)
		{
			delete[] subdevices;
			subdevices = NULL;
		}
		subdevices_size = 0;
	}

	// partitionByCounts - creates numSubDevices subdevices for CPU root device with property CL_DEVICE_PARTITION_BY_COUNTS_*
	void partitionByCounts(cl_device_id in_device, cl_uint numSubDevices)
	{
		subdevices = new cl_device_id[numSubDevices];
		ASSERT_NO_FATAL_FAILURE(createPartitionByCounts(in_device, subdevices, numSubDevices));
		subdevices_size = numSubDevices;
	}

	// mergeFirstSubdeviceWithAnotherDevice - copies subdevices[0] and in_device into out_devices respectively
	// out_device must be at least of size 2
	void mergeFirstSubdeviceWithGPU(cl_device_id in_device, cl_device_id* out_devices)
	{
		if(NULL==out_devices)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		if(NULL==subdevices)
		{
			ASSERT_TRUE(false) << "subdevices were not initialized";
		}
		out_devices[0] = subdevices[0];
		out_devices[1] = in_device;
	}


	void createAndMergeWithGPU(OpenCLDescriptor& ocl_descriptor)
	{
		cl_device_id in_devices[]={0,0};
		ASSERT_NO_FATAL_FAILURE(getCPUGPUDevices(ocl_descriptor.platforms, in_devices));
		ASSERT_NO_FATAL_FAILURE(partitionByCounts(in_devices[0], 2));
		ASSERT_NO_FATAL_FAILURE(mergeFirstSubdeviceWithGPU(in_devices[1],ocl_descriptor.devices));
	}

	// mergeAllSubdevicesWithGPU - copies all subdevices elements and in_device into out_devices respectively
	// out_device must be at least of size (subdevices_size+1)
	void mergeAllSubdevicesWithGPU(cl_device_id in_device, cl_device_id* out_devices)
	{
		if(NULL==out_devices)
		{
			ASSERT_TRUE(false) << "Null argument provided";
		}
		if(NULL==subdevices)
		{
			ASSERT_TRUE(false) << "subdevices were not initialized";
		}
		int i=0; 
		for(i=0; i<subdevices_size; ++i)
		{
			out_devices[i] = subdevices[i];
		}
		out_devices[i] = in_device;
	}
};

//	ImageTypedCommonRuntime - base class for testing reading of images, should not be instantiated as a test case directly.
//	If new image format tests should be added:
//	1.	sub-class ImageTypedCommonRuntime
//	2.	set needed image_format in sub-class' SetUp function with image_channel_order and image_channel_data_type
//	3.	add appropriate buffer type - for example for "CL_FLOAT" - will need to add "cl_float4" buffer type
//	Refer to vc8_image.cpp for examples
template <typename T>
class ImageTypedCommonRuntime : public virtual CommonRuntime{
public:
	TypeNameGetter<T> typeNameGetter;
	T value_;
	cl_image_format image_format;
	const char* kernelName;

	virtual void SetUp() 
	{
		image_format.image_channel_order = CL_RGBA;
		image_format.image_channel_data_type = CL_FLOAT;
	}
};


#endif /* COMMON_RUNTIME_GTEST_ */