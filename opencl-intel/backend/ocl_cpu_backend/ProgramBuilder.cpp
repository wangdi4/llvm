/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ProgramBuilder.cpp

\*****************************************************************************/
#define NOMINMAX

#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "ProgramBuilder.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "Program.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "plugin_manager.h"
#include "MetaDataApi.h"
#include "BitCodeContainer.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
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
#include <algorithm>


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

} //namespace Utils

ProgramBuilder::ProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& config):
    m_pBackendFactory(pBackendFactory),
    m_useVTune(config.GetUseVTune())
{
}

ProgramBuilder::~ProgramBuilder()
{
}

cl_dev_err_code ProgramBuilder::BuildProgram(Program* pProgram, const ICLDevBackendOptions* pOptions)
{
    assert(pProgram && "Program parameter must not be NULL");
    ProgramBuildResult buildResult;

    try
    {
        Compiler* pCompiler = GetCompiler();

        CompilerBuildOptions buildOptions(pProgram->GetDebugInfoFlag(),
                                          pProgram->GetProfilingFlag(),
                                          pProgram->GetDisableOpt(),
                                          pProgram->GetFastRelaxedMath(),
                                          false,
                                          pOptions? pOptions->GetIntValue(CL_DEV_BACKEND_OPTION_APF_LEVEL, 0) : 0);

        std::auto_ptr<llvm::Module> spModule( pCompiler->BuildProgram( Utils::GetProgramMemoryBuffer(pProgram),
                                                                       &buildOptions,
                                                                       &buildResult));

        pProgram->SetExecutionEngine(pCompiler->GetExecutionEngine());
        pProgram->SetBuiltinModule(pCompiler->GetRtlModule());

        PostOptimizationProcessing(pProgram, spModule.get(), pOptions);

        //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Start iterating over kernels");
        KernelSet* pKernels = CreateKernels( pProgram,
                                             spModule.get(),
                                             buildResult);

        pProgram->SetKernelSet( pKernels);
        pProgram->SetModule( spModule.release());
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        buildResult.LogS() << e.what() << "\n";
        buildResult.SetBuildResult( e.GetErrorCode());
        pProgram->SetBuildLog( buildResult.GetBuildLog() );
        throw e;
    }

    pProgram->SetBuildLog( buildResult.GetBuildLog());
    return buildResult.GetBuildResult();
}

KernelJITProperties* ProgramBuilder::CreateKernelJITProperties( unsigned int vectorSize) const
{
    KernelJITProperties* pProps = m_pBackendFactory->CreateKernelJITProperties();
    pProps->SetUseVTune(m_useVTune);
    pProps->SetVectorSize(vectorSize);
    return pProps;
}

KernelProperties* ProgramBuilder::CreateKernelProperties(const Program* pProgram,
                                                         Function *func,
                                                         Function *pWrapperFunc,
                                                         const ProgramBuildResult& buildResult) const
{
    // Set optimal WG size
    unsigned int optWGSize = 128; // TODO: to be checked

    size_t hintWGSize[MAX_WORK_DIM] = {0,0,0};
    size_t reqdWGSize[MAX_WORK_DIM] = {0,0,0};

    Module *pModule = func->getParent();
    MetaDataUtils mdUtils(pModule);

    if( !mdUtils.isKernelsHasValue())
        throw  Exceptions::CompilerException("Internal error", CL_DEV_BUILD_ERROR);

    KernelMetaDataHandle kmd;
    for (MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels(), e = mdUtils.end_Kernels(); i != e; ++i)
    {
        if( func == (*i)->getFunction() ) //TODO stripPointerCasts()
        {
            kmd = *i;
            break;
        }
    }

    if( NULL == kmd.get())
    {
        throw Exceptions::CompilerException("Internal Error");
    }

    if( kmd->getWorkGroupSizeHint()->hasValue() )
    {
        hintWGSize[0] = kmd->getWorkGroupSizeHint()->getXDim(); // TODO: SExt <=> ZExt
        hintWGSize[1] = kmd->getWorkGroupSizeHint()->getYDim(); // TODO: SExt <=> ZExt
        hintWGSize[2] = kmd->getWorkGroupSizeHint()->getZDim(); // TODO: SExt <=> ZExt

        if(hintWGSize[0])
        {
            optWGSize = 1;
            for(int i=0; i<MAX_WORK_DIM; ++i)
            {
                if(hintWGSize[i])
                {
                    optWGSize*=hintWGSize[i];
                }
            }
        }
    }

    if( kmd->getReqdWorkGroupSize()->hasValue() )
    {
        reqdWGSize[0] = kmd->getReqdWorkGroupSize()->getXDim(); // TODO: SExt <=> ZExt
        reqdWGSize[1] = kmd->getReqdWorkGroupSize()->getYDim(); // TODO: SExt <=> ZExt
        reqdWGSize[2] = kmd->getReqdWorkGroupSize()->getZDim(); // TODO: SExt <=> ZExt

        if(reqdWGSize[0])
        {
            optWGSize = 1;
            for(int i=0; i<MAX_WORK_DIM; ++i)
            {
                if(reqdWGSize[i])
                {
                    optWGSize*=reqdWGSize[i];
                }
            }
        }
    }

    // Check whether the kernel creates WI ids by itself (work group loops were not created by barrier)
    // This also means that this a 1-sise Jit (no vector kernel)
    std::string wrapperName = pWrapperFunc->getName().str();
    std::string vecPrefix = "__Vectorized_.";
    std::string wrapperVecName = vecPrefix + wrapperName;
    bool bJitCreateWIids = buildResult.GetNoBarrierSet().count(wrapperName);
    TLLVMKernelInfo localBufferInfo = buildResult.GetKernelsLocalBufferInfo(func);

    TKernelInfo info = buildResult.GetKernelsInfo(wrapperName);
    TKernelInfo vecinfo = buildResult.GetKernelsInfo(wrapperVecName);

    KernelProperties* pProps = new KernelProperties();

    pProps->SetOptWGSize(optWGSize);
    pProps->SetReqdWGSize(reqdWGSize);
    pProps->SetHintWGSize(hintWGSize);
    pProps->SetTotalImplSize(localBufferInfo.stTotalImplSize);
    pProps->SetHasBarrier( info.hasBarrier);
    size_t executionLength = std::max(vecinfo.kernelExecutionLength, info.kernelExecutionLength);
    pProps->SetKernelExecutionLength(executionLength);

    pProps->SetDAZ( pProgram->GetDAZ());
    pProps->SetCpuId( GetCompiler()->GetCpuId() );
    pProps->SetJitCreateWIids(bJitCreateWIids);

    // Private memory size contains the max size between
    // the needed size for scalar and needed size for vectorized versions.
    unsigned int privateMemorySize = buildResult.GetPrivateMemorySize().at(pWrapperFunc->getName().str());
    // TODO: This is workaround till the SDK hanlde case of zero private memory size!
    privateMemorySize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(std::max<unsigned int>(1, privateMemorySize));
    pProps->SetPrivateMemorySize( privateMemorySize );

    return pProps;
}



}}}
