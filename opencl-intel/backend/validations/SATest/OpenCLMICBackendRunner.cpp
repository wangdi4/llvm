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

    OpenCLProgramConfiguration *pOCLProgramConfig = static_cast<OpenCLProgramConfiguration *>(programConfig);

    ICLDevBackendSerializationServicePtr spSerializationService(NULL);
    ret = m_pServiceFactory->GetSerializationService(NULL, spSerializationService.getOutPtr());
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create serialization service failed");
    }

    // TODO: Implement
    }
}

void OpenCLMICBackendRunner::LoadInputBuffer( OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
{
    // TODO: implement
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

void OpenCLMICBackendRunner::ExecuteKernel( IBufferContainerList& input,
                                            IRunResult * runResult,
                                            ICLDevBackendProgram_* program,
                                            ICLDevBackendExecutionService* pExecutionService,
                                            OpenCLKernelConfiguration * oclConfig,
                                            const BERunOptions* runConfig )
{
    // TODO: implement
}

void OpenCLMICBackendRunner::FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs )
{
    // TODO: implement
}

OpenCLMICBackendRunner::OpenCLMICBackendRunner()
{
    OpenCLMICBackendWrapper::Init();
    m_pServiceFactory = OpenCLMICBackendWrapper::GetInstance().GetBackendServiceFactory();
}

OpenCLMICBackendRunner::~OpenCLMICBackendRunner()
{
    OpenCLMICBackendWrapper::Terminate();
}

} // namespace Validation
