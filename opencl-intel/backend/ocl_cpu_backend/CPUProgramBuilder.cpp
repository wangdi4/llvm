// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CPUBlockToKernelMapper.h"
#include "CPUJITContainer.h"
#include "CPUProgramBuilder.h"
#include "CompilationUtils.h"
#include "debuggingservicetype.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "MetadataAPI.h"
#include "Program.h"
#include "StaticObjectLoader.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"

#include "BitCodeContainer.h"
#include "CPUSerializationService.h"
#include "ObjectCodeContainer.h"
#include "cache_binary_handler.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

using namespace Intel::OpenCL::ELFUtils;

CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory *pBackendFactory,
                                     const ICompilerConfig &config)
    : ProgramBuilder(pBackendFactory, config), m_compiler(config),
      m_isFpgaEmulator(FPGA_EMU_DEVICE == config.TargetDevice()),
      m_isEyeQEmulator(EYEQ_EMU_DEVICE == config.TargetDevice()) {}

CPUProgramBuilder::~CPUProgramBuilder()
{
}

void CPUProgramBuilder::BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const
{
    assert(pCache && "Object Cache is null");
    assert(pProgram && "Program Object is null");

    if(pCache->getCachedModule().empty() || nullptr == pCache->getCachedObject())
    {
        pProgram->SetObjectCodeContainer(nullptr);
        return ;
    }

    // calculate the required buffer size
    size_t serializationSize = 0;
    std::auto_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(nullptr));
    pCPUSerializationService->GetSerializationBlobSize(
        SERIALIZE_PERSISTENT_IMAGE, pProgram, &serializationSize);

    size_t irSize = pProgram->GetProgramIRCodeContainer()->GetCodeSize();
    size_t optModuleSize = pCache->getCachedModule().size();
    size_t objSize = pCache->getCachedObject()->getBufferSize();

    CLElfLib::E_EH_MACHINE bitOS = m_compiler.GetCpuId().Is64BitOS() ? CLElfLib::EM_X86_64 : CLElfLib::EM_860;

    //Checking maximum supported instruction
    CLElfLib::E_EH_FLAGS maxSupportedVectorISA = CLElfLib::EH_FLAG_SSE4;
    if (m_compiler.GetCpuId().HasAVX512ICL())
        maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_ICL;
    else if (m_compiler.GetCpuId().HasAVX512SKX())
        maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_SKX;
    else if (m_compiler.GetCpuId().HasAVX2())
        maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX2;
    else if (m_compiler.GetCpuId().HasAVX1())
        maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX1;

    std::auto_ptr<CacheBinaryWriter> pWriter(new CacheBinaryWriter(bitOS, maxSupportedVectorISA));

    // fill the IR bit code
    const char* irStart = ((const char*)(pProgram->GetProgramIRCodeContainer()->GetCode()));
    pWriter->AddSection(g_irSectionName, irStart, irSize);

    // fill offload image in the object buffer
    std::vector<char> metaStart(serializationSize);
    pCPUSerializationService->SerializeProgram(
        SERIALIZE_PERSISTENT_IMAGE,
        pProgram,
        &(metaStart[0]), serializationSize);
    pWriter->AddSection(g_metaSectionName, &(metaStart[0]), serializationSize);

    // fill the raw module bits
    const std::string& optModule = pCache->getCachedModule();
    pWriter->AddSection(g_optSectionName, &optModule[0], optModuleSize);

    // fill the Object bits
    const char* objStart = pCache->getCachedObject()->getBuffer().data();
    pWriter->AddSection(g_objSectionName, objStart, objSize);

    // fill the Version section
    unsigned int currentVersion = OCL_CACHED_BINARY_VERSION;
    pWriter->AddSection(g_objVerSectionName, (char*)&currentVersion, sizeof(unsigned int));

    // get the binary
    size_t binarySize = pWriter->GetBinarySize();
    std::vector<char> pBinaryBlob(binarySize);
    if(pWriter->GetBinary(&(pBinaryBlob[0])))
    {
        ObjectCodeContainer* pObjectCodeContainer = new ObjectCodeContainer(&pBinaryBlob[0], binarySize);
        pProgram->SetObjectCodeContainer(pObjectCodeContainer);
    }
    else
    {
        pProgram->SetObjectCodeContainer(nullptr);
    }
}

bool CPUProgramBuilder::ReloadProgramFromCachedExecutable(Program* pProgram)
{
    const char* pCachedObject =
        (char*)(pProgram->GetObjectCodeContainer()->GetCode());
    size_t cacheSize = pProgram->GetObjectCodeContainer()->GetCodeSize();
    assert(pCachedObject && "Object Code Container is null");

    // get sizes
    CacheBinaryReader reader(pCachedObject,cacheSize);
    size_t serializationSize = reader.GetSectionSize(g_metaSectionName);
    size_t optModuleSize = reader.GetSectionSize(g_optSectionName);
    size_t objectSize = reader.GetSectionSize(g_objSectionName);

    // get the buffers entries
    const char* bitCodeBuffer = (const char*)reader.GetSectionData(g_irSectionName);
    assert(bitCodeBuffer && "BitCode Buffer is null");

    const char* serializationBuffer = (const char*)reader.GetSectionData(g_metaSectionName);
    assert(serializationBuffer && "Serialization Buffer is null");

    const char* optModuleBuffer = (const char*)reader.GetSectionData(g_optSectionName);
    assert(optModuleBuffer && "OptModule Buffer is null");

    const char* objectBuffer = (const char*)reader.GetSectionData(g_objSectionName);
    assert(objectBuffer && "Object Buffer is null");

    // Set IR
    BitCodeContainer* bcc = new BitCodeContainer(bitCodeBuffer, reader.GetSectionSize(g_irSectionName));
    pProgram->SetBitCodeContainer(bcc);

    // update the builtin module
    pProgram->SetBuiltinModule(GetCompiler()->GetBuiltinModuleList());

    // parse the optimized module
    llvm::StringRef data = llvm::StringRef(optModuleBuffer, optModuleSize);
    std::unique_ptr<llvm::MemoryBuffer> Buffer = llvm::MemoryBuffer::getMemBufferCopy(data);

    Compiler* pCompiler = GetCompiler();
    llvm::Module* pModule = pCompiler->ParseModuleIR(Buffer.get());
    pCompiler->CreateExecutionEngine(pModule);

    llvm::ExecutionEngine* pEngine = (llvm::ExecutionEngine*)GetCompiler()->GetExecutionEngine();

    // create cache manager
    pProgram->SetExecutionEngine(pEngine);
    pProgram->SetModule(pModule);

    ObjectCodeCache* pCache = new ObjectCodeCache((llvm::Module*)pProgram->GetModule(), objectBuffer, objectSize);
    pEngine->setObjectCache(pCache);
    static_cast<CPUProgram*>(pProgram)->SetObjectCache(pCache);

    // deserialize the management objects
    std::auto_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(nullptr));
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

    // update kernel mapper (OCL2.0) and run global ctors
    PostBuildProgramStep( pProgram, pModule, nullptr );
    return true;
}

Kernel *CPUProgramBuilder::CreateKernel(llvm::Function *pFunc,
                                        const std::string &funcName,
                                        KernelProperties *pProps,
                                        bool useTLSGlobals) const {
    std::vector<cl_kernel_argument> arguments;
    std::vector<unsigned int>       memoryArguments;

    // TODO : consider separating into a different analisys pass
    CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,
                                           pFunc, useTLSGlobals, arguments,
                                           memoryArguments);

    return m_pBackendFactory->CreateKernel( funcName, arguments, memoryArguments, pProps );
}

KernelSet* CPUProgramBuilder::CreateKernels(Program* pProgram,
                                    llvm::Module* pModule,
                                    const char* pBuildOpts,
                                    ProgramBuildResult& buildResult) const
{
    using namespace Intel::MetadataAPI;

    std::unique_ptr<KernelSet> spKernels(new KernelSet);

    for (auto *pFunc : KernelList(pModule))
    {
        // Obtain kernel function from annotation
        auto kimd = KernelInternalMetadataAPI(pFunc);
        // Obtain kernel wrapper function from metadata info
        assert(kimd.KernelWrapper.hasValue() && "Always expect a kernel wrapper to be present");
        llvm::Function *pWrapperFunc = kimd.KernelWrapper.get();

        // Create a kernel and kernel JIT properties
        CompilerBuildOptions buildOptions(pBuildOpts);
        std::unique_ptr<KernelProperties> spKernelProps(
            CreateKernelProperties(pProgram, pFunc, buildOptions, buildResult));

        // get the vector size used to generate the function
        unsigned int vecSize = kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
        spKernelProps->SetMinGroupSizeFactorial(vecSize);

        std::unique_ptr<KernelJITProperties> spKernelJITProps( CreateKernelJITProperties( vecSize ));

        intel::DebuggingServiceType debugType = intel::getDebuggingServiceType(
            buildOptions.GetDebugInfoFlag(), pModule, buildOptions.GetUseNativeDebuggerFlag());
        bool useTLSGlobals = (debugType == intel::Native) &&
                             !m_isFpgaEmulator && !m_isEyeQEmulator;
        std::unique_ptr<Kernel> spKernel(
            CreateKernel(pFunc, pWrapperFunc->getName().str(),
                         spKernelProps.get(), useTLSGlobals));

        // We want the JIT of the wrapper function to be called
        AddKernelJIT(static_cast<CPUProgram*>(pProgram),
                     spKernel.get(),
                     pModule,
                     pWrapperFunc,
                     spKernelJITProps.release());

        //TODO (AABOUD): is this redundant code?
        const llvm::Type *vTypeHint = nullptr; //pFunc->getVectTypeHint(); //TODO: Read from metadata (Guy)
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
        if (kimd.VectorizedKernel.hasValue())
        {
            Function *pVecFunc = kimd.VectorizedKernel.get();
            assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
                   "if the vector kernel is inlined the entry of the vector "
                   "kernel should be nullptr");
            if(nullptr != pVecFunc && !dontVectorize)
            {
                auto vkimd = KernelInternalMetadataAPI(pVecFunc);
                // Obtain kernel wrapper function from metadata info
                llvm::Function *pWrapperVecFunc = vkimd.KernelWrapper.get(); //TODO: stripPointerCasts());
                //Update vecSize according to vectorWidth of vectorized function
                vecSize = vkimd.VectorizedWidth.get();
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

        {
          auto kimd = KernelMetadataAPI(pFunc);

          assert(!(kimd.VecLenHint.hasValue() && kimd.ReqdIntelSGSize.hasValue()) &&
                 "Error! intel_vec_len_hint "
                 "and intel_reqd_sub_group_size "
                 "can't be specified at the same time!");

          if (kimd.VecLenHint.hasValue() &&
              SUPPORTED !=
                  m_compiler.GetCpuId().isTransposeSizeSupported(
                      (ETransposeSize)kimd.VecLenHint.get())) {
            buildResult.LogS() << "Warning! Specified vectorization width "
                                  "for kernel <"
                               << spKernel->GetKernelName()
                               << "> is not supported by the architecture. "
                                  "Fall back to autovectorization mode.\n";
          }
          if (kimd.ReqdIntelSGSize.hasValue() &&
              SUPPORTED !=
                  m_compiler.GetCpuId().isTransposeSizeSupported(
                      (ETransposeSize)kimd.ReqdIntelSGSize.get())) {
            buildResult.LogS() << "Error! Specified intel_reqd_sub_group_size "
                                  "for kernel <"
                               << spKernel->GetKernelName()
                               << "> is not supported.\n";
            buildResult.SetBuildResult(CL_DEV_BUILD_ERROR);
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
                spKernelProps->GetMinGroupSizeFactorial() << ")\n";
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
                                                     pProps );
    pKernel->AddKernelJIT( pJIT );
}

void CPUProgramBuilder::PostOptimizationProcessing(Program* pProgram, llvm::Module* spModule, const ICLDevBackendOptions* pOptions) const
{
    char*  pInjectedObjStart = nullptr;
    size_t injectedObjSize;

    // Check if we are going to do injection
    if (pOptions
        && pOptions->GetValue(CL_DEV_BACKEND_OPTION_INJECTED_OBJECT, &pInjectedObjStart, &injectedObjSize)
        && pInjectedObjStart != nullptr)
    {
        std::auto_ptr<StaticObjectLoader> pObjectLoader(new StaticObjectLoader());
        // Build the MemoryBuffer object from the supplied options
        std::unique_ptr<llvm::MemoryBuffer> pInjectedObj =
            llvm::MemoryBuffer::getMemBuffer( llvm::StringRef(pInjectedObjStart, injectedObjSize));

        pObjectLoader->addPreCompiled(spModule, pInjectedObj.release());
        // Add the injected object to the execution engine cache
        CPUProgram* pCPUProgram = static_cast<CPUProgram*>(pProgram);
        pCPUProgram->GetExecutionEngine()->setObjectCache(pObjectLoader.release());
    }
}

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

  CPUProgram* pCPUProgram = static_cast<CPUProgram*>(pProgram);
  pCPUProgram->GetExecutionEngine()->finalizeObject();
  pCPUProgram->GetExecutionEngine()->runStaticConstructorsDestructors(
      /*isDtors=*/false);
}
}}} // namespace
