
// Copyright (c) 2006-2008 Intel Corporation
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
//
// defines internal structures for params passing between MIC host and device
//
///////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <common/COITypes_common.h>
#include "cl_device_api.h"
#include "cl_types.h"

namespace Intel { namespace OpenCL { namespace MICDevice {

//
// NOTE: please be carefull for alignments!
//

//
// copy_program_to_device
//   Buffers:
//       buffer1 - normal buffer with serialized program [IN]
//       buffer2 - normal buffer with COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT [OUT]
//   MiscData
//       input - COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
//       output - none
//
struct COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
{
    uint64_t uid_program_on_device;
    uint64_t required_executable_size;
    uint64_t number_of_kernels;
};

struct COPY_PROGRAM_TO_DEVICE_KERNEL_INFO
{
    uint64_t    kernel_id;
    uint64_t    device_info_ptr;
};

struct COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT
{
    uint64_t    filled_kernels;
    // array of pointers to device kernel structs with size == number_of_kernels
    // in COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
    COPY_PROGRAM_TO_DEVICE_KERNEL_INFO device_kernel_info_pts[1];
};

#define COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT_SIZE( number_of_kernels ) \
    ( sizeof(COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT) + sizeof(COPY_PROGRAM_TO_DEVICE_KERNEL_INFO)*((number_of_kernels) - 1))


// Enum of directives
enum DIRECTIVE_ID
{
	KERNEL = 0,
	BUFFER,
	BARRIER,
	PRINTF
};

struct kernel_directive
{
	uint64_t kernelAddress;
};

struct buffer_directive
{
	unsigned int bufferIndex;
	uint64_t offset_in_blob;
	cl_mem_obj_descriptor mem_obj_desc;
};

struct barrier_directive
{
	COIEVENT end_barrier;
};

struct printf_directive
{
	unsigned int bufferIndex;
	uint64_t size;
};

struct directive_pack
{
	DIRECTIVE_ID id;
	union
	{
		kernel_directive kernelDirective;
		buffer_directive bufferDirective;
		barrier_directive barrierDirective;
		printf_directive printfDirective;
	};
};

struct cl_mic_work_description_type
{
	unsigned int workDimension;
	uint64_t globalWorkOffset[MAX_WORK_DIM];
	uint64_t globalWorkSize[MAX_WORK_DIM];
	uint64_t localWorkSize[MAX_WORK_DIM];

	cl_mic_work_description_type() {}

	cl_mic_work_description_type(const unsigned int workDim, const size_t* gWorkOffset, const size_t* gWorkSize, const size_t* lWorkSize)
	{
		setParams(workDim, gWorkOffset, gWorkSize, lWorkSize);
	}

	cl_mic_work_description_type& operator=(const cl_work_description_type& other)
	{
		setParams(other.workDimension, other.globalWorkOffset, other.globalWorkSize, other.localWorkSize);
		return *this;
	}

	// Copy the input data to this object data, CANNOT use memcpy because the change in type (size_t to uint64_t)
	void setParams(const unsigned int workDim, const size_t* gWorkOffset, const size_t* gWorkSize, const size_t* lWorkSize)
	{
		workDimension = workDim;
		uint64_t* groupedDlobalWork[3] = {globalWorkOffset, globalWorkSize, localWorkSize};
		const size_t* otherGroupedDlobalWork[3] = {gWorkOffset, gWorkSize, lWorkSize};
		for (unsigned int i = 0; i < 3; i++)
		{
			for (unsigned int j = 0; j < MAX_WORK_DIM; j++)
			{
				groupedDlobalWork[i][j] = otherGroupedDlobalWork[i][j];
			}
		}
	}

	// Copy this object data to cl_work_description_type object, CANNOT use memcpy because the change in type (uint64_t to size_t)
	void convertToClWorkDescriptionType(cl_work_description_type* outWorkDescType)
	{
		outWorkDescType->workDimension = workDimension;
		size_t* groupedGlobalWork[3] = {outWorkDescType->globalWorkOffset, outWorkDescType->globalWorkSize, outWorkDescType->localWorkSize};
		uint64_t* otherGroupedGlobalWork[3] = {globalWorkOffset, globalWorkSize, localWorkSize};
		for (unsigned int i = 0; i < 3; i++)
		{
			for (unsigned int j = 0; j < MAX_WORK_DIM; j++)
			{
				groupedGlobalWork[i][j] = otherGroupedGlobalWork[i][j];
			}
		}
	}

};

struct dispatcher_data
{
	// Dispatcher function arguments
	kernel_directive kernelDirective;
	bool isInOrderQueue;
	// The buffer index of misc data
	unsigned int miscDataBuffIndex;
	cl_mic_work_description_type workDesc;
	// Pre-execution directives count
	unsigned int preExeDirectivesCount;
	// Post-execution directives count
	unsigned int postExeDirectivesCount;
	// OpenCL kernel arguments size in bytes
	uint64_t kernelArgSize;
	// offset of pre execution directives array
	uint64_t preExeDirectivesArrOffset;
	// offset of post execution directives array
	uint64_t postExeDirectivesArrOffset;
	// offset of kernel arguments blob
	uint64_t kernelArgBlobOffset;

	/* Claculate the offsets of 'preExeDirectivesArrOffset' / 'postExeDirectivesArrOffset' / 'kernelArgBlobOffset'.
	   Call it only after u set the parameters - 'preExeDirectivesCount' / 'postExeDirectivesCount' */
	void calcAndSetOffsets()
	{
		preExeDirectivesArrOffset = sizeof(dispatcher_data);
		postExeDirectivesArrOffset = preExeDirectivesArrOffset + (preExeDirectivesCount * sizeof(directive_pack));
		kernelArgBlobOffset = postExeDirectivesArrOffset + (postExeDirectivesCount * sizeof(directive_pack));
	}

	/* Return the size of the "header meta data" (this struct) plus the size of "preExeDirectivesArr" + "postExeDirectivesArr" + kernelArgSize */
	size_t getDispatcherDataSize()
	{
		return kernelArgBlobOffset + kernelArgSize;
	}
};

struct misc_data
{
	void init()
	{
		invocationTime = 0;
		startRunningTime = 0;
		completionTime = 0;
		errCode = CL_DEV_SUCCESS;
	}
	cl_ulong invocationTime;
	cl_ulong startRunningTime;
	cl_ulong completionTime;
	cl_dev_err_code errCode;
};

enum OPTIONAL_DISPATCH_BUFFERS
{
	DISPATCHER_DATA = 0,
	MISC_DATA,
	PRINTF_BUFFER,

	AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS
};

}}}

