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
#include "CompilationUtils.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include <algorithm>
#include <sstream>

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

        PostOptimizationProcessing(pProgram, spModule.get(), pOptions);

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
        updateGlobalVariableTotalSize(pProgram, spModule.get());
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

void ProgramBuilder::updateGlobalVariableTotalSize(Program* pProgram, Module* pModule)
{
    MetaDataUtils mdUtils(pModule);
    if (mdUtils.empty_ModuleInfoList()) return;
    Intel::ModuleInfoMetaDataHandle handle = mdUtils.getModuleInfoListItem(0);
    pProgram->SetGlobalVariableTotalSize(handle->getGlobalVariableTotalSize());
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

    std::stringstream kernelAttributes;

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

        kernelAttributes << "work_group_size_hint(";
        kernelAttributes << hintWGSize[0];
        kernelAttributes << ",";
        kernelAttributes << hintWGSize[1];
        kernelAttributes << ",";
        kernelAttributes << hintWGSize[2];
        kernelAttributes << ")";
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

        if (!kernelAttributes.str().empty()) kernelAttributes << " ";
        kernelAttributes << "reqd_work_group_size(";
        kernelAttributes << reqdWGSize[0];
        kernelAttributes << ",";
        kernelAttributes << reqdWGSize[1];
        kernelAttributes << ",";
        kernelAttributes << reqdWGSize[2];
        kernelAttributes << ")";
    }

    if (kmd->isVecTypeHintHasValue())
    {
      Type* VTHTy = kmd->getVecTypeHint()->getType();

      int vecSize = 1;

      if (VTHTy->isVectorTy()) {
        vecSize = VTHTy->getVectorNumElements();
        VTHTy = VTHTy->getVectorElementType();
      }

      if (!kernelAttributes.str().empty()) kernelAttributes << " ";
      kernelAttributes << "vec_type_hint(";

      // Temporal patch - MetaDataApi doesn't support this for now
      // so dig down to get the signedness from the metadata
      // Expected metadata format for vec_type_hint:
      // !8 = metadata !{metadata !"vec_type_hint", <8 x i32> undef, i32 0}
      //                                tag^       type^         isSigned^
      llvm::NamedMDNode *MDArgInfo = pModule->getNamedMetadata("opencl.kernels");
      MDNode *FuncInfo = NULL;
      for (int i = 0, e = MDArgInfo->getNumOperands(); i < e; i++) {
        FuncInfo = MDArgInfo->getOperand(i);
        Value *field0 = FuncInfo->getOperand(0)->stripPointerCasts();

        if(func == dyn_cast<Function>(field0))
          break;
      }
      assert(FuncInfo && "Couldn't find this kernel in the kernel list");
      if(NULL == FuncInfo)
        throw Exceptions::CompilerException("Internal Error. FuncInfo is NULL");
      
      MDNode *MDVecTHint = NULL;
      //look for vec_type_hint metadata
      for (int i = 1, e = FuncInfo->getNumOperands(); i < e; i++) {
        MDNode *tmpMD = dyn_cast<MDNode>(FuncInfo->getOperand(i));
        MDString *tag = (tmpMD ? dyn_cast<MDString>(tmpMD->getOperand(0)) : NULL);
        if (tag && (tag->getString() == "vec_type_hint")) {
          MDVecTHint = tmpMD;
          break;
        }
      }
      assert(MDVecTHint && "vec_type_hint info isn't available for this kernel");
      if(NULL == MDVecTHint)
        throw Exceptions::CompilerException("Internal Error. MDVecTHint is NULL");

      // Look for operand 2 - isSigned
      ConstantInt *isSigned = dyn_cast<ConstantInt>(MDVecTHint->getOperand(2));
      assert(isSigned && "isSigned should be a constant integer value");

      if (isSigned && isSigned->isZero())
        kernelAttributes << "u";

      if (VTHTy->isFloatTy()) {
        kernelAttributes << "float";
      } else if (VTHTy->isDoubleTy()) {
        kernelAttributes << "double";
      } else if (VTHTy->isIntegerTy(8)) {
        kernelAttributes << "char";
      } else if (VTHTy->isIntegerTy(16)) {
        kernelAttributes << "short";
      } else if (VTHTy->isIntegerTy(32)) {
        kernelAttributes << "int";
      } else if (VTHTy->isIntegerTy(64)) {
        kernelAttributes << "long";
      }

      if (vecSize>1) kernelAttributes << vecSize;
      kernelAttributes << ")";
    }

    KernelInfoMetaDataHandle skimd = mdUtils.getKernelsInfoItem(func);
    //Need to check if NoBarrierPath Value exists, it is not guaranteed that
    //KernelAnalysisPass is running in all scenarios.
    const bool HasNoBarrierPath = skimd->isNoBarrierPathHasValue() && skimd->getNoBarrierPath();
    const unsigned int localBufferSize = skimd->getLocalBufferSize();
    const bool hasBarrier = skimd->getKernelHasBarrier();
    const bool hasGlobalSync = skimd->getKernelHasGlobalSync();
    const size_t scalarExecutionLength = skimd->getKernelExecutionLength();
    const unsigned int scalarBufferStride = skimd->getBarrierBufferSize();
    unsigned int privateMemorySize = skimd->getPrivateMemorySize();

    size_t vectorExecutionLength = 0;
    unsigned int vectorBufferStride = 0;
    //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
    //Vectorized is running in all scenarios.
    if (skimd->isVectorizedKernelHasValue() && skimd->getVectorizedKernel() != NULL) {
      KernelInfoMetaDataHandle vkimd = mdUtils.getKernelsInfoItem(skimd->getVectorizedKernel());
      vectorExecutionLength = vkimd->getKernelExecutionLength();
      vectorBufferStride = vkimd->getBarrierBufferSize();
      privateMemorySize = std::max<unsigned int>(privateMemorySize, vkimd->getPrivateMemorySize());
    }

    // Execution length contains the max size between
    // the length of scalar and the length of vectorized versions.
    const size_t executionLength = std::max(scalarExecutionLength, vectorExecutionLength);
    // Private memory size contains the max size between
    // the needed size for scalar and needed size for vectorized versions.
    unsigned int barrierBufferSize = std::max<unsigned int>(scalarBufferStride, vectorBufferStride);
    // Aligh barrier buffer and private memory size
    barrierBufferSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(barrierBufferSize);
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
    pProps->SetHasGlobalSync(hasGlobalSync);
    pProps->SetKernelExecutionLength(executionLength);
    pProps->SetKernelAttributes(kernelAttributes.str());

    pProps->SetDAZ(pProgram->GetDAZ());
    pProps->SetCpuId(GetCompiler()->GetCpuId());
    if (HasNoBarrierPath)
      pProps->EnableVectorizedWithTail();

    pProps->SetBarrierBufferSize(barrierBufferSize);
    // CSSD100016517 workaround:
    //   GetPrivateMemorySize returns the min. required private memory
    //   size per work-item even if there are no work-group level built-ins.
    pProps->SetPrivateMemorySize(privateMemorySize);

    // set isBlock property
    pProps->SetIsBlock(BlockUtils::IsBlockInvocationKernel(*func));

    // OpenCL 2.0 related properties
    if(OclVersion::CL_VER_2_0 <= CompilationUtils::getCLVersionFromModuleOrDefault(*pModule)  &&
       CompilationUtils::fetchCompilerOption(*pModule, "-cl-uniform-work-group-size").empty()) {
      pProps->SetIsNonUniformWGSizeSupported(true);
    } else {
      pProps->SetIsNonUniformWGSizeSupported(false);
    }

    return pProps;
}



}}}
