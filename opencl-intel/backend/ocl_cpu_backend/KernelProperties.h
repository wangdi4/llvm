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
// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include <assert.h>
#include <string>
#include "cl_dev_backend_api.h"
#include "ICLDevBackendKernel.h"
#include "TargetArch.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelJITProperties
{
public:
    KernelJITProperties();

    void SetVectorSize(unsigned int size){ m_vectorSize = size; }
    void SetUseVTune(bool value) { m_useVTune = value; }

    unsigned int GetVectorSize() const{ return m_vectorSize;}
    bool GetUseVTune() const          { return m_useVTune;}

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);

protected:
    bool m_useVTune;
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
     * @returns the required barrier buffer memory size for single Work Item execution
     *  0 when there are no WG level built-ins in the kernel.
     */
    virtual size_t GetBarrierBufferSize() const;

    /**
     * @returns the min required private memory size for single Work Item execution.
     *          It includes memory allocated on the stack statically (per WI) plus
     *          barrier buffer size. It also migth include some extra space for
     *          external functions called by the kernel.
     */
    virtual size_t GetPrivateMemorySize() const;

    /**
     * @param   wgSizeUpperBound - maximum possible WG size.
     * @param   wgPrivateMemSizeUpperBound - maximum possible private memory size per WG.
     * @return  the max. possible WG size with respect to the specified limits.
     */
    virtual size_t GetMaxWorkGroupSize(size_t const wgSizeUpperBound,
                                       size_t const wgPrivateMemSizeUpperBound) const;

    /**
     * @returns the required minimum group size factorial
     *  1 when no minimum is required
     */
    unsigned int GetMinGroupSizeFactorial() const;

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
     * @returns an estimation of the kernel execution
     */
    virtual size_t GetKernelExecutionLength() const;

    /**
     * @returns a string of the kernel attributes
     */
    virtual const char *GetKernelAttributes() const;

    /**
     * @returns true if the specified kernel has barrier operation in the kernel body,
     *  false otherwise
     */
    virtual bool HasBarrierOperation() const;

    /**
     * @returns true if the specified kernel has global synchronization
     *  in the kernel body (e.g. atomic_add to global memory),
     *  false otherwise
     */
    virtual bool HasGlobalSyncOperation() const;

    /**
     * @returns true if the specified kernel calls other kernerls in the kernel body,
     *  false otherwise
     */
    virtual bool HasKernelCallOperation() const;

    /**
     * @returns true if the specified kernel is created from clang's block
     *  false otherwise
     */
    virtual bool IsBlock() const;
    /**
     * @returns true if the specified kernel doesn't support non-unifrom WG size
     *  false otherwise
     */
    virtual bool IsNonUniformWGSizeSupported() const;

    /**
     * Kernel Properties methods
     */
    void SetTotalImplSize(size_t size) { m_totalImplSize = size;}
    void SetOptWGSize(unsigned int size) { m_optWGSize = size;}
    void SetKernelExecutionLength(size_t length) { m_kernelExecutionLength = length;}
    void SetKernelAttributes(std::string attributes) { m_kernelAttributes = attributes;}
    void SetReqdWGSize(const size_t* psize );
    void SetHintWGSize(const size_t* psize );
    void SetDAZ(bool value)        { m_DAZ = value; }
    void SetHasBarrier(bool value) { m_hasBarrier = value; }
    void SetHasGlobalSync(bool value) { m_hasGlobalSync = value; }
    void SetBarrierBufferSize(size_t size) { m_barrierBufferSize = size; }
    void SetPrivateMemorySize(size_t size) { m_privateMemorySize = size; }
    void SetCpuId( const Intel::CPUId &cpuId ) { m_cpuId = cpuId; }
    void SetMinGroupSizeFactorial(unsigned int size) { m_minGroupSizeFactorial = size; }
    void EnableVectorizedWithTail() { m_isVectorizedWithTail = true; }
    void SetPointerSize(unsigned int value) { m_uiSizeT = value; }
    void SetIsBlock(const bool value) { m_bIsBlock = value; }
    void SetIsNonUniformWGSizeSupported(const bool value) { m_bIsNonUniformWGSizeSupported = value; }

    unsigned int  GetOptWGSize()      const { return m_optWGSize; }
    const size_t* GetReqdWGSize()     const { return m_reqdWGSize; }
    const size_t* GetHintWGSize()     const { return m_hintWGSize; }
    bool          GetDAZ()            const { return m_DAZ; }
    const CPUId   &GetCpuId()          const { return m_cpuId; }
    bool          IsVectorizedWithTail() const { return m_isVectorizedWithTail; }
    // Get size of pointer in bytes
    unsigned int  GetPointerSize()     const { return m_uiSizeT;}

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats); 


protected:
    bool m_hasBarrier;
    bool m_hasGlobalSync;
    bool m_DAZ;
    Intel::CPUId m_cpuId;       // selected cpuId for current kernel codegen
    unsigned int m_optWGSize;
    size_t m_reqdWGSize[MAX_WORK_DIM];  // Required work-group size that was declared during kernel compilation
    size_t m_hintWGSize[MAX_WORK_DIM];  // Hint to work-group size that was declared during kernel compilation
    size_t m_totalImplSize;
    size_t m_barrierBufferSize;
    size_t m_privateMemorySize;
    size_t m_kernelExecutionLength;
    std::string m_kernelAttributes;
    unsigned int m_minGroupSizeFactorial;
    bool m_isVectorizedWithTail;
    unsigned int m_uiSizeT;
    bool m_bIsBlock;
    bool m_bIsNonUniformWGSizeSupported;
};

}}}
