// Copyright 2010-2022 Intel Corporation.
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

#include "CPUProgramBuilder.h"
#include "CPUJITContainer.h"
#include "CompilerConfig.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "OCLAddressSpace.h"
#include "Program.h"
#include "StaticObjectLoader.h"
#include "debuggingservicetype.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include "BitCodeContainer.h"
#include "CPUSerializationService.h"
#include "ObjectCodeContainer.h"
#include "cache_binary_handler.h"
#include "cl_sys_defines.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

using namespace Intel::OpenCL::ELFUtils;

CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory *pBackendFactory,
                                     std::unique_ptr<ICompilerConfig> config)
    : ProgramBuilder(pBackendFactory, std::move(config)), m_compiler(*m_config),
      m_isFpgaEmulator(FPGA_EMU_DEVICE == m_config->TargetDevice()) {}

CPUProgramBuilder::~CPUProgramBuilder()
{
}

void CPUProgramBuilder::BuildProgramCachedExecutable(ObjectCodeCache* pCache, Program* pProgram) const
{
    assert(pCache && "Object Cache is null");
    assert(pProgram && "Program Object is null");

    if(!pCache->isObjectAvailable())
    {
        pProgram->SetObjectCodeContainer(nullptr);
        return ;
    }

    // calculate the required buffer size
    size_t serializationSize = 0;
    std::unique_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(nullptr));
    pCPUSerializationService->GetSerializationBlobSize(
        SERIALIZE_PERSISTENT_IMAGE, pProgram, &serializationSize);

    size_t irSize = pProgram->GetProgramIRCodeContainer()->GetCodeSize();
    std::unique_ptr<llvm::MemoryBuffer> cachedObject = pCache->getObject(nullptr);
    size_t objSize = cachedObject->getBufferSize();

    CLElfLib::E_EH_MACHINE bitOS = m_compiler.GetCpuId()->Is64BitOS()
                                       ? CLElfLib::EM_X86_64
                                       : CLElfLib::EM_860;

    //Checking maximum supported instruction
    CLElfLib::E_EH_FLAGS maxSupportedVectorISA = CLElfLib::EH_FLAG_SSE4;
    if (m_compiler.GetCpuId()->HasAVX512ICL())
      maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_ICL;
    else if (m_compiler.GetCpuId()->HasAVX512SKX())
      maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_SKX;
    else if (m_compiler.GetCpuId()->HasAVX2())
      maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX2;
    else if (m_compiler.GetCpuId()->HasAVX1())
      maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX1;

    std::unique_ptr<CacheBinaryWriter> pWriter(new CacheBinaryWriter(bitOS, maxSupportedVectorISA));

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

    // fill the Object bits
    const char* objStart = cachedObject->getBuffer().data();
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
  ObjectCodeContainer *objCC = pProgram->GetObjectCodeContainer();
  assert(objCC && "Object code container is null");
  const char *pCachedObject = (const char *)(objCC->GetCode());
  size_t cacheSize = objCC->GetCodeSize();
  assert(pCachedObject && "Object code is null");

  // get sizes
  CacheBinaryReader reader(pCachedObject, cacheSize);
  size_t serializationSize = reader.GetSectionSize(g_metaSectionName);
  size_t irSize = reader.GetSectionSize(g_irSectionName);
  size_t objectSize = reader.GetSectionSize(g_objSectionName);

  const unsigned int *versionBuffer =
      (const unsigned int *)reader.GetSectionData(g_objVerSectionName);
  assert(versionBuffer && "Version Buffer is null");
  pProgram->m_binaryVersion = *versionBuffer;

  // get the buffers entries
  const char *bitCodeBuffer =
      (const char *)reader.GetSectionData(g_irSectionName);
  assert(bitCodeBuffer && "BitCode Buffer is null");

  const char *serializationBuffer =
      (const char *)reader.GetSectionData(g_metaSectionName);
  assert(serializationBuffer && "Serialization Buffer is null");

  const char *objectBuffer =
      (const char *)reader.GetSectionData(g_objSectionName);
  assert(objectBuffer && "Object Buffer is null");

  // Set IR
  BitCodeContainer *bcc = new BitCodeContainer(
      bitCodeBuffer, reader.GetSectionSize(g_irSectionName));
  pProgram->SetBitCodeContainer(bcc);

  // update the builtin module
  pProgram->SetBuiltinModule(GetCompiler()->GetBuiltinModuleList());

  // parse the IR bit code
  llvm::StringRef data = llvm::StringRef(bitCodeBuffer, irSize);
  std::unique_ptr<llvm::MemoryBuffer> Buffer =
      llvm::MemoryBuffer::getMemBufferCopy(data);

  Compiler *pCompiler = GetCompiler();
  std::unique_ptr<llvm::Module> M = pCompiler->ParseModuleIR(Buffer.get());

  bool useLLDJIT =
      m_compiler.isObjectFromLLDJIT(llvm::StringRef(objectBuffer, objectSize));
  if (useLLDJIT) {
    intel::DebuggingServiceType userType =
        intel::getUserDefinedDebuggingServiceType();

    if (userType != intel::None && userType != intel::Native) {
      // user has overriden cached object type
      pProgram->SetModule(std::move(M));
      return false;
    }
    }
    pCompiler->materializeSpirTriple(M.get());

    // create cache manager
    pProgram->SetModule(std::move(M));

    ObjectCodeCache* pCache = new ObjectCodeCache(pProgram->GetModule(),
                                                  objectBuffer, objectSize);

    CPUProgram *cpuProgram = static_cast<CPUProgram *>(pProgram);

    if (useLLDJIT) {
        m_compiler.CreateExecutionEngine(pProgram->GetModule());
        m_compiler.SetObjectCache(pCache);
        cpuProgram->SetExecutionEngine(m_compiler.GetOwningExecutionEngine());
    } else {
        // create LLJIT
        std::unique_ptr<llvm::orc::LLJIT> LLJIT =
            pCompiler->CreateLLJIT(pProgram->GetModule(), nullptr, nullptr);

        // add object buffer to LLJIT
        if (llvm::Error err = LLJIT->addObjectFile(pCache->getObject(nullptr))) {
            llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
            throw Exceptions::CompilerException("Failed to add object to LLJIT");
        }

        pProgram->SetLLJIT(std::move(LLJIT));
    }

    cpuProgram->SetObjectCache(pCache);

    // deserialize the management objects
    std::unique_ptr<CPUSerializationService> pCPUSerializationService(new CPUSerializationService(nullptr));
    pCPUSerializationService->ReloadProgram(SERIALIZE_PERSISTENT_IMAGE,
                                            pProgram, serializationBuffer,
                                            serializationSize, pProgram->m_binaryVersion);

    // init refcounted runtime service shared storage between program and kernels
    RuntimeServiceSharedPtr lRuntimeService =
                          RuntimeServiceSharedPtr(new RuntimeServiceImpl);
    // set runtime service for the program
    pProgram->SetRuntimeService(lRuntimeService);

    // update kernels with RuntimeService
    Utils::UpdateKernelsWithRuntimeService( lRuntimeService, pProgram->GetKernelSet() );

    return true;
}

Kernel *CPUProgramBuilder::CreateKernel(llvm::Function *pFunc,
                                        const std::string &funcName,
                                        KernelProperties *pProps,
                                        bool useTLSGlobals) const {
    std::vector<KernelArgument> arguments;
    std::vector<unsigned int>       memoryArguments;

    // TODO : consider separating into a different analisys pass
    llvm::CompilationUtils::parseKernelArguments(
        pFunc->getParent() /* = pModule */, pFunc, useTLSGlobals, arguments,
        memoryArguments);

    return m_pBackendFactory->CreateKernel(funcName, arguments, memoryArguments, pProps);
}

KernelSet* CPUProgramBuilder::CreateKernels(Program* pProgram,
                                    const char* pBuildOpts,
                                    ProgramBuildResult& buildResult) const
{
    using namespace DPCPPKernelMetadataAPI;

    std::unique_ptr<KernelSet> spKernels(new KernelSet);

    llvm::Module* pModule = pProgram->GetModule();
    auto Kernels = llvm::CompilationUtils::getKernels(*pModule);
    for (auto *pFunc : Kernels) {
      llvm::Function *pWrapperFunc = nullptr;
      // Obtain kernel function from annotation
      auto kimd = KernelInternalMetadataAPI(pFunc);
      // Obtain kernel wrapper function from metadata info
      if (kimd.KernelWrapper.hasValue())
        pWrapperFunc = kimd.KernelWrapper.get();
      else if (pFunc->hasFnAttribute("kernel_wrapper"))
        pWrapperFunc = llvm::CompilationUtils::getFnAttributeFunction(
            *pModule, *pFunc, "kernel_wrapper");
      assert(pWrapperFunc && "Always expect a kernel wrapper to be present");
      // Create a kernel and kernel JIT properties
      CompilerBuildOptions buildOptions(pBuildOpts);
      std::unique_ptr<KernelProperties> spKernelProps(
          CreateKernelProperties(pProgram, pFunc, buildOptions, buildResult));

      // get the vector size used to generate the function
      unsigned int vecSize =
          kimd.VectorizedWidth.hasValue() ? kimd.VectorizedWidth.get() : 1;
      spKernelProps->SetMinGroupSizeFactorial(vecSize);

      std::unique_ptr<KernelJITProperties> spKernelJITProps(
          CreateKernelJITProperties(vecSize));

      intel::DebuggingServiceType debugType = intel::getDebuggingServiceType(
          buildOptions.GetDebugInfoFlag(), pModule,
          buildOptions.GetUseNativeDebuggerFlag());
      bool useTLSGlobals = (debugType == intel::Native);
      std::unique_ptr<Kernel> spKernel(
          CreateKernel(pFunc, pWrapperFunc->getName().str(),
                       spKernelProps.get(), useTLSGlobals));

      // We want the JIT of the wrapper function to be called
      AddKernelJIT(static_cast<CPUProgram *>(pProgram), spKernel.get(),
                   pWrapperFunc, spKernelJITProps.release());

      // TODO (AABOUD): is this redundant code?
      const llvm::Type *vTypeHint =
          nullptr; // pFunc->getVectTypeHint(); //TODO: Read from metadata (Guy)
      bool dontVectorize = false;

      if (nullptr != vTypeHint) {
        // currently if the vector_type_hint attribute is set
        // we types that vector length is below 4, vectorizer restriction
        const llvm::FixedVectorType *pVect =
            llvm::dyn_cast<llvm::FixedVectorType>(vTypeHint);
        if ((nullptr != pVect) && pVect->getNumElements() >= 4) {
          dontVectorize = true;
        }
      }

      // Need to check if Vectorized Kernel Value exists, it is not guaranteed
      // that Vectorized is running in all scenarios.
      Function *pVecFunc = nullptr;
      llvm::Function *pWrapperVecFunc = nullptr;
      if (kimd.VectorizedKernel.hasValue()) {
        pVecFunc = kimd.VectorizedKernel.get();
        assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
               "if the vector kernel is inlined the entry of the vector "
               "kernel should be nullptr");
        if (nullptr != pVecFunc && !dontVectorize) {
          auto vkimd = KernelInternalMetadataAPI(pVecFunc);
          // Obtain kernel wrapper function from metadata info
          pWrapperVecFunc =
              vkimd.KernelWrapper.get(); // TODO: stripPointerCasts());
          // Update vecSize according to vectorWidth of vectorized function
          vecSize = vkimd.VectorizedWidth.get();
        }
      } else if (pFunc->hasFnAttribute("vectorized_kernel")) {
        pVecFunc = llvm::CompilationUtils::getFnAttributeFunction(
            *pModule, *pFunc, "vectorized_kernel");
        assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
               "if the vector kernel is inlined the entry of the vector "
               "kernel should be nullptr");
        if (nullptr != pVecFunc && !dontVectorize) {
          pWrapperVecFunc = llvm::CompilationUtils::getFnAttributeFunction(
              *pModule, *pFunc, "kernel_wrapper");
          llvm::CompilationUtils::getFnAttributeInt(pFunc, "vectorized_width",
                                                    vecSize);
        }
      }
      if (nullptr != pVecFunc && !dontVectorize) {
        // Create the vectorized kernel - no need to pass argument list here
        assert(pWrapperVecFunc && "vectorized kernel should have wrapper");
        std::unique_ptr<KernelJITProperties> spVKernelJITProps(
            CreateKernelJITProperties(vecSize));
        spKernelProps->SetMinGroupSizeFactorial(vecSize);
        AddKernelJIT(static_cast<CPUProgram *>(pProgram), spKernel.get(),
                     pWrapperVecFunc, spVKernelJITProps.release());
      }

      if (dontVectorize) {
        buildResult.LogS() << "Vectorization of kernel \""
                           << spKernel->GetKernelName()
                           << "\" was disabled by the developer\n";
      } else if (vecSize <= 1) {
        buildResult.LogS() << "Kernel \"" << spKernel->GetKernelName()
                           << "\" was not vectorized\n";
      } else {
        buildResult.LogS() << "Kernel \"" << spKernel->GetKernelName()
                           << "\" was successfully vectorized ("
                           << spKernelProps->GetMinGroupSizeFactorial()
                           << ")\n";
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

void CPUProgramBuilder::AddKernelJIT(CPUProgram* pProgram, Kernel* pKernel,
                                     llvm::Function* pFunc, KernelJITProperties* pProps) const
{
    IKernelJITContainer *pJIT =
        new CPUJITContainer(pProgram->GetPointerToFunction(pFunc->getName()),
                            pFunc, pProgram->GetModule(), pProps);
    pKernel->AddKernelJIT( pJIT );
}

void CPUProgramBuilder::PostOptimizationProcessing(Program* pProgram) const
{
    llvm::Module* spModule = pProgram->GetModule();

    // Collect sizes of global variables
    if (!spModule->global_empty())
    {
        size_t GlobalVariableTotalSize = 0;
        std::vector<cl_prog_gv> GlobalVariables;
        const llvm::DataLayout &DL = spModule->getDataLayout();
        for (auto &GV : spModule->globals())
        {
            // If there is a global variable like '__llvm_gcov_ctr.x', it means
            // that the code was built with profiling enabled.
            if (GV.getName().contains("__llvm_gcov_ctr"))
                pProgram->SetCodeProfilingStatus(PROFILING_GCOV);

            llvm::PointerType *PT = GV.getType();
            unsigned AS = PT->getAddressSpace();
            if (!IS_ADDR_SPACE_GLOBAL(AS) && !IS_ADDR_SPACE_CONSTANT(AS))
                continue;

            size_t Size = DL.getTypeAllocSize(PT->getContainedType(0));
            GlobalVariableTotalSize += Size;

            // Global variable with common or external linkage
            // (supported in SPIR 2.0) in global address space can be queried
            // by cl_intel_global_variable_pointers extension.
            // BTW, available_externally linkage is also supported in SPIR 2.0,
            // however, currently there is no use case of this linkage in the
            // extension.
            if (!GV.hasCommonLinkage() && !GV.hasExternalLinkage())
                continue;

            GlobalVariables.push_back({STRDUP(GV.getName().str().c_str()),
                                       Size, nullptr});
        }
        pProgram->SetGlobalVariableTotalSize(GlobalVariableTotalSize);
        pProgram->SetGlobalVariables(std::move(GlobalVariables));
    }

    // Record global Ctor/Dtor names
    pProgram->RecordCtorDtors(*spModule);
}

void CPUProgramBuilder::JitProcessing(
    Program *program, const ICLDevBackendOptions *options,
    std::unique_ptr<llvm::TargetMachine> targetMachine,
    ObjectCodeCache *objCache) {
  // Get/create JIT instance
  llvm::Module *module = program->GetModule();
  bool useLLDJIT = m_compiler.useLLDJITForExecution(module);
  if (useLLDJIT) {
    m_compiler.SetObjectCache(objCache);
    program->SetExecutionEngine(m_compiler.GetOwningExecutionEngine());
  } else {
    auto LLJIT =
        m_compiler.CreateLLJIT(module, std::move(targetMachine), objCache);
    llvm::orc::IRCompileLayer::NotifyCompiledFunction notifyCompiled =
        [&](llvm::orc::MaterializationResponsibility & /*R*/,
            llvm::orc::ThreadSafeModule TSM) -> void {
      program->SetModule(std::move(TSM));
    };
    LLJIT->getIRCompileLayer().setNotifyCompiled(std::move(notifyCompiled));
    // Print LLJIT log to strings, and then save them to program build log when
    // handle exception
    LLJIT->getExecutionSession().setErrorReporter([=](Error Err) {
      logAllUnhandledErrors(
          std::move(Err),
          (static_cast<CPUProgram *>(program))->getLLJITLogStream(),
          "JIT session error: ");
    });

    program->SetLLJIT(std::move(LLJIT));

    // Load clang profile library if the code was built with profiling
    if (program->GetCodeProfilingStatus() != PROFILING_NONE)
      program->LoadProfileLib();
  }

  // Check if we are going to do injection
  char *injectedObjStart = nullptr;
  size_t injectedObjSize;
  if (options &&
      options->GetValue(CL_DEV_BACKEND_OPTION_INJECTED_OBJECT,
                        &injectedObjStart, &injectedObjSize) &&
      injectedObjStart != nullptr) {
    // Build the MemoryBuffer object from the supplied options
    std::unique_ptr<llvm::MemoryBuffer> injectedObj =
        llvm::MemoryBuffer::getMemBuffer(
            llvm::StringRef(injectedObjStart, injectedObjSize));

    if (useLLDJIT) {
      std::unique_ptr<StaticObjectLoader> objectLoader(new StaticObjectLoader());
      objectLoader->addPreCompiled(module, injectedObj.release());
      // Add the injected object to the execution engine cache
      static_cast<CPUProgram *>(program)->GetExecutionEngine()->setObjectCache(
          objectLoader.release());
    } else {
      llvm::orc::LLJIT *LLJIT = program->GetLLJIT();
      if (auto Err = LLJIT->addObjectFile(std::move(injectedObj))) {
        llvm::logAllUnhandledErrors(std::move(Err), llvm::errs());
        throw Exceptions::CompilerException("Fail to add object file");
      }
    }
  }

  if (useLLDJIT) {
    CPUProgram *cpuProgram = static_cast<CPUProgram *>(program);
    cpuProgram->GetExecutionEngine()->generateCodeForModule(module);
    return;
  }

  // Record kernel names and trigger JIT compilation of kernels
  std::vector<std::string> kernelNames;
  using namespace DPCPPKernelMetadataAPI;
  auto Kernels = KernelList(module).getList();
  if (Kernels.empty()) {
    auto FSet = llvm::CompilationUtils::getKernels(*module);
    for (auto *F : FSet)
      Kernels.push_back(F);
  }
  for (auto *pFunc : Kernels) {
    llvm::Function *pWrapperFunc = nullptr;
    auto kimd = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(pFunc);
    if (kimd.KernelWrapper.hasValue())
      pWrapperFunc = kimd.KernelWrapper.get();
    else if (pFunc->hasFnAttribute("kernel_wrapper"))
      pWrapperFunc = module->getFunction(
          pFunc->getFnAttribute("kernel_wrapper").getValueAsString());
    assert(pWrapperFunc && "Always expect a kernel wrapper to be present");
    kernelNames.push_back(pWrapperFunc->getName().str());
  }
  llvm::orc::LLJIT *LLJIT = program->GetLLJIT();
  if (!kernelNames.empty()) {
    if (auto err = LLJIT->addIRModule(llvm::orc::ThreadSafeModule(
            program->GetModuleOwner(),
            std::make_unique<llvm::LLVMContext>()))) {
      llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
      throw Exceptions::CompilerException("Failed to add IR Module");
    }
    CPUProgram *cpuProgram = static_cast<CPUProgram *>(program);
    for (std::string &kernelName : kernelNames)
      (void)cpuProgram->GetPointerToFunction(kernelName);
  } else {
    // There are no kernels to lookup and LLJIT won't be triggered.
    // So we need to compile the module into object buffer.
    if (auto err =
            LLJIT->addObjectFile(m_compiler.SimpleCompile(module, objCache))) {
      llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
      throw Exceptions::CompilerException("Failed to add object file");
    }
  }
}

}}} // namespace
