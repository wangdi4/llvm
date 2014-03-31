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
// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include "cl_dev_backend_api.h"
#include "cl_types.h"
#include "ICLDevBackendProgram.h"
#include "RuntimeService.h"
#include "Serializer.h"
#include <string>
#include <memory>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelSet;
class BitCodeContainer;
class ObjectCodeContainer;

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
     * @returns the virtual IR code container which represents the program
     */
    virtual const ICLDevBackendCodeContainer* GetProgramIRCodeContainer() const;

    /**
     * Gets the program Code; Program code is an abstraction between which contain all
     * the kernel's codes (the executable code, the IR and with some metadata)
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
     * OpenCL 2.0 introduced a feature called Extended Execution. Programs may have so called
     * block kernels which can be enqueued for execution w\o host interaction from inside
     * running kernels.
     * This method returns how many non-block kernels in the program. I.e. the kernels
     * enqueud for execution by a host.
     *
     * @returns
     *  if the program already build:
     *      the number of the non-block kernels in the program will be returned
     */
    virtual int GetNonBlockKernelsCount() const;

    /**
     * Gets how many kernels in the program
     *
     * @returns
     *  if the program already build:
     *      the number of the kernels in the program will be returned
     *  else
     *      0 will be returned
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
     * Gets the total amount of storage, in bytes, used by
     * program variables in the global address space.
     *
     * @returns
     *  if the program already build:
     *      the total size of global variables in program
     *  otherwise
     *      0 will be returned
     */
    virtual size_t GetGlobalVariableTotalSize() const;

    /**
     * Sets the total amount of storage, in bytes, used by
     * program variables in the global address space.
     */
    void SetGlobalVariableTotalSize(size_t);

    /**
     * Sets the Object Code Container (program will take ownership of the container)
     */
    void SetObjectCodeContainer(ObjectCodeContainer* objCodeContainer);
    ObjectCodeContainer* GetObjectCodeContainer();

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
    KernelSet* GetKernelSet() { return m_kernels.get(); }

    /**
     * Store the given LLVM module (as a plain pointer)
     * into the program
     *
     * Note: will take ownership on passed module
     */
    void SetModule( void* pModule);
    virtual void SetBuiltinModule(void* pModule) {}

    virtual void SetExecutionEngine(void *eE) {}

    bool GetDisableOpt() const;

    bool GetDebugInfoFlag() const;

    bool GetProfilingFlag() const;
    bool GetFastRelaxedMath() const;

    bool GetDAZ() const;

    /// get runtime service
    RuntimeServiceSharedPtr GetRuntimeService() const{
      return m_RuntimeService;
    }

    /// set runtime service
    void SetRuntimeService(const RuntimeServiceSharedPtr& rs ) {
      m_RuntimeService = rs;
    }

    /**
     * Returns the LLVM module (as a plain pointer)
     */
    void* GetModule();

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats); 

protected:
    ObjectCodeContainer* m_pObjectCodeContainer;
    BitCodeContainer* m_pIRCodeContainer;
    std::string       m_buildLog;
    std::auto_ptr<KernelSet> m_kernels;
    /// Runtime service. Reference counted
    RuntimeServiceSharedPtr m_RuntimeService;
    size_t            m_globalVariableTotalSize;

private:
    // Disable copy ctor and assignment operator
    Program( const Program& );
    bool operator = (const Program& );

};

}}}
