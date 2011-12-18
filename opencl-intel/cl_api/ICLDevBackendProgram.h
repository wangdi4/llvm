#ifndef ICLDevBackendProgram_H
#define ICLDevBackendProgram_H

#include "cl_device_api.h"
#include "ICLDevBackendKernel.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This interface represent the bitcode container responsible
 * for holding the bitcode buffer
 */
class ICLDevBackendCodeContainer
{
public:
    virtual ~ICLDevBackendCodeContainer() {}

    /**
     * @returns a pointer to the bitcode buffer
     */
    virtual const cl_prog_container_header* GetCode() const = 0;

    /**
     * @returns the size of the bitcode buffer
     */
    virtual size_t GetCodeSize() const = 0;
};

/**
 * This interface represent the JIT code (per module) properties
 */
class ICLDevBackendProgramJITCodeProperties
{
    /**
     * @returns the size of the JIT code
     */
    virtual size_t GetCodeSize() const = 0;

public:
    virtual ~ICLDevBackendProgramJITCodeProperties() {}
};


/**
 * An interface class to OpenCL program object provided by the Back-end Compiler
 * This class is responsible of compiling the program to JIT
 */
class ICLDevBackendProgram_
{
public:
    virtual ~ICLDevBackendProgram_() {}

    /**
     * @returns an unsigned long which represents the program id - this id is unique
     *  per program - ; in case of failure 0 will be returned
     */
    virtual unsigned long long int GetProgramID() const = 0;

    /**
     * Gets the program build log
     *
     * @Returns
     *  if the log already exist , pointer to the build log will be returned; otherwise NULL 
     *  will be returned
     */
	virtual const char* GetBuildLog() const = 0;

    /**
     * Gets the program Code Container; Program code is an abstraction which contain all
     * the kernel's codes (the executable code with it's metadata)
     *
     * @returns code container interface.
     */
    virtual const ICLDevBackendCodeContainer* GetProgramCodeContainer() const = 0;

    /**
     * Retrieves a pointer to a kernel object by kernel name
     *
     * @param pKernelName pointer to null terminated string that specifiy the kernel name
     * @param ppKernel pointer which will be modified to point to requested kernel object
     *      notice it will return the kernel object itself (not a copy)
     *
     * @returns
     *  if the program already build:
     *		CL_DEV_SUCCESS		        - if kernel descriptor was successfully retrieved
     *		CL_DEV_INVALID_KERNEL_NAME	- if kernel name was not found
     *  else
     *      CL_DEV_NOT_SUPPORTED will be returned
     */
	virtual cl_dev_err_code GetKernelByName(
        const char* pKernelName, 
        const ICLDevBackendKernel_** ppKernel) const = 0;

    /**
     * Gets how many kernels in the program
     *
     * @returns
     *  if the program already build:
     *      the number of the kernels in the program will be returned
     *  else
     *      -1 will be returned
     */
    virtual int GetKernelsCount() const = 0;

    /**
     * Retrieves a pointer to a kernel object by kernel index
     *
     * @param kernelIndex is the index of the kernel should be in the range [0 .. (KernelsCount - 1)]
     * @param pKernel pointer which will hold the returned kernel object
     *      notice it will return the kernel object itself (not a copy)
     *
     * @returns
     *  if the program already build:
     *		CL_DEV_SUCCESS		        - if kernel descriptor was successfully retrieved
     *		CL_DEV_INVALID_KERNEL_INDEX	- if kernel was not found or incorrect index
     *  else
     *      CL_DEV_NOT_SUPPORTED will be returned
     */
	virtual cl_dev_err_code	GetKernel(
        int kernelIndex, 
        const ICLDevBackendKernel_** pKernel) const = 0;

    /**
     * Releases program instance
     */
    virtual void Release() = 0;

    /**
     * Gets the program JIT code Properties;
     *
     * @returns JIT code properties interface, NULL in case of failure
     */
    virtual const ICLDevBackendProgramJITCodeProperties* GetProgramJITCodeProperties() const = 0;
};

}}} // namespace

#endif // ICLDevBackendProgram_H
