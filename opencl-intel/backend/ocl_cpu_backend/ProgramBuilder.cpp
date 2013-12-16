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
#define DEBUG_TYPE "ProgramBuilder"

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
#include "BlockUtils.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Debug.h"
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

#ifdef ENABLE_KNL
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#endif //ENABLE_KNL

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

/// @brief helper funtion to set RuntimeService in Kernel objects from KernelSet
void UpdateKernelsWithRuntimeService( const RuntimeServiceSharedPtr& rs, KernelSet * pKernels )
{
  for(unsigned cnt = 0; cnt < pKernels->GetCount(); ++cnt){
    Kernel * pK = pKernels->GetKernel(cnt);
    pK->SetRuntimeService(rs);
  }
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

        // init refcounted runtime service shared storage between program and kernels
        RuntimeServiceSharedPtr lRuntimeService =
                          RuntimeServiceSharedPtr(new RuntimeServiceImpl);
        // set runtime service for the program
        pProgram->SetRuntimeService(lRuntimeService);
#ifdef ENABLE_KNL
        std::string buffer, filename("/tmp/kernel");
        llvm::raw_string_ostream stream(buffer);
        stream << (*spModule.get());
        std::ostringstream s;
        s << getpid();
        filename += s.str();
        stream.flush();
        {
            std::ofstream outf((filename+".ll").c_str());
            outf << buffer;
        }

        std::string llvmOCLBinPath(getenv("LLVM_OCL_DIR"));
        int res = system((llvmOCLBinPath + "/llvm-as " + filename + ".ll").c_str());
        if (res) {
          system(("mv " + filename + ".ll " + filename + "_fail.ll").c_str());
          throw Exceptions::DeviceBackendExceptionBase("llvm-as does not work", CL_DEV_ERROR_FAIL);
        }
        std::string llvmKNLBinPath(getenv("LLVM_KNL_DIR"));
        res = system((llvmKNLBinPath + "/llvm-dis " + filename + ".bc").c_str());
        if (res) {
          system(("mv " + filename + ".ll " + filename + "_fail.ll").c_str());
          system(("mv " + filename + ".bc " + filename + "_fail.bc").c_str());
          throw Exceptions::DeviceBackendExceptionBase("llvm-dis does not work", CL_DEV_ERROR_FAIL);
        }

        std::string llcOptions("-mcpu=knl -relocation-model=pic -force-align-stack -fp-contract=fast ");
        llcOptions += filename + ".ll ";
        llcOptions += "-filetype=obj -o " + filename + ".o";
        res = system((llvmKNLBinPath + "/llc " + llcOptions).c_str());
        if (res != 0) {
          system(("mv " + filename + ".ll " + filename + "_fail.ll").c_str());
          system(("mv " + filename + ".bc " + filename + "_fail.bc").c_str());
          throw Exceptions::DeviceBackendExceptionBase("llc does not work", CL_DEV_ERROR_FAIL);
        }
        llvm::OwningPtr<llvm::MemoryBuffer> injectedObject;
        if (llvm::MemoryBuffer::getFile((filename + ".o").c_str(), injectedObject)) {
          system(std::string("rm " + filename + ".bc " + filename + ".ll").c_str());
          throw Exceptions::DeviceBackendExceptionBase("can't find object file",
                                                       CL_DEV_ERROR_FAIL);
        }
        LoadObject(pProgram, spModule.get(), injectedObject->getBufferStart(),
                   injectedObject->getBufferSize());
        system(std::string("rm " + filename + ".bc " + filename + ".ll " + filename + ".o").c_str());
#else // ENABLE_KNL
        PostOptimizationProcessing(pProgram, spModule.get(), pOptions);
#endif // ENABLE_KNL
        if (!(pOptions && pOptions->GetBooleanValue(CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT, false)))
        {
            //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Start iterating over kernels");
            KernelSet* pKernels = CreateKernels( pProgram,
                                                 spModule.get(),
                                                 buildResult);
            // update kernels with RuntimeService
            Utils::UpdateKernelsWithRuntimeService( lRuntimeService, pKernels );

            pProgram->SetKernelSet( pKernels );
        }

        // call post build method
        PostBuildProgramStep( pProgram, spModule.get(), pOptions );
        pProgram->SetModule( spModule.release() );

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

    if(NULL == kmd.get())
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

    KernelInfoMetaDataHandle skimd = mdUtils.getKernelsInfoItem(func);
    //Need to check if NoBarrierPath Value exists, it is not guaranteed that
    //KernelAnalysisPass is running in all scenarios.
    const bool bJitCreateWIids = skimd->isNoBarrierPathHasValue() && skimd->getNoBarrierPath();
    const unsigned int localBufferSize = skimd->getLocalBufferSize();
    const bool hasBarrier = skimd->getKernelHasBarrier();
    const size_t scalarExecutionLength = skimd->getKernelExecutionLength();
    const unsigned int scalarBufferStride = skimd->getBarrierBufferSize();

    size_t vectorExecutionLength = 0;
    unsigned int vectorBufferStride = 0;
    //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
    //Vectorized is running in all scenarios.
    if (skimd->isVectorizedKernelHasValue() && skimd->getVectorizedKernel() != NULL) {
      KernelInfoMetaDataHandle vkimd = mdUtils.getKernelsInfoItem(skimd->getVectorizedKernel());
      vectorExecutionLength = vkimd->getKernelExecutionLength();
      vectorBufferStride = vkimd->getBarrierBufferSize();
    }
    // Execution length contains the max size between
    // the length of scalar and the length of vectorized versions.
    const size_t executionLength = std::max(scalarExecutionLength, vectorExecutionLength);
    // Private memory size contains the max size between
    // the needed size for scalar and needed size for vectorized versions.
    unsigned int privateMemorySize = std::max<unsigned int>(scalarBufferStride, vectorBufferStride);
    privateMemorySize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(privateMemorySize);

    KernelProperties* pProps = new KernelProperties();

    // Kernel should keep size of pointer specified inside module
    // to allow cross-platform compilation
    unsigned int ptrSizeInBytes = pModule->getPointerSize()*4;
    pProps->SetPointerSize(ptrSizeInBytes);

    pProps->SetOptWGSize(optWGSize);
    pProps->SetReqdWGSize(reqdWGSize);
    pProps->SetHintWGSize(hintWGSize);
    pProps->SetTotalImplSize(localBufferSize);
    pProps->SetHasBarrier(hasBarrier);
    pProps->SetKernelExecutionLength(executionLength);

    pProps->SetDAZ(pProgram->GetDAZ());
    pProps->SetCpuId(GetCompiler()->GetCpuId());
    pProps->SetJitCreateWIids(bJitCreateWIids);

    pProps->SetPrivateMemorySize(privateMemorySize);

    // set isBlock property
    pProps->SetIsBlock(BlockUtils::IsBlockInvocationKernel(*func));

    return pProps;
}



}}}
