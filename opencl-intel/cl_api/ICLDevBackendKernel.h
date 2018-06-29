// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef ICLDevBackendKernel_H
#define ICLDevBackendKernel_H

#include "cl_device_api.h"
#include "cl_types.h"
#include "ICLDevBackendProgram.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

typedef void JIT_PTR(const cl_uniform_kernel_args *, const size_t *, void *);
/**
 * This interface represent the bitcode container responsible
 * for holding the bitcode buffer
 */
class ICLDevBackendJITContainer
{
public:
    virtual ~ICLDevBackendJITContainer() {}

    /**
     * @returns a pointer to the JIT buffer
     */
    virtual const void* GetJITCode() const = 0;

    /**
     * @returns the size of the JIT buffer
     */
    virtual size_t GetJITCodeSize() const = 0;

    /**
     * @param pointer Pointer to an instruction contained in this kernel's
     * JITted code.
     * @returns the the source line number from which the instruction
     * pointed to was compiled. If the pointer does not point to an
     * instruction in this kernel, or if line number information is missing,
     * this returns -1.
     */
    virtual int GetLineNumber(void* pointer) const = 0;
};


/**
 * An interface class which contains all the OCL kernel proporties to be queried
 */
class ICLDevBackendKernelProporties
{
public:
    virtual ~ICLDevBackendKernelProporties() {}

    /**
     * @returns the number of Work Items handled by each kernel instance,
     *  0 will be returned in case of failure or not present
     */
	virtual unsigned int GetKernelPackCount() const = 0;

    /**
     * @returns the required work-group size that was declared during kernel compilation.
     *  NULL when this attribute is not present;
     *  whereas work-group size is array of MAX_WORK_DIM entries
     */
    virtual const size_t* GetRequiredWorkGroupSize() const = 0;

    /**
     * @returns the required barrier buffer size for single Work Item execution
     *  0 when is not available
     */
    virtual size_t GetBarrierBufferSize() const = 0;

    /** @returns the min. required private memory size for single Work Item execution.
     *           Used by clEnqueueNDRangeKernel and clGetKernelWorkGroupInfo
     */
    virtual size_t GetPrivateMemorySize() const = 0;

    /** @returns the max possible SG size.
     */
    virtual size_t GetMaxSubGroupSize(size_t size, const size_t* WGSizes) const = 0;

    /** @returns the number of SGs will be generated for specified local work sizes.
     */
    virtual size_t GetNumberOfSubGroups(size_t size, const size_t* WGSizes) const = 0;

    /** @returns the maximum number of sub-groups that may make up each work-group to execute kernel.
     */
    virtual size_t GetMaxNumSubGroups() const = 0;

    /**
     * @returns the required number of sub-groups that was declared during kernel compilation.
     *  0 when this attribute is not present.
     */
    virtual size_t GetRequiredNumSubGroups() const = 0;

    /** @returns the max. possible WG size with respect to the specified limits.
     */
    virtual size_t GetMaxWorkGroupSize(size_t const maxWGSize, size_t const maxWGPrivateSize) const = 0;

    /**
     * @returns the size in bytes of the implicit local memory buffer required by this kernel
     * (implicit local memory buffer is the size of all the local buffers declared and used
     *  in the kernel body)
     */
    virtual size_t GetImplicitLocalMemoryBufferSize() const = 0;

     /**
      * @returns a herustic of the kernel execution length
      */
     virtual size_t GetKernelExecutionLength() const = 0;

     /**
      * @returns a string of the kernel attributes
      */
     virtual const char *GetKernelAttributes() const = 0;

    /**
     * @returns true if the specified kernel has print operation in the kernel body,
     *  false otherwise
     */
    virtual bool HasPrintOperation() const = 0;

    /**
     * @returns true if the specified kernel has barrier operation in the kernel body,
     *  false otherwise
     */
    virtual bool HasBarrierOperation() const = 0;

    /**
     * @returns true if the specified kernel has debug info,
     *  false otherwise
     */
    virtual bool HasDebugInfo() const = 0;

    /**
     * @returns true if the specified kernel calls other kernerls in the kernel body,
     *  false otherwise
     */
    virtual bool HasKernelCallOperation() const = 0;

    /**
     * @returns the required minimum group size factorial
     *  1 when no minimum is required
     */
    virtual unsigned int GetMinGroupSizeFactorial() const = 0;
    /**
     * @returns true if the specified kernel is clang's block
     *  false otherwise
     */
    virtual bool IsBlock() const = 0;

    /**
     * @returns true if the specified kernel is an autorun kernel
     *  false otherwise
     */
    virtual bool IsAutorun() const = 0;

    /**
     * @returns true if the specified kernel is a single-work item kernel
     *  false otherwise
     *
     */
    virtual bool IsTask() const = 0;

    /**
     * @returns true if the specified kernel needs to serialize workgroups
     *  false otherwise
     */
    virtual bool NeedSerializeWGs() const = 0;

    /**
     * @returns true if the specified kernel is compiled with support of non-unifrom WG size
     *  false otherwise
     */
    virtual bool IsNonUniformWGSizeSupported() const = 0;
};

class ICLDevBackendKernel_;
/**
 * This interface is responsible for running the kernel for NDRange Command
 * on specified WG
 */
class ICLDevBackendKernelRunner
{
public:
    /**
     * This struct holds the state of the thread
     */
    struct ICLDevExecutionState {
        unsigned int MXCSRstate;
    };

    virtual const ICLDevBackendKernel_* GetKernel() const = 0;

    virtual ~ICLDevBackendKernelRunner() {}

    /**
     * @effects prepare the kernel for execution
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code InitRunner(void* pKernelUniformArgs) const = 0;

    /**
     * prepares the kernel uniform arguments (implicit args only)
     * Notice: Explicit Arguments should be filled by the RT\Device agent
     *   before execution
     * @param pKernelUniformArgs pointer to the Uniform arguments object to be
     *   updated by the device backend
     * @param pDevMemObjArray [internal use] pointer to the explicit arguments
     * @param devMemObjArrayLength [internal use] size of the array
     * @param numOfComputeUnits number of compute units ND-range is to be run on
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code PrepareKernelArguments(
      void* pKernelUniformArgs,
      const cl_mem_obj_descriptor* *pDevMemObjArray,  // TODO-NDRANGE: change type
      unsigned int devMemObjArrayLength,
      size_t numOfComputeUnits) const = 0;

    /**
     * @effects prepares the thread for kernel execution
     * NOTICE: only the thread which called this api is ready to execute the thread
     * @param state object to save the old state in
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code PrepareThreadState(ICLDevExecutionState& state) const = 0;

    /**
     * Execute the specified kernel with the given arguments
     * @param pKernelUniformArgs uniformed arguments for execution
     * @param pGroupID the workgroup id to be executed
     * @param pRuntimeHandle a handle which will be passed to some built-ins
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code RunGroup(
      const void *pKernelUniformArgs,
      const size_t *pGroupID,
      void *pRuntimeHandle) const = 0;

    /**
     * @effects restore the thread state after kernel execution
     * @param state object to restore from
     * @returns CL_DEV_SUCCESS in success; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code RestoreThreadState(ICLDevExecutionState& state) const = 0;
};

/**
 * An interface class to OpenCL kernel object provided by the Back-end Compiler
 * This class is represents the whole kernel data and the JIT code
 */
class ICLDevBackendKernel_
{
public:
    virtual ~ICLDevBackendKernel_() {}

    /**
     * @returns an unsigned long which represents the kernel id - this id is unique
     *  per kernel - ; in case of failure 0 will be returned
     */
    virtual unsigned long long int GetKernelID() const = 0;

    /**
     * @returns a pointer to the kernel name, in case of failure NULL will be returned
     */
    virtual const char*	GetKernelName() const = 0;

    /**
     * Gets the kernels paramaters count
     *
     * @returns the count of the parameters, in case of failure -1 will be returned
     */
    virtual int GetKernelParamsCount() const = 0;

    /**
     * Gets the kernel parameters description
     *
     * @returns
     *  In success will return the kernel arguments descriptor; otherwise, NULL
     *  value will be returned
     */
    virtual const cl_kernel_argument* GetKernelParams() const = 0;

    /**
     * Gets the kernel parameters extended information
     *
     * @returns
     *  In success will return the kernel arguments information; otherwise, NULL
     *  value will be returned
     */
    virtual const cl_kernel_argument_info* GetKernelArgInfo() const = 0;

    /**
     * Gets the description of the kernel body, the returned object contains all the kernel
     * body proporties
     *
     * @returns reference to IKernelDescription object
     */
    virtual const ICLDevBackendKernelProporties* GetKernelProporties() const = 0;

    /**
     * @param pointer Pointer to an instruction contained in this kernel's
     * JITted code.
     * @returns the the source line number from which the instruction
     * pointed to was compiled. If the pointer does not point to an
     * instruction in this kernel, or if line number information is missing,
     * this returns -1.
     */
     virtual int GetLineNumber(void* pointer) const = 0;

     /**
      * @returns the size of argument/parameter buffer requried by the kernel
      */
     virtual size_t GetExplicitArgumentBufferSize() const = 0;

     /**
      * @returns the required alignement of the argument buffer
      */
     virtual size_t GetArgumentBufferRequiredAlignment() const = 0;

     /**
      * @returns the number of memory object arguments passed to the kernel
      */
     virtual unsigned int GetMemoryObjectArgumentCount() const = 0;

     /**
      * @returns the array of indexes of memory object arguments passed to the kernel
      */
     virtual const unsigned int* GetMemoryObjectArgumentIndexes() const = 0;

    /**
     * @retruns the kernelRunner object which is responsible for running the kernel
     */
    virtual const ICLDevBackendKernelRunner* GetKernelRunner() const = 0;

};

}}} // namespace

#endif // ICLDevBackendKernel_H
