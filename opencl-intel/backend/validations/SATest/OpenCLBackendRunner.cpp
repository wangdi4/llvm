/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLBackendRunner.cpp

\*****************************************************************************/
#include "BackendOptions.h"
#include "OpenCLBackendRunner.h"
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
#include "OCLKernelDataGenerator.h"
#include "OpenCLKernelArgumentsParser.h"

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/FileSystem.h>

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>

#include "Buffer.h"
#include "mem_utils.h"

#define DEBUG_TYPE "OpenCLBackendRunner"
#include <llvm/Support/raw_ostream.h>
// debug macros
#include <llvm/Support/Debug.h>

#include <string.h>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{

OpenCLBackendRunner::OpenCLBackendRunner(const BERunOptions& runConfig)
{
    OpenCLBackendWrapper::Init(runConfig);
    m_pServiceFactory = OpenCLBackendWrapper::GetInstance().GetBackendServiceFactory();
    m_RandomDataGeneratorSeed = runConfig.GetValue<uint64_t>(RC_COMMON_RANDOM_DG_SEED, 0);
}

OpenCLBackendRunner::~OpenCLBackendRunner()
{
    OpenCLBackendWrapper::Terminate();
}

ICLDevBackendProgram_* OpenCLBackendRunner::CreateProgram(const OpenCLProgram * oclProgram,
                                                         /*const*/ ICLDevBackendCompilationService* pCompileService)
{
    assert( pCompileService);
    assert( oclProgram);
    assert( oclProgram->GetProgramContainerSize() > 0 && "Invalid binary buffer.\n");

    const char* pBinary = oclProgram->GetProgramContainer();
    size_t uiBinarySize = oclProgram->GetProgramContainerSize();
    assert( pBinary);

    ICLDevBackendProgram_* pProgram = NULL;
    cl_dev_err_code ret = pCompileService->CreateProgram(pBinary, uiBinarySize, &pProgram);
    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException("Create program failed.\n");
    }
    m_pModule = oclProgram->ParseToModule();

    return pProgram;
}

void OpenCLBackendRunner::BuildProgram(ICLDevBackendProgram_* pProgram,
                                       /*const*/ ICLDevBackendCompilationService* pCompileService,
                                       IRunResult * runResult,
                                       const BERunOptions* runConfig,
                                       const IProgramConfiguration* pProgramConfig)
{
    Sample buildTime;

    buildTime.Start();
    DEBUG(llvm::dbgs() << "Build program started.\n");
    BuildProgramOptions buildProgramOptions;

    std::string injectedObjectPath = runConfig->GetValue<std::string>(RC_BR_OBJECT_FILE, "");
    if (injectedObjectPath == "")
      injectedObjectPath = static_cast<const OpenCLProgramConfiguration*>(pProgramConfig)->GetInjectedObjectPath();

    std::unique_ptr<llvm::MemoryBuffer> injectedObject;
    // Try to load the injected object file if specified
    if (injectedObjectPath != "")
    {
        auto errorOrBuffer = llvm::MemoryBuffer::getFile(injectedObjectPath);
        if (!errorOrBuffer)
        {
            throw Exception::TestRunnerException("Loading the specified object file \"" + injectedObjectPath + "\" has failed.\n");
        }
        injectedObject = std::move(*errorOrBuffer);
        buildProgramOptions.SetInjectedObject(injectedObject->getBufferStart(), injectedObject->getBufferSize());
    }

    if (runConfig->GetValue<bool>(RC_BR_STOP_BEFORE_JIT, false))
      buildProgramOptions.SetStopBeforeJIT();

    cl_int ret = pCompileService->BuildProgram(pProgram, &buildProgramOptions);
    DEBUG(llvm::dbgs() << "Build program finished.\n");
    buildTime.Stop();

    if ( CL_DEV_FAILED(ret) )
    {
        throw Exception::TestRunnerException(std::string("Build program failed.\nBack-end build log:\n") + pProgram->GetBuildLog());
    }

    if( runConfig->GetValue<bool>(RC_BR_PRINT_BUILD_LOG, false) )
    {
        llvm::outs() << "Build log:\n" << pProgram->GetBuildLog() << '\n';
    }

    if( runConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false) )
    {
        Performance& perfResults = (Performance&)runResult->GetPerformance();
        perfResults.SetBuildTime(buildTime);
    }

}

void OpenCLBackendRunner::LoadInputBuffer(OpenCLKernelConfiguration* pKernelConfig, IContainer* pContainer )
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
    case Random:
        {
            OpenCLKernelArgumentsParser parser;
            if(pKernelConfig->GetGeneratorConfig() != 0 )
            {
                throw Exception::InvalidArgument("[OpenCLBackendRunner]Unused OCLKernelDataGeneratorConfig found. \
                                                 Switch InputDataFileType to \'config\' or remove OCLKernelDataGeneratorConfig block");
            }
            OCLKernelArgumentsList argDescriptions = parser.KernelArgumentsParser(pKernelConfig->GetKernelName(), m_pModule);
            argDescriptions = OpenCLKernelArgumentsParser::KernelArgHeuristics(argDescriptions, pKernelConfig->GetGlobalWorkSize(), pKernelConfig->GetWorkDimension());
            OCLKernelDataGeneratorConfig *cfg = OCLKernelDataGeneratorConfig::defaultConfig(argDescriptions);
            cfg->setSeed(m_RandomDataGeneratorSeed);
            OCLKernelDataGenerator gen(argDescriptions, *cfg);

            gen.Read(pContainer);
            break;
        }
    case Config:
        {
            const OCLKernelDataGeneratorConfig *cfg = pKernelConfig->GetGeneratorConfig();
            //if there is no OCLKernelDataGeneratorConfig in configuration file or it is incorrect
            //GetGeneratorConfig will return zero value
            if(cfg == 0)
            {
                throw Exception::InvalidArgument("[OpenCLBackendRunner]No config is provided. \
                                                 Add OCLKernelDataGeneratorConfig block or try to use another InputDataFileType");
            }
            OpenCLKernelArgumentsParser parser;
            OCLKernelArgumentsList argDescriptions = parser.KernelArgumentsParser(pKernelConfig->GetKernelName(), m_pModule);
            argDescriptions = OpenCLKernelArgumentsParser::KernelArgHeuristics(argDescriptions, pKernelConfig->GetGlobalWorkSize(), pKernelConfig->GetWorkDimension());
            OCLKernelDataGenerator gen(argDescriptions, *cfg);

            gen.Read(pContainer);
            break;
        }
    default:
        throw Exception::TestRunnerException("Unsupported input file type\n");
    }
}

void OpenCLBackendRunner::FillIgnoreList( std::vector<bool>& ignoreList, const cl_kernel_argument* pKernelArgs, int kernelNumArgs )
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
        //    CL_KRNL_ARG_UINT,             // Argument is an unsigned integer.
        //    CL_KRNL_ARG_FLOAT,			// Argument is a float.
        //    CL_KRNL_ARG_DOUBLE,			// Argument is a double.
        //    CL_KRNL_ARG_VECTOR,			// Argument is a vector of basic types, like int8, float4, etc.
        //    CL_KRNL_ARG_VECTOR_BY_REF,    //!< Argument is a byval pointer to a vector of basic types, like int8, float4, etc.
        //    CL_KRNL_ARG_SAMPLER,          // Argument is a sampler object
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
        case CL_KRNL_ARG_VECTOR_BY_REF:     //!< Argument is a byval pointer to a vector of basic types, like int8, float4, etc.
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
        case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_3D:        // Argument is a pointer to 3D image
        case CL_KRNL_ARG_PTR_IMG_2D_ARR:
        case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_1D:
        case CL_KRNL_ARG_PTR_IMG_1D_ARR:
        case CL_KRNL_ARG_PTR_IMG_1D_BUF:
            // TODO: disable read-only images  are ready
            ignoreList[i] = false;
            break;
        case CL_KRNL_ARG_COMPOSITE:         // Argument is a user defined struct
            // ignore arguments passed by value
            ignoreList[i] = true;
            break;
        default:
            throw Exception::InvalidArgument("OpenCLBackendRunner::FillIgnoreList "
                "Unknown kernel argument type\n");
        } // switch(pKernelArgs[i].type)
    }
}

} // namespace


