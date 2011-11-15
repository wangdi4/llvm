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

File Name:  OpenCLMICBackendRunner.cpp

\*****************************************************************************/
#include "OpenCLMICBackendRunner.h"
#include "OpenCLMICBackendWrapper.h"
#include "ICLDevBackendOptions.h"
#include "SATestException.h"
#include "Performance.h"
#include "BinaryDataReader.h"
#include "XMLDataReader.h"
#include "OpenCLRunConfiguration.h"

// TODO: FillIgnoreList and LoadInputBuffer functions must be shared between MICBackendRunner and ProgramRunner.

namespace Validation {

    class BackendOptions: public ICLDevBackendOptions
    {
    public:
        void InitFromRunConfiguration(const BERunOptions& runConfig)
        {
            m_transposeSize = runConfig.GetValue<ETransposeSize>(RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_AUTO);
            m_cpu           = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto");
            m_cpuFeatures   = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
            m_useVTune      = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);
            m_fileName      = runConfig.GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "-");
        }

        bool GetBooleanValue(int optionId, bool defaultValue) const
        {
            switch(optionId)
            {
            case CL_DEV_BACKEND_OPTION_USE_VTUNE :
                return m_useVTune;
            default:
                return defaultValue;
            }
        }

        virtual int GetIntValue( int optionId, int defaultValue) const
        {
            return CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE == optionId ? (int)m_transposeSize
                : defaultValue;
        }

        virtual const char* GetStringValue(int optionId, const char* defaultValue)const
        {
            switch(optionId)
            {
            case CL_DEV_BACKEND_OPTION_CPU_ARCH :
                return m_cpu.c_str();
            case CL_DEV_BACKEND_OPTION_CPU_FEATURES:
                return m_cpuFeatures.c_str();
            case CL_DEV_BACKEND_OPTION_DUMPFILE :
                return m_fileName.c_str();
            default:
                return defaultValue;
            }
        }

        virtual void SetStringValue(int optionId, const char* value)
        {
            switch(optionId)
            {
            case CL_DEV_BACKEND_OPTION_CPU_ARCH :
                m_cpu = std::string(value);
            case CL_DEV_BACKEND_OPTION_CPU_FEATURES:
                m_cpuFeatures = std::string(value);
            default:
                return ;
            }
        }

        virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
        {
            return false;
        }

    private:
        ETransposeSize m_transposeSize;
        std::string    m_cpu;
        std::string    m_cpuFeatures;
        bool           m_useVTune;
        std::string    m_fileName;
    };

void OpenCLMICBackendRunner::Run(IRunResult* runResult,
                                 IProgram* program,
                                 IProgramConfiguration* programConfig,
                                 const IRunComponentConfiguration* runConfig )
{
    assert((program != NULL) && "Program is not initialized");
    assert((programConfig != NULL) && "Program Configuration is not initialized");
    assert((runConfig != NULL) && "Run Configuration is not initialized");
    assert((runResult != NULL) && "Run Result is not initialized");

    const BERunOptions         *pOCLRunConfig     = static_cast<const BERunOptions *>(runConfig);
    OpenCLProgram              *pOCLProgram       = static_cast<OpenCLProgram *>(program);

    BackendOptions options;
    options.InitFromRunConfiguration(*pOCLRunConfig);

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

    OpenCLProgramConfiguration *pOCLProgramConfig = static_cast<OpenCLProgramConfiguration *>(programConfig);

    ICLDevBackendSerializationServicePtr spSerializationService(NULL);
    ret = m_pServiceFactory->GetSerializationService(&options, spSerializationService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create serialization service failed");
    }

    m_pMICService->SerializeProgram(spSerializationService.get(), spProgram.get());

    m_pMICService->RunKernels(pOCLRunConfig, pOCLProgramConfig, runResult, spProgram.get());

    }
}

void OpenCLMICBackendRunner::LoadInputBuffer( OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
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

ICLDevBackendProgram_* OpenCLMICBackendRunner::CreateProgram( OpenCLProgram * oclProgram, ICLDevBackendCompilationService* pCompileService )
{
    assert( pCompileService);
    assert( oclProgram);
    assert( oclProgram->GetProgramContainerSize() > 0 && "Invalid binary buffer ");

    const cl_prog_container_header* pHeader = oclProgram->GetProgramContainer();
    assert( pHeader );


    ICLDevBackendProgram_* pProgram = NULL;
    cl_dev_err_code ret = pCompileService->CreateProgram(pHeader, &pProgram);
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create program failed");
    }

    return pProgram;
}

void OpenCLMICBackendRunner::BuildProgram( ICLDevBackendProgram_* pProgram,
                                           ICLDevBackendCompilationService* pCompileService,
                                           IRunResult * runResult,
                                           const BERunOptions* runConfig )
{
    Sample buildTime;

    buildTime.Start();
    cl_int ret = pCompileService->BuildProgram(pProgram, NULL);
    buildTime.Stop();

    if( runConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)runResult->GetPerformance();
        perfResults.SetBuildTime(buildTime);
    }

    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Build program failed");
    }
}

void OpenCLMICBackendRunner::FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs )
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
            throw Exception::InvalidArgument("FillIgnoreList "
                "Unknown kernel argument type");
        } // switch(pKernelArgs[i].type)
    }
}

OpenCLMICBackendRunner::OpenCLMICBackendRunner(const IRunComponentConfiguration* pRunConfiguration)
{
    m_pMICService = new DeviceCommunicationService();
    m_pMICService->Init(static_cast<const BERunOptions*>(pRunConfiguration));

    OpenCLMICBackendWrapper::Init();
    m_pServiceFactory = OpenCLMICBackendWrapper::GetInstance().GetBackendServiceFactory();
}

OpenCLMICBackendRunner::~OpenCLMICBackendRunner()
{
    delete m_pMICService;
    OpenCLMICBackendWrapper::Terminate();
}

} // namespace Validation
