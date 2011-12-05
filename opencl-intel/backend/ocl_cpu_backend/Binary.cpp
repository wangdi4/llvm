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

File Name:  Binary.cpp

\*****************************************************************************/

#include "cl_types.h"
#include "Binary.h"
#include "KernelProperties.h"
#include "Kernel.h"    //TEMPORARY
#include "exceptions.h"
#include "Executable.h"
#include "TypeAlignment.h"
#include "ExplicitArgument.h"
#include "ExplicitLocalMemArgument.h"
#include "ExplicitGlobalMemArgument.h"

#include <string.h>
#include <memory>

size_t GCD(size_t a, size_t b);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

//cl_uint IN WorkDimension, const size_t* IN pGlobalOffset, const size_t* IN pGlobalWorkSize, const size_t* IN pLocalWorkSize


template <typename T>
static T align_to(T addr, size_t alignment)
{
    assert(sizeof(T) == sizeof(size_t));
    return reinterpret_cast<T>((reinterpret_cast<size_t>(addr) + alignment-1) & ~(alignment-1));
}

template <>
size_t align_to<size_t>(size_t addr, size_t alignment)
{
    return (addr + alignment-1) & ~(alignment-1);
}


Binary::Binary(IAbstractBackendFactory* pBackendFactory, 
               const KernelProperties* pKernelProperties,
               const std::vector<cl_kernel_argument>& args,
               const cl_work_description_type* pWorkInfo,
               const IKernelJITContainer* pScalarJIT,
               const IKernelJITContainer* pVectorJIT,
               char* IN pArgsBuffer, 
               size_t IN ArgBuffSize):
     m_pBackendFactory(pBackendFactory),
     m_pEntryPoint(pScalarJIT->GetJITCode()),
     m_stFormalParamSize(0),
     m_stKernelParamSize(0),
     m_stAlignedKernelParamSize(0),
     m_stStackSize(0),
     m_stPrivateMemorySize(pKernelProperties->GetPrivateMemorySize()),
     m_pLocalParams(NULL), 
     m_totalImplSize(pKernelProperties->GetImplicitLocalMemoryBufferSize()),
     m_DAZ( pKernelProperties->GetDAZ()),
     m_cpuId( pKernelProperties->GetCpuId()),
     m_cpuFeatures( pKernelProperties->GetCpuFeatures()),
     m_bVectorized(false),
     m_uiVectorWidth(1), 
     m_pVectEntryPoint(0),
     m_pUsedEntryPoint(0)
{
    InitWorkInfo(pWorkInfo);
    
    m_pLocalParams = align_to(m_pLocalParamsBase, TypeAlignment::MAX_ALIGNMENT);
#ifdef _DEBUG
    memset(m_pLocalParams, 0x88, CPU_MAX_PARAMETER_SIZE);
#endif

    InitParams(args, pArgsBuffer);

    //TODO: move this information from Binary to KernelProperties
    size_t jitStackSize = pScalarJIT->GetProps()->GetStackSize();

    if( NULL != pVectorJIT )
    {
        m_bVectorized = true;
        m_pVectEntryPoint = pVectorJIT->GetJITCode();
        m_uiVectorWidth = pVectorJIT->GetProps()->GetVectorSize();

        assert (m_pVectEntryPoint);

        jitStackSize = std::max<size_t>(jitStackSize, pVectorJIT->GetProps()->GetStackSize());
    }

    m_stStackSize = (jitStackSize +                        // Kernel stack area
                     sizeof(void*) +                       // Return address
                     m_stKernelParamSize +                 // Kernel call stack size
                     0x1F) & ~0x1F;                        // alignment up to 32 bytes


    if(m_bVectorized && (m_WorkInfo.LocalSize[0] % m_uiVectorWidth)) {
        // Disable vectorization for workgroup sizes that are not
        // a multiple of the vector width (Guy)
        m_bVectorized = false;
    }
    assert( (!m_bVectorized || m_uiWGSize != 1) && "vectorized with WGsize = 1!" );

    m_pUsedEntryPoint = m_bVectorized ? m_pVectEntryPoint : m_pEntryPoint;

    unsigned int uiWGSizeLocal = m_uiWGSize;
    if ( m_bVectorized ) {
      uiWGSizeLocal = uiWGSizeLocal / m_uiVectorWidth;
    }
    m_stWIidsBufferSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(
      uiWGSizeLocal * sizeof(size_t) * CPU_MAX_WI_DIM_POW_OF_2);

}

void Binary::InitParams(const std::vector<cl_kernel_argument>& args, char* pArgsBuffer)
{
    char* pArgValueDest = m_pLocalParams;
    char* pArgValueSrc = pArgsBuffer;
    
    // Iterate over kernel's explicit arguments
    for(std::vector<cl_kernel_argument>::const_iterator argIterator = args.begin(), e = args.end(); 
      argIterator != e; ++argIterator) {
      
      cl_kernel_argument arg = *argIterator;
      
      std::auto_ptr<IArgument> pArg;
      
      if ((arg.type == CL_KRNL_ARG_PTR_GLOBAL)
        || (arg.type == CL_KRNL_ARG_PTR_CONST)) {
        pArg = std::auto_ptr<ExplicitGlobalMemArgument>(new ExplicitGlobalMemArgument(pArgValueDest, arg));
      } 
      else if (arg.type == CL_KRNL_ARG_PTR_LOCAL) {
        
        // *((size_t*)pArgValueSrc) : 
        // pArgValueSrc contains the local buffer size, the local buffer pointer
        // will be known during creation of Executbale
        
        // pArgValueDest - m_pLocalParams:
        // We want to save the offset of the argument from the beggining of
        // an arguments buffer, so that we know where to initialize the local buffer
        // pointer value in the Executable's arguments buffer (which is a copy of m_pLocalParams
        
        std::auto_ptr<ExplicitLocalMemArgument> pLocalMemArg(
                                                  new ExplicitLocalMemArgument(arg, 
                                                                              *((size_t*)pArgValueSrc), 
                                                                              pArgValueDest - m_pLocalParams));
        
        m_kernelLocalMem.push_back(*pLocalMemArg);
        pArg = pLocalMemArg;
      }
      else {
        pArg = std::auto_ptr<ExplicitArgument>(new ExplicitArgument(pArgValueDest, arg));
      }
      
      pArg->setValue(pArgValueSrc);
      
      // Advance the src buffer according to argument's size (the sec buffer is packed)
      pArgValueSrc += pArg->getSize();
      // Advance the dest buffer according to argument's size and alignment
      pArgValueDest += pArg->getAlignedSize();
    }
    
    // Calculate parameter sizes
    m_stFormalParamSize = pArgValueDest - m_pLocalParams;

    m_stKernelParamSize = m_stFormalParamSize +
                          4*sizeof(void*)+ // OCL specific argument (WG Info, pBaseGlobalID, ect)
                          sizeof(void*)+   // pWI-ids[]
                          sizeof(void*)+   // Pointer to IDevExecutable
                          sizeof(size_t)+2*sizeof(void*); // iterCount, pSpecialBuffer, pcurrWI

    m_stAlignedKernelParamSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(m_stKernelParamSize);
}

void Binary::InitWorkInfo(const cl_work_description_type* pWorkInfo)
{
    assert(pWorkInfo && pWorkInfo->localWorkSize[0] != 0 && "LocalWorkSize must be properly initialized");
    memset(&m_WorkInfo, 0, sizeof(sWorkInfo));

    m_WorkInfo.uiWorkDim = pWorkInfo->workDimension;

#if defined(_WIN32)
    memcpy_s(m_WorkInfo.GlobalOffset, MAX_WORK_DIM*sizeof(size_t),   pWorkInfo->globalWorkOffset, MAX_WORK_DIM*sizeof(size_t));
    memcpy_s(m_WorkInfo.GlobalSize,   MAX_WORK_DIM*sizeof(size_t),   pWorkInfo->globalWorkSize,   MAX_WORK_DIM*sizeof(size_t));
    memcpy_s(m_WorkInfo.LocalSize,    MAX_WORK_DIM*sizeof(size_t),   pWorkInfo->localWorkSize,    MAX_WORK_DIM*sizeof(size_t));
#else
    memcpy(m_WorkInfo.GlobalOffset, /*MAX_WORK_DIM*sizeof(size_t),*/ pWorkInfo->globalWorkOffset, MAX_WORK_DIM*sizeof(size_t));
    memcpy(m_WorkInfo.GlobalSize,   /*MAX_WORK_DIM*sizeof(size_t),*/ pWorkInfo->globalWorkSize,   MAX_WORK_DIM*sizeof(size_t));
    memcpy(m_WorkInfo.LocalSize,    /*MAX_WORK_DIM*sizeof(size_t),*/ pWorkInfo->localWorkSize,    MAX_WORK_DIM*sizeof(size_t));
#endif

    // In case of (m_WorkInfo.uiWorkDim < MAX_WORK_DIM)
    //  need to set local and global size to 1 for OOB dimensions
    for ( unsigned int i=m_WorkInfo.uiWorkDim; i<MAX_WORK_DIM; ++i ) {
      m_WorkInfo.GlobalSize[i] = 1;
      m_WorkInfo.LocalSize[i] = 1;
      // no need to set GlobalOffset as it is used later only according to dimension
    }

    // Calculate number of work groups and WG size
    m_uiWGSize = 1;
    for(unsigned int i=0; i<m_WorkInfo.uiWorkDim; ++i)
    {
        m_WorkInfo.WGNumber[i] = m_WorkInfo.GlobalSize[i]/m_WorkInfo.LocalSize[i];
        m_uiWGSize *= m_WorkInfo.LocalSize[i];
    }
}

cl_dev_err_code Binary::GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
                                                     size_t* INOUT pBufferCount ) const
{
    assert(pBufferCount);
    if ( (NULL == pBufferSizes) )
    {
        size_t buffCount = m_kernelLocalMem.size();
        buffCount += (m_totalImplSize != 0);    // Implicit local buffers
        ++buffCount;                            // Stack size
        *pBufferCount = buffCount;
        return CL_DEV_SUCCESS;
    }
    assert(pBufferSizes);

    // Fill sizes of explicit local buffers
    unsigned int i;
    for (i = 0; i < m_kernelLocalMem.size(); ++i)
    {
        pBufferSizes[i] = m_kernelLocalMem[i].getBufferSize();
    }
    // Fill size of the implicit local buffer
    if ( m_totalImplSize  != 0 )
    {
        pBufferSizes[i] = m_totalImplSize;
        ++i;
    }

    // Fill size of private area for all work-items
    // [WORK-AROUND] and also the area for kernel params and local WI ids
    pBufferSizes[i] = m_stAlignedKernelParamSize + m_stWIidsBufferSize +
      (m_stPrivateMemorySize * m_uiWGSize);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code Binary::CreateExecutable(void* IN *pMemoryBuffers, 
                                         size_t IN stBufferCount, 
                                         ICLDevBackendExecutable_* OUT *pExec)
{
    unsigned int uiWGSizeLocal = m_uiWGSize;
    assert(pExec);

    if ( m_bVectorized ) {
      uiWGSizeLocal = uiWGSizeLocal / m_uiVectorWidth;
    }
    Executable* pExecutable =  m_pBackendFactory->CreateExecutable(this); 

    // Initial the context to be start of the stack frame
    if ( NULL == pExecutable )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // The last buffer is the stack region
    cl_dev_err_code res = pExecutable->Init(pMemoryBuffers, pMemoryBuffers[stBufferCount-1], uiWGSizeLocal);
    *pExec = pExecutable;

    return res;
}



void Binary::Release()
{
    delete this;
}

}}}