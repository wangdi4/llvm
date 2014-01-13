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

File Name:  OpenCLCPUBackendRunner.cpp

\*****************************************************************************/
#include "BackendOptions.h"
#include "OpenCLCPUBackendRunner.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLArgsBuffer.h"
#include "OpenCLBackendWrapper.h"

#include "SATestException.h"
#include "Performance.h"

#include "cpu_dev_limits.h"
#include "XMLDataWriter.h"
#include "XMLDataReader.h"
#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "BufferContainerList.h"

#include "DataVersion.h"

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include "Buffer.h"
#include "mem_utils.h"


#include "cpu_dev_limits.h"
#include "ExplicitArgument.h"
#include "ExplicitGlobalMemArgument.h"
#include "BlockLiteral.h"
#include "ExplicitBlockLiteralArgument.h"

#define DEBUG_TYPE "OpenCLCPUBackendRunner"
#include <llvm/Support/raw_ostream.h>
// debug macros
#include <llvm/Support/Debug.h>

#include <string.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{

namespace Utils
{
    std::string GetDataFilePath(const std::string& fileName, const std::string& baseDirectory);
}

extern void GenINT3();


class OpenCLExecutionContext
{
public:
    OpenCLExecutionContext(const ICLDevBackendKernel_* pKernel, const BERunOptions* config, 
                           cl_work_description_type workInfo, uint8_t* pArgsBuffer, size_t argsBufferSize):

        m_stPrivMemAllocSize(2*CPU_DEV_MIN_WI_PRIVATE_SIZE),
        m_uiVectorWidth(1),     // vector size that is actually used, to be updated from kernel arguments
        m_stFormalParamSize(0), // formal parameters size
        m_stKernelParamSize(0), // m_stFormalParamSize + OCL specific argument: WG Inf and so on
        m_stAlignedKernelParamSize(0), // m_stKernelParamSize + aligment
        m_pBlockLiteral(NULL)   // pointer to BlockLiteral
    {
        // allocate buffer for private memory
        auto_ptr_aligned pPrivateMem( (char*)align_malloc(m_stPrivMemAllocSize, CPU_DEV_MAXIMUM_ALIGN));        
        m_pPrivateMem = pPrivateMem;

        // allocate buffer for kernel's arguments
        size_t size = pKernel->GetExplicitArgumentBufferSize();
        auto_ptr_aligned pArgumentBuffer  ( (char*)align_malloc(size + sizeof(cl_uniform_kernel_args), CPU_DEV_MAXIMUM_ALIGN));
        m_pArgumentBuffer = pArgumentBuffer;        
        
        // init kernel parameters
        InitParams(pKernel,(char*)pArgsBuffer, workInfo);

        // init the memory buffer sizes
        auto_ptr_ex< size_t, ArrayDP<size_t> > spBufferSizes;
        size_t bufferSizesCount = 0;
        GetMemoryBuffersDescriptions(NULL, &bufferSizesCount);
        spBufferSizes.reset(new size_t[bufferSizesCount]);
        GetMemoryBuffersDescriptions(spBufferSizes.get(), &bufferSizesCount);

        size_t* pBuffSizes = spBufferSizes.get();

        // The last buffer is private memory (stack) size
        size_t count = bufferSizesCount - 1;

        // Check allocated size of the private memory, and allocate new if necessary.
        if ( m_stPrivMemAllocSize < pBuffSizes[count] )
        {
            m_stPrivMemAllocSize = pBuffSizes[count];
            m_pPrivateMem.reset((char*)align_malloc(m_stPrivMemAllocSize, CPU_DEV_MAXIMUM_ALIGN));
        }

        m_tExecState.MXCSRstate = 0;
        m_pBuffPtr = m_pPrivateMem.get();
    }

    ~OpenCLExecutionContext() {
        if(m_pBlockLiteral)
            BlockLiteral::FreeMem(m_pBlockLiteral);
    }

    void InitParams(const ICLDevBackendKernel_* pKernel, char* pArgsBuffer, cl_work_description_type workInfo)
    {
        char* pArgValueDest = m_pArgumentBuffer.get();
        
        const int kernelParamCnt = pKernel->GetKernelParamsCount();

        for (int i = 0; i < kernelParamCnt; i++) {
            const cl_kernel_argument arg = pKernel->GetKernelParams()[i];

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
                    pArg.reset(new ExplicitGlobalMemArgument(pArgValueDest, arg));
                break;
                case CL_KRNL_ARG_PTR_BLOCK_LITERAL: {

                    assert(i == 0 && "Block literal is not 0th argument in kernel");
                    // pArgValueSrc - offset value
                    // offset value is offset in bytes from the beginning of pArgsBuffer
                    // arguments buffer to memory location where BlockLiteral is stored
                    // offset value is of unsigned(32bit) type
                    
                    const size_t offs = (size_t) * (unsigned *)(pArgsBuffer);
                    // deserialize in source memory BlockLiteral.
                    // Actually it updates BlockDesc ptr field in BlockLiteral
                    BlockLiteral *pBL = BlockLiteral::DeserializeInBuffer(pArgsBuffer + offs);
                    
                    // clone BlockLiteral to this Binary object. assume ownership
                    assert(m_pBlockLiteral == NULL && "m_pBlockLiteral should be NULL");
                    m_pBlockLiteral = BlockLiteral::Clone(pBL);
                    
                    // create special type of argument for passing BlockLiteral
                    pArg.reset(new ExplicitBlockLiteralArgument(pArgValueDest, arg, m_pBlockLiteral));                    
                    break;
                }
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

        pKernelArgs->pJITEntryPoint = NULL;// Filled by the BE

        pKernelArgs->VectorWidth = 0;// Filled by the BE

        memset(pKernelArgs->WGCount,0,sizetMaxWorkDim); // Updated by the BE, based on GLOBAL/LOCAL

        pKernelArgs->WorkDim = workInfo.workDimension; // Filled by the runtime

        m_pKernelRunner = pKernel->GetKernelRunner();

        m_pKernelRunner->PrepareKernelArguments((void*)(m_pArgumentBuffer.get()), 0, 0);

        //local group size calculated by PrepareKernelArguments (using heuristic)
        memcpy(m_LocalSize, pKernelArgs->LocalSize, sizetMaxWorkDim);

        m_uiVectorWidth = pKernelArgs->VectorWidth;

        m_uiWGSize = 1;
        for(unsigned int i=0; i< pKernelArgs->WorkDim ; ++i)  {
            m_uiWGSize *= pKernelArgs->LocalSize[i];
        }
        m_uiWGSize = m_uiWGSize / m_uiVectorWidth;
        m_stWIidsBufferSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(m_uiWGSize * sizeof(size_t) * MAX_WI_DIM_POW_OF_2);

        m_stPrivateMemorySize = pKernel->GetKernelProporties()->GetPrivateMemorySize();

        // Calculate parameter sizes
        m_stFormalParamSize = pArgValueDest - m_pArgumentBuffer.get();

        unsigned int ptrSize = (sizeof(void*));
        m_stKernelParamSize = m_stFormalParamSize +
                              3 * ptrSize + // OCL specific argument: WG Info, pWGID, pBaseGlobalID
                              ptrSize  + // pWI-ids[]
                              ptrSize  + // Pointer to IDevExecutable
                              ptrSize  + // iterCount
                              ptrSize;   // ExtendedExecutionContext

        m_stAlignedKernelParamSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(m_stKernelParamSize);
    }

    // Returns the actual number of Work Items handled by each executable instance
    const size_t* GetWorkGroupSize() const
    {
        return m_LocalSize;
    }

    unsigned int GetVectorSize()
    {
        return m_uiVectorWidth;
    }

    void ExecuteWorkGroup( size_t x, size_t y, size_t z)
    {

        size_t groupId[MAX_WORK_DIM] = {x, y, z};

        cl_dev_err_code rc = m_pKernelRunner->PrepareThreadState(m_tExecState);
        if (CL_DEV_FAILED(rc))
        {
            throw Exception::TestRunnerException("PrepareThread failed\n");
        }

        m_sample.Start();

        DEBUG(llvm::dbgs() << "Starting execution of the " << x << ", " << y << ", " << z << " group.\n");

        cl_dev_err_code ret = m_pKernelRunner->RunGroup((const void*)(m_pArgumentBuffer.get()),groupId,m_pBuffPtr);

        if (CL_DEV_FAILED(ret))
        {
            throw Exception::TestRunnerException("Execution failed.\n");
        }

        DEBUG(llvm::dbgs() << "Finished execution of the " << x << ", " << y << ", " << z << " group.\n");

        m_sample.Stop();

        rc = m_pKernelRunner->RestoreThreadState(m_tExecState);
        if (CL_DEV_FAILED(rc))
        {
            throw Exception::TestRunnerException("RestoreThreadState failed\n");
        }

    }

    void ResetSampling()
    {
        m_sample = Sample();
    }

    Sample GetSampling()
    {
        return m_sample;
    }



    void GetMemoryBuffersDescriptions(size_t* IN pBufferSizes, 
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
            return;
        }
        // Fill sizes of explicit local buffers
        std::copy(m_kernelLocalMemSizes.begin(), m_kernelLocalMemSizes.end(), pBufferSizes);
        // Fill size of private area for all work-items
        // [WORK-AROUND] and also the area for kernel params and local WI ids
        pBufferSizes[m_kernelLocalMemSizes.size()] =
            m_stAlignedKernelParamSize + m_stWIidsBufferSize +
            m_stPrivateMemorySize * m_uiVectorWidth * m_uiWGSize;
    }

private:
    const ICLDevBackendKernelRunner * m_pKernelRunner;
    ICLDevBackendKernelRunner::ICLDevExecutionState m_tExecState;
    void*                   m_pBuffPtr;

    auto_ptr_aligned        m_pPrivateMem;
    auto_ptr_aligned        m_pArgumentBuffer;
    size_t                  m_stPrivMemAllocSize;

    Sample                  m_sample;
    unsigned int            m_uiVectorWidth; // vector size that was actually used

    size_t                  m_stFormalParamSize;
    size_t                  m_stKernelParamSize;
    size_t                  m_stAlignedKernelParamSize;
    unsigned int            m_uiWGSize;
    size_t                  m_stPrivateMemorySize;
    size_t                  m_stWIidsBufferSize;
    std::vector<size_t>     m_kernelLocalMemSizes;
    BlockLiteral *          m_pBlockLiteral;

    //work group size
    size_t                  m_LocalSize[MAX_WORK_DIM];
};


OpenCLCPUBackendRunner::OpenCLCPUBackendRunner(const BERunOptions& runConfig):
    OpenCLBackendRunner(runConfig)
{
}

OpenCLCPUBackendRunner::~OpenCLCPUBackendRunner()
{
}

void OpenCLCPUBackendRunner::Run(IRunResult* runResult,
                              const IProgram* program,
                              const IProgramConfiguration* programConfig,
                              const IRunComponentConfiguration* runConfig)
{
    assert((program != NULL) && "Program is not initialized\n");
    assert((programConfig != NULL) && "Program Configuration is not initialized\n");
    assert((runConfig != NULL) && "Run Configuration is not initialized\n");
    assert((runResult != NULL) && "Run Result is not initialized\n");

    const OpenCLProgramConfiguration *pOCLProgramConfig = static_cast<const OpenCLProgramConfiguration *>(programConfig);
    const BERunOptions  *pOCLRunConfig = static_cast<const BERunOptions *>(runConfig);
    const OpenCLProgram *pOCLProgram   = static_cast<const OpenCLProgram *>(program);

    std::auto_ptr<CPUBackendOptions> options;
    if( pOCLRunConfig->GetValue<bool>(RC_BR_USE_SDE, false))
    {
        options.reset(new SDEBackendOptions());
    }
    else
    {
        options.reset(new CPUBackendOptions());
    }

    options->InitFromRunConfiguration(*pOCLRunConfig);

    ICLDevBackendExecutionServicePtr spExecutionService(NULL);
    ICLDevBackendCompileServicePtr   spCompileService(NULL);
    ICLDevBackendImageServicePtr     spImageService(NULL);

    DEBUG(llvm::dbgs() << "Get execution service started.\n");
    cl_dev_err_code ret = m_pServiceFactory->GetExecutionService(options.get(), spExecutionService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        if ( CL_DEV_NOT_SUPPORTED == ret )
            throw Exception::TestRunnerException("Can't create execution service for current device. Try build back-end with SDE support.\n");
        throw Exception::TestRunnerException("Create execution service failed\n");
    }
    DEBUG(llvm::dbgs() << "Get execution service finished.\n");

    options->InitTargetDescriptionSession(spExecutionService.get());

    DEBUG(llvm::dbgs() << "Get compilation service started.\n");
    ret = m_pServiceFactory->GetCompilationService(options.get(), spCompileService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        if (ret == CL_DEV_INVALID_OPERATION_MODE)
        {
            throw Exception::TestRunnerException("Invalid CPU architecture was set.", VALIDATION_INVALID_OPERATION_MODE);
        }
        throw Exception::TestRunnerException("Create compilation service failed\n");
    }
    DEBUG(llvm::dbgs() << "Get compilation service finished.\n");

    DEBUG(llvm::dbgs() << "Get image service started.\n");
    ret = m_pServiceFactory->GetImageService(options.get(), spImageService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create image service failed\n");
    }
    DEBUG(llvm::dbgs() << "Get image service finished.\n");

    {
    //
    // Program need to be released before the compilation service - thus inner scope is necessary
    //
    ProgramHolder programHolder( spCompileService.get() );

    for( uint32_t i = 0; i < pOCLRunConfig->GetValue<uint32_t>(RC_BR_BUILD_ITERATIONS_COUNT, 1); ++i)
    {
        programHolder.setProgram( CreateProgram(pOCLProgram, spCompileService.get()) );
        PriorityBooster booster(!pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

        BuildProgram(programHolder.getProgram(), spCompileService.get(), runResult, pOCLRunConfig, pOCLProgramConfig);
    }

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "").empty() )
    {
        //currently dumping to the file is temporary unsupported
        const ICLDevBackendCodeContainer* pCodeContainer = programHolder.getProgram()->GetProgramCodeContainer();
        ProgramDumpConfig dumpOptions(pOCLRunConfig);
        spCompileService->DumpCodeContainer( pCodeContainer, &dumpOptions);
    }

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, "").empty())
    {
        std::string filename = Utils::GetDataFilePath( pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, ""),
                                                       pOCLProgramConfig->GetBaseDirectory());

        spCompileService->DumpJITCodeContainer(programHolder.getProgram()->GetProgramCodeContainer(),
            filename);
    }

    if (pOCLRunConfig->GetValue<bool>(RC_BR_BUILD_ONLY, false))
    {
        return;
    }

    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pOCLProgramConfig->beginKernels();
        it != pOCLProgramConfig->endKernels();
        ++it )
    {
        BufferContainerList input;
        LoadInputBuffer(*it, &input);

        PriorityBooster booster(!pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));
        for( uint32_t i = 0; i < pOCLRunConfig->GetValue<uint32_t>(RC_BR_EXECUTE_ITERATIONS_COUNT, 1); ++i)
        {
            ExecuteKernel(input, runResult, programHolder.getProgram(), spImageService.get(), *it, pOCLRunConfig);
        }
    }
    }// ProgramHolder scope end
}


static void initWorkInfo(cl_work_description_type* workInfo, const OpenCLKernelConfiguration * pKernelConfig, const BERunOptions* pRunConfig){

    std::copy(pKernelConfig->GetLocalWorkSize(),
              pKernelConfig->GetLocalWorkSize() + MAX_WORK_DIM,
              workInfo->localWorkSize);

    std::copy(pKernelConfig->GetGlobalWorkOffset(),
              pKernelConfig->GetGlobalWorkOffset() + MAX_WORK_DIM,
              workInfo->globalWorkOffset);

    std::copy(pKernelConfig->GetGlobalWorkSize(),
              pKernelConfig->GetGlobalWorkSize() + MAX_WORK_DIM,
              workInfo->globalWorkSize);

    //TODO: this number should be similar to how the runtime set it,
    //      i.e. number-of-working-threads
    workInfo->minWorkGroupNum = 1; //Intel::OpenCL::Utils::GetNumberOfProcessors();
    workInfo->workDimension = pKernelConfig->GetWorkDimension();
    // adjust the local work group sized in case we are running
    // in validation mode. Adjusting is mainly selecting the appropriate
    // local work group size if one was not selected by user. We need
    // to adjust to be able to use the same value as a reference runner.
    // In Performance mode no adjustment is needed and we let the Volcano
    // engine to select one
    if( !pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        for (size_t i = 0; i < workInfo->workDimension; ++i)
        {
            if(workInfo->localWorkSize[i] == 0)
            {
                workInfo->localWorkSize[i] =
                    std::min<uint32_t>( static_cast<uint32_t>(pKernelConfig->GetGlobalWorkSize()[i]),
                     pRunConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0));
                // the values specified in globalWorkSize[0], ..,
                // globalWorkSize[work_dim - 1] must be evenly divisible by
                // the corresponding values specified in
                // localWorkSize[0], .., localWorkSize[work_dim - 1]
                if (static_cast<uint32_t>(pKernelConfig->GetGlobalWorkSize()[i]) % workInfo->localWorkSize[i] != 0)
                {
                    workInfo->localWorkSize[i] = 1;
                }
            }
        }
    }
}

void OpenCLCPUBackendRunner::ExecuteKernel(IBufferContainerList& input,
                                        IRunResult * runResult,
                                        ICLDevBackendProgram_* pProgram,
                                        ICLDevBackendImageService* pImageService,
                                        OpenCLKernelConfiguration * pKernelConfig,
                                        const BERunOptions* pRunConfig)
{

    assert( NULL != pProgram);
    assert(pImageService);

    // Get kernel to run
    std::string kernelName = pKernelConfig->GetKernelName();
    const ICLDevBackendKernel_* pKernel = NULL;

    cl_dev_err_code errCode = pProgram->GetKernelByName(kernelName.c_str(), &pKernel);

    // currently pProgram->GetKernelByName returns CL_DEV_INVALID_KERNEL_NAME or CL_DEV_SUCCESS only
    switch(errCode) {
    case CL_DEV_SUCCESS:
        break;
    case CL_DEV_INVALID_KERNEL_NAME:
        throw Exception::TestRunnerException(std::string("kernel name ") + kernelName + std::string(" was not found in the source code\n"));
        break;
    default:
        throw Exception::TestRunnerException("Unexpected error code from ICLDevBackendProgram_::GetKernelByName method");
    }

    // Get kernel arguments
    int kernelNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();
    std::vector<bool> ignoreList;
    FillIgnoreList(ignoreList, pKernelArgs, kernelNumArgs);
    runResult->SetComparatorIgnoreList(kernelName.c_str(), ignoreList);

    DataVersion::ConvertData(&input, m_pModule->getNamedMetadata("opencl.kernels"), kernelName);

    // Create the argument buffer
    OpenCLArgsBuffer argsBuffer(pKernelArgs, kernelNumArgs, &input, pImageService,
            !pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

    cl_work_description_type workInfo;
    
    initWorkInfo(&workInfo, pKernelConfig, pRunConfig);

    OpenCLExecutionContext spContext( pKernel, pRunConfig, workInfo, argsBuffer.GetArgsBuffer(), argsBuffer.GetArgsBufferSize());

    size_t  regions[MAX_WORK_DIM];
    size_t dim = workInfo.workDimension;
    // init the work group regions
    for (size_t i=0; i < dim; ++i)
    {
        regions[i] = (size_t)( pKernelConfig->GetGlobalWorkSize()[i] / spContext.GetWorkGroupSize()[i]);
        //TODO: for non-uniform WG size it may not be true
        assert( pKernelConfig->GetGlobalWorkSize()[i] % spContext.GetWorkGroupSize()[i] == 0  && "Global work size is not multiple of work group size" );
    }

    // Note:
    //     Think of refactoring this code for more elegant solution:
    //     Probably using the task executor model used in OCL SDK

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        spContext.ResetSampling();
    }

    if( pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false))
    {
        GenINT3();
    }

    if (pRunConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false))
    {
        spContext.ExecuteWorkGroup(0, 0, 0);
    }
    else
    {
        switch( dim )
        {
        case 1:
            for( size_t x = 0; x < regions[0]; ++x)
            {
                spContext.ExecuteWorkGroup( x, 0, 0);
            }
            break;
        case 2:
            for(size_t y = 0; y < regions[1]; ++y)
            {
                for( size_t x = 0; x < regions[0]; ++x)
                {
                    spContext.ExecuteWorkGroup(x, y, 0);
                }
            }
            break;
        case 3:
            for(size_t z = 0; z < regions[2]; ++z)
            {
                for(size_t y = 0; y < regions[1]; ++y)
                {
                    for( size_t x = 0; x < regions[0]; ++x)
                    {
                        spContext.ExecuteWorkGroup(x, y, z);
                    }
                }
            }
            break;
        default:
            throw Exception::TestRunnerException("Wrong number of dimensions while running the kernel\n");
        }
    }

    if( pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false))
    {
        GenINT3();
    }

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)runResult->GetPerformance();
        perfResults.SetExecutionTime(kernelName, spContext.GetVectorSize(), spContext.GetSampling());
    }
    else // Do not save output in PERF mode.
    {
        // Copy output into runResult output buffer container list
        IBufferContainerList& output = runResult->GetOutput(kernelName.c_str());

        argsBuffer.CopyOutput(output, &input);
    }

}

} // namespace


