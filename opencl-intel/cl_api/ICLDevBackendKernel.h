#ifndef ICLDevBackendKernel_H
#define ICLDevBackendKernel_H

#include "cl_device_api.h"
#include "ICLDevBackendProgram.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

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
     * @returns the required private memory size for single Work Item execution
     *  0 when is not available
     */
    virtual size_t GetPrivateMemorySize() const = 0;

    /**
     * @returns the size in bytes of the implicit local memory buffer required by this kernel
     * (implicit local memory buffer is the size of all the local buffers declared and used
     *  in the kernel body)
     */
    virtual size_t GetImplicitLocalMemoryBufferSize() const = 0;

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
     * @returns true if the specified kernel calls other kernerls in the kernel body, 
     *  false otherwise
     */
    virtual bool HasKernelCallOperation() const = 0;
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
     * Gets the description of the kernel body, the returned object contains all the kernel
     * body proporties
     *
     * @returns reference to IKernelDescription object
     */
    virtual const ICLDevBackendKernelProporties* GetKernelProporties() const = 0;

};

}}} // namespace

#endif // ICLDevBackendKernel_H
