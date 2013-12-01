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

File Name:  Kernel.h

\*****************************************************************************/
// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE 
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS  
#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "RuntimeService.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelProperties;
class KernelJITProperties;

class IKernelJITContainer: public ICLDevBackendJITContainer
{
public:
    /*
     * ICLDevBackendJITContainer methods
     */
    virtual const void* GetJITCode() const = 0;
    virtual size_t GetJITCodeSize()  const = 0;

    /*
     * Free machine code
     */
    virtual void FreeJITCode() {}

    /*
     * JITContainer methods
     */
    virtual KernelJITProperties* GetProps() const = 0;
};


class Kernel: public ICLDevBackendKernel_
{
public:
    Kernel():m_pProps(NULL) { };

    Kernel(const std::string& name,
           const std::vector<cl_kernel_argument>& args,
           const std::vector<unsigned int>& memArgs,
           KernelProperties* pProps);

    virtual ~Kernel();

    /*
     * ICLDevBackendKernel interface implementation
     */

    /**
     * @returns an unsigned long which represents the kernel id - this id is unique
     *  per kernel - ; in case of failure 0 will be returned
     */
    virtual unsigned long long int GetKernelID() const;

    /**
     * @returns a pointer to the kernel name, in case of failure NULL will be returned
     */
    virtual const char* GetKernelName() const;

    /**
     * Gets the kernels paramaters count
     *
     * @returns the count of the parameters, in case of failure -1 will be returned
     */
    virtual int GetKernelParamsCount() const;

    /**
     * Gets the kernel parameters description
     * 
     * @returns
     *  In success will return the kernel arguments descriptor; otherwise, NULL 
     *  value will be returned
     */
    virtual const cl_kernel_argument* GetKernelParams() const;

    /**
     * Gets the kernel parameters extended information
     * 
     * @returns
     *  In success will return the kernel arguments information; otherwise, NULL 
     *  value will be returned
     */
    virtual const cl_kernel_argument_info* GetKernelArgInfo() const;

    /**
     * Gets the description of the kernel body, the returned object contains all the kernel
     * body proporties
     *
     * @returns reference to IKernelDescription object
     */
    virtual const ICLDevBackendKernelProporties* GetKernelProporties() const;

    /**
     * @param pointer Pointer to an instruction contained in this kernel's
     * JITted code.
     * @returns the the source line number from which the instruction
     * pointed to was compiled. If the pointer does not point to an
     * instruction in this kernel, or if line number information is missing,
     * this returns -1.
     */
    virtual int GetLineNumber(void* pointer) const;

     /**
     * @returns the size of argument/parameter buffer requried by the kernel
     */
     virtual size_t GetArgumentBufferSize() const;

     /**
     * @returns the number of memory object arguments passed to the kernel
     */
     virtual unsigned int GetMemoryObjectArgumentCount() const;

     /**
     * @returns the array of indexes of memory object arguments passed to the kernel
     */
     virtual const unsigned int* GetMemoryObjectArgumentIndexes() const;

    /*
     * Kernel class methods
     */

    /**
     * Returns the vector of kernel parameters
     */ 
    const std::vector<cl_kernel_argument>* GetKernelParamsVector() const;

    /**
     * Adds kernel JIT version to the kernel. 
     */ 
    void AddKernelJIT( IKernelJITContainer* pJIT);

    /**
     * Returns the kernel JIT buffer for the specified index
     */ 
    const IKernelJITContainer* GetKernelJIT( unsigned int index) const ;

    /**
     * Returns the count of the JIT buffer for current kernel
     */
    unsigned int GetKernelJITCount() const;

    /**
     * Asks the execution engine to free the machine code
     */
    void FreeAllJITs();

    /**
     * Calculate the local workgroup sizes if one was not specified in the input 
     * work sizes
     */ 
    void CreateWorkDescription( const cl_work_description_type* pInputWorkSizes, 
                                cl_work_description_type&       outputWorkSizes) const;
    /**
     * get RuntimeService
     */ 
    RuntimeServiceSharedPtr GetRuntimeService() const {
      return m_RuntimeService;
    }

    /**
     * set RuntimeService
     */ 
    void SetRuntimeService(const RuntimeServiceSharedPtr& rs ){
      assert(rs.get() && "RuntimeService is non-initialized");
      m_RuntimeService = rs;
    }

protected:

    std::string m_name;
    std::vector<cl_kernel_argument>     m_args;
    std::vector<unsigned int>           m_memArgs;
    KernelProperties*                   m_pProps;
    std::vector<IKernelJITContainer*>   m_JITs;
    // RuntimeService. Refcounted
    RuntimeServiceSharedPtr             m_RuntimeService;

private:
    // Disable copy ctor and assignment operator
    Kernel( const Kernel& );
    bool operator = (const Kernel& );
};

/**
 * Kernels holder
 * 
 * The main usage of this class is for keeping the creation of the kernels 
 * and updating the program as one transaction. 
 */
class KernelSet
{
public:
    ~KernelSet();

    void AddKernel(Kernel* pKernel)
    { 
        m_kernels.push_back(pKernel); 
    } 

    size_t GetCount() const 
    { 
        return m_kernels.size(); 
    }

    bool   Empty() const 
    { 
        return m_kernels.empty(); 
    }

    Kernel* GetKernel(int index) const;

    Kernel* GetKernel(const char* name) const;

private:
    std::vector<Kernel*> m_kernels;
};


}}}
