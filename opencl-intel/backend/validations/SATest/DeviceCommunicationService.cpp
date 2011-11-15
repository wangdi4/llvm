/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  DeviceCommunicationService.cpp

\*****************************************************************************/

#include "DeviceCommunicationService.h"
#include "SATestException.h"
#include "BufferContainerList.h"
#include "BinaryDataReader.h"
#include "XMLDataReader.h"
#include "Performance.h"
#include "OpenCLMICArgsBuffer.h"
#include "RunResult.h"

#include <iostream>
#include <assert.h>

#define DEBUG_TYPE "DeviceCommunicationService"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace Validation;
using namespace Intel::OpenCL::DeviceBackend;

// Device side functions used as entry points
// WARINING! This array must be in line with enum DEVICE_SIDE_FUNCTION
const char* DeviceCommunicationService::m_device_function_names[DEVICE_SIDE_FUNCTION_COUNT] =
{
    "getBackendTargetDescriptionSize",  // GET_BACKEND_TARGET_DESCRIPTION_SIZE
    "getBackendTargetDescription",      // GET_BACKEND_TARGET_DESCRIPTION
    "executeKernels"                    // EXECUTE_KERNELS
};

#define CHECK_COI_RESULT(_COIFUNC)                                                  \
    {                                                                               \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        throw Exception::COIUsageException(std::string(#_COIFUNC" retruned ") +     \
            std::string(COIResultGetName(result)));                                 \
    }                                                                               \
}

#define CHECK_COI_RESULT_NO_THROW(_COIFUNC)                                         \
{                                                                                   \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        std::cerr << #_COIFUNC" retruned " << COIResultGetName(result) << "\n";     \
    }                                                                               \
}

#define CHECK_BACKEND_RESULT(_BEFUNC)                                           \
    {                                                                           \
    cl_dev_err_code result = _BEFUNC;                                           \
    if (CL_DEV_FAILED(result))                                                  \
    {                                                                           \
        throw Exception::BackendException(std::string(#_BEFUNC" failed"));      \
    }                                                                           \
}

DeviceCommunicationService::DeviceCommunicationService():
    m_pTargetDesc(NULL),
    m_targetDescSize(0)
{
}

DeviceCommunicationService::~DeviceCommunicationService()
{
}

void DeviceCommunicationService::Init(const BERunOptions *pRunConfig)
{
    DEBUG(llvm::dbgs()<< "Initializing DeviceCommunicationService.\n");
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

    m_procAndPipe.Create(engine);

    // Retrieve handle to function belonging to sink side process
    CHECK_COI_RESULT(
        COIProcessGetFunctionHandles(
        m_procAndPipe.GetProcessHandler(),  // Process to query for the function
        DEVICE_SIDE_FUNCTION_COUNT,         // The number of functions to look up
        m_device_function_names,            // The names of the functions to look up
        m_device_functions                  // A handle to the functions
        ));

    COIEVENT barrier;
    // Enqueue the function for execution
    // Pass in misc_data and return value pointer to run function
    // Get a barrier to wait on until the run function completion
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(), m_device_functions[GET_BACKEND_TARGET_DESCRIPTION_SIZE],// Pipeline handle and function handle
        0, NULL, NULL,                                                      // Buffers and access flags
        0, NULL,                                                            // Input dependencies
        NULL,   0,                                                          // Misc Data to pass to the function
        &m_targetDescSize, sizeof(uint64_t),                                // Return values that will be passed back
        &barrier                                                            // Barrier to signal when it completes
        ));

    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, false, NULL, NULL));
    DEBUG(llvm::dbgs()<< "Target description size: " << m_targetDescSize << "\n");

    COIBuffersWrapper targetDesc;
    // allocate buffer for target description
    targetDesc.AddBuffer(size_t(m_targetDescSize), m_procAndPipe.GetProcessHandler(), COI_SINK_WRITE_ENTIRE);

    // Enqueue the function for execution
    // Pass in misc_data and return value pointer to run function
    // Get a barrier to wait on until the run function completion
    uint64_t filledSize = 0;
    CHECK_COI_RESULT(
        COIPipelineRunFunction(
        m_procAndPipe.GetPipelineHandler(), m_device_functions[GET_BACKEND_TARGET_DESCRIPTION], // Pipeline handle and function handle
        1, targetDesc.GetBufferHandler(0), targetDesc.GetBufferAccessFlags(0),                  // Buffers and access flags
        0, NULL,                            // Input dependencies
        NULL, 0,                            // Misc Data to pass to the function
        &filledSize, sizeof(uint64_t),      // Return values that will be passed back
        &barrier                            // Barrier to signal when it completes
        ));

    char*   bufferPtr;
    targetDesc.Map(COI_MAP_READ_ONLY, 1, &barrier, (void**)&bufferPtr, 0);

    assert(m_targetDescSize != filledSize);
    m_pTargetDesc.reset(new char[m_targetDescSize]);
    std::copy(bufferPtr, bufferPtr + filledSize, m_pTargetDesc.get());

    targetDesc.UnMap();
    DEBUG(llvm::dbgs()<< "DeviceCommunicationService initialized.\n");
}

COI_ISA_TYPE DeviceCommunicationService::GetCOIISAType(std::string cpuArch)
{
    // TODO: at the moment SATest is supposed to support KNF architecture only.
    // This function have to be fixed to support other MIC architectures.
    if(std::string("auto-remote") == cpuArch) return COI_ISA_KNF;
    if(std::string("knf") == cpuArch) return COI_ISA_KNF;
    // if(std::string("knc") == cpuArch) return COI_ISA_KNC;
    return COI_ISA_INVALID;
}

void DeviceCommunicationService::SerializeProgram(ICLDevBackendSerializationService* pSerializer,
                                                  ICLDevBackendProgram_* pProgram)
{
    DEBUG(llvm::dbgs()<< "DeviceCommunicationService starts program serialization.\n");
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
    DEBUG(llvm::dbgs()<< "DeviceCommunicationService program serialization finished.\n");
}

void FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs )
{
    ignoreList.resize(kernelNumArgs);
    // perform pass OpenCL back-end kernel arguments and
    // mark which arguments to ignore in comparator
    for(int i=0; i<kernelNumArgs; ++i)
    {

        //// Defines possible values for kernel argument types
        //typedef enum _cl_kernel_arg_type
        //{
        //    CL_KRNL_ARG_INT		= 0,	// Argument is a signed integer.
        //    CL_KRNL_ARG_UINT,			// Argument is an unsigned integer.
        //    CL_KRNL_ARG_FLOAT,			// Argument is a float.
        //    CL_KRNL_ARG_DOUBLE,			// Argument is a double.
        //    CL_KRNL_ARG_VECTOR,			// Argument is a vector of basic types, like int8, float4, etc.
        //    CL_KRNL_ARG_SAMPLER,		// Argument is a sampler object
        //    CL_KRNL_ARG_PTR_LOCAL,		// Argument is a pointer to array declared in local memory
        //    //	Memory object types bellow this line
        //    CL_KRNL_ARG_PTR_GLOBAL,		// Argument is a pointer to array in global memory of various types
        //    // The array type could be char, short, int, float or double
        //    // User must pass a handle to a memory buffer for this argument type
        //    CL_KRNL_ARG_PTR_CONST,		// Argument is a pointer to buffer declared in constant(global) memory
        //    CL_KRNL_ARG_PTR_IMG_2D,		// Argument is a pointer to 2D image
        //    CL_KRNL_ARG_PTR_IMG_3D,		// Argument is a pointer to 3D image
        //    CL_KRNL_ARG_COMPOSITE			// Argument is a user defined struct
        //} cl_kernel_arg_type;

        switch(pKernelArgs[i].type)
        {
        case CL_KRNL_ARG_INT:               // Argument is a signed integer.
        case CL_KRNL_ARG_UINT:              // Argument is an unsigned integer.
        case CL_KRNL_ARG_FLOAT:             // Argument is a float.
        case CL_KRNL_ARG_DOUBLE:            // Argument is a double.
        case CL_KRNL_ARG_VECTOR:            // Argument is a vector of basic types, like int8, float4, etc.
            // ignore arguments passed by value
            ignoreList[i] = true;
            break;
        case CL_KRNL_ARG_PTR_LOCAL:
            // ignore ptr to __local memory
            ignoreList[i] = true;
            break;
        case CL_KRNL_ARG_PTR_GLOBAL:
            // ptr to __global memory. do not ignore it
            ignoreList[i] = false;
            break;
        case CL_KRNL_ARG_PTR_CONST:        // Argument is a pointer to buffer declared in constant(global) memory
            // ignore constant buffers
            ignoreList[i] = true;
            break;
        case CL_KRNL_ARG_SAMPLER:
            // ignore sampler object
            ignoreList[i] = true;
            break;
        case CL_KRNL_ARG_PTR_IMG_2D:        // Argument is a pointer to 2D image
        case CL_KRNL_ARG_PTR_IMG_3D:        // Argument is a pointer to 3D image
            // TODO: disable read-only images  are ready
            ignoreList[i] = false;
            break;
        case CL_KRNL_ARG_COMPOSITE:         // Argument is a user defined struct
            // ignore arguments passed by value
            ignoreList[i] = true;
            break;
        default:
            throw Exception::InvalidArgument("Comparator::CompareOCLKernelRun "
                "Unknown kernel argument type");
        } // switch(pKernelArgs[i].type)
    }
}

void DeviceCommunicationService::CopyOutputData( BufferContainer& output,
                       const ICLDevBackendKernel_* pKernel,
                       DispatcherData& dispatcherData,
                       size_t& coiFuncArgsId )
{
    // Get kernel arguments
    uint32_t kernelNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();

    for ( uint32_t i = 0; i < kernelNumArgs; ++i)
    {
        IMemoryObject* pMemObj = output.GetMemoryObject(i);
        void * pData = pMemObj->GetDataPtr();

        if (CL_KRNL_ARG_PTR_IMG_2D == pKernelArgs[i].type || CL_KRNL_ARG_PTR_IMG_3D == pKernelArgs[i].type)
        {

            ImageDesc imageDesc = GetImageDescription(pMemObj->GetMemoryObjectDesc());

            // Copy output data from the device.
            char*   imagePtr;
            m_coiFuncArgs.Map(COI_MAP_READ_ONLY, 0, NULL, (void**)&imagePtr, coiFuncArgsId);
            std::copy(imagePtr, imagePtr + imageDesc.GetImageSizeInBytes(), (char*)pData);
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
            std::copy(bufferPtr, bufferPtr + bufferDesc.GetBufferSizeInBytes(), (char*)pData);
            m_coiFuncArgs.UnMap();

            ++coiFuncArgsId;

        }
    }
}

void DeviceCommunicationService::PrepareInputData( BufferContainerList& input,
                                                  char **kernelNameAndArgs,
                                                  ICLDevBackendProgram_* pProgram,
                                                  OpenCLKernelConfiguration *const& pKernelConfig,
                                                  const BERunOptions * pRunConfig,
                                                  IRunResult* runResult,
                                                  DispatcherData& dispatcherData )
{
    // Get kernel to run
    std::string kernelName = pKernelConfig->GetKernelName();
    DEBUG(llvm::dbgs()<< "DeviceCommunicationService is preparing input data for " << kernelName << ".\n");
    const ICLDevBackendKernel_* pKernel = NULL;
    pProgram->GetKernelByName(kernelName.c_str(), &pKernel);
    dispatcherData.kernelDirective.kernelNameSize = kernelName.size() + 1;
    dispatcherData.kernelDirective.offset_in_blob = 0;

    // Get kernel arguments
    int kernelNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument* pKernelArgs = pKernel->GetKernelParams();
    std::vector<bool> ignoreList;
    FillIgnoreList(ignoreList, pKernelArgs, kernelNumArgs);
    static_cast<RunResult*>(runResult)->SetComparatorIgnoreList(kernelName.c_str(), ignoreList);

    dispatcherData.workDesc.setParams(pKernelConfig->GetWorkDimension(),
        pKernelConfig->GetGlobalWorkOffset(),
        pKernelConfig->GetGlobalWorkSize(),
        pKernelConfig->GetLocalWorkSize());

    // Create the argument buffer
    OpenCLMICArgsBuffer argsBuffer(pKernelArgs, kernelNumArgs, &input, &dispatcherData, m_coiFuncArgs, m_procAndPipe.GetProcessHandler());

    std::vector<DirectivePack> directives;
    std::vector<BufferDirective> buffDirectives = argsBuffer.GetDirectivePacks();
    DEBUG(llvm::dbgs()<< "Buffer directives size: " << buffDirectives.size() << "\n");
    directives.resize(buffDirectives.size());
    for (size_t i = 0; i < buffDirectives.size(); ++i)
    {
         directives[i].id = BUFFER;
         directives[i].bufferDirective = buffDirectives[i];
    }
    *kernelNameAndArgs = new char[dispatcherData.kernelDirective.kernelNameSize + dispatcherData.kernelArgSize + directives.size()*sizeof(DirectivePack)];

    memcpy(*kernelNameAndArgs, kernelName.c_str(), dispatcherData.kernelDirective.kernelNameSize);
    memcpy(*kernelNameAndArgs + dispatcherData.kernelDirective.kernelNameSize, argsBuffer.GetArgsBuffer(), dispatcherData.kernelArgSize);
    memcpy(*kernelNameAndArgs + dispatcherData.kernelDirective.kernelNameSize + dispatcherData.kernelArgSize, &(directives[0]), directives.size()*sizeof(DirectivePack));

    dispatcherData.preExeDirectivesCount = argsBuffer.GetDirectivePacks().size();
    // Create buffer with dispatcher data for all kernels.
    m_coiFuncArgs.CreateBufferFromMemory(dispatcherData.kernelDirective.kernelNameSize +
        dispatcherData.kernelArgSize + directives.size() * sizeof(DirectivePack),
        m_procAndPipe.GetProcessHandler(), COI_SINK_READ, (void*)(*kernelNameAndArgs));

    argsBuffer.CopyOutput(runResult, &input, kernelName.c_str());
}

void DeviceCommunicationService::RunKernels(const BERunOptions *pRunConfig,
                                            OpenCLProgramConfiguration *pProgramConfig,
                                            IRunResult* pRunResult,
                                            ICLDevBackendProgram_* pProgram)
{
    DEBUG(llvm::dbgs()<< "DeviceCommunicationService runs test kernels.\n");
    uint64_t numOfKernels = (uint64_t)pProgramConfig->numberOfKenelConfigurations();

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
            pRunConfig, pRunResult, kernelDataDispatchers[dispatcherIndex]);
    }


    // Copy execution options to the buffer.
    exeOptions->measurePerformance = pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false);
    exeOptions->useTraceMarks = pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false);
    exeOptions->useVTune = pRunConfig->GetValue<bool>(RC_BR_USE_VTUNE, false);
    exeOptions->runSingleWG = pRunConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false);
    exeOptions->defaultLocalWGSize = pRunConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, false);
    exeOptions->executeIterationsCount = pRunConfig->GetValue<uint32_t>(RC_BR_EXECUTE_ITERATIONS_COUNT, false);

    // Create buffer with dispatcher data for all kernels.
    m_coiFuncArgs.CreateBufferFromMemory(sizeof(DispatcherData) * numOfKernels + sizeof(ExecutionOptions),
        m_procAndPipe.GetProcessHandler(), COI_SINK_READ, kernelDataDispatchers);

    // TODO: probably it's not good idea pass timers using RunFunction return value.
    // If number of execution iterations is huge it could be slow. Consider using COIBuffer in case if number of executeIterations is large.
    // Performance data from MIC
    uint32_t numOfTimers = numOfKernels*exeOptions->executeIterationsCount + 1;
    auto_ptr_ex<Sample, ArrayDP<Sample> > micTimers(new Sample[numOfTimers]);

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
        micTimers.get(), sizeof(Sample)*(numOfTimers),                 // Return values that will be passed back
        &barrier                                                    // Barrier to signal when it completes
        ));
    CHECK_COI_RESULT(COIEventWait(1, &barrier, -1, false, NULL, NULL));

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)pRunResult->GetPerformance();
        perfResults.SetDeserializationTime(micTimers.get()[0]);
    }

    dispatcherIndex = 0;
    size_t currBufferId = 1; // 0 - is program buffer
    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it, ++dispatcherIndex )
    {
        std::string kernelName = (*it)->GetKernelName();
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
            // Get kernel to run
            const ICLDevBackendKernel_* pKernel = NULL;
            pProgram->GetKernelByName(kernelName.c_str(), &pKernel);

            IBufferContainer* buffCont = pRunResult->GetOutput(kernelName.c_str()).GetBufferContainer(0);

            // Copy result data to the runRunsults.
            CopyOutputData(*static_cast<BufferContainer*>(buffCont), pKernel, kernelDataDispatchers[dispatcherIndex], currBufferId);
            ++currBufferId;
        }
    }

    if( pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)pRunResult->GetPerformance();
        perfResults.SetSerializationTime(m_serializationTime);
    }

    for (uint32_t i = 0; i < pProgramConfig->numberOfKenelConfigurations(); ++i)
    {
        delete [] kernelArgsValues[i];
    }
    delete [] kernelArgsValues;
}

void DeviceCommunicationService::LoadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    assert( NULL != pKernelConfig);

    switch( pKernelConfig->GetInputFileType())
    {
    case Binary:
        {
            BinaryContainerListReader reader( pKernelConfig->GetInputFilePath() );
            reader.Read(pContainer);
            break;
        }
    case Xml:
        {
            XMLBufferContainerListReader reader( pKernelConfig->GetInputFilePath() );
            reader.Read(pContainer);
            break;
        }
    default:
        throw Exception::TestRunnerException("Unsupported input file type");
    }
}

void COIProcessAndPipelineWrapper::Create( COIENGINE engine )
{
    // Create a process on the MIC.
    CHECK_COI_RESULT(
        COIProcessCreateFromFile(
        engine,             // The engine to create the process on.
        SATEST_NATIVE_NAME, // The local path to the sink side binary to launch.
        0, NULL,            // argc and argv for the sink process.
        false, NULL,        // Environment variables to set for the sink process.
        true, NULL,         // Enable the proxy but don't specify a proxy root path.
        256*1024*1024,                  // The amount of memory to reserve for COIBuffers. 0 - dynamic allocation.
        NULL,               // Path to search for dependencies
        &m_process          // The resulting process handle.
        ));

    // Create a pipeline associated with process created earlier.
    COIRESULT res = 
        COIPipelineCreate(
        m_process,          // Process to associate the pipeline with
        NULL,               // Do not set any sink thread affinity for the pipeline
        0,                  // Use the default stack size for the pipeline thread
        &m_pipeline         // Handle to the new pipeline
        );
    if (COI_SUCCESS != res)
    {
        // Destroy successfully created process.
        COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            );
        throw Exception::COIUsageException(std::string("Unable to create COI pipeline\n")+std::string(COIResultGetName(res)));
        return;
    }

    // Load SVML built-ins library.
    res = COIProcessLoadLibraryFromFile(
        m_process,          // in_Process
        SVML_LIBRARY_NAME,  // in_FileName
        NULL,               // in_so-name if not exists in file
        SVML_LIBRARY_PATH,  // in_LibrarySearchPath
        &m_library );
    if ((COI_SUCCESS != res) && (COI_ALREADY_EXISTS != res))
    {
        // Destroy successfully created pipeline.
        COIPipelineDestroy(m_pipeline);
        // Destroy successfully created process.
        COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            );
        throw Exception::COIUsageException(std::string("Unable to load SVML built-ins library to the device: \n")+std::string(COIResultGetName(res)));
        return;
    }
    m_created = true;
}

COIProcessAndPipelineWrapper::~COIProcessAndPipelineWrapper()
{
    if (m_created)
    {
        // Unload SVML built-ins library
        CHECK_COI_RESULT_NO_THROW(COIProcessUnloadLibrary(m_process, m_library));

        // Destroy the pipeline
        CHECK_COI_RESULT_NO_THROW(COIPipelineDestroy(m_pipeline));

        // Destroy the process
        CHECK_COI_RESULT_NO_THROW(
            COIProcessDestroy(
            m_process,      // Process handle to be destroyed.
            -1,             // Wait indefinitely until main() (on the MIC side) returns.
            false,          // Don't force to exit. Let it finish executing functions enqueued and exit gracefully.
            NULL,           // Don't care about the exit result.
            NULL            // Also don't care what the exit reason was.
            ));
    }
}

void COIBuffersWrapper::AddBuffer( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, uint32_t flags )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::AddBuffer.\n");
    DEBUG(llvm::dbgs()<< "Buffer size: " << bufferSize << "\n");
    DEBUG(llvm::dbgs()<< "COI buffer flags: " << flags << "\n");
    COIBUFFER buff;
    CHECK_COI_RESULT(
        COIBufferCreate(
        bufferSize,             // Size of the buffer
        COI_BUFFER_NORMAL,      // Type of the buffer
        flags,
        NULL,                   // Pointer to the Initialization data
        1, &process,            // Processes that will use the buffer
        &buff                   // Buffer handle that was created
        ));
    m_buffers.push_back(buff);
    m_flags.push_back(access);
}

COIBuffersWrapper::~COIBuffersWrapper( void )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper desctructor.\n");
    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        CHECK_COI_RESULT_NO_THROW(COIBufferDestroy(m_buffers[i]));
    }
}

// TODO: Add multiple buffer mapping support.
void COIBuffersWrapper::Map( COI_MAP_TYPE mapType, int numOfDepends, COIEVENT* dependencies, void** data, size_t id )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::Map.\n");
    CHECK_COI_RESULT(
        COIBufferMap(
        m_buffers[id],               // Buffer handle to map
        0, 0,                   // Starting offset and number of bytes to map
        mapType,                // Map type
        numOfDepends, dependencies,         // Input dependencies
        NULL,                   // No completion barrier indicates synchronous map
        &m_map,                 // Map instance handle
        data                    // Pointer to access the buffer data.
        ));
}


void COIBuffersWrapper::UnMap()
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::UnMap.\n");
    // Unmap the buffer mapping Instance 'mi'
    CHECK_COI_RESULT(COIBufferUnmap(m_map, 0, NULL, NULL));
}

void COIBuffersWrapper::CreateBufferFromMemory( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, void* pData )
{
    DEBUG(llvm::dbgs()<< "Calling COIBuffersWrapper::CreateBufferFromMemory.\n");
    COIBUFFER buff;
    CHECK_COI_RESULT(
        COIBufferCreateFromMemory(
        bufferSize,                 // Size of the buffer
        COI_BUFFER_NORMAL,          // Type of the buffer
        0,
        pData,                      // Pointer to the Initialization data
        1, &process,                // Processes that will use the buffer
        &buff                       // Buffer handle that was created
        ));
    m_buffers.push_back(buff);
    m_flags.push_back(access);
}
