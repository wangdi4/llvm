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

File Name:  CPUProgramBuilder.cpp

\*****************************************************************************/

#include "CPUBlockToKernelMapper.h"
#include "CPUJITContainer.h"
#include "CPUProgramBuilder.h"
#include "CompilationUtils.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "MetaDataApi.h"
#include "Program.h"
#include "StaticObjectLoader.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include "BitCodeContainer.h"
#include "CPUSerializationService.h"
#include "ObjectCodeContainer.h"
#include "cache_binary_handler.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

namespace Utils
{
/**
 * Returns the true if the given function name is a kernel function in the given module
 */
bool IsKernel(llvm::Module* pModule, const char* szFuncName)
{
    MetaDataUtils mdUtils(pModule);

    for( MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels(), e = mdUtils.end_Kernels(); i != e; ++i )
    {
        llvm::Function *pFuncVal = NULL;
        // Obtain kernel function from annotation
        if( (*i)->isFunctionHasValue() )
        {
            pFuncVal = (*i)->getFunction();
        }

        //TODO: check stripPointerCasts()
        if ( NULL == pFuncVal )
        {
            continue;   // Not a function pointer
        }

        if ( !pFuncVal->getName().compare(szFuncName) )
        {
            return true;
        }
    }

    // Function not found
    return false;
}

}
CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& config):
    ProgramBuilder(pBackendFactory, config),
    m_compiler(config)
{
}

CPUProgramBuilder::~CPUProgramBuilder()
{
}

void CPUProgramBuilder::BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const
{
    assert(pCache && "Object Cache is null");
    assert(pProgram && "Program Object is null");

    if(pCache->getCachedModule().empty() || NULL == pCache->getCachedObject())
    {
        pProgram->SetObjectCodeContainer(NULL);
        return ;
    }

    // calculate the required buffer size
    size_t serializationSize = 0;
    std::auto_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(NULL));
    pCPUSerializationService->GetSerializationBlobSize(
        SERIALIZE_PERSISTENT_IMAGE, pProgram, &serializationSize);

    size_t irSize = pProgram->GetProgramIRCodeContainer()->GetCodeSize();
    size_t optModuleSize = pCache->getCachedModule().size();
    size_t objSize = pCache->getCachedObject()->getBufferSize();

    std::auto_ptr<CacheBinaryHandler::CacheBinaryWriter> pWriter(new CacheBinaryHandler::CacheBinaryWriter());

    // fill the IR bit code
    const char* irStart = ((const char*)(pProgram->GetProgramIRCodeContainer()->GetCode()));
    pWriter->AddSection(CacheBinaryHandler::g_irSectionName, irStart, irSize);

    // fill offload image in the object buffer
    std::vector<char> metaStart(serializationSize);
    pCPUSerializationService->SerializeProgram(
        SERIALIZE_PERSISTENT_IMAGE, 
        pProgram,
        &(metaStart[0]), serializationSize);
    pWriter->AddSection(CacheBinaryHandler::g_metaSectionName, &(metaStart[0]), serializationSize);

    // fill the raw module bits
    const std::string& optModule = pCache->getCachedModule();
    pWriter->AddSection(CacheBinaryHandler::g_optSectionName, &optModule[0], optModuleSize);

    // fill the Object bits
    const char* objStart = pCache->getCachedObject()->getBuffer().data();
    pWriter->AddSection(CacheBinaryHandler::g_objSectionName, objStart, objSize);

    // get the binary
    size_t binarySize = pWriter->GetBinarySize();
    std::vector<char> pBinaryBlob(binarySize+sizeof(cl_prog_container_header));
    if(pWriter->GetBinary(&(pBinaryBlob[0])+sizeof(cl_prog_container_header)))
    {
        ((cl_prog_container_header*)&(pBinaryBlob[0]))->container_size = binarySize;
        ObjectCodeContainer* pObjectCodeContainer = new ObjectCodeContainer((cl_prog_container_header*)&(pBinaryBlob[0]));
        pProgram->SetObjectCodeContainer(pObjectCodeContainer);
    }
    else
    {
        pProgram->SetObjectCodeContainer(NULL);
    }
}

void CPUProgramBuilder::ReloadProgramFromCachedExecutable(Program* pProgram)
{
    const char* pCachedObject = 
        (char*)(pProgram->GetObjectCodeContainer()->GetCode());
    size_t cacheSize = pProgram->GetObjectCodeContainer()->GetCodeSize();
    assert(pCachedObject && "Object Code Container is null");

    // get sizes
    CacheBinaryHandler::CacheBinaryReader reader = CacheBinaryHandler::CacheBinaryReader(pCachedObject,cacheSize);
    size_t serializationSize = reader.GetSectionSize(CacheBinaryHandler::g_metaSectionName);
    size_t optModuleSize = reader.GetSectionSize(CacheBinaryHandler::g_optSectionName);
    size_t objectSize = reader.GetSectionSize(CacheBinaryHandler::g_objSectionName);

    // get the buffers entries
    const char* bitCodeBuffer = (const char*)reader.GetSectionData(CacheBinaryHandler::g_irSectionName);
    const char* serializationBuffer = (const char*)reader.GetSectionData(CacheBinaryHandler::g_metaSectionName);
    const char* optModuleBuffer = (const char*)reader.GetSectionData(CacheBinaryHandler::g_optSectionName);
    const char* objectBuffer = (const char*)reader.GetSectionData(CacheBinaryHandler::g_objSectionName);

    // Set IR
    BitCodeContainer* bcc = new BitCodeContainer((const cl_prog_container_header*)bitCodeBuffer);
    pProgram->SetBitCodeContainer(bcc);

    // update the builtin module
    pProgram->SetBuiltinModule(GetCompiler()->GetRtlModule());

    // parse the optimized module
    llvm::StringRef data = llvm::StringRef(optModuleBuffer, optModuleSize);
    std::auto_ptr<llvm::MemoryBuffer> Buffer(llvm::MemoryBuffer::getMemBufferCopy(data));

    llvm::Module* pModule = GetCompiler()->ParseModuleIR(Buffer.get());
    GetCompiler()->CreateExecutionEngine(pModule);

    llvm::ExecutionEngine* pEngine = (llvm::ExecutionEngine*)GetCompiler()->GetExecutionEngine();

    // create cache manager
    pProgram->SetExecutionEngine(pEngine);
    pProgram->SetModule(pModule);

    ObjectCodeCache* pCache = new ObjectCodeCache((llvm::Module*)pProgram->GetModule(), objectBuffer, objectSize);
    pEngine->setObjectCache(pCache);

    // deserialize the management objects
    std::auto_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(NULL));
    pCPUSerializationService->ReloadProgram(
        SERIALIZE_PERSISTENT_IMAGE,
        pProgram, 
        serializationBuffer,
        serializationSize); 

    // init refcounted runtime service shared storage between program and kernels
    RuntimeServiceSharedPtr lRuntimeService =
                          RuntimeServiceSharedPtr(new RuntimeServiceImpl);
    // set runtime service for the program
    pProgram->SetRuntimeService(lRuntimeService);

    // update kernels with RuntimeService
    Utils::UpdateKernelsWithRuntimeService( lRuntimeService, pProgram->GetKernelSet() );

    // update kernel mapper (OCL2.0)
    PostBuildProgramStep( pProgram, pModule, NULL );
}

Kernel* CPUProgramBuilder::CreateKernel(llvm::Function* pFunc, const std::string& funcName, KernelProperties* pProps) const
{
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,  pFunc, arguments, memoryArguments);

    return m_pBackendFactory->CreateKernel( funcName, arguments, memoryArguments, pProps );
}

KernelSet* CPUProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule,
                                    ProgramBuildResult& buildResult) const
{
    std::auto_ptr<KernelSet> spKernels( new KernelSet );
    MetaDataUtils mdUtils(pModule);

    MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels();
    MetaDataUtils::KernelsList::const_iterator e = mdUtils.end_Kernels();

    for ( ; i != e; ++i)
    {
        // Obtain kernel function from annotation
        llvm::Function *pFunc = (*i)->getFunction(); // TODO: stripPointerCasts());
        KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
        // Obtain kernel wrapper function from metadata info
        llvm::Function *pWrapperFunc = kimd->getKernelWrapper(); //TODO: stripPointerCasts());

        // Create a kernel and kernel JIT properties
        std::auto_ptr<KernelProperties> spKernelProps( CreateKernelProperties( pProgram,
                                                                               pFunc,
                                                                               buildResult));

        // get the vector size used to generate the function
        unsigned int vecSize = kimd->isVectorizedWidthHasValue() ? kimd->getVectorizedWidth() : 1;
        spKernelProps->SetMinGroupSizeFactorial(vecSize);

        std::auto_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        std::auto_ptr<Kernel> spKernel( CreateKernel( pFunc,
                                                      pWrapperFunc->getName().str(),
                                                      spKernelProps.get()));

        // We want the JIT of the wrapper function to be called
        AddKernelJIT(static_cast<CPUProgram*>(pProgram),
                     spKernel.get(),
                     pModule,
                     pWrapperFunc,
                     spKernelJITProps.release());


        //TODO (AABOUD): is this redundant code?
        const llvm::Type *vTypeHint = NULL; //pFunc->getVectTypeHint(); //TODO: Read from metadata (Guy)
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

        //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
        //Vectorized is running in all scenarios.
        if (kimd->isVectorizedKernelHasValue())
        {
            Function *pVecFunc = kimd->getVectorizedKernel();
            assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
                   "if the vector kernel is inlined the entry of the vector "
                   "kernel should be NULL");
            if(NULL != pVecFunc && !dontVectorize)
            {
                KernelInfoMetaDataHandle vkimd = mdUtils.getKernelsInfoItem(pVecFunc);
                // Obtain kernel wrapper function from metadata info
                llvm::Function *pWrapperVecFunc = vkimd->getKernelWrapper(); //TODO: stripPointerCasts());
                //Update vecSize according to vectorWidth of vectorized function
                vecSize = vkimd->getVectorizedWidth();
                // Create the vectorized kernel - no need to pass argument list here
                std::auto_ptr<KernelJITProperties> spVKernelJITProps(CreateKernelJITProperties(vecSize));
                spKernelProps->SetMinGroupSizeFactorial(vecSize);
                AddKernelJIT(static_cast<CPUProgram*>(pProgram),
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
            buildResult.LogS() << "Kernel <" << spKernel->GetKernelName() << "> was successfully vectorized\n";
        }
#ifdef OCL_DEV_BACKEND_PLUGINS
        // Notify the plugin manager
        m_pluginManger.OnCreateKernel(pProgram, spKernel.get(), pFunc);
#endif
        spKernels->AddKernel(spKernel.release());
        spKernelProps.release();
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Iterating completed");

    buildResult.LogS() << "Done.";
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::INFO_LEVEL, L"Exit");
    return spKernels.release();
}


void CPUProgramBuilder::AddKernelJIT(CPUProgram* pProgram, Kernel* pKernel, llvm::Module* pModule,
                                     llvm::Function* pFunc, KernelJITProperties* pProps) const
{
    IKernelJITContainer* pJIT = new CPUJITContainer( pProgram->GetPointerToFunction(pFunc),
                                                     pFunc,
                                                     pModule,
                                                     pProps);
    pKernel->AddKernelJIT( pJIT );
}

void CPUProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* pModule, const ICLDevBackendOptions* pOptions) const
{
    char*  pInjectedObjStart = NULL;
    size_t injectedObjSize;

    // Check if we are going to do injection
    if (pOptions
        && pOptions->GetValue(CL_DEV_BACKEND_OPTION_INJECTED_OBJECT, &pInjectedObjStart, &injectedObjSize)
        && pInjectedObjStart != NULL)
    {
#ifdef ENABLE_KNL
        LoadObject(pProgram, pModule, pInjectedObjStart, injectedObjSize);
#else
        std::auto_ptr<StaticObjectLoader> pObjectLoader(new StaticObjectLoader());
        // Build the MemoryBuffer object from the supplied options
        std::auto_ptr<llvm::MemoryBuffer> pInjectedObj(
            llvm::MemoryBuffer::getMemBuffer( llvm::StringRef(pInjectedObjStart, injectedObjSize)) );

        pObjectLoader->addPreCompiled(spModule, pInjectedObj.release());
        // Add the injected object to the execution engine cache
        CPUProgram* pCPUProgram = static_cast<CPUProgram*>(pProgram);
        pCPUProgram->GetExecutionEngine()->setObjectCache(pObjectLoader.release());
#endif //ENABLE_KNL
    }
}

#ifdef ENABLE_KNL
void CPUProgramBuilder::LoadObject(Program* pProgram, llvm::Module* spModule,
                                   const char *pInjectedObjStart,
                                   size_t injectedObjSize) const
{
    std::auto_ptr<StaticObjectLoader> pObjectLoader(new StaticObjectLoader());
    // Build the MemoryBuffer object from the supplied options
    std::auto_ptr<llvm::MemoryBuffer> pInjectedObj(
        llvm::MemoryBuffer::getMemBuffer( llvm::StringRef(pInjectedObjStart, injectedObjSize)) );

    pObjectLoader->addPreCompiled(spModule, pInjectedObj.release());
    // Add the injected object to the execution engine cache
    CPUProgram* pCPUProgram = static_cast<CPUProgram*>(pProgram);
    pCPUProgram->GetExecutionEngine()->setObjectCache(pObjectLoader.release());
}
#endif //ENABLE_KNL

IBlockToKernelMapper * CPUProgramBuilder::CreateBlockToKernelMapper(Program* pProgram, const llvm::Module* pModule) const
{
    return new CPUBlockToKernelMapper(pProgram, pModule);
}


void CPUProgramBuilder::PostBuildProgramStep(Program* pProgram, llvm::Module* pModule,
  const ICLDevBackendOptions* pOptions) const
{
  assert(pProgram && pModule && "inputs are NULL");

  // create block to kernel mapper
  IBlockToKernelMapper * pMapper = CreateBlockToKernelMapper(pProgram, pModule);
  assert(pMapper && "IBlockToKernelMapper object is NULL");
  assert(!pProgram->GetRuntimeService().isNull() && "RuntimeService in Program is NULL");
  // set in RuntimeService new BlockToKernelMapper object
  pProgram->GetRuntimeService()->SetBlockToKernelMapper(pMapper);
}

}}} // namespace
