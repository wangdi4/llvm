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

File Name:  MICProgramBuilder.cpp

\*****************************************************************************/
#include <set>
#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "ProgramBuilder.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "MICProgram.h"
#include "MICKernel.h"
#include "MICKernelProperties.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Target/TargetMachine.h"
#include "MICJITEngine/include/MICCodeGenerationEngine.h"
#include "MICJITEngine/include/ModuleJITHolder.h"
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
#include "MICProgramBuilder.h"
#include "ModuleJITHolder.h"
#include "MICJITContainer.h"
#include "CompilationUtils.h"

using std::string;
extern "C" void fillNoBarrierPathSet(llvm::Module *M, std::set<std::string>& noBarrierPath);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*
 * Utility methods
 */
namespace Utils 
{
 
}

MICProgramBuilder::MICProgramBuilder(IAbstractBackendFactory* pBackendFactory, const IMICCompilerConfig& config):
    ProgramBuilder(pBackendFactory, config),
    m_compiler(config)
{
}

MICProgramBuilder::~MICProgramBuilder()
{
}

MICKernel* MICProgramBuilder::CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const
{
    std::vector<cl_kernel_argument> arguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, arguments);

    return static_cast<MICKernel*>(m_pBackendFactory->CreateKernel( funcName, arguments, pProps ));
}

KernelSet* MICProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule, 
                                    ProgramBuildResult& buildResult) const
{
    buildResult.LogS() << "Build started\n";
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    llvm::NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");
    llvm::NamedMDNode *WrapperMD = pModule->getOrInsertNamedMetadata("opencl.wrappers");
    if ( !pModuleMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return spKernels.release();
    }

    std::set<std::string> noBarrier;
    fillNoBarrierPathSet(pModule, noBarrier);
    std::vector<FunctionWidthPair>::const_iterator vecIter = buildResult.GetFunctionsWidths().begin();
    for (unsigned i = 0, e = pModuleMetadata->getNumOperands(); i != e; ++i) 
    {
        // Obtain kernel function from annotation
        llvm::MDNode *elt = pModuleMetadata->getOperand(i);
        llvm::MDNode *welt = WrapperMD->getOperand(i);
        // We expect the metadata nodes to be llvm::Function
        // In case the cast is wrong an assertion failure will be thrown
        llvm::Function *pFunc = llvm::cast<llvm::Function>(elt->getOperand(0)->stripPointerCasts());
        // The wrapper function that receives a single buffer as argument is the last node in the metadata 
        llvm::Function *pWrapperFunc = llvm::cast<llvm::Function>(welt->getOperand(0)->stripPointerCasts());
        
        // Create a kernel and kernel JIT properties 
        std::auto_ptr<KernelProperties> spMICKernelProps( CreateKernelProperties( pProgram, 
                                                                                  pFunc, 
                                                                                  pWrapperFunc,
                                                                                  buildResult));
        // get the vector size used to generate the function
        unsigned int vecSize = 1;
        
        if( spMICKernelProps->GetJitCreateWIids() && vecIter != buildResult.GetFunctionsWidths().end())
        {
            vecSize = vecIter->second;
            spMICKernelProps->SetMinGroupSizeFactorial(vecSize);
        }
                                                                              
        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        // Create a kernel 
        std::auto_ptr<MICKernel> spKernel( CreateKernel( pFunc, 
                                                         pWrapperFunc->getName().str(),
                                                         spMICKernelProps.get()));
        spKernel->SetKernelID(i);

        AddKernelJIT( static_cast<const MICProgram*>(pProgram), 
                      spKernel.get(), 
                      pModule, 
                      pWrapperFunc, 
                      spKernelJITProps.release());

        // Check if vectorized kernel present
        if( !buildResult.GetFunctionsWidths().empty())
        {
            assert(vecIter != buildResult.GetFunctionsWidths().end());
            assert( !(spMICKernelProps->GetJitCreateWIids() && vecIter->first) &&
                "if the vector kernel is inlined the entry of the vector kernel should be NULL");

            const llvm::Type *vTypeHint = NULL; //pFunc->getVectTypeHint(); //TODO: R namespace micead from metadata (Guy)
            bool dontVectorize = false;

            if( NULL != vTypeHint)
            {
                //currently if the vector_type_hint attribute is set
                //we types that vector length is below 4, vectorizer restriction
                const llvm::VectorType* pVect = llvm::dyn_cast<llvm::VectorType>(vTypeHint);
                if( ( NULL != pVect) && pVect->getNumElements() >= 4)
                {
                    dontVectorize = true;
                }
            }

            if(NULL != vecIter->first && !dontVectorize)
            {
                //
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties>  spVKernelJITProps(CreateKernelJITProperties( vecIter->second));
                spMICKernelProps->SetMinGroupSizeFactorial(vecIter->second);
                AddKernelJIT( static_cast<const MICProgram*>(pProgram), spKernel.get(), pModule, vecIter->first, spVKernelJITProps.release());
            }
            if ( dontVectorize )
            {
                buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
            }
            else if ( vecIter->second <= 1)
            {
                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was not vectorized\n";
            }
            else 
            {
                buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized\n";
            }
            vecIter++;
        }
#ifdef OCL_DEV_BACKEND_PLUGINS  
        // Notify the plugin managerModuleJITHolder
        m_pluginManager.OnCreateKernel(pProgram, spKernel.get(), pFunc);
#endif
        spKernels->AddKernel(spKernel.release());
        spMICKernelProps.release();
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Iterating completed");
    
    buildResult.LogS() << "Done.";
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::INFO_LEVEL, L"Exit");
    return spKernels.release();
}

void MICProgramBuilder::AddKernelJIT( const MICProgram* pProgram, Kernel* pKernel, llvm::Module* pModule, llvm::Function* pFunc, KernelJITProperties* pProps) const
{
    IKernelJITContainer* pJIT = new MICJITContainer( pProgram->GetModuleJITHolder(),
                                                     (unsigned long long int)pFunc,
                                                     pProps);
    pKernel->AddKernelJIT( pJIT );
}

void MICProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule) const
{
    assert(spModule && "Invalid module for post optimization processing.");
    ModuleJITHolder* pModuleJIT = new ModuleJITHolder(); 
    std::auto_ptr<const llvm::ModuleJITHolder> spMICModuleJIT(m_compiler.GetModuleHolder(*spModule));

    // populate the Module JIT
    pModuleJIT->SetJITCodeSize(spMICModuleJIT->getJITCodeSize());
    pModuleJIT->SetJITCodeStartPoint(spMICModuleJIT->getJITCodeStartPoint());
    pModuleJIT->SetJITBufferPointer(spMICModuleJIT->getJITBufferPointer());
    pModuleJIT->SetJITAlignment(spMICModuleJIT->getJITCodeAlignment());

    for(llvm::KernelMap::const_iterator 
        it = spMICModuleJIT->getKernelMap().begin();
        it != spMICModuleJIT->getKernelMap().end();
        it++)
    {
        KernelInfo kernelInfo;
        kernelInfo.kernelOffset = it->second.offset;
        kernelInfo.kernelSize   = it->second.size;
        KernelID kernelID = (const KernelID)it->first;
        pModuleJIT->RegisterKernel(kernelID, kernelInfo);
    }
    static_cast<MICProgram*>(pProgram)->SetModuleJITHolder(pModuleJIT);
}

}}} // namespace

