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

#include "MICProgramBuilder.h"

#include "IAbstractBackendFactory.h"
#include "CompilationUtils.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "MetaDataApi.h"
#include "MICJITContainer.h"
#include "MICKernel.h"
#include "MICProgram.h"
#include "Program.h"
#include "MICSerializationService.h"
#include "BitCodeContainer.h"
#include "ObjectCodeContainer.h"
#include "cache_binary_handler.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/LLVMModuleJITHolder.h"
#include "llvm/Support/Casting.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
using namespace Intel::OpenCL::ELFUtils;

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
    std::vector<unsigned int>       memoryArguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, arguments, memoryArguments);

    return static_cast<MICKernel*>(m_pBackendFactory->CreateKernel( funcName, arguments, memoryArguments, pProps ));
}

bool MICProgramBuilder::ReloadProgramFromCachedExecutable(Program* pProgram)
{
    const char* pCachedObject =
        (char*)(pProgram->GetObjectCodeContainer()->GetCode());
    size_t cacheSize = pProgram->GetObjectCodeContainer()->GetCodeSize();
    assert(pCachedObject && "Object Code Container is null");

    // get sizes
    CacheBinaryReader reader(pCachedObject, cacheSize);
    size_t serializationSize = reader.GetSectionSize(g_metaSectionName);

    // get the buffers entries
    const char* bitCodeBuffer = (const char*)reader.GetSectionData(g_irSectionName);
    size_t bitCodeBufferSize = reader.GetSectionSize(g_irSectionName);
    const char* serializationBuffer = (const char*)reader.GetSectionData(g_metaSectionName);

    // Set IR
    BitCodeContainer* bcc = new BitCodeContainer(bitCodeBuffer, bitCodeBufferSize);
    pProgram->SetBitCodeContainer(bcc);

    llvm::Module* pModule = GetCompiler()->ParseModuleIR((llvm::MemoryBuffer*)bcc->GetMemoryBuffer());

    // create cache manager
    pProgram->SetModule(pModule);

    // deserialize the management objects
    std::auto_ptr<MICSerializationService> pMICSerializationService(new MICSerializationService(nullptr));
     pMICSerializationService->ReloadProgram(
       SERIALIZE_OFFLOAD_IMAGE,
       pProgram,
       serializationBuffer,
       serializationSize);

    // Reload Addresses
    IDynamicFunctionsResolver* pResolver = static_cast<IDynamicFunctionsResolver*>(GetCompiler()->GetFunctionAddressResolver());
    static_cast<MICProgram*>(pProgram)->GetModuleJITHolder()->RelocateSymbolAddresses(pResolver);

    // init refcounted runtime service shared storage between program and kernels
    RuntimeServiceSharedPtr lRuntimeService(new RuntimeServiceImpl);
    // set runtime service for the program
    pProgram->SetRuntimeService(lRuntimeService);

    // update kernels with RuntimeService
    Utils::UpdateKernelsWithRuntimeService( lRuntimeService, pProgram->GetKernelSet() );

    // update kernel mapper (OCL2.0) and run global ctors
    PostBuildProgramStep( pProgram, pModule, nullptr );
    return true;
}

void MICProgramBuilder::BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const
{
    assert(pProgram && "Program Object is null");

    // calculate the required buffer size
    size_t serializationSize = 0;
     std::auto_ptr<MICSerializationService> pMICSerializationService(new MICSerializationService(nullptr));
     pMICSerializationService->GetSerializationBlobSize(
        SERIALIZE_OFFLOAD_IMAGE, pProgram, &serializationSize);

    std::auto_ptr<CacheBinaryWriter> pWriter(new CacheBinaryWriter());

    // fill the IR bit code
    size_t irSize = pProgram->GetProgramIRCodeContainer()->GetCodeSize();
    const char* irStart = ((const char*)(pProgram->GetProgramIRCodeContainer()->GetCode()));
    pWriter->AddSection(g_irSectionName, irStart, irSize);

    // fill offload image in the object buffer
    std::vector<char> metaStart(serializationSize);
    pMICSerializationService->SerializeProgram(
        SERIALIZE_OFFLOAD_IMAGE,
        pProgram,
        &(metaStart[0]), serializationSize);
    pWriter->AddSection(g_metaSectionName, &(metaStart[0]), serializationSize);

    // get the binary
    size_t binarySize = pWriter->GetBinarySize();
    std::vector<char> pBinaryBlob(binarySize);
    if(pWriter->GetBinary(&(pBinaryBlob[0])))
    {
        ObjectCodeContainer* pObjectCodeContainer = new ObjectCodeContainer(&pBinaryBlob[0],binarySize);
        pProgram->SetObjectCodeContainer(pObjectCodeContainer);
    }
    else
    {
        pProgram->SetObjectCodeContainer(nullptr);
    }
}

KernelSet* MICProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule,
                                    ProgramBuildResult& buildResult) const
{
    buildResult.LogS() << "Build started\n";
    std::auto_ptr<KernelSet> spKernels( new KernelSet );

    MetaDataUtils mdUtils(pModule);

    MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels();
    MetaDataUtils::KernelsList::const_iterator e = mdUtils.end_Kernels();

    for (unsigned int kernelId = 0 ; i != e; ++i, ++kernelId)
    {
        // Obtain kernel function from annotation
        llvm::Function *pFunc = (*i)->getFunction(); // TODO: stripPointerCasts());
        KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
        // Obtain kernel wrapper function from metadata info
        llvm::Function *pWrapperFunc = kimd->getKernelWrapper(); //TODO: stripPointerCasts());

        // Create a kernel and kernel JIT properties
        std::auto_ptr<KernelProperties> spMICKernelProps( CreateKernelProperties( pProgram,
                                                                                  pFunc,
                                                                                  buildResult));
        // get the vector size used to generate the function
        unsigned int vecSize = kimd->isVectorizedWidthHasValue() ? kimd->getVectorizedWidth() : 1;
        spMICKernelProps->SetMinGroupSizeFactorial(vecSize);

        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        // Create a kernel
        std::auto_ptr<MICKernel> spKernel( CreateKernel( pFunc,
                                                         pWrapperFunc->getName().str(),
                                                         spMICKernelProps.get()));
        spKernel->SetKernelID(kernelId);

        AddKernelJIT( static_cast<const MICProgram*>(pProgram),
                      spKernel.get(),
                      pModule,
                      pWrapperFunc,
                      spKernelJITProps.release());

        //TODO (AABOUD): is this redundant code?
        const llvm::Type *vTypeHint = nullptr; //pFunc->getVectTypeHint(); //TODO: R namespace micead from metadata (Guy)
        bool dontVectorize = false;

        if( nullptr != vTypeHint)
        {
            //currently if the vector_type_hint attribute is set
            //we types that vector length is below 4, vectorizer restriction
            const llvm::VectorType* pVect = llvm::dyn_cast<llvm::VectorType>(vTypeHint);
            if( ( nullptr != pVect) && pVect->getNumElements() >= 4)
            {
                dontVectorize = true;
            }
        }

        //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
        //Vectorized is running in all scenarios.
        if (kimd->isVectorizedKernelHasValue())
        {
            Function *pVecFunc = kimd->getVectorizedKernel();
            assert(!(spMICKernelProps->IsVectorizedWithTail() && pVecFunc) &&
                   "if the vector kernel is inlined the entry of the vector "
                   "kernel should be NULL");

            if(nullptr != pVecFunc && !dontVectorize)
            {
                KernelInfoMetaDataHandle vkimd = mdUtils.getKernelsInfoItem(pVecFunc);
                // Obtain kernel wrapper function from metadata info
                llvm::Function *pWrapperVecFunc = vkimd->getKernelWrapper(); //TODO: stripPointerCasts());
                //Update vecSize according to vectorWidth of vectorized function
                vecSize = vkimd->getVectorizedWidth();
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties> spVKernelJITProps(CreateKernelJITProperties(vecSize));
                spMICKernelProps->SetMinGroupSizeFactorial(vecSize);
                AddKernelJIT(static_cast<const MICProgram*>(pProgram),
                              spKernel.get(),
                              pModule,
                              pWrapperVecFunc,
                              spVKernelJITProps.release());
            }
        }
        if ( dontVectorize )
        {
            buildResult.LogS() << "Vectorization of kernel <" << spKernel->GetKernelName() << "> was disabled by the developer\n";
        }
        else if (vecSize <= 1)
        {
            buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was not vectorized\n";
        }
        else
        {
            buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized (" <<
                spMICKernelProps->GetMinGroupSizeFactorial() << ")\n";
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

void MICProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const
{
    assert(spModule && "Invalid module for post optimization processing.");

    std::string dumpAsm("");
    if(nullptr != pOptions)
    {
      dumpAsm = pOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DUMPFILE, "");
    }
    std::auto_ptr<llvm::LLVMModuleJITHolder> spMICModuleJIT(m_compiler.GetModuleHolder(*spModule, dumpAsm));
    ModuleJITHolder* pModuleJIT = new ModuleJITHolder();

    CopyJitHolder(spMICModuleJIT.get(), pModuleJIT);

    static_cast<MICProgram*>(pProgram)->SetModuleJITHolder(pModuleJIT);
}

void MICProgramBuilder::CopyJitHolder(LLVMModuleJITHolder* from, ModuleJITHolder* to) const
{
    // Copy scalar values
    to->SetJITCodeSize(from->getJITCodeSize());
    to->SetJITAlignment(from->getJITCodeAlignment());

    // Copy memory chunks info
    std::auto_ptr<LLVMMemoryHolder> LML = from->getJITBufferHolder();
    MemoryHolder &ML = to->GetJITMemoryHolder();
    for (unsigned i = 0; i < LML->getNumChunks(); i++)
      ML.addChunk(LML->getOwnershipChunkPtr(i), LML->getChunkSize(i));

    // Copy kernels
    for(llvm::KernelMap::const_iterator it = from->getKernelMap().begin(),
        end = from->getKernelMap().end(); it != end; it++)
    {
        KernelInfo kernelInfo;
        llvm::KernelInfo llvmKernelInfo = it->second;

        // Copy basic values
        kernelInfo.functionId   = llvmKernelInfo.functionId;
        kernelInfo.kernelOffset = llvmKernelInfo.kernelOffset;
        kernelInfo.kernelSize   = llvmKernelInfo.kernelSize;
        kernelInfo.filename     = llvmKernelInfo.filename;

        // Copy line number table
        kernelInfo.lineNumberTable.resize(llvmKernelInfo.lineNumberTable.size());
        for (unsigned i = 0; i < llvmKernelInfo.lineNumberTable.size(); i++)
        {
            const llvm::LineNumberEntry& from = llvmKernelInfo.lineNumberTable[i];
            LineNumberEntry& to = kernelInfo.lineNumberTable[i];
            to.line = from.line;
            to.offset = from.offset;
        }

        // Copy inlined functions data
        // (can't use memcpy here as there are strings involved)
        kernelInfo.inlinedFunctions.resize(llvmKernelInfo.inlinedFunctions.size());
        for (unsigned i = 0; i < llvmKernelInfo.inlinedFunctions.size(); i++)
        {
            const llvm::InlinedFunction& from = llvmKernelInfo.inlinedFunctions[i];
            InlinedFunction& to = kernelInfo.inlinedFunctions[i];
            to.id = from.id;
            to.parentId = from.parentId;
            to.from = from.from;
            to.size = from.size;
            to.funcname = from.funcname;
            to.filename = from.filename;
        }

        KernelID kernelID = (const KernelID)it->first;
        to->RegisterKernel(kernelID, kernelInfo);
    }
    // Copy Relocation Table
    for(llvm::RelocationTable_t::const_iterator it = from->getRelocationTable().begin(),
        end = from->getRelocationTable().end(); it != end; it++)
    {
        RelocationInfo info;
        info.symName = (*it).symName;
        info.offset  = (*it).atOffset;

        to->RegisterRelocation(info);
    }
    // Copy Relocation Table
    for(llvm::DynRelocationTable_t::const_iterator it =
        from->getDynamicRelocationTable().begin(),
        end = from->getDynamicRelocationTable().end(); it != end; it++)
    {
        DynRelocationInfo info;
        info.offset = (*it).atOffset;
        info.addend = (*it).addend;

        to->RegisterDynRelocation(info);
    }
}

IBlockToKernelMapper * MICProgramBuilder::CreateBlockToKernelMapper(Program* pProgram, const llvm::Module* pModule) const
{
  assert(0 && "CreateBlockToKernelMapper not implemented");
  abort();
  return nullptr;
}
}}} // namespace
