/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Program.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include <memory>
#include "cl_dev_backend_api.h"
#include "cl_types.h"
#include "ICLDevBackendProgram.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelSet;
class BitCodeContainer;

class Program: public ICLDevBackendProgram_
{
public:
    Program();
    virtual ~Program();

    /**
     * @returns an unsigned long which represents the program id - this id is unique
     *  per program - ; in case of failure 0 will be returned
     */
    virtual unsigned long long int GetProgramID() const;

    /**
     * Gets the program build log
     *
     * @Returns
     *  if the log already exist , pointer to the build log will be returned; otherwise NULL 
     *  will be returned
     */
    virtual const char* GetBuildLog() const;

    /**
     * Gets the program Code Size; Program code is an abstraction between which contain all
     * the kernel's codes (the executable code with it's metadata)
     *
     * @returns the size of the kernel code, 0 in case of failure
     */
    virtual const ICLDevBackendCodeContainer* GetProgramCodeContainer() const;

    /**
     * Gets the program JIT Code Properties; 
     *
     * @returns JIT Code properties interface, NULL in case of failure
     */
    virtual const ICLDevBackendProgramJITCodeProperties* GetProgramJITCodeProperties() const;

    /**
     * Retrieves a pointer to a kernel object by kernel name
     *
     * @param pKernelName pointer to null terminated string that specifiy the kernel name
     * @param ppKernel pointer which will be modified to point to requested kernel object
     *      notice it will return the kernel object itself (not a copy)
     *
     * @returns
     *  if the program already build:
     *      CL_DEV_SUCCESS              - if kernel descriptor was successfully retrieved
     *      CL_DEV_INVALID_KERNEL_NAME  - if kernel name was not found
     *  else
     *      CL_DEV_NOT_SUPPORTED will be returned
     */
    virtual cl_dev_err_code GetKernelByName(
        const char* pKernelName, 
        const ICLDevBackendKernel_** ppKernel) const;

    /**
     * Gets how many kernels in the program
     *
     * @returns
     *  if the program already build:
     *      the number of the kernels in the program will be returned
     *  else
     *      -1 will be returned
     */
    virtual int GetKernelsCount() const;

    /**
     * Retrieves a pointer to a kernel object by kernel index
     *
     * @param kernelIndex is the index of the kernel should be in the range [0 .. (KernelsCount - 1)]
     * @param pKernel pointer which will hold the returned kernel object
     *      notice it will return the kernel object itself (not a copy)
     *
     * @returns
     *  if the program already build:
     *      CL_DEV_SUCCESS              - if kernel descriptor was successfully retrieved
     *      CL_DEV_INVALID_KERNEL_INDEX - if kernel was not found or incorrect index
     *  else
     *      CL_DEV_NOT_SUPPORTED will be returned
     */
    virtual cl_dev_err_code GetKernel(
        int kernelIndex, 
        const ICLDevBackendKernel_** ppKernel) const;

    /**
     * Releases program instance
     */
    virtual void Release();


    /**
     * Sets the Bit Code Container (program will take ownership of the container)
     */
    void SetBitCodeContainer(BitCodeContainer* bitCodeContainer);

    /*
     * Program specific methods 
     */
    void SetBuildLog( const std::string& buildLog );

    /**
     * Store the given kernel set into the program
     * 
     * Note: will take ownership on passed kernel set
     */
    void SetKernelSet( KernelSet* pKernels); 

    /**
     * Store the given LLVM module (as a plain pointer) 
     * into the program
     *
     * Note: will take ownership on passed module
     */
    void SetModule( void* pModule);

    bool GetDisableOpt() const;

    bool GetDebugInfoFlag() const;

    bool GetFastRelaxedMath() const;

    bool GetDAZ() const;

protected:
    BitCodeContainer* m_pCodeContainer;
    std::string       m_buildLog;
    std::auto_ptr<KernelSet> m_kernels;

};

}}}