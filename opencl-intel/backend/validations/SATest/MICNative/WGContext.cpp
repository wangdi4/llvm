/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  WGContext.cpp

\*****************************************************************************/

#include <stdio.h>

#include "WGContext.h"
#include "mic_dev_limits.h"

#include "IArgument.h"
#include "ExplicitGlobalMemArgument.h"
#include "ExplicitBlockLiteralArgument.h"


    void WGContext::GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
                                      size_t* INOUT pBufferCount )
    {
        // We only require one buffer allocation used for storing the
        // kernel arguments and the Barrier's local ID indeces (if any)
        // We also report the overall memory needed for local memory even though
        // we do not need to allocate it, because this info could be used
        // for the hueristics which computes
        // work-group size.
        assert(pBufferCount);
        if (!pBufferSizes) {
            // +1 for additional area for: kernel params + WI ids buffer + private memory [see below]
            *pBufferCount = m_kernelLocalMemSizes.size() + 1;
            return;
        }
        // Fill sizes of explicit local buffers
        std::copy(m_kernelLocalMemSizes.begin(), m_kernelLocalMemSizes.end(), pBufferSizes);
        // Fill size of private area for all work-items
        // and also the area for kernel params and local WI ids
        pBufferSizes[m_kernelLocalMemSizes.size()] =
            m_stAlignedKernelParamSize +
            m_stPrivateMemorySize * m_uiVectorWidth * m_uiWGSize;
    }

    void WGContext::InitParams(const ICLDevBackendKernel_* pKernel, char* pArgsBuffer, cl_work_description_type workInfo)
    {
        char* pArgValueDest = m_pArgumentBuffer.get();
        
        const int kernelParamCnt = pKernel->GetKernelParamsCount();
        for (int i = 0; i < kernelParamCnt; i++) {
            const cl_kernel_argument arg = pKernel->GetKernelParams()[i];

            Validation::auto_ptr_ex<IArgument> pArg;

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
                    pArg.reset(new ExplicitGlobalMemArgument(pArgValueDest, arg));
                break;
                default:
                switch (arg.type) {
                    case CL_KRNL_ARG_PTR_LOCAL:
                        // Local memory buffers are allocating on the JIT's stack. The value
                        // we get here is not the pointer, but the buffer size. We handle
                        // this parameter like other arguments passed by value
                        // + 16 is for taking alignment padding into consideration
                        m_kernelLocalMemSizes.push_back(*reinterpret_cast<size_t *>(pArgsBuffer) + 16);

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
                pArg.reset(new ExplicitArgument(pArgValueDest, arg));
            }

            pArg->setValue(pArgsBuffer);
          
            // Advance the src buffer according to argument's size (the sec buffer is packed)
            pArgsBuffer += pArg->getSize();
            // Advance the dest buffer according to argument's size and alignment
            pArgValueDest += pArg->getAlignedSize();
        }

        cl_uniform_kernel_args *pKernelArgs = (cl_uniform_kernel_args *) (m_pArgumentBuffer.get() + pKernel->GetExplicitArgumentBufferSize());

        size_t sizetMaxWorkDim = sizeof(size_t)*MAX_WORK_DIM;

        memcpy(pKernelArgs->GlobalOffset, workInfo.globalWorkOffset, sizetMaxWorkDim); // Filled by the runtime
        
        memcpy(pKernelArgs->GlobalSize, workInfo.globalWorkSize, sizetMaxWorkDim); // Filled by the runtime

        memcpy(pKernelArgs->LocalSize, workInfo.localWorkSize, sizetMaxWorkDim); // Filled by the runtime, updated by the BE in case of (0,0,0)

        pKernelArgs->minWorkGroupNum = size_t(workInfo.minWorkGroupNum); // Filled by the runtime, Required by the heuristic

        pKernelArgs->pUniformJITEntryPoint    = NULL;// Filled by the BE
        pKernelArgs->pNonUniformJITEntryPoint = NULL;// Filled by the BE

        memset(pKernelArgs->WGCount,0,sizetMaxWorkDim); // Updated by the BE, based on GLOBAL/LOCAL

        pKernelArgs->WorkDim = workInfo.workDimension; // Filled by the runtime

        cl_dev_err_code rc = m_pKernelRunner->PrepareKernelArguments((void*)(m_pArgumentBuffer.get()), 0, 0);
        if ( CL_DEV_FAILED(rc) )
        {
            printf("PrepareKernelArguments failed\n"); fflush(0);
        }

        //local group size calculated by PrepareKernelArguments (using heuristic)
        memcpy(m_LocalSize, pKernelArgs->LocalSize, sizetMaxWorkDim);
        m_pKernelRunner->InitRunner((void*)(m_pArgumentBuffer.get()));

        m_uiVectorWidth = pKernel->GetKernelProporties()->GetMinGroupSizeFactorial();

        m_uiWGSize = 1;
        for(unsigned int i=0; i< pKernelArgs->WorkDim ; ++i)  {
            m_uiWGSize *= pKernelArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i];
        }
        m_uiWGSize = m_uiWGSize / m_uiVectorWidth;

        m_stPrivateMemorySize = pKernel->GetKernelProporties()->GetPrivateMemorySize();

        unsigned int ptrSize = (sizeof(void*));

        m_stKernelParamSize = pArgValueDest - m_pArgumentBuffer.get() + // Calculate parameter sizes
                              3 * ptrSize + // OCL specific argument: WG Info, pWGID, pBaseGlobalID
                              ptrSize  + // pWI-ids[]
                              ptrSize  + // Pointer to IDevExecutable
                              ptrSize  + // iterCount
                              ptrSize;   // ExtendedExecutionContext

        m_stAlignedKernelParamSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(m_stKernelParamSize);
    }


cl_dev_err_code WGContext::PrepareThread()
{
    cl_dev_err_code rc = m_pKernelRunner->PrepareThreadState(m_tExecState);
    if (CL_DEV_FAILED( rc ))
    {
        printf("PrepareThread failed\n"); fflush(0);
    }
    return rc;
}

cl_dev_err_code WGContext::RestoreThreadState()
{
    cl_dev_err_code rc = CL_DEV_SUCCESS;
    rc = m_pKernelRunner->RestoreThreadState(m_tExecState);
    if (CL_DEV_FAILED( rc ))
    {
        printf("RestoreThreadState failed\n"); fflush(0);
    }
    return rc;
}

cl_dev_err_code WGContext::Execute(const size_t *pGroupID)
{
    cl_dev_err_code rc = m_pKernelRunner->RunGroup((const void*)(m_pArgumentBuffer.get()),pGroupID,m_pPrivateMem.get());
    if (CL_DEV_FAILED( rc ))
    {
        printf("Execute failed\n"); fflush(0);
    }
    return rc;
}


WGContext::WGContext(const ICLDevBackendKernel_* pKernel,
                     cl_work_description_type* workInfo, void* pArgsBuffer, size_t argsBufferSize):
    m_stPrivMemAllocSize(MIC_DEFAULT_WG_SIZE*MIC_DEV_MIN_WI_PRIVATE_SIZE),
    m_stAlignedKernelParamSize(0),
    m_stKernelParamSize(0),
    m_uiVectorWidth(1)
{
    // Create private memory
    m_pPrivateMem.reset( (char*)Validation::align_malloc(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN));
    m_tExecState.MXCSRstate = 0;
    m_pKernelRunner = pKernel->GetKernelRunner();
    // allocate buffer for kernel's arguments
    size_t argSize = pKernel->GetExplicitArgumentBufferSize();
    
    m_pArgumentBuffer.reset( (char*)Validation::align_malloc(argSize + sizeof(cl_uniform_kernel_args), MIC_DEV_MAXIMUM_ALIGN));

    InitParams(pKernel,(char*)pArgsBuffer, *workInfo);

    Validation::auto_ptr_ex< size_t, Validation::ArrayDP<size_t> > spBufferSizes;
    size_t bufferSizesCount = 0;
    GetMemoryBuffersDescriptions(NULL, &bufferSizesCount);
    spBufferSizes.reset(new size_t[bufferSizesCount]);
    GetMemoryBuffersDescriptions(spBufferSizes.get(), &bufferSizesCount);

    size_t* pBuffSizes = spBufferSizes.get();

    // The last buffer is private memory (stack) size
    size_t count = bufferSizesCount - 1;

    // Check allocated size of the private memory, and allocate new if nessesary.
    if ( m_stPrivMemAllocSize < pBuffSizes[count] )
    {
        m_stPrivMemAllocSize = pBuffSizes[count];
        m_pPrivateMem.reset((char*)Validation::align_malloc(m_stPrivMemAllocSize, MIC_DEV_MAXIMUM_ALIGN));
    }
}

WGContext::~WGContext() {}
