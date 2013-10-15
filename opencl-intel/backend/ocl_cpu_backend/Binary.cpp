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

#include "Binary.h"
#include "KernelProperties.h"
#include "Kernel.h"    //TEMPORARY
#include "exceptions.h"
#include "Executable.h"
#include "TypeAlignment.h"
#include "ExplicitArgument.h"
#include "ExplicitGlobalMemArgument.h"

#include "ExplicitBlockLiteralArgument.h"
#include <assert.h>
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
               ICLDevBackendBufferPrinter* pPrinter,
               IDeviceCommandManager* pDeviceCommandManager,
               const IBlockToKernelMapper *pBlockToKernelMapper,
               const KernelProperties* pKernelProperties,
               const std::vector<cl_kernel_argument>& args,
               const cl_work_description_type* pWorkInfo,
               const IKernelJITContainer* pScalarJIT,
               const IKernelJITContainer* pVectorJIT,
               char* IN pArgsBuffer, 
               size_t IN ArgBuffSize):
     m_pBackendFactory(pBackendFactory),
     m_pPrinter(pPrinter),
     m_ExtendedExecutionContext(pDeviceCommandManager, pBlockToKernelMapper),
     m_stFormalParamSize(0),
     m_stKernelParamSize(0),
     m_stAlignedKernelParamSize(0),
     m_stPrivateMemorySize(pKernelProperties->GetPrivateMemorySize()),
     m_pLocalParams(NULL), 
     m_DAZ( pKernelProperties->GetDAZ()),
     m_cpuId( pKernelProperties->GetCpuId()),
     m_bJitCreateWIids(pKernelProperties->GetJitCreateWIids()),
     m_uiVectorWidth(1), 
     m_pUsedEntryPoint(0),
     m_uiSizeT(pKernelProperties->GetPointerSize()),
     m_pBlockLiteral(NULL)
{
    InitWorkInfo(pWorkInfo);
    
    m_pLocalParams = align_to(m_pLocalParamsBase, TypeAlignment::MAX_ALIGNMENT);
#ifdef _DEBUG
    memset(m_pLocalParams, 0x88, CPU_MAX_PARAMETER_SIZE);
#endif

    InitParams(args, pArgsBuffer);
    
    if( m_bJitCreateWIids )
    {
        // vectorized kernel is inlined 
        m_pUsedEntryPoint = pScalarJIT->GetJITCode();
        m_uiVectorWidth   = pScalarJIT->GetProps()->GetVectorSize();
    }
    else
    {
        // vectorized and scalar kernels could both be present
        if( NULL != pVectorJIT )
        {
            m_pUsedEntryPoint = pVectorJIT->GetJITCode();
            m_uiVectorWidth   = pVectorJIT->GetProps()->GetVectorSize();
        }
        else
        {
            m_pUsedEntryPoint = pScalarJIT->GetJITCode();
            m_uiVectorWidth   = pScalarJIT->GetProps()->GetVectorSize();
        }

        if(m_WorkInfo.LocalSize[0] % m_uiVectorWidth) 
        {
            // Disable vectorization for workgroup sizes that are not
            // a multiple of the vector width (Guy)
            m_pUsedEntryPoint = pScalarJIT->GetJITCode();
            m_uiVectorWidth   = 1;
        }
        assert( !( 1 != m_uiVectorWidth && 1 == m_uiWGSize) && "vectorized with WGsize = 1!" );
    }
    
    m_uiWGSize = m_uiWGSize / m_uiVectorWidth;
    m_stWIidsBufferSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN( 
                            m_uiWGSize * sizeof(size_t) * MAX_WI_DIM_POW_OF_2);

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

      switch (arg.type) {
      case CL_KRNL_ARG_PTR_GLOBAL:
      case CL_KRNL_ARG_PTR_CONST:
      case CL_KRNL_ARG_PTR_IMG_2D:
      case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
      case CL_KRNL_ARG_PTR_IMG_3D:
      case CL_KRNL_ARG_PTR_IMG_2D_ARR:
      case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
      case CL_KRNL_ARG_PTR_IMG_1D:
      case CL_KRNL_ARG_PTR_IMG_1D_ARR:
      case CL_KRNL_ARG_PTR_IMG_1D_BUF:
        pArg = std::auto_ptr<ExplicitGlobalMemArgument>(new ExplicitGlobalMemArgument(pArgValueDest, arg));
        break;
      case CL_KRNL_ARG_PTR_BLOCK_LITERAL: {
        assert(argIterator == args.begin() &&
               "Block literal is not 0th argument in kernel");
        // pArgValueSrc - offset value
        // offset value is offset in bytes from the beginning of pArgsBuffer
        // arguments buffer to memory location where BlockLiteral is stored
        // offset value is of unsigned(32bit) type
        const size_t offs = (size_t) * (unsigned *)(pArgValueSrc);
        // deserialize in source memory BlockLiteral.
        // Actually it updates BlockDesc ptr field in BlockLiteral
        BlockLiteral *pBL = BlockLiteral::DeserializeInBuffer(pArgsBuffer + offs);

        // clone BlockLiteral to this Binary object. assume ownership
        assert(m_pBlockLiteral == NULL && "m_pBlockLiteral should be NULL");
        m_pBlockLiteral = BlockLiteral::Clone(pBL);

        // create special type of argument for passing BlockLiteral
        pArg = std::auto_ptr<ExplicitBlockLiteralArgument>(
            new ExplicitBlockLiteralArgument(pArgValueDest, arg,
                                             m_pBlockLiteral));
        break;
      }
      default:
        switch (arg.type) {
        case CL_KRNL_ARG_PTR_LOCAL:
          // Local memory buffers are allocating on the JIT's stack. The value
          // we get here is not the pointer, but the buffer size. We handle
          // this parameter like other arguments passed by value
          // + 16 is for taking alignment padding into consideration
          m_kernelLocalMemSizes.push_back(*reinterpret_cast<size_t *>(pArgValueSrc) + 16);
        // Fall through
        case CL_KRNL_ARG_INT:
        case CL_KRNL_ARG_UINT:
        case CL_KRNL_ARG_FLOAT:
        case CL_KRNL_ARG_DOUBLE:
        case CL_KRNL_ARG_VECTOR:
        case CL_KRNL_ARG_VECTOR_BY_REF:
        case CL_KRNL_ARG_SAMPLER:
        case CL_KRNL_ARG_COMPOSITE:
          break;
        default:
          assert(false && "Unknown kind of argument");
        }
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

    unsigned int ptrSize = GetPointerSize();
    m_stKernelParamSize = m_stFormalParamSize +
                          3 * ptrSize + // OCL specific argument: WG Info, pWGID, pBaseGlobalID
                          ptrSize  + // pWI-ids[]
                          ptrSize  + // Pointer to IDevExecutable
                          ptrSize  + // iterCount
                          ptrSize;   // ExtendedExecutionContext

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
    // We only require one buffer allocation from the Runtime used for storing the
    // kernel arguments and the Barrier's local ID indeces
    // We also report the overall memory needed for local memory even though
    // we do not need the Runtime to allocate it, because the Runtime needs this
    // information for OpenCL queries and for the hueristics which computes
    // work-group size.
    assert(pBufferCount);
    if (!pBufferSizes) {
        // +1 for additional area for: kernel params + WI ids buffer + private memory [see below]
        *pBufferCount = m_kernelLocalMemSizes.size() + 1;
        return CL_DEV_SUCCESS;
    }
    // Fill sizes of explicit local buffers
    std::copy(m_kernelLocalMemSizes.begin(), m_kernelLocalMemSizes.end(), pBufferSizes);
    // Fill size of private area for all work-items
    // [WORK-AROUND] and also the area for kernel params and local WI ids
    pBufferSizes[m_kernelLocalMemSizes.size()] =
        m_stAlignedKernelParamSize + m_stWIidsBufferSize +
        m_stPrivateMemorySize * m_uiVectorWidth * m_uiWGSize;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code Binary::CreateExecutable(void* IN *pMemoryBuffers, 
                                         size_t IN stBufferCount, 
                                         ICLDevBackendExecutable_* OUT *pExec)
{
    assert(pExec);

    Executable* pExecutable =  m_pBackendFactory->CreateExecutable(this); 

    // Initial the context to be start of the stack frame
    if ( NULL == pExecutable )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // The last buffer is the stack region
    cl_dev_err_code res = pExecutable->Init(pMemoryBuffers, pMemoryBuffers[stBufferCount-1], m_uiWGSize);
    *pExec = pExecutable;

    return res;
}



void Binary::Release()
{
    delete this;
}

Binary::~Binary()
{
  if(m_pBlockLiteral)
    BlockLiteral::FreeMem(m_pBlockLiteral);
}
}}}
