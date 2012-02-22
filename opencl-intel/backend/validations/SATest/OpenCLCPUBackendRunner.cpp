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

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include "Buffer.h"
#include "mem_utils.h"

#define DEBUG_TYPE "OpenCLCPUBackendRunner"
#include <llvm/Support/raw_ostream.h>
// debug macros
#include <llvm/Support/Debug.h>

#include <string.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{

extern void GenINT3();

class OpenCLExecutionContext
{
public:
    OpenCLExecutionContext(const BERunOptions* config):
        m_stPrivMemAllocSize(2*CPU_DEV_MIN_WI_PRIVATE_SIZE),
        m_config(config)
    {
        auto_ptr_aligned pLocalMem  ( (char*)align_malloc(CPU_DEV_LCL_MEM_SIZE, CPU_DEV_MAXIMUM_ALIGN));
        auto_ptr_aligned pPrivateMem( (char*)align_malloc(m_stPrivMemAllocSize, CPU_DEV_MAXIMUM_ALIGN));

        m_pLocalMem   = pLocalMem;
        m_pPrivateMem = pPrivateMem;
    }

    void CreateContext(ICLDevBackendBinary_* pBinary, size_t* pBuffSizes, size_t count)
    {
        m_pContext.reset(NULL);

        void*   pBuffPtr[CPU_MAX_LOCAL_ARGS+2]; // Additional two for implicit and private

        // Allocate local memories
        char*   pCurrPtr = m_pLocalMem.get();
        // The last buffer is private memory (stack) size
        --count;
        for(size_t i=0;i<count;++i)
        {
            pBuffPtr[i] = pCurrPtr;
            pCurrPtr += pBuffSizes[i];
        }

        // Check allocated size of the private memory, and allocate new if necessary.
        if ( m_stPrivMemAllocSize < pBuffSizes[count] )
        {
            m_stPrivMemAllocSize = pBuffSizes[count];
            m_pPrivateMem.reset((char*)align_malloc(m_stPrivMemAllocSize, CPU_DEV_MAXIMUM_ALIGN));
        }

        pBuffPtr[count] = m_pPrivateMem.get();
        DEBUG(llvm::dbgs() << "Create executable started.\n");
        int rc = pBinary->CreateExecutable(pBuffPtr, count+1, m_pContext.getOutPtr());
        DEBUG(llvm::dbgs() << "Create executable finished.\n");
        if (CL_DEV_FAILED(rc))
        {
            throw Exception::TestRunnerException("Create executable failed\n");
        }
    }

    void ExecuteWorkGroup( size_t x, size_t y, size_t z)
    {
        size_t groupId[MAX_WORK_DIM] = {x, y, z};

        // In production sequence the Runtime calls Executable::PrepareThread()
        // and Executable::RestoreThreadState() respectively before and 
        // after executing a work group. 
        // These routines setup and restore the MXCSR register and zero the upper parts of YMMs. 
        int rc = m_pContext->PrepareThread();
        if (CL_DEV_FAILED(rc))
        {
            throw Exception::TestRunnerException("PrepareThread failed\n");
        }

        m_sample.Start();

        if( m_config->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false))
        {
            GenINT3();
        }
        DEBUG(llvm::dbgs() << "Starting execution of the " << x << ", " << y << ", " << z << " group.\n");

        cl_dev_err_code ret = m_pContext->Execute(groupId, NULL, NULL);
        if (CL_DEV_FAILED(ret))
        {
            throw Exception::TestRunnerException("Execution failed.\n");
        }

        DEBUG(llvm::dbgs() << "Finished execution of the " << x << ", " << y << ", " << z << " group.\n");
        if( m_config->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false))
        {
            GenINT3();
        }

        m_sample.Stop();

        rc = m_pContext->RestoreThreadState();
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

private:
    ICLDevBackendExecutablePtr  m_pContext;
    auto_ptr_aligned            m_pLocalMem;
    auto_ptr_aligned            m_pPrivateMem;
    size_t                      m_stPrivMemAllocSize;
    const BERunOptions*         m_config;
    Sample                      m_sample;
};

class OpenCLBinaryContext
{
public:
    OpenCLBinaryContext(const ICLDevBackendKernel_* pKernel,
                        const ICLDevBackendExecutionService* pExecutionService,
                        OpenCLKernelConfiguration * pKernelConfig,
                        const BERunOptions* pRunConfig,
                        uint8_t* pArgsBuffer,
                        size_t argsBufferSize)
    {
        assert(NULL != pArgsBuffer);
        assert(NULL != pKernel);
        assert(NULL != pKernelConfig);

        // get local work group sizes
        cl_work_description_type workInfo;

        std::copy(pKernelConfig->GetLocalWorkSize(),
                  pKernelConfig->GetLocalWorkSize() + MAX_WORK_DIM,
                  workInfo.localWorkSize);

        std::copy(pKernelConfig->GetGlobalWorkOffset(),
                  pKernelConfig->GetGlobalWorkOffset() + MAX_WORK_DIM,
                  workInfo.globalWorkOffset);

        std::copy(pKernelConfig->GetGlobalWorkSize(),
                  pKernelConfig->GetGlobalWorkSize() + MAX_WORK_DIM,
                  workInfo.globalWorkSize);

        m_dim = workInfo.workDimension = pKernelConfig->GetWorkDimension();

        // adjust the local work group sized in case we are running
        // in validation mode. Adjusting is mainly selecting the appropriate
        // local work group size if one was not selected by user. We need
        // to adjust to be able to use the same value as a reference runner.
        // In Performance mode no adjustment is needed and we let the Volcano
        // engine to select one
        if( !pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
        {
            for (size_t i = 0; i < m_dim; ++i)
            {
                if(workInfo.localWorkSize[i] == 0)
                {
                    workInfo.localWorkSize[i] =
                        std::min<uint32_t>( static_cast<uint32_t>(pKernelConfig->GetGlobalWorkSize()[i]),
                         pRunConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0));
                }
            }
        }

        // create the binary

        DEBUG(llvm::dbgs() << "Creating binary started.\n");
        cl_int ret = pExecutionService->CreateBinary(
                                           pKernel,
                                           pArgsBuffer,
                                           argsBufferSize,
                                           &workInfo,
                                           m_spBinary.getOutPtr());
        DEBUG(llvm::dbgs() << "Creating binary finished.\n");
        if ( CL_DEV_FAILED(ret) )
        {
            throw Exception::TestRunnerException("Create binary failed\n");
        }

        // init the memory buffer sizes
        m_spBinary->GetMemoryBuffersDescriptions(NULL, &m_BufferSizesCount);
        m_spBufferSizes.reset(new size_t[m_BufferSizesCount]);
        m_spBinary->GetMemoryBuffersDescriptions(m_spBufferSizes.get(), &m_BufferSizesCount);

        // init the work group regions
        for (size_t i=0; i < m_dim; ++i)
        {
            m_regions[i] = (size_t)( pKernelConfig->GetGlobalWorkSize()[i]/m_spBinary->GetWorkGroupSize()[i]);
        }
    }

    const size_t* GetWorkGroupSize() const
    {
        assert( m_spBinary.get() != NULL );
        return m_spBinary->GetWorkGroupSize();
    }

    const size_t* GetWorkGroupRegions() const
    {
        return m_regions;
    }

    size_t GetWorkDimension() const
    {
        return m_dim;
    }

    OpenCLExecutionContext* CreateExecutionContext(const BERunOptions* config)
    {
        std::auto_ptr<OpenCLExecutionContext> spContext( new OpenCLExecutionContext(config));
        spContext->CreateContext(m_spBinary.get(), m_spBufferSizes.get(), m_BufferSizesCount);
        return spContext.release();
    }

private:
    std::auto_ptr<size_t> m_spBufferSizes; //buffer size info
    size_t  m_BufferSizesCount;
    size_t  m_regions[MAX_WORK_DIM];
    size_t  m_dim;
    ICLDevBackendBinaryPtr m_spBinary;
};

OpenCLCPUBackendRunner::OpenCLCPUBackendRunner()
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
    const BERunOptions         *pOCLRunConfig     = static_cast<const BERunOptions *>(runConfig);
    const OpenCLProgram        *pOCLProgram       = static_cast<const OpenCLProgram *>(program);

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
    ICLDevBackendImageServicePtr        spImageService(NULL);

    DEBUG(llvm::dbgs() << "Get execution service started.\n");
    cl_dev_err_code ret = m_pServiceFactory->GetExecutionService(options.get(), spExecutionService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
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
    ret = m_pServiceFactory->GetImageService(NULL, spImageService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create image service failed\n");
    }
    DEBUG(llvm::dbgs() << "Get image service finished.\n");

    {
    //
    // Program need to be released before the compilation service - thus inner scope is necessary
    //
    ICLDevBackendProgramPtr spProgram(NULL);

    for( uint32_t i = 0; i < pOCLRunConfig->GetValue<uint32_t>(RC_BR_BUILD_ITERATIONS_COUNT, 1); ++i)
    {
        spProgram.reset( CreateProgram(pOCLProgram, spCompileService.get()) );
        PriorityBooster booster(!pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

        BuildProgram(spProgram.get(), spCompileService.get(), runResult, pOCLRunConfig);
    }

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "").empty() )
    {
        //currently dumping to the file is temporary unsupported
        const ICLDevBackendCodeContainer* pCodeContainer = spProgram->GetProgramCodeContainer();
        ProgramDumpConfig dumpOptions(pOCLRunConfig);
        spCompileService->DumpCodeContainer( pCodeContainer, &dumpOptions);
    }

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, "").empty())
    {
        spCompileService->DumpJITCodeContainer(spProgram->GetProgramCodeContainer(),
            pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, ""),
            pOCLProgramConfig->GetBaseDirectory());
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
            ExecuteKernel(input, runResult, spProgram.get(), spExecutionService.get(), spImageService.get(), *it, pOCLRunConfig);
        }
    }
    }
}

void OpenCLCPUBackendRunner::ExecuteKernel(IBufferContainerList& input,
                                        IRunResult * runResult,
                                        ICLDevBackendProgram_* pProgram,
                                        ICLDevBackendExecutionService* pExecutionService,
                                        ICLDevBackendImageService* pImageService,
                                        OpenCLKernelConfiguration * pKernelConfig,
                                        const BERunOptions* pRunConfig)
{
    assert( NULL != pProgram);
    assert(pImageService);

    // Get kernel to run
    std::string kernelName = pKernelConfig->GetKernelName();
    const ICLDevBackendKernel_* pKernel = NULL;
    pProgram->GetKernelByName(kernelName.c_str(), &pKernel);

    // Get kernel arguments
    int kernelNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();
    std::vector<bool> ignoreList;
    FillIgnoreList(ignoreList, pKernelArgs, kernelNumArgs);
    runResult->SetComparatorIgnoreList(kernelName.c_str(), ignoreList);

    // Create the argument buffer
    OpenCLArgsBuffer argsBuffer(pKernelArgs, kernelNumArgs, &input, pImageService);

    OpenCLBinaryContext binary( pKernel,
                                pExecutionService,
                                pKernelConfig,
                                pRunConfig,
                                argsBuffer.GetArgsBuffer(),
                                argsBuffer.GetArgsBufferSize());

    // we are using single execution context (single threaded)
    std::auto_ptr<OpenCLExecutionContext> spContext( binary.CreateExecutionContext( pRunConfig) );

    // Note:
    //     Think of refactoring this code for more elegant solution:
    //     Probably using the task executor model used in OCL SDK
    const size_t * dims = binary.GetWorkGroupRegions();
    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        spContext->ResetSampling();
    }

    if (pRunConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false))
    {
        spContext->ExecuteWorkGroup(0, 0, 0);
    }
    else
    {
        switch( binary.GetWorkDimension())
        {
        case 1:
            for( size_t x = 0; x < dims[0]; ++x)
            {
                spContext->ExecuteWorkGroup( x, 0, 0);
            }
            break;
        case 2:
            for(size_t y = 0; y < dims[1]; ++y)
            {
                for( size_t x = 0; x < dims[0]; ++x)
                {
                    spContext->ExecuteWorkGroup(x, y, 0);
                }
            }
            break;
        case 3:
            for(size_t z = 0; z < dims[2]; ++z)
            {
                for(size_t y = 0; y < dims[1]; ++y)
                {
                    for( size_t x = 0; x < dims[0]; ++x)
                    {
                        spContext->ExecuteWorkGroup(x, y, z);
                    }
                }
            }
            break;
        default:
            throw Exception::TestRunnerException("Wrong number of dimensions while running the kernel\n");
        }
    }

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)runResult->GetPerformance();
        perfResults.SetExecutionTime(kernelName, spContext->GetSampling());
    }
    else // Do not save output in PERF mode.
    {
        // Copy output into runResult output buffer container list
        IBufferContainerList& output = runResult->GetOutput(kernelName.c_str());
        argsBuffer.CopyOutput(output, &input);
    }
}

} // namespace


