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

} //namespace Utils 

ProgramBuilder::ProgramBuilder(IAbstractBackendFactory* pBackendFactory):
    m_pBackendFactory(pBackendFactory)
{
}

ProgramBuilder::~ProgramBuilder()
{
}

cl_dev_err_code ProgramBuilder::BuildProgram(Program* pProgram, const ProgramBuilderBuildOptions* pOptions)
{
    assert(pProgram && "Program parameter must not be NULL");
    ProgramBuildResult buildResult;

    try
    {
        Compiler* pCompiler = GetCompiler();


        CompilerBuildOptions buildOptions( pProgram->GetDebugInfoFlag(),
                                               pProgram->GetDisableOpt(),
                                               pProgram->GetFastRelaxedMath(), false);

        std::auto_ptr<llvm::Module> spModule( pCompiler->BuildProgram( Utils::GetProgramMemoryBuffer(pProgram),
                                                                       &buildOptions,
                                                                       &buildResult));

        PostOptimizationProcessing(pProgram, spModule.get());

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

KernelProperties* ProgramBuilder::CreateKernelProperties(const Program* pProgram,
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
    pProps->SetCpuId( GetCompiler()->GetSelectedCPU());
    pProps->SetCpuFeatures( GetCompiler()->GetSelectedCPUFeatures() );

    return pProps;
}

 

}}}
