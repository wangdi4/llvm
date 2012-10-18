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

File Name:  Binary.h

\*****************************************************************************/

#ifndef __BINARY_H__
#define __BINARY_H__

#include "cl_dev_backend_api.h"
#include "cpu_dev_limits.h"
#include "ExplicitLocalMemArgument.h"
#include "TypeAlignment.h"
#include "IAbstractBackendFactory.h"
#include "TargetArch.h"
#include "ExecutionContext.h"

#include <vector>

namespace llvm {
    class Module;
    class Function;
    class ExecutionEngine;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelProperties;
class IKernelJITContainer;
class Executable;

class Binary: public ICLDevBackendBinary_
{
public:
    Binary( IAbstractBackendFactory* pBackendFactory, 
            ICLDevBackendBufferPrinter* pPrinter,
            const KernelProperties* pKernelProperties,
            const std::vector<cl_kernel_argument>& args,
            const cl_work_description_type* pWorkInfo,
            const IKernelJITContainer* pScalarJIT,
            const IKernelJITContainer* pVectorJIT,
            char* IN pArgsBuffer, 
            size_t IN ArgBuffSize);

    // Returns the required number of memory buffers and their sizes 
    //  pBuffersSizes - an array of sizes of buffers
    //  pBufferCount - the number of buffers required for executing the kernels
    // Returns
    //      CL_DEV_BE_SUCCESS - the execution completed successfully
    //
    virtual cl_dev_err_code GetMemoryBuffersDescriptions(size_t* IN pBuffersSizes, size_t* INOUT pBufferCount ) const;

    // Returns the actual number of Work Items handled by each executable instance
    virtual const size_t* GetWorkGroupSize() const
    {
        return m_WorkInfo.LocalSize;
    }

    // Returns the actual vector size that was used for the kernel 
    virtual unsigned int GetVectorSize() const
    {
        return m_uiVectorWidth;
    }

    // Create execution context that will be used by specific execution threads
    virtual cl_dev_err_code CreateExecutable(void* IN *pMemoryBuffers, 
                                             size_t IN stBufferCount, 
                                             ICLDevBackendExecutable_* OUT *pContext);
    /**
     * Releases binary instance
     */
    virtual void Release();

    // Local methods //TEMPORARY
    size_t GetFormalParametersSize() const {return m_stFormalParamSize;}
    size_t GetKernelParametersSize() const {return m_stKernelParamSize;}
    size_t GetAlignedKernelParametersSize() const {return m_stAlignedKernelParamSize;}
    size_t GetLocalWIidsSize() const {return m_stWIidsBufferSize;}
    size_t GetPrivateMemorySize() const    {return m_stPrivateMemorySize;}
    bool   GetDAZ() const                  {return m_DAZ; }
    const Intel::CPUId &GetCpuId() const   {return m_cpuId; }
    void*  GetFormalParameters() const     {return m_pLocalParams;}
    size_t GetImplicitLocalMemoryBufferSize() const {return m_totalImplSize;}

    ICLDevBackendBufferPrinter* GetDevicePrinter() const { return m_pPrinter;}
    
protected:
    // pointer to the Backend Factory, not owned by this class
    IAbstractBackendFactory* m_pBackendFactory; 

private:
    void InitWorkInfo(const cl_work_description_type* pWorkInfo);
    void InitParams(const std::vector<cl_kernel_argument>& args, char* pArgsBuffer);

private:
    // TODO : add getter instead?
    friend class Executable;

    // for printer service - not owned by this class
    ICLDevBackendBufferPrinter* m_pPrinter;

    size_t                  m_stFormalParamSize;
    size_t                  m_stKernelParamSize;
    size_t                  m_stAlignedKernelParamSize;
    size_t                  m_stWIidsBufferSize;
    size_t                  m_stPrivateMemorySize;
    sWorkInfo               m_WorkInfo;
    char*                   m_pLocalParams;
    std::vector<ExplicitLocalMemArgument> m_kernelLocalMem;
    char                    m_pLocalParamsBase[CPU_MAX_PARAMETER_SIZE*4 + TypeAlignment::MAX_ALIGNMENT];
    unsigned int            m_uiWGSize;
    size_t                  m_totalImplSize;
    bool                    m_DAZ;
    Intel::CPUId            m_cpuId;
    bool                    m_bJitCreateWIids;
    // Vectorizer data
    unsigned int            m_uiVectorWidth;
    const void*             m_pUsedEntryPoint;
};



}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __BINARY_H__