// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include <assert.h>
#include <string>
#include "cl_dev_backend_api.h"
#include "ICLDevBackendKernel.h"
#include "TargetArch.h"
#include "Serializer.h"
#include "ICompilerConfig.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelJITProperties
{
public:
    KernelJITProperties();
    virtual ~KernelJITProperties();

    void SetVectorSize(unsigned int size){ m_vectorSize = size; }
    void SetUseVTune(bool value) { m_useVTune = value; }
    void SetMaxPrivateMemorySize(size_t value) { m_maxPrivateMemorySize = value; }

    unsigned int GetVectorSize() const{ return m_vectorSize;}
    bool GetUseVTune() const          { return m_useVTune;}
    size_t GetMaxPrivateMemorySize() const { return m_maxPrivateMemorySize; }

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);

protected:
    bool m_useVTune;
    unsigned int m_vectorSize;
    size_t m_maxPrivateMemorySize;
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
    virtual size_t GetMaxPrivateMemorySize() const;

    /**
     * @param   wgSizeUpperBound - maximum possible WG size.
     * @param   wgPrivateMemSizeUpperBound - maximum possible private memory size per WG.
     * @return  the max. possible WG size with respect to the specified limits.
     */
    virtual size_t GetMaxWorkGroupSize(size_t const wgSizeUpperBound,
                                       size_t const wgPrivateMemSizeUpperBound) const;

    virtual size_t GetNumberOfSubGroups(size_t size, const size_t* WGSizes) const;

    virtual size_t GetMaxNumSubGroups(size_t const wgSizeUpperBound) const;

    virtual size_t GetRequiredNumSubGroups() const;

    virtual size_t GetMaxSubGroupSize(size_t size, const size_t* WGSizes) const;

    /**
     * @returns locals size that would give the desired number of subgroups
     * @param     desiredSGCount - requested number of subgroups
     * @param     wgSizeUpperBound - maximum possible WG size
     * @param     wgPrivateMemSizeUpperBound - maximum possible private memory size per WG
     * @param OUT pValue - output local sizes
     * @param     dim - number of dimensions we need to fill
     */
    virtual void GetLocalSizeForSubGroupCount(size_t const desiredSGCount,
                                              size_t const wgSizeUpperBound,
                                              size_t const wgPrivateMemSizeUpperBound,
                                              size_t* pValue,
                                              size_t const dim) const;

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
     * @returns true if the specified kernel has debug info,
     *  false otherwise
     */
    virtual bool HasDebugInfo() const;

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
     * @returns true if the specified kernel is an autorun kernel
     *  false otherwise
     */
    virtual bool IsAutorun() const;

    /**
     * @returns target device for the platform
     */
    virtual bool TargetDevice() const;

    /**
     * @returns true if the specified kernel is a single-work item kernel
     *  false otherwise
     */
    virtual bool IsTask() const;

    /**
     * @returns true if the specified kernel may use global work offset
     *  false otherwise
     */
    virtual bool CanUseGlobalWorkOffset() const;

    /**
     * @returns true if the specified kernel needs to serialize workgroups
     *  false otherwise
     */
    virtual bool NeedSerializeWGs() const;

    /**
     * @returns true if the specified kernel doesn't support non-unifrom WG size
     *  false otherwise
     */
    virtual bool IsNonUniformWGSizeSupported() const;

    /**
     * @returns required Intel sub group size (0 if none was required)
     */
    size_t GetRequiredSubGroupSize() const override {
        return m_reqdSubGroupSize;
    };

    /**
     * Kernel Properties methods
     */
    void SetTotalImplSize(size_t size) { m_totalImplSize = size;}
    void SetOptWGSize(unsigned int size) { m_optWGSize = size;}
    void SetKernelExecutionLength(size_t length) { m_kernelExecutionLength = length;}
    void SetKernelAttributes(std::string attributes) { m_kernelAttributes = attributes;}
    void SetReqdWGSize(const size_t* psize );
    void SetHintWGSize(const size_t* psize );
    void SetReqdNumSG(size_t value) { m_reqdNumSG = value; }
    void SetDAZ(bool value)        { m_DAZ = value; }
    void SetHasBarrier(bool value) { m_hasBarrier = value; }
    void SetHasGlobalSync(bool value) { m_hasGlobalSync = value; }
    void SetHasNativeSubgroups(bool value) { m_hasNativeSubgroups = value; }
    void SetBarrierBufferSize(size_t size) { m_barrierBufferSize = size; }
    void SetPrivateMemorySize(size_t size) { m_privateMemorySize = size; }
    void SetMaxPrivateMemorySize(size_t size) { m_maxPrivateMemorySize = size; }
    void SetVectorizationWidth(size_t VF) { m_vectorizationWidth = VF; }
    void SetCpuId( const Intel::CPUId &cpuId ) { m_cpuId = cpuId; }
    void SetMinGroupSizeFactorial(unsigned int size) { m_minGroupSizeFactorial = size; }
    void EnableVectorizedWithTail() { m_isVectorizedWithTail = true; }
    void SetPointerSize(unsigned int value) { m_uiSizeT = value; }
    void SetIsBlock(const bool value) { m_bIsBlock = value; }
    void SetIsAutorun(const bool value) { m_bIsAutorun = value; }
    void SetTargetDevice(const DeviceMode value) { m_targetDevice = value; }
    void SetNeedSerializeWGs(const bool value) { m_bNeedSerializeWGs = value; }
    void SetIsTask(const bool value) { m_bIsTask = value; }
    void SetCanUseGlobalWorkOffset(const bool value) { m_bCanUseGlobalWorkOffset = value; }
    void SetIsNonUniformWGSizeSupported(const bool value) { m_bIsNonUniformWGSizeSupported = value; }
    void SetCanUniteWG(const bool value) { m_canUniteWG = value; }
    void SetVerctorizeOnDimention(unsigned int value) { m_verctorizeOnDimention = value; }
    void SetHasDebugInfo(const bool value) { m_debugInfo = value; }
    void SetRequiredSubGroupSize(const size_t value) { m_reqdSubGroupSize = value; }

    unsigned int  GetOptWGSize()      const { return m_optWGSize; }
    const size_t* GetReqdWGSize()     const { return m_reqdWGSize; }
    const size_t* GetHintWGSize()     const { return m_hintWGSize; }
    bool          GetDAZ()            const { return m_DAZ; }
    const CPUId   &GetCpuId()          const { return m_cpuId; }
    bool          IsVectorizedWithTail() const { return m_isVectorizedWithTail; }
    // Get size of pointer in bytes
    unsigned int  GetPointerSize()     const { return m_uiSizeT;}
    bool          GetCanUniteWG() const { return m_canUniteWG; }
    unsigned int  GetVectorizedDimention() const { return m_verctorizeOnDimention;}

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);


protected:
    bool m_hasBarrier;
    bool m_hasGlobalSync;
    bool m_hasNativeSubgroups;
    bool m_DAZ;
    Intel::CPUId m_cpuId;       // selected cpuId for current kernel codegen
    unsigned int m_optWGSize;
    size_t m_reqdWGSize[MAX_WORK_DIM];  // Required work-group size that was declared during kernel compilation
    size_t m_hintWGSize[MAX_WORK_DIM];  // Hint to work-group size that was declared during kernel compilation
    size_t m_totalImplSize;
    size_t m_barrierBufferSize;
    size_t m_privateMemorySize;
    size_t m_maxPrivateMemorySize;
    size_t m_reqdNumSG;
    size_t m_kernelExecutionLength;
    size_t m_vectorizationWidth;
    size_t m_reqdSubGroupSize;
    std::string m_kernelAttributes;
    unsigned int m_minGroupSizeFactorial;
    bool m_isVectorizedWithTail;
    unsigned int m_uiSizeT;
    bool m_bIsBlock;
    bool m_bIsAutorun;
    bool m_bNeedSerializeWGs;
    bool m_bIsTask;
    bool m_bCanUseGlobalWorkOffset;
    bool m_bIsNonUniformWGSizeSupported;
    bool m_canUniteWG;
    unsigned int m_verctorizeOnDimention;
    bool m_debugInfo;
    DeviceMode m_targetDevice;
};

}}}
