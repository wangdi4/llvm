/////////////////////////////////////////////////////////////////////////
// cl_dev_backend_api.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once

namespace Intel { namespace OpenCL { namespace DeviceBackend {
	// Defines possible values for kernel argument types
	typedef enum cl_exec_mem_type
	{
		CL_EXEC_LOCAL_MEMORY_TYPE = 0, // this is a buffer used by the executable for local memory 
		CL_EXEC_INTERNAL_MEMORY_TYPE, // this is a buffer used by the executable for internal uses (opaque)
	};

	//*****************************************************************************************
	//	class ICLDevBackendProgram

	// Creates a program object from a provided container. 
	// The back-end compiler is not required to compile the code at this point, it is only expected to cache the byte code.
	//
	// Input
	//		pContainer	- A container of the program byte code to compile. 
	//                    The container is expected to be persistent only during the method call. The implementation is required to cache this information.
	//		pProgram	- The new program object.
	// Returns
	//		CL_DEV_BE_SUCCESS if the program object is created successfully and a valid non zero program object. 
	//	    Otherwise it returns a NULL program object with one of the following error values:
	//			CL_DEV_BE_INVALID_BINARY - provided binary within the container is not supported or binary data is broken
	//			CL_DEV_BE_OUT_OF_MEMORY  - there is not enough memory to create the program
	//
	// Note:
	// An important part of the IR is the required / hinted work group size per kernel obtained from the front end
	// __attribute__((reqd_work_group_size(X, Y, Z))), 
	// __attribute__(work_group_size_hint(X, Y, Z))
	// __attribute__((vec_type_hint (type)))
	// static cl_int CreateProgram(const cl_prog_container* IN pContainer, ICLDevBackendProgram** OUT pProgram ) = 0;

	class ICLDevBackendKernel;
	// An interface class to an OpenCL program object provided by the Back-end Compiler
	class ICLDevBackendProgram
	{
	public:
		// Builds the program, the generated program byte code is compiler specific and is not necessarily the final target machine code. 
		// 
		// Input
		//		pOptions    - A pointer to a string that describes the build options to be used for building the program executable.
		//					  
		// Returns
		//		CL_DEV_BUILD_ERROR	 - the last call to clBuildProgram generated errors
		//		CL_DEV_BUILD_WARNING	 - the last call to clBuildProgram generated warnings
		//		CL_DEV_SUCCESS	     - the last call to clBuildProgram was successful
		//		CL_DEV_INVALID_BUILD_OPTIONS - if the build options specified by options are invalid
		//		CL_DEV_OUT_OF_MEMORY   - if the there is a failure to allocate memory 
		//
		// Assumptions
		//      
		//
		// Dasher Comments:
		//    1. Dasher performs LLVM-> Multiple DI (one per kernel)->Partial Compilation (w/o state specialization phase)
		//    2. all errors would happen during source->llvm
		//    3. we are fine with compilation errors but what about link errors?
		//    4. we need to make sure we allocate all of the resources based on the return values of the build program API (OUT_OF_MEMORY)
		//    5. Texture sampler format is known only at runtime (setArgs)
		//
		virtual cl_int	BuildProgram( const char IN *pOptions ) = 0;

		// Get the program build log
		// Input
		//		pSize - A pointer to the size of the log buffer. if pLog is NULL, pSize will contain the size of the buffer
		//		pLog  - A pointer to the a null terminated string containing the build log
		// Returns
		//      CL_DEV_SUCCESS				- upon success
		virtual cl_int GetBuildLog(size_t INOUT *pSize, char* OUT pLog) const = 0;

		// get a container of the program 
		// Input
		//		pDescriptor - A pointer to valid program binary descriptor that is requested
		// Returns
		//		Returns a pointer to the internally stored container, if this descriptor does not exists returns NULL
		virtual cl_int GetContainer( size_t INOUT *pSize, cl_prog_container* OUT pContainer  ) const = 0;

		// Retrieves a pointer to a kernel object by kernel name
		// Input
		//		pKernelName	- A pointer to null terminated string that specified kernel name
		// Output
		//		pKernel		- A pointer to returned kernel object
		// Returns
		//		CL_DEV_SUCCESS		- if kernel descriptor was successfully retrieved
		//		CL_DEV_ERROR_FAIL	- if kernel name was not found
		virtual cl_int	GetKernel(const char* IN pKernelName, const ICLDevBackendKernel** OUT pKernel) const = 0;

		// Retrieves a vector of pointers to a function descriptors
		// User should provide a buffer for vector storage
		// Input
		//		pKernels	- A pointer to buffer that will hold pointers to kernel objects
		//		puiRetCount	- A pointer to the number of kernels which are pointer by pKernels
		// Returns
		//		CL_DEV_SUCCESS		- if vector successfully was retrieved
		//		CL_DEV_ERROR_FAIL	- if provided buffer is not enough or one of the parameters is invalid
		virtual cl_int	GetAllKernels(const ICLDevBackendKernel** IN pKernels, cl_uint* INOUT puiRetCount) const = 0;

		// Releases program instance
		virtual void	Release() = 0;
	};

	//*****************************************************************************************
	//	class ICLDevBackendKernel
	// An interface class that defined interface for OCL kernel object
	class ICLDevBackendBinary;
	class ICLDevBackendKernel
	{
	public:			

		// Creates an executable object
		// Input
		//		pArgsBuffer     - This buffer contains the list of arguments of the kernel ( see protocol for explanation of how this is organized )
		//		BufferSize      - indicates the size of the arguments buffer
		//		WorkDimension   - is the number of dimensions used to specify the global work-items and work-items in the work-group. 
		//						  work_dim must be greater than zero and less than or equal to three.
		//		pGlobalWorkOffset - points to an array of work_dim unsigned values that describe the offset of global work-items in work_dim dimensions 
		//						  that will execute the kernel function. The total number of global work-items is computed as global_work_size[0] * … * global_work_size[work_dim – 1].
		//		pGlobalWorkSize - points to an array of work_dim unsigned values that describe the number of global work-items in work_dim dimensions 
		//						  that will execute the kernel function. The total number of global work-items is computed as global_work_size[0] * … * global_work_size[work_dim – 1].
		//		pLocalWorkSize  - points to an array of work_dim unsigned values that describe the number of work-items that make up a work-group 
		//						  (also referred to as the size of the work-group) that will execute the kernel specified by kernel.
		//		pBinary     - the pointer to the executable object.
		//
		// Returns
		//      CL_DEV_SUCCESS if the arguments were set successfully and a valid non zero executable object
		//	    Otherwise it returns a NULL executable object with one of the following error values:
		//			CL_DEV_OUT_OF_MEMORY  - there is not enough memory to perform the operation
		//			... more need to fill that (TBD)
		// Notes:
		//	    In Dasher this will cause final specialization of the code based on the state.
		virtual cl_int CreateBinary(void* IN pArgsBuffer, 
			size_t IN BufferSize, 
			cl_uint IN WorkDimension,		
			const size_t* IN pGlobalWorkOffeset, 
			const size_t* IN pGlobalWorkSize, 
			const size_t* IN pLocalWorkSize,
			ICLDevBackendBinary** OUT pBinary) const = 0;

		// return a pointer to the Kernel Arguments
		virtual cl_int GetKernelParams( const cl_kernel_argument* OUT *pArgsBuffer, cl_uint* OUT ArgCount ) const = 0;

		// Returns a pointer to the kernel name
		virtual const char*	GetKernelName() const = 0;

		// Returns the size in bytes of the local memory buffer required by this kernel
		virtual size_t GetImplicitLocalMemoryBufferSize() const = 0;

		// Returns the number of Work Items handled by each kernel instance
		virtual size_t GetKernelPackSize() const = 0;

		// Returns the required work-group size that was declared during kernel compilation.
		// NULL when is not this attribute is not present
		virtual const size_t* GetRequiredWorkgroupSize() const = 0;

		// Returns the required stack size for single Work Item execution
		// 0 when is not available
		virtual size_t  GetPrivateMemorySize() const = 0;

		// Releases kernel instance
		virtual void	Release() = 0;
	};

	//*****************************************************************************************
	class ICLDevBackendExecutable;
	//	class ICLDevBackendBinary
	// An interface class that defined interface for OCL kernel executable object
	class ICLDevBackendBinary
	{
	public:
		// Executes the executable (one instance of the kernel)
		// Input
		//	pMemoryBuffers - an array of pointers to memory buffers
		//	pBufferCount - the number of buffers in pMemoryBuffers
		//  pGlobalId - a 3 dimension array which containing the global id to work on in each dimension
		//  pLocalId - a 3 dimension array which containing the local id to work on in each dimension
		//  pItemsToProcess - a 3 dimension array which contains the number of work items to process in each dimension 
		// Returns
		//		CL_DEV_SUCCESS - the execution completed successfully
		//		CL_DEV_ERROR_FAIL - the execution failed
		//
		virtual cl_uint Execute( void* IN pMemoryBuffers, 
			const size_t* IN pBufferCount, 
			const size_t* IN pGlobalId, 
			const size_t* IN pLocalId, 
			const size_t* IN pItemsToProcess ) const = 0;

		// Returns the required number of memory buffers, their sizes and their types 
		// Input
		//	pBuffersSizes - an array of sizes of buffers
		//	pBuffersTypes - an array of types of buffers
		//	pBufferCount - the number of buffers required for executing the kernels
		// Returns
		//		CL_DEV_BE_SUCCESS - the execution completed successfully
		//
		virtual cl_uint GetMemoryBuffersDescriptions(size_t* IN pBuffersSizes, 
			cl_exec_mem_type* IN pBuffersTypes, 
			size_t* INOUT pBufferCount ) const = 0;

		// Returns the actual number of Work Items handled by each executable instance
		virtual const size_t* GetWorkGroupSize() const = 0;

		// Create execution context which will be used across different execution threads
		virtual cl_uint CreateExecutable(void* IN *pMemoryBuffers, 
			unsigned int IN uiBufferCount, ICLDevBackendExecutable* OUT *pContext) = 0;

		// Returns the kernel object which generated this executable
		virtual const ICLDevBackendKernel* GetKernel() const = 0;

		// Releases executable instance
		virtual void	Release() = 0;
	};

	//*****************************************************************************************
	//	class ICLDevBackendExecutable
	// An interface class that defined interface for context object
	class ICLDevBackendExecutable
	{
	public:

		// Executes the context on specific core
		// Input
		//  pGroupId - a 3 dimension array which containing the group id to be executed
		//  pLocalOffset - a 3 dimension array which containing the local offset in each dimension where to start execution
		//  pItemsToProcess - a 3 dimension array which contains the number of work items to process in each dimension 
		// Returns
		//		CL_DEV_SUCCESS - the execution completed successfully
		//		CL_DEV_ERROR_FAIL - the execution failed
		//
		virtual cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess) = 0;

		// Releases the context object
		virtual void	Release() = 0;

		// Returns the binary object which generated this executable context
		virtual const ICLDevBackendBinary* GetBinary() const = 0;
	};

}}}