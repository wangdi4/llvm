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

File Name:  KernelProperties.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include "cl_dev_backend_api.h"
#include "ICLDevBackendKernel.h"
#include "TLLVMKernelInfo.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelJITProperties
{
public:
    KernelJITProperties();

    void SetVectorSize(unsigned int size){ m_vectorSize = size; }
    void SetStackSize(unsigned int size) { m_stackSize = size;}
    void SetVTuneId(unsigned int id) { m_VTuneId = id; }
    void SetUseVTune(bool value) { m_useVTune = value; }

    unsigned int GetVectorSize() const{ return m_vectorSize;}
    unsigned int GetStackSize() const { return m_stackSize;}
    unsigned int GetVTuneId() const   { return m_VTuneId; }
    bool GetUseVTune() const          { return m_useVTune;}

protected:
    bool m_useVTune;
    unsigned int m_VTuneId;
    unsigned int m_stackSize;
    unsigned int m_vectorSize;
};


class KernelProperties: public ICLDevBackendKernelProporties
{
public:
    KernelProperties();
    /*
     * ICLDevBackendKernelProporties methods
     */ 

    /**
     * @returns the number of Work Items handled by each kernel instance,
     *  0 will be returned in case of failure or not present
     */
    virtual unsigned int GetKernelPackCount() const;

    /**
     * @returns the required work-group size that was declared during kernel compilation.
     *  NULL when this attribute is not present; 
     *  whereas work-group size is array of MAX_WORK_DIM entries
     */
    virtual const size_t* GetRequiredWorkGroupSize() const;

    /**
     * @returns the required private memory size for single Work Item execution
     *  0 when is not available
     */
    virtual size_t GetPrivateMemorySize() const;

    /**
     * @returns the size in bytes of the implicit local memory buffer required by this kernel
     * (implicit local memory buffer is the size of all the local buffers declared and used
     *  in the kernel body)
     */
    virtual size_t GetImplicitLocalMemoryBufferSize() const;

    /**
     * @returns true if the specified kernel has print operation in the kernel body, 
     *  false otherwise
     */
    virtual bool HasPrintOperation() const;

    /**
     * @returns true if the specified kernel has barrier operation in the kernel body, 
     *  false otherwise
     */
    virtual bool HasBarrierOperation() const;

    /**
     * @returns true if the specified kernel calls other kernerls in the kernel body, 
     *  false otherwise
     */
    virtual bool HasKernelCallOperation() const;

    /*
     * Kernel Properties methods
     */ 
    void SetTotalImplSize(size_t size) { m_totalImplSize = size;}
    void SetOptWGSize(unsigned int size) { m_optWGSize = size;} 
    void SetReqdWGSize(const size_t* psize );
    void SetHintWGSize(const size_t* psize );
    void SetDAZ(bool value)        { m_DAZ = value; }
    void SetPrivateMemorySize(size_t size) { m_privateMemorySize = size; }
    void SetCpuId( unsigned int cpuId ) { m_cpuId = cpuId; }
    void SetCpuFeatures( unsigned int cpuFeatures ) { m_cpuFeatures = cpuFeatures; }
    
    unsigned int  GetOptWGSize()      const { return m_optWGSize; } 
    const size_t* GetReqdWGSize()     const { return m_reqdWGSize; }
    const size_t* GetHintWGSize()     const { return m_hintWGSize; }
    bool          GetDAZ()            const { return m_DAZ; }
    unsigned int  GetCpuId()          const { return m_cpuId; }
    unsigned int  GetCpuFeatures()    const { return m_cpuFeatures; }

protected:
    bool m_DAZ;
    unsigned int m_cpuId;       // selected cpuId for current kernel codegen
    unsigned int m_cpuFeatures; // selected cpu features /see ECPUFeatures enum
    unsigned int m_optWGSize;
    size_t m_reqdWGSize[MAX_WORK_DIM];  // Required work-group size that was declared during kernel compilation
    size_t m_hintWGSize[MAX_WORK_DIM];  // Hint to work-group size that was declared during kernel compilation
    size_t m_totalImplSize;
    size_t m_privateMemorySize;

};


}}}