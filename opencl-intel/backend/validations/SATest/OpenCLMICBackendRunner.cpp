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

File Name:  OpenCLMICBackendRunner.cpp

\*****************************************************************************/
#include "OpenCLBackendWrapper.h"
#include "OpenCLMICBackendRunner.h"
#include "ICLDevBackendOptions.h"
#include "SATestException.h"
#include "Performance.h"
#include "BinaryDataReader.h"
#include "XMLDataReader.h"
#include "OpenCLRunConfiguration.h"
#include "BackendOptions.h"

#include "BufferContainerList.h"
#include "OpenCLMICArgsBuffer.h"
#include "RunResult.h"
#include "CPUDetect.h"

#include <iostream>
#include <assert.h>
#include <string>
#include <limits.h>

#define DEBUG_TYPE "OpenCLMICBackendRunner"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;

#define CHECK_BACKEND_RESULT(_BEFUNC)                                           \
    {                                                                           \
    cl_dev_err_code result = _BEFUNC;                                           \
    if (CL_DEV_FAILED(result))                                                  \
    {                                                                           \
        throw Exception::BackendException(std::string(#_BEFUNC" failed"));      \
    }                                                                           \
}

namespace Validation {

// Device side functions used as entry points
// WARINING! This array must be in line with enum DEVICE_SIDE_FUNCTION declared in OpenCLMICBackendRunner.h.
const char* OpenCLMICBackendRunner::m_device_function_names[DEVICE_SIDE_FUNCTION_COUNT] =
{
    "initDevice",                       // INIT_DEVICE
    "getBackendTargetDescriptionSize",  // GET_BACKEND_TARGET_DESCRIPTION_SIZE
    "getBackendTargetDescription",      // GET_BACKEND_TARGET_DESCRIPTION
    "executeKernels"                    // EXECUTE_KERNELS
};

void OpenCLMICBackendRunner::Run(IRunResult* runResult,
                                 const IProgram* program,
                                 const IProgramConfiguration* programConfig,
                                 const IRunComponentConfiguration* runConfig )
{
    assert((program != NULL) && "Program is not initialized");
    assert((programConfig != NULL) && "Program Configuration is not initialized");
    assert((runConfig != NULL) && "Run Configuration is not initialized");
    assert((runResult != NULL) && "Run Result is not initialized");

    const BERunOptions         *pOCLRunConfig     = static_cast<const BERunOptions *>(runConfig);
    const OpenCLProgram        *pOCLProgram       = static_cast<const OpenCLProgram *>(program);

    MICBackendOptions options;
    options.InitFromRunConfiguration(*pOCLRunConfig);

    options.InitTargetDescriptionSession(m_targetDescSize, m_pTargetDesc);
    ICLDevBackendCompileServicePtr   spCompileService(NULL);

    cl_dev_err_code ret = m_pServiceFactory->GetCompilationService(&options, spCompileService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create compilation service failed");
    }

    {

    //
    // Program need to be released before the compilation service - thus inner scope is necessary
    //

    /////////////// Build program ////////////////

    ICLDevBackendProgramPtr spProgram(NULL);

    for( uint32_t i = 0; i < pOCLRunConfig->GetValue<uint32_t>(RC_BR_BUILD_ITERATIONS_COUNT, 1); ++i)
    {
        spProgram.reset( CreateProgram(pOCLProgram, spCompileService.get()) );
        PriorityBooster booster(!pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

        BuildProgram(spProgram.get(), spCompileService.get(), runResult, pOCLRunConfig);
    }

    /////////////// Dump optimized LLVM IR if required ////////////////

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "").empty() &&
        !pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false))
    {
        //currently dumping to the file is temporary unsupported
        const ICLDevBackendCodeContainer* pCodeContainer = spProgram->GetProgramCodeContainer();
        spCompileService->DumpCodeContainer( pCodeContainer, &options);
    }

    if (pOCLRunConfig->GetValue(RC_BR_BUILD_ONLY, false))
    {
        return;
    }

    /////////////// Execute program ////////////////

    // We need set of COI Buffers for
    // 1. Test program
    // 2. Configurations: program configurations and run configurations
    // 3. Kernel arguments

    const OpenCLProgramConfiguration *pOCLProgramConfig = static_cast<const OpenCLProgramConfiguration *>(programConfig);

    ICLDevBackendSerializationServicePtr spSerializationService(NULL);
    ret = m_pServiceFactory->GetSerializationService(&options, spSerializationService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create serialization service failed");
    }

    SerializeProgram(spSerializationService.get(), spProgram.get());

    RunKernels(pOCLRunConfig, pOCLProgramConfig, runResult, spProgram.get());

    }
}

OpenCLMICBackendRunner::OpenCLMICBackendRunner(const IRunComponentConfiguration* pRunConfiguration):
    m_pTargetDesc(NULL),
    m_targetDescSize(0)
{
    DEBUG(llvm::dbgs()<< "Initializing OpenCLMICBackendRunner.\n");
    COIENGINE engine;
    uint32_t num_engines = 0;
    // Make sure there is a MIC device available
    CHECK_COI_RESULT(
        COIEngineGetCount(GetCOIISAType(pRunConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "")), &num_engines));
    if (num_engines < 1)
    {
        throw Exception::GeneralException("COIERROR: Need at least one engine");
    }

    // Get a handle to the "first" MIC engine
    CHECK_COI_RESULT(
        COIEngineGetHandle(GetCOIISAType(pRunConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "")), 0, &engine));

    m_procAndPipe.Create(engine, pRunConfig);

    // Retrieve handle to function belonging to sink side process
    CHECK_COI_RESULT(
        COIProcessGetFunctionHandles(
        m_procAndPipe.GetProcessHandler(),  // Process to query for the function
        DEVICE_SIDE_FUNCTION_COUNT,         // The number of functions to look up
        m_device_function_names,            // The names of the functions to look up
        m_device_functions                  // A handle to the functions
        ));

    COIEVENT barrier;
    int32_t initDeviceReturnValue = 0; // success
    // Initialize device
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(), m_device_functions[INIT_DEVICE],// Pipeline handle and function handle
        0, NULL, NULL,                                                      // Buffers and access flags
        0, NULL,                                                            // Input dependencies
        NULL, 0,                                                            // Misc Data to pass to the function
        &initDeviceReturnValue, sizeof(initDeviceReturnValue),              // Return values that will be passed back
        &barrier                                                            // Barrier to signal when it completes
        ));
    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, true, NULL, NULL));
    if ( initDeviceReturnValue )
    {
        assert (initDeviceReturnValue == -1);
        throw Exception::TestRunnerException(std::string("InitDevice function has failed."));
    }

    // Enqueue the function for execution
    // Pass in misc_data and return value pointer to run function
    // Get a barrier to wait on until the run function completion
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(),                     // Pipeline handle
        m_device_functions[GET_BACKEND_TARGET_DESCRIPTION_SIZE],// Function handle
        0, NULL, NULL,                                          // Buffers and access flags
        0, NULL,                                                // Input dependencies
        NULL,   0,                                              // Misc Data to pass to the function
        &m_targetDescSize, sizeof(uint64_t),                    // Return values that will be passed back
        &barrier                                                // Barrier to signal when it completes
        ));

    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, true, NULL, NULL));
    DEBUG(llvm::dbgs()<< "Target description size: " << m_targetDescSize << "\n");

    COIBuffersWrapper targetDesc;
    // allocate buffer for target description
    targetDesc.AddBuffer(size_t(m_targetDescSize), m_procAndPipe.GetProcessHandler(), COI_SINK_WRITE_ENTIRE );

    // Enqueue the function for execution
    // Pass in misc_data and return value pointer to run function
    // Get a barrier to wait on until the run function completion
    uint64_t returnErrorCode = 0;
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(),                 // Pipeline handle
        m_device_functions[GET_BACKEND_TARGET_DESCRIPTION], // Function handle
        1, targetDesc.GetBufferHandler(0),                  // Buffer
        targetDesc.GetBufferAccessFlags(0),                 // Access flags
        0, NULL,                                            // Input dependencies
        NULL, 0,                                            // Misc Data to pass to the function
        &returnErrorCode, sizeof(uint64_t),                 // Return values that will be passed back
        &barrier                                            // Barrier to signal when it completes
        ));
    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, true, NULL, NULL));

    if ( returnErrorCode )
    {
        throw Exception::TestRunnerException(std::string("GetBackendTargetDescription function has failed."));
    }
    char*   bufferPtr;
    targetDesc.Map(COI_MAP_READ_ONLY, 0, NULL, (void**)&bufferPtr, 0);

    m_pTargetDesc.reset(new char[m_targetDescSize]);
    std::copy(bufferPtr, bufferPtr + m_targetDescSize, m_pTargetDesc.get());

    targetDesc.UnMap();
    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner initialized.\n");
}

OpenCLMICBackendRunner::~OpenCLMICBackendRunner()
{
}

void OpenCLMICBackendRunner::SerializeProgram(ICLDevBackendSerializationService* pSerializer,
                                                  ICLDevBackendProgram_* pProgram)
{
    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner starts program serialization.\n");
    size_t programSize = 0;
    CHECK_BACKEND_RESULT(pSerializer->GetSerializationBlobSize(SERIALIZE_TO_DEVICE, pProgram, &programSize));

    // First buffer - test program.
    m_coiFuncArgs.AddBuffer( programSize, m_procAndPipe.GetProcessHandler(), COI_SINK_READ,
        COI_OPTIMIZE_SOURCE_WRITE | COI_OPTIMIZE_SINK_READ);
    void *blob;
    m_coiFuncArgs.Map(COI_MAP_WRITE_ENTIRE_BUFFER, 0, NULL, (void**)&blob, 0);
    m_serializationTime.Start();
    CHECK_BACKEND_RESULT(pSerializer->SerializeProgram(SERIALIZE_TO_DEVICE, pProgram, blob, programSize));
    m_serializationTime.Stop();
    m_coiFuncArgs.UnMap();

    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner program serialization: program size: " << programSize << "\n" << "Program blob: " << (char*)blob << "\n");
    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner program serialization finished.\n");
}

void OpenCLMICBackendRunner::CopyOutputData( BufferContainer& output,
                       const ICLDevBackendKernel_* pKernel,
                       size_t& coiFuncArgsId )
{
    // Get kernel arguments
    uint32_t kernelNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();
    DEBUG(llvm::dbgs()<< "Copy output from the device. BufferContainer size: " << output.GetMemoryObjectCount() << ".\n");
    DEBUG(llvm::dbgs()<< "Number of kernel arguments: " << kernelNumArgs << ".\n");

    for ( uint32_t i = 0; i < kernelNumArgs; ++i)
    {
        DEBUG(llvm::dbgs()<< "Copying data for argument #" << i << ".\n");
        IMemoryObject* pMemObj = output.GetMemoryObject(i);
        void * pData = pMemObj->GetDataPtr();

        if (CL_KRNL_ARG_PTR_IMG_2D == pKernelArgs[i].type || CL_KRNL_ARG_PTR_IMG_3D == pKernelArgs[i].type)
        {

            ImageDesc imageDesc = GetImageDescription(pMemObj->GetMemoryObjectDesc());

            // Copy output data from the device.
            char*   imagePtr;
            m_coiFuncArgs.Map(COI_MAP_READ_ONLY, 0, NULL, (void**)&imagePtr, coiFuncArgsId);
            std::copy(imagePtr, imagePtr + imageDesc.GetImageSizeInBytes(), (char*)pData);
            DEBUG(llvm::dbgs()<< "Read image from the buffer #" << coiFuncArgsId << ". Image size (bytes): " << imageDesc.GetImageSizeInBytes() << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as floats): " << *(float*)imagePtr << ", " << *(((float*)imagePtr) + 1)  << ", " << *(((float*)imagePtr) + 2) << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as ints): " << *(int*)imagePtr << ", " << *(((int*)imagePtr) + 1)  << ", " << *(((int*)imagePtr) + 2) << ".\n");
            m_coiFuncArgs.UnMap();

            ++coiFuncArgsId;

        }
        else if ( CL_KRNL_ARG_PTR_GLOBAL <= pKernelArgs[i].type )
        {
            // Kernel argument is a buffer - need to pass a pointer in the arguments buffer
            BufferDesc bufferDesc = GetBufferDescription(pMemObj->GetMemoryObjectDesc());

            // Copy output data from the device.
            char*   bufferPtr;
            m_coiFuncArgs.Map(COI_MAP_READ_ONLY, 0, NULL, (void**)&bufferPtr, coiFuncArgsId);
            DEBUG(llvm::dbgs()<< "Output data before copying the results.\n");
            DEBUG(llvm::dbgs()<< "First three values (as floats): " << *(float*)pData << ", " << *(((float*)pData) + 1)  << ", " << *(((float*)pData) + 2) << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as ints): " << *(int*)pData << ", " << *(((int*)pData) + 1)  << ", " << *(((int*)pData) + 2) << ".\n");
            std::copy(bufferPtr, bufferPtr + bufferDesc.GetBufferSizeInBytes(), (char*)pData);
            DEBUG(llvm::dbgs()<< "Read array from the buffer #" << coiFuncArgsId << ". Array size (bytes): " << bufferDesc.GetBufferSizeInBytes() << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as floats): " << *(float*)bufferPtr << ", " << *(((float*)bufferPtr) + 1)  << ", " << *(((float*)bufferPtr) + 2) << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as ints): " << *(int*)bufferPtr << ", " << *(((int*)bufferPtr) + 1)  << ", " << *(((int*)bufferPtr) + 2) << ".\n");
            DEBUG(llvm::dbgs()<< "Output data after copying the results.\n");
            DEBUG(llvm::dbgs()<< "First three values (as floats): " << *(float*)pData << ", " << *(((float*)pData) + 1)  << ", " << *(((float*)pData) + 2) << ".\n");
            DEBUG(llvm::dbgs()<< "First three values (as ints): " << *(int*)pData << ", " << *(((int*)pData) + 1)  << ", " << *(((int*)pData) + 2) << ".\n");
            m_coiFuncArgs.UnMap();

            ++coiFuncArgsId;

        }
    }
}

void OpenCLMICBackendRunner::PrepareInputData( BufferContainerList& input,
                                                  char **kernelNameAndArgs,
                                                  ICLDevBackendProgram_* pProgram,
                                                  OpenCLKernelConfiguration *const& pKernelConfig,
                                                  IRunResult* runResult,
                                                  DispatcherData& dispatcherData )
{
    // Get kernel to run
    std::string kernelName = pKernelConfig->GetKernelName();
    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner is preparing input data for " << kernelName << " kernel.\n");
    const ICLDevBackendKernel_* pKernel = NULL;
    pProgram->GetKernelByName(kernelName.c_str(), &pKernel);
    dispatcherData.kernelDirective.kernelNameSize = kernelName.size() + 1;
    dispatcherData.kernelDirective.offset_in_blob = 0;

    // Get kernel arguments
    int kernelNumArgs = pKernel->GetKernelParamsCount();
    DEBUG(llvm::dbgs()<< "Number of arguments for " << kernelName << " kernel is " << kernelNumArgs << ".\n");
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();
    std::vector<bool> ignoreList;
    FillIgnoreList(ignoreList, pKernelArgs, kernelNumArgs);
    static_cast<RunResult*>(runResult)->SetComparatorIgnoreList(kernelName.c_str(), ignoreList);

    dispatcherData.workDesc.setParams(pKernelConfig->GetWorkDimension(),
        pKernelConfig->GetGlobalWorkOffset(),
        pKernelConfig->GetGlobalWorkSize(),
        pKernelConfig->GetLocalWorkSize());

    // Create the argument buffer
    OpenCLMICArgsBuffer argsBuffer(pKernelArgs, kernelNumArgs, &input, m_coiFuncArgs, m_procAndPipe.GetProcessHandler());
    dispatcherData.kernelArgSize = argsBuffer.GetArgsBufferSize();

    std::vector<DirectivePack> directives;
    std::vector<BufferDirective> buffDirectives = argsBuffer.GetDirectivePacks();
    DEBUG(llvm::dbgs()<< "Kerneldirectives size: " << dispatcherData.kernelDirective.kernelNameSize << "\n");
    DEBUG(llvm::dbgs()<< "Buffer directives size: " << buffDirectives.size() << "\n");
    directives.resize(buffDirectives.size());
    for (size_t i = 0; i < buffDirectives.size(); ++i)
    {
         directives[i].id = BUFFER;
         directives[i].bufferDirective = buffDirectives[i];
    }
    *kernelNameAndArgs = new char[dispatcherData.kernelDirective.kernelNameSize + dispatcherData.kernelArgSize + directives.size()*sizeof(DirectivePack)];

    memcpy(*kernelNameAndArgs, kernelName.c_str(), dispatcherData.kernelDirective.kernelNameSize);
    DEBUG(llvm::dbgs()<< "kernelNameAndArgs:kernelName " << *kernelNameAndArgs << "\n");
    memcpy(*kernelNameAndArgs + dispatcherData.kernelDirective.kernelNameSize, argsBuffer.GetArgsBuffer(), dispatcherData.kernelArgSize);
    memcpy(*kernelNameAndArgs + dispatcherData.kernelDirective.kernelNameSize + dispatcherData.kernelArgSize, &(directives[0]), directives.size()*sizeof(DirectivePack));

    dispatcherData.preExeDirectivesCount = argsBuffer.GetDirectivePacks().size();
    DEBUG(llvm::dbgs()<< "Number of pre-execution directives for " << kernelName << " kernel: " << dispatcherData.preExeDirectivesCount << "\n");
    // Create buffer with dispatcher data for all kernels.
    m_coiFuncArgs.AddBuffer(dispatcherData.kernelDirective.kernelNameSize +
        dispatcherData.kernelArgSize + directives.size() * sizeof(DirectivePack),
        m_procAndPipe.GetProcessHandler(), COI_SINK_READ, 0, (void*)(*kernelNameAndArgs));

    //argsBuffer.CopyOutput(runResult, &input, kernelName.c_str());
    IBufferContainerList& output = runResult->GetOutput(kernelName.c_str());
    argsBuffer.CopyFirstBC(&output, &input);
}

void OpenCLMICBackendRunner::RunKernels(const BERunOptions *pRunConfig,
                                            OpenCLProgramConfiguration *pProgramConfig,
                                            IRunResult* pRunResult,
                                            ICLDevBackendProgram_* pProgram)
{
    uint64_t numOfKernels = (uint64_t)pProgramConfig->GetNumberOfKernelConfigurations();
    DEBUG(llvm::dbgs()<< "OpenCLMICBackendRunner runs " << numOfKernels << " test kernels.\n");

    auto_ptr_ex<char, ArrayDP<char> > kernelDataDispatchersAndExeOptionsBuffer(new char[sizeof(DispatcherData)*numOfKernels + sizeof(ExecutionOptions)]);
    DispatcherData *kernelDataDispatchers = (DispatcherData*)kernelDataDispatchersAndExeOptionsBuffer.get();
    ExecutionOptions *exeOptions = (ExecutionOptions*)(kernelDataDispatchers + numOfKernels);
    auto_ptr_ex<BufferContainerList, ArrayDP<BufferContainerList> > kernelArgBufferValuesBuffer(new BufferContainerList[numOfKernels]);
    BufferContainerList *kernelArgBufferValues = kernelArgBufferValuesBuffer.get();
    char **kernelArgsValues = new char*[numOfKernels];

    uint32_t dispatcherIndex = 0;
    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it, ++dispatcherIndex )
    {
        LoadInputBuffer(*it, kernelArgBufferValues + dispatcherIndex);

        PrepareInputData(kernelArgBufferValues[dispatcherIndex], kernelArgsValues+dispatcherIndex, pProgram, *it,
            pRunResult, kernelDataDispatchers[dispatcherIndex]);
    }


    // Copy execution options to the buffer.
    exeOptions->measurePerformance = pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false);
    exeOptions->useTraceMarks = pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false);
    exeOptions->useVTune = pRunConfig->GetValue<bool>(RC_BR_USE_VTUNE, false);
    exeOptions->runSingleWG = pRunConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false);
    exeOptions->defaultLocalWGSize = pRunConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, false);
    exeOptions->executeIterationsCount = pRunConfig->GetValue<uint32_t>(RC_BR_EXECUTE_ITERATIONS_COUNT, false);
    DEBUG(llvm::dbgs()<< "Local work size: " << exeOptions->defaultLocalWGSize << "\n");

    // Create buffer with dispatcher data for all kernels.
    m_coiFuncArgs.AddBuffer(sizeof(DispatcherData) * numOfKernels + sizeof(ExecutionOptions),
        m_procAndPipe.GetProcessHandler(), COI_SINK_READ, 0, kernelDataDispatchers);

    DEBUG(llvm::dbgs()<< "kernelDataDispatchers[0].preExeDirectivesCount " << kernelDataDispatchers[0].preExeDirectivesCount << "\n");
    // TODO: probably it's not good idea pass timers using RunFunction return value.
    // If number of execution iterations is huge it could be slow. Consider using COIBuffer in case if number of executeIterations is large.
    // Performance data from MIC
    uint32_t numOfTimers = numOfKernels*exeOptions->executeIterationsCount + 1;
    auto_ptr_ex<Sample, ArrayDP<Sample> > micTimers(new Sample[numOfTimers]);

    DEBUG(llvm::dbgs()<< "Number of buffers for RunFunction function: " << m_coiFuncArgs.GetNumberOfBuffers() << "\n");

    COIEVENT barrier;
    // Enqueue the function for execution
    // Pass in misc_data and return value pointer to run function
    // Get a barrier to wait on until the run function completion
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(), m_device_functions[EXECUTE_KERNELS],            // Pipeline handle and function handle
        m_coiFuncArgs.GetNumberOfBuffers(), m_coiFuncArgs.GetBufferHandler(0), m_coiFuncArgs.GetBufferAccessFlags(0),  // Buffers and access flags
        0, NULL,                                                    // Input dependencies
        &numOfKernels, sizeof(uint64_t),                            // Misc Data to pass to the function
        micTimers.get(), sizeof(Sample)*(numOfTimers),              // Return values that will be passed back
        &barrier                                                    // Barrier to signal when it completes
        ));
    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, true, NULL, NULL));

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)pRunResult->GetPerformance();
        perfResults.SetDeserializationTime(micTimers.get()[0]);
    }

    // This map contains kernel invocation counter during test execution (e.g. the same kernel can be called multiple time with different work group sizes.
    // This counter is used to obtain correct BufferContainer (represents kernel output) from the BufferContainerList.
    std::map<std::string, uint32_t> kernelRunCounter;
    dispatcherIndex = 0;
    size_t currCOIBufferId = 1; // 0 - is program buffer
    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it, ++dispatcherIndex )
    {
        std::string kernelName = (*it)->GetKernelName();
        DEBUG(llvm::dbgs()<< "Copying back-end results from the device for " << kernelName << " kernel.\n");
        if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
        {
            DEBUG(llvm::dbgs()<< "Performance data for " << kernelName << "\n");
            Performance& perfResults = (Performance&)pRunResult->GetPerformance();
            for (uint32_t j = 0; j < exeOptions->executeIterationsCount; ++j)
            {
                DEBUG(llvm::dbgs()<< "Iteration: " << j << "\tIndex: " << dispatcherIndex*exeOptions->executeIterationsCount + j + 1 << "\tTotal time: " << micTimers.get()[dispatcherIndex*exeOptions->executeIterationsCount + j + 1].TotalTime() << "\t total ticks: " << micTimers.get()[dispatcherIndex*exeOptions->executeIterationsCount + j + 1].TotalTicks() << "\t samples count: " << micTimers.get()[dispatcherIndex*exeOptions->executeIterationsCount + j + 1].SamplesCount() << "\n");
                perfResults.SetExecutionTime(kernelName, micTimers.get()[dispatcherIndex*exeOptions->executeIterationsCount + j + 1]);
            }
        }
        else
        {
            // Get kernel from the program - needed to obtain information about kernel arguments.
            const ICLDevBackendKernel_* pKernel = NULL;
            pProgram->GetKernelByName(kernelName.c_str(), &pKernel);

            // Calculate BufferContainer index in BufferContainerList
            uint32_t buffContainerId = 0;
            if (kernelRunCounter.find(kernelName) != kernelRunCounter.end())
            {
                buffContainerId = kernelRunCounter.find(kernelName)->second;
            }
            IBufferContainer* buffCont = pRunResult->GetOutput(kernelName.c_str()).GetBufferContainer(buffContainerId);
            DEBUG(llvm::dbgs()<< "BufferContainer id for " << kernelName << " " << buffContainerId << ".\n");

            // Copy result data to the runRunsults.
            DEBUG(llvm::dbgs()<< "Copy output data starting from COI buffer #" << currCOIBufferId << ".\n");
            CopyOutputData(*static_cast<BufferContainer*>(buffCont), pKernel, currCOIBufferId);

            ++buffContainerId;
            kernelRunCounter[kernelName] = buffContainerId;
            ++currCOIBufferId;
        }
    }

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)pRunResult->GetPerformance();
        perfResults.SetSerializationTime(m_serializationTime);
    }

    for (uint32_t i = 0; i < pProgramConfig->GetNumberOfKernelConfigurations(); ++i)
    {
        delete [] kernelArgsValues[i];
    }
    delete [] kernelArgsValues;
}

} // namespace Validation
