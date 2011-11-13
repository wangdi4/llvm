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

File Name:  Compiler.cpp

\*****************************************************************************/
#define NOMINMAX

#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "Program.h"
#include "Kernel.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "plugin_manager.h"
#include "BitCodeContainer.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Argument.h"
#include "llvm/Type.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/LLVMContext.h"
#include "VTune/JITProfiling.h"
using std::string;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 * Utility methods
 */
namespace Utils 
{

/**
 * Returns the memory buffer of the Program object bytecode
 */
llvm::MemoryBuffer* GetProgramMemoryBuffer(Program* pProgram)
{
    const BitCodeContainer* pCodeContainer = static_cast<const BitCodeContainer*>(pProgram->GetProgramCodeContainer());
    return (llvm::MemoryBuffer*)pCodeContainer->GetMemoryBuffer();
}

/**
 * Generates the log record (to the given stream) enumerating the given external function names
 */
void LogUndefinedExternals( llvm::raw_ostream& logs, const std::vector<std::string>& externals)
{
    logs << "Error: unimplemented function(s) used:\n";

    for( std::vector<std::string>::const_iterator i = externals.begin(), e = externals.end(); i != e; ++i)
    {
        logs << *i << "\n";
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::ERROR_LEVEL, L"implemented function(s) used:\n<%s>", m_strLastError.c_str());
}


} //namespace Utils 


ProgramBuildResult::ProgramBuildResult():
    m_result(CL_DEV_SUCCESS),
    m_logStream(m_buildLog) { }

bool ProgramBuildResult::Succeeded() const 
{ 
    return CL_DEV_SUCCEEDED(m_result); 
}

bool ProgramBuildResult::Failed() const 
{ 
    return CL_DEV_FAILED(m_result); 
}

std::string ProgramBuildResult::GetBuildLog() const 
{ 
    m_logStream.flush();
    return m_buildLog; 
}

llvm::raw_ostream& ProgramBuildResult::LogS() 
{ 
    return m_logStream; 
}

void ProgramBuildResult::SetBuildResult( cl_dev_err_code code ) 
{
    m_result = code; 
}

cl_dev_err_code ProgramBuildResult::GetBuildResult() 
{ 
    return m_result; 
}

/*
 * This is a static method which must be called from the 
 * single threaded environment before any instance of Compiler
 * class is created
 */
void Compiler::Init()
{
    //TODO: Add handling of failure of MT init
    llvm::llvm_start_multithreaded();
    // OpecnCL assumes stack is aligned
    // We are forccing LLVM to align the stack
    std::vector<char *> args;
    char arg1[] = "OclBackend";
    args.push_back(arg1);
    char arg2[] = "-force-align-stack";
    args.push_back(arg2);
    // SSE requirest maximum alignment of parameters of 16 bytes
    // AVX265 requirest maximum alignment of parameters of 32 bytes
    char arg3[] = "-stack-alignment=32";
    args.push_back(arg3);
    llvm::cl::ParseCommandLineOptions(args.size(), &args[0]);

    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmPrinters();
}

/*
 * This is a static method which must be called from the 
 * single threaded environment after all instanced of the
 * Compiler class are deleted
 */
void Compiler::Terminate()
{
    llvm::llvm_shutdown();
}

Compiler::Compiler(IAbstractBackendFactory* pBackendFactory, const CompilerConfig& config):
    m_pLLVMContext( new llvm::LLVMContext ),
    m_config(config),
    m_pBackendFactory(pBackendFactory)
{
}

Compiler::~Compiler()
{
    delete m_pLLVMContext;
}

cl_dev_err_code Compiler::BuildProgram(Program* pProgram, const CompilerBuildOptions* pOptions)
{
    assert(pProgram && "Program parameter must not be NULL");
    ProgramBuildResult buildResult;

    try
    { 
        //TODO: Add log
        std::auto_ptr<llvm::Module> spModule(ParseModuleIR(pProgram)); 
        assert(spModule.get() && "Cannot Created llvm Module from the Program Bit Code");

        FunctionWidthVector vectorizedFunctions;
        KernelsInfoMap kernelsMap;
        size_t privateMemorySize = 0;

        //
        // Apply IR=>IR optimizations
        //
        intel::OptimizerConfig optimizerConfig( m_selectedCpuId, 
                                                m_config.GetTransposeSize(), 
                                                m_selectedCpuFeatures,
                                                m_config.GetIRDumpOptionsAfter(),
                                                m_config.GetIRDumpOptionsBefore(),
                                                m_config.GetDumpIRDir());
        Optimizer optimizer( pProgram, this, spModule.get(), &optimizerConfig);
        optimizer.Optimize();
        if( optimizer.hasUndefinedExternals() )
        {
            Utils::LogUndefinedExternals( buildResult.LogS(), optimizer.GetUndefinedExternals());
            throw Exceptions::CompilerException( "Failed to parse IR", CL_DEV_INVALID_BINARY);
        }
        //
        // Scan the module for kernels
        //
        //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Start iterating over kernels");

        optimizer.GetVectorizedFunctions( vectorizedFunctions);
        optimizer.GetKernelsInfo( kernelsMap);
        privateMemorySize = optimizer.getPrivateMemorySize();

        PostOptimizationProcessing(pProgram, spModule.get());

        KernelSet* pKernels = CreateKernels( pProgram,
                                             spModule.get(), 
                                             buildResult,
                                             vectorizedFunctions, 
                                             kernelsMap,
                                             privateMemorySize);
        //
        // Update the program with build log, kernels and module
        //
        pProgram->SetKernelSet( pKernels);
        pProgram->SetModule( spModule.release()); 
        buildResult.SetBuildResult( CL_DEV_SUCCESS);
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        buildResult.LogS() << e.what() << "\n";
        buildResult.SetBuildResult( e.GetErrorCode());
    }

    pProgram->SetBuildLog( buildResult.GetBuildLog());
    return buildResult.GetBuildResult();
}

llvm::Module* Compiler::ParseModuleIR(Program* pProgram)
{
    //
    // Parse the module IR 
    //
    std::string strErr;
    llvm::MemoryBuffer* pIRBuffer = Utils::GetProgramMemoryBuffer(pProgram);
    llvm::Module* pModule = llvm::ParseBitcodeFile( pIRBuffer, *m_pLLVMContext, &strErr);
    if ( NULL == pModule || !strErr.empty())
    {
        throw Exceptions::CompilerException(std::string("Failed to parse IR: ") + strErr, CL_DEV_INVALID_BINARY);
    }
    return pModule;
}


KernelProperties* Compiler::CreateKernelProperties(const Program* pProgram,
                                                   llvm::MDNode *elt, 
                                                   const TLLVMKernelInfo& info)
{
    // Set optimal WG size
    unsigned int optWGSize = 0;

    optWGSize = 128; // TODO: to be checked

    size_t hintWGSize[MAX_WORK_DIM] = {0,0,0};
    size_t reqdWGSize[MAX_WORK_DIM] = {0,0,0};

    if( NULL != elt )
    {

        llvm::MDNode *wgsh = llvm::dyn_cast<llvm::MDNode>(elt->getOperand(2));
        hintWGSize[0] = llvm::dyn_cast<llvm::ConstantInt>(wgsh->getOperand(0))->getValue().getZExtValue();
        hintWGSize[1] = llvm::dyn_cast<llvm::ConstantInt>(wgsh->getOperand(1))->getValue().getZExtValue();
        hintWGSize[2] = llvm::dyn_cast<llvm::ConstantInt>(wgsh->getOperand(2))->getValue().getZExtValue();
        if(hintWGSize[0])
        {
            optWGSize = 1;
            for(int i=0; i<MAX_WORK_DIM; ++i)
            {
                if(hintWGSize[i]) optWGSize*=hintWGSize[i];
            }
        }

        // Set required WG size
        llvm::MDNode *rwgs = llvm::dyn_cast<llvm::MDNode>(elt->getOperand(1));
        reqdWGSize[0] = llvm::dyn_cast<llvm::ConstantInt>(rwgs->getOperand(0))->getValue().getZExtValue();
        reqdWGSize[1] = llvm::dyn_cast<llvm::ConstantInt>(rwgs->getOperand(1))->getValue().getZExtValue();
        reqdWGSize[2] = llvm::dyn_cast<llvm::ConstantInt>(rwgs->getOperand(2))->getValue().getZExtValue();
        if(reqdWGSize[0])
        {
            optWGSize = 1;
            for(int i=0; i<MAX_WORK_DIM; ++i)
            {
                if(reqdWGSize[i]) optWGSize*=reqdWGSize[i];
            }
        }
    }

    KernelProperties* pProps = m_pBackendFactory->CreateKernelProperties();

    pProps->SetOptWGSize(optWGSize);
    pProps->SetReqdWGSize(reqdWGSize);
    pProps->SetHintWGSize(hintWGSize);
    pProps->SetTotalImplSize(info.stTotalImplSize);
    pProps->SetDAZ( pProgram->GetDAZ());
    pProps->SetCpuId( m_selectedCpuId );
    pProps->SetCpuFeatures( m_selectedCpuFeatures );

    return pProps;
}

llvm::Module* Compiler::CreateRTLModule(BuiltinLibrary* pLibrary)
{
    llvm::MemoryBuffer* pRtlBuffer = pLibrary->GetRtlBuffer();
    std::auto_ptr<llvm::Module> spModule(llvm::ParseBitcodeFile(pRtlBuffer, *m_pLLVMContext));
 
    if ( NULL == spModule.get())
    {
        // Failed to load runtime library
        spModule.reset( new llvm::Module("dummy", *m_pLLVMContext));
        if ( NULL == spModule.get() ) 
        {
            throw Exceptions::CompilerException("Failed to allocate/parse buitin module");
        }
    } 
    else 
    {
        spModule->setModuleIdentifier("RTLibrary");
    }

    // Setting the target triple as a tripple the backend was compiled under
#if defined(_M_X64)
    spModule->setTargetTriple( "x86_64-pc-win32");
#elif defined(__LP64__)
    spModule->setTargetTriple( "amd64");
#else
    spModule->setTargetTriple( "i686-pc-win32");
#endif 
    return spModule.release();
}


}}}