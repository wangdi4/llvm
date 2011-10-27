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
#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "cl_dev_backend_api.h"
#include "cpu_dev_limits.h"
#include "ExplicitLocalMemArgument.h"
#include "TypeAlignment.h"

namespace llvm {
    class Module;
    class Function;
    class ExecutionEngine;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class KernelProperties;
class IKernelJITContainer;
class Executable;

struct sWorkInfo
{
    unsigned int    uiWorkDim;
    size_t          GlobalOffset[MAX_WORK_DIM];
    size_t          GlobalSize[MAX_WORK_DIM];
    size_t          LocalSize[MAX_WORK_DIM];
    size_t          WGNumber[MAX_WORK_DIM];
};

class Binary: public ICLDevBackendBinary_
{
public:
    Binary( const KernelProperties* pKernelProperties,
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
    size_t GetStackSize() const            {return m_stStackSize;}
    size_t GetPrivateMemorySize() const    {return m_stPrivateMemorySize;}
    bool   GetDAZ() const                  {return m_DAZ; }
    unsigned int GetCpuId() const          {return m_cpuId; }
    unsigned int GetCpuFeatures() const    {return m_cpuFeatures;}
    void*  GetFormalParameters() const     {return m_pLocalParams;}
    size_t GetImplicitLocalMemoryBufferSize() const {return m_totalImplSize;}
    
protected:
    virtual Executable* CreateExecutableImp(Binary* pBinary) const = 0;

private:
    void InitWorkInfo(const cl_work_description_type* pWorkInfo);
    void InitParams(const std::vector<cl_kernel_argument>& args, char* pArgsBuffer);

private:
    // TODO : add getter instead?
    friend class Executable;
    friend class ImplicitArgsUtils;

    const void*             m_pEntryPoint;
    size_t                  m_stFormalParamSize;
    size_t                  m_stKernelParamSize;
    size_t                  m_stAlignedKernelParamSize;
    size_t                  m_stWIidsBufferSize;
    size_t                  m_stStackSize;
    size_t                  m_stPrivateMemorySize;
    sWorkInfo               m_WorkInfo;
    char*                   m_pLocalParams;
    std::vector<ExplicitLocalMemArgument> m_kernelLocalMem;
    char                    m_pLocalParamsBase[CPU_MAX_PARAMETER_SIZE*4 + TypeAlignment::MAX_ALIGNMENT];
    unsigned int            m_uiWGSize;
    size_t                  m_totalImplSize;
    bool                    m_DAZ;
    unsigned int            m_cpuId;
    unsigned int            m_cpuFeatures;
    // Vectorizer data
    bool                    m_bVectorized;
    unsigned int            m_uiVectorWidth;
    const void*             m_pVectEntryPoint;
    // Points to m_pEntryPoint or m_pVectEntryPoint according to m_bVectorized!
    const void*             m_pUsedEntryPoint;
};



}}}