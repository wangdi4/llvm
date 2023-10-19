// Copyright 2010 Intel Corporation.
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
#include "AsmCompiler.h"
#include "BitCodeContainer.h"
#include "CPUJITContainer.h"
#include "CPUSerializationService.h"
#include "CompilerConfig.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "ObjectCodeContainer.h"
#include "Program.h"
#include "StaticObjectLoader.h"
#include "cache_binary_handler.h"
#include "cl_sys_defines.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace Intel::OpenCL::ELFUtils;

CPUProgramBuilder::CPUProgramBuilder(IAbstractBackendFactory *pBackendFactory,
                                     std::unique_ptr<ICompilerConfig> config)
    : ProgramBuilder(pBackendFactory, std::move(config)), m_compiler(*m_config),
      m_isFpgaEmulator(FPGA_EMU_DEVICE == m_config->TargetDevice()) {}

CPUProgramBuilder::~CPUProgramBuilder() {}

void CPUProgramBuilder::BuildProgramCachedExecutable(ObjectCodeCache *pCache,
                                                     Program *pProgram) const {
  assert(pCache && "Object Cache is null");
  assert(pProgram && "Program Object is null");

  if (!pCache->isObjectAvailable()) {
    pProgram->SetObjectCodeContainer(nullptr);
    return;
  }

  // calculate the required buffer size
  size_t serializationSize = 0;
  std::unique_ptr<CPUSerializationService> pCPUSerializationService(
      new CPUSerializationService(nullptr));
  pCPUSerializationService->GetSerializationBlobSize(
      SERIALIZE_PERSISTENT_IMAGE, pProgram, &serializationSize);

  size_t irSize = pProgram->GetProgramIRCodeContainer()->GetCodeSize();
  std::unique_ptr<MemoryBuffer> cachedObject = pCache->getObject(nullptr);
  size_t objSize = cachedObject->getBufferSize();

  CLElfLib::E_EH_MACHINE bitOS = m_compiler.GetCpuId()->Is64BitOS()
                                     ? CLElfLib::EM_X86_64
                                     : CLElfLib::EM_860;

  // Checking maximum supported instruction
  CLElfLib::E_EH_FLAGS maxSupportedVectorISA = CLElfLib::EH_FLAG_SSE4;
  if (m_compiler.GetCpuId()->HasGNR())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_GNR;
  else if (m_compiler.GetCpuId()->HasSPR())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_SPR;
  else if (m_compiler.GetCpuId()->HasAVX512ICX())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_ICX;
  else if (m_compiler.GetCpuId()->HasAVX512ICL())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_ICL;
  else if (m_compiler.GetCpuId()->HasAVX512CLX())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_CLX;
  else if (m_compiler.GetCpuId()->HasAVX512SKX())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX512_SKX;
  else if (m_compiler.GetCpuId()->HasAVX2())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX2;
  else if (m_compiler.GetCpuId()->HasAVX1())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_AVX1;
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CPU_DMR
  if (m_compiler.GetCpuId()->HasDMR())
    maxSupportedVectorISA = CLElfLib::EH_FLAG_DMR;
#endif // INTEL_FEATURE_CPU_DMR
#endif // INTEL_CUSTOMIZATION

  std::unique_ptr<CacheBinaryWriter> pWriter(
      new CacheBinaryWriter(bitOS, maxSupportedVectorISA));

  // fill the IR bit code
  const char *irStart =
      ((const char *)(pProgram->GetProgramIRCodeContainer()->GetCode()));
  pWriter->AddSection(g_irSectionName, irStart, irSize);

  // fill offload image in the object buffer
  std::vector<char> metaStart(serializationSize);
  pCPUSerializationService->SerializeProgram(
      SERIALIZE_PERSISTENT_IMAGE, pProgram, &(metaStart[0]), serializationSize);
  pWriter->AddSection(g_metaSectionName, &(metaStart[0]), serializationSize);

  // fill the Object bits
  const char *objStart = cachedObject->getBuffer().data();
  pWriter->AddSection(g_objSectionName, objStart, objSize);

  // fill the Version section
  unsigned int currentVersion = OCL_CACHED_BINARY_VERSION;
  pWriter->AddSection(g_objVerSectionName, (char *)&currentVersion,
                      sizeof(unsigned int));

  // get the binary
  size_t binarySize = pWriter->GetBinarySize();
  std::vector<char> pBinaryBlob(binarySize);
  if (pWriter->GetBinary(&(pBinaryBlob[0]))) {
    ObjectCodeContainer *pObjectCodeContainer =
        new ObjectCodeContainer(&pBinaryBlob[0], binarySize);
    pProgram->SetObjectCodeContainer(pObjectCodeContainer);
  } else {
    pProgram->SetObjectCodeContainer(nullptr);
  }
}

bool CPUProgramBuilder::ReloadProgramFromCachedExecutable(Program *pProgram) {
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

  // parse the IR bit code
  StringRef data = StringRef(bitCodeBuffer, irSize);
  std::unique_ptr<MemoryBuffer> Buffer = MemoryBuffer::getMemBufferCopy(data);

  Compiler *pCompiler = GetCompiler();
  std::unique_ptr<Module> M = pCompiler->ParseModuleIR(Buffer.get());

  pCompiler->materializeSpirTriple(M.get());

  // create cache manager
  pProgram->SetModule(std::move(M));

  ObjectCodeCache *pCache =
      new ObjectCodeCache(pProgram->GetModule(), objectBuffer, objectSize);

  CPUProgram *cpuProgram = static_cast<CPUProgram *>(pProgram);

  bool useLLDJIT =
      m_compiler.isObjectFromLLDJIT(StringRef(objectBuffer, objectSize));
  if (useLLDJIT) {
    m_compiler.CreateExecutionEngine(pProgram->GetModule());
    m_compiler.SetObjectCache(pCache);
    cpuProgram->SetExecutionEngine(m_compiler.GetOwningExecutionEngine());
  } else {
    // create LLJIT
    std::unique_ptr<orc::LLJIT> LLJIT =
        pCompiler->CreateLLJIT(pProgram->GetModule(), nullptr, nullptr);

    // add object buffer to LLJIT
    if (Error err = LLJIT->addObjectFile(pCache->getObject(nullptr))) {
      logAllUnhandledErrors(std::move(err), errs());
      throw Exceptions::CompilerException("Failed to add object to LLJIT");
    }

    pProgram->SetLLJIT(std::move(LLJIT));
  }

  cpuProgram->SetObjectCache(pCache);

  // deserialize the management objects
  std::unique_ptr<CPUSerializationService> pCPUSerializationService(
      new CPUSerializationService(nullptr));
  pCPUSerializationService->ReloadProgram(
      SERIALIZE_PERSISTENT_IMAGE, pProgram, serializationBuffer,
      serializationSize, pProgram->m_binaryVersion);

  // init refcounted runtime service shared storage between program and kernels
  RuntimeServiceSharedPtr lRuntimeService =
      RuntimeServiceSharedPtr(new RuntimeServiceImpl);
  // set runtime service for the program
  pProgram->SetRuntimeService(lRuntimeService);

  // update kernels with RuntimeService
  Utils::UpdateKernelsWithRuntimeService(lRuntimeService,
                                         pProgram->GetKernelSet());

  return true;
}

Kernel *CPUProgramBuilder::CreateKernel(Function *pFunc,
                                        const std::string &funcName,
                                        KernelProperties *pProps,
                                        bool useTLSGlobals) const {
  std::vector<KernelArgument> arguments;
  std::vector<unsigned int> memoryArguments;

  // TODO : consider separating into a different analisys pass
  CompilationUtils::parseKernelArguments(pFunc->getParent() /* = pModule */,
                                         pFunc, useTLSGlobals, arguments,
                                         memoryArguments);

  return m_pBackendFactory->CreateKernel(funcName, arguments, memoryArguments,
                                         pProps);
}

std::unique_ptr<KernelSet>
CPUProgramBuilder::CreateKernels(Program *pProgram, const char *pBuildOpts,
                                 ProgramBuildResult &buildResult) const {
  using namespace SYCLKernelMetadataAPI;

  std::unique_ptr<KernelSet> spKernels(new KernelSet);

  Module *pModule = pProgram->GetModule();
  auto Kernels = CompilationUtils::getKernels(*pModule);
  for (auto *pFunc : Kernels) {
    Function *pWrapperFunc = nullptr;
    // Obtain kernel function from annotation
    auto kimd = KernelInternalMetadataAPI(pFunc);
    // Obtain kernel wrapper function from metadata info
    if (kimd.KernelWrapper.hasValue())
      pWrapperFunc = kimd.KernelWrapper.get();
    else if (pFunc->hasFnAttribute("kernel_wrapper"))
      pWrapperFunc = CompilationUtils::getFnAttributeFunction(*pModule, *pFunc,
                                                              "kernel_wrapper");
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

    std::unique_ptr<Kernel> spKernel(
        CreateKernel(pFunc, pWrapperFunc->getName().str(), spKernelProps.get(),
                     CompilationUtils::hasTLSGlobals(*pModule)));

    // We want the JIT of the wrapper function to be called
    AddKernelJIT(static_cast<CPUProgram *>(pProgram), spKernel.get(),
                 pWrapperFunc, spKernelJITProps.release());

    // Need to check if Vectorized Kernel Value exists, it is not guaranteed
    // that Vectorized is running in all scenarios.
    Function *pVecFunc = nullptr;
    Function *pWrapperVecFunc = nullptr;
    if (kimd.VectorizedKernel.hasValue()) {
      pVecFunc = kimd.VectorizedKernel.get();
      assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
             "if the vector kernel is inlined the entry of the vector "
             "kernel should be nullptr");
      if (nullptr != pVecFunc) {
        auto vkimd = KernelInternalMetadataAPI(pVecFunc);
        // Obtain kernel wrapper function from metadata info
        pWrapperVecFunc =
            vkimd.KernelWrapper.get(); // TODO: stripPointerCasts());
        // Update vecSize according to vectorWidth of vectorized function
        vecSize = vkimd.VectorizedWidth.get();
      }
    } else if (pFunc->hasFnAttribute("vectorized_kernel")) {
      pVecFunc = CompilationUtils::getFnAttributeFunction(*pModule, *pFunc,
                                                          "vectorized_kernel");
      assert(!(spKernelProps->IsVectorizedWithTail() && pVecFunc) &&
             "if the vector kernel is inlined the entry of the vector "
             "kernel should be nullptr");
      if (nullptr != pVecFunc) {
        pWrapperVecFunc = CompilationUtils::getFnAttributeFunction(
            *pModule, *pFunc, "kernel_wrapper");
        CompilationUtils::getFnAttributeInt(pFunc, "vectorized_width", vecSize);
      }
    }
    if (nullptr != pVecFunc) {
      // Create the vectorized kernel - no need to pass argument list here
      assert(pWrapperVecFunc && "vectorized kernel should have wrapper");
      std::unique_ptr<KernelJITProperties> spVKernelJITProps(
          CreateKernelJITProperties(vecSize));
      spKernelProps->SetMinGroupSizeFactorial(vecSize);
      AddKernelJIT(static_cast<CPUProgram *>(pProgram), spKernel.get(),
                   pWrapperVecFunc, spVKernelJITProps.release());
    }

    if (vecSize <= 1) {
      buildResult.LogS() << "Kernel \"" << spKernel->GetKernelName()
                         << "\" was not vectorized\n";
    } else {
      buildResult.LogS() << "Kernel \"" << spKernel->GetKernelName()
                         << "\" was successfully vectorized ("
                         << spKernelProps->GetMinGroupSizeFactorial() << ")\n";
    }
#ifdef OCL_DEV_BACKEND_PLUGINS
    // Notify the plugin manager
    m_pluginManger.OnCreateKernel(pProgram, spKernel.get(), pFunc);
#endif
    spKernels->AddKernel(std::move(spKernel));
    spKernelProps.release();
  }
  // LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Iterating
  // completed");

  buildResult.LogS() << "Done.";
  // LLVMBackend::GetInstance()->m_logger->Log(Logger::INFO_LEVEL, L"Exit");
  return spKernels;
}

void CPUProgramBuilder::AddKernelJIT(CPUProgram *pProgram, Kernel *pKernel,
                                     Function *pFunc,
                                     KernelJITProperties *pProps) const {
  IKernelJITContainer *pJIT =
      new CPUJITContainer(pProgram->GetPointerToFunction(pFunc->getName()),
                          pFunc, pProgram->GetModule(), pProps);
  pKernel->AddKernelJIT(pJIT);
}

void CPUProgramBuilder::PostOptimizationProcessing(Program *pProgram) const {
  Module *spModule = pProgram->GetModule();

  // Collect sizes of global variables
  if (!spModule->global_empty()) {
    size_t GlobalVariableTotalSize = 0;
    std::vector<cl_prog_gv> GlobalVariables;
    const DataLayout &DL = spModule->getDataLayout();
    for (auto &GV : spModule->globals()) {
      // If there is a global variable like '__llvm_gcov_ctr.x', it means
      // that the code was built with profiling enabled.
      if (GV.getName().contains("__llvm_gcov_ctr"))
        pProgram->SetCodeProfilingStatus(PROFILING_GCOV);

      PointerType *PT = GV.getType();
      unsigned AS = PT->getAddressSpace();
      if (CompilationUtils::ADDRESS_SPACE_GLOBAL != AS &&
          CompilationUtils::ADDRESS_SPACE_CONSTANT != AS)
        continue;

      size_t Size = DL.getTypeAllocSize(GV.getValueType());
      GlobalVariableTotalSize += Size;

      // Global variable with common or external linkage
      // (supported in SPIR 2.0) in global address space can be queried
      // by cl_intel_global_variable_pointers extension.
      // BTW, available_externally linkage is also supported in SPIR 2.0,
      // however, currently there is no use case of this linkage in the
      // extension.
      if (!GV.hasCommonLinkage() && !GV.hasExternalLinkage())
        continue;

      // Try to get decorations for device globals.
      StringRef DecoName = "";
      bool DeviceImageScope = false;
      unsigned int HostAccessMode = HOST_ACCESS_READ_WRITE;
      if (MDNode *Node = GV.getMetadata("spirv.Decorations.HostAccess")) {
        // Get the host access mode
        HostAccessMode =
            mdconst::extract<ConstantInt>(Node->getOperand(1))->getZExtValue();
        assert(HostAccessMode <= HOST_ACCESS_NONE &&
               "HostAccess mode is invalid");
        // Get the decoration name
        DecoName = cast<MDString>(Node->getOperand(2))->getString();

        // If a device global has property device_image_scope, its member
        // variable should be the base type. Otherwise, the member variable
        // should be a USM pointer.
        Type *DeviceGlobalTy =
            cast<StructType>(GV.getValueType())->getElementType(0);
        if (!DeviceGlobalTy->isPointerTy() ||
            cast<PointerType>(DeviceGlobalTy)->getAddressSpace() ==
                CompilationUtils::ADDRESS_SPACE_GENERIC) {
          DeviceImageScope = true;
        }
      }

      GlobalVariables.push_back({STRDUP(GV.getName().str().c_str()),
                                 STRDUP(DecoName.str().c_str()), HostAccessMode,
                                 DeviceImageScope, Size, nullptr});
    }
    pProgram->SetGlobalVariableTotalSize(GlobalVariableTotalSize);
    pProgram->SetGlobalVariables(std::move(GlobalVariables));
  }

  // Record global Ctor/Dtor names
  pProgram->RecordCtorDtors(*spModule);
}

static unsigned getAssemblyDumpFileId() {
  static std::atomic<unsigned> FileId(0);
  FileId++;
  return FileId.load(std::memory_order_relaxed);
}

static void dumpAssembly(Module *M, TargetMachine *TM,
                         const std::string &Filename) {
  std::error_code EC;
  auto Out = std::make_unique<ToolOutputFile>(Filename, EC, sys::fs::OF_None);
  if (EC)
    throw Exceptions::CompilerException(EC.message());

  TargetLibraryInfoImpl TLII(Triple(M->getTargetTriple()));
  legacy::PassManager PM;
  PM.add(new TargetLibraryInfoWrapperPass(TLII));
  if (TM->addPassesToEmitFile(PM, Out->os(),
                              /*DwoOut*/ nullptr, CodeGenFileType::AssemblyFile,
                              /*DisableVerify*/ true))
    throw Exceptions::CompilerException(
        "failed to add passes to dump assembly file");
  PM.run(*M);
  Out->keep();
}

static std::unique_ptr<MemoryBuffer>
loadAssemblyAndCompileToObj(const SmallVectorImpl<StringRef> &AsmFilenames,
                            const std::string &Triple,
                            ProgramBuildResult &buildResult) {
  unsigned FileId = [&] {
    static std::atomic<unsigned> FileId(0);
    unsigned Id = FileId.load(std::memory_order_relaxed);
    FileId++;
    return Id;
  }();
  assert(FileId < AsmFilenames.size() &&
         "Number of filenames in CL_CONFIG_REPLACE_ASM should match with the "
         "number of OpenCL programs");
  StringRef AsmFilename = AsmFilenames[FileId];
  std::string WarningMsg = "WARNING: replace device kernel assembly with " +
                           AsmFilename.str() + "\n";
  dbgs() << WarningMsg;
  buildResult.LogS() << WarningMsg;
  ErrorOr<std::unique_ptr<MemoryBuffer>> AsmMB =
      MemoryBuffer::getFile(AsmFilename);
  if (!AsmMB)
    throw Exceptions::CompilerException(AsmMB.getError().message());

  SmallVector<char, 256> ResultPath;
  int FD;
  if (auto EC = sys::fs::createTemporaryFile(AsmFilename, "o", FD, ResultPath))
    throw Exceptions::CompilerException(EC.message());
  std::string ObjFilename(ResultPath.data(),
                          ResultPath.data() + ResultPath.size());
  raw_fd_ostream OS(FD, /*shouldClose*/ false);

  int res = AsmCompiler::compileAsmToObjectFile(std::move(*AsmMB), &OS, Triple);
  if (res)
    throw Exceptions::CompilerException("fail to compile asm to object file");
  OS.flush();
  if (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    throw Exceptions::CompilerException(EC.message());
  ErrorOr<std::unique_ptr<MemoryBuffer>> ObjMB =
      MemoryBuffer::getFile(ObjFilename);
  if (!ObjMB)
    throw Exceptions::CompilerException(ObjMB.getError().message());

  if (sys::fs::remove(ObjFilename))
    assert(false && "Failed to remove temp obj file");

  return std::move(*ObjMB);
}

static std::unique_ptr<MemoryBuffer>
loadObject(const SmallVectorImpl<StringRef> &ObjFilenames,
           ProgramBuildResult &buildResult) {
  unsigned FileId = [&] {
    static std::atomic<unsigned> FileId(0);
    unsigned Id = FileId.load(std::memory_order_relaxed);
    FileId++;
    return Id;
  }();
  assert(FileId < ObjFilenames.size() &&
         "Number of filenames in CL_CONFIG_REPLACE_OBJ should match with the "
         "number of OpenCL programs");
  std::string WarningMsg = "WARNING: replace device kernel object with " +
                           ObjFilenames[FileId].str() + "\n";
  dbgs() << WarningMsg;
  buildResult.LogS() << WarningMsg;
  ErrorOr<std::unique_ptr<MemoryBuffer>> ObjMB =
      MemoryBuffer::getFile(ObjFilenames[FileId]);
  if (!ObjMB)
    throw Exceptions::CompilerException(ObjMB.getError().message());
  return std::move(*ObjMB);
}

void CPUProgramBuilder::JitProcessing(
    Program *program, const ICLDevBackendOptions *options,
    std::unique_ptr<TargetMachine> targetMachine, ObjectCodeCache *objCache,
    ProgramBuildResult &buildResult) {
  // Get/create JIT instance
  Module *module = program->GetModule();

  std::string envStr;
  if (Intel::OpenCL::Utils::getEnvVar(envStr, "CL_CONFIG_DUMP_ASM") &&
      Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(envStr)) {
    std::string filename = generateDumpFilename(program->GenerateHash(),
                                                getAssemblyDumpFileId(), ".s");
    dumpAssembly(module, targetMachine.get(), filename);
  }

  bool useLLDJIT = m_compiler.useLLDJITForExecution(module);
  if (useLLDJIT) {
    m_compiler.SetObjectCache(objCache);
    program->SetExecutionEngine(m_compiler.GetOwningExecutionEngine());
  } else {
    auto LLJIT =
        m_compiler.CreateLLJIT(module, std::move(targetMachine), objCache);
    orc::IRCompileLayer::NotifyCompiledFunction notifyCompiled =
        [&](orc::MaterializationResponsibility & /*R*/,
            orc::ThreadSafeModule TSM) -> void {
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
  // Replace OpenCL programs with contents in a list of assembly filenames or a
  // list of object filenames separated with comma. The number of filenames must
  // match with the number of OpenCL programs in the application.
  SmallVector<StringRef, 4> replaceAsmFilenames;
  SmallVector<StringRef, 4> replaceObjFilenames;
#ifndef INTEL_PRODUCT_RELEASE
  std::string envStrAsm;
  if (Intel::OpenCL::Utils::getEnvVar(envStrAsm, "CL_CONFIG_REPLACE_ASM"))
    SplitString(envStrAsm, replaceAsmFilenames, ",");
  std::string envStrObj;
  if (Intel::OpenCL::Utils::getEnvVar(envStrObj, "CL_CONFIG_REPLACE_OBJ"))
    SplitString(envStrObj, replaceObjFilenames, ",");
#endif
  if ((options &&
       options->GetValue(CL_DEV_BACKEND_OPTION_INJECTED_OBJECT,
                         &injectedObjStart, &injectedObjSize) &&
       injectedObjStart != nullptr) ||
      !replaceAsmFilenames.empty() || !replaceObjFilenames.empty()) {
    std::unique_ptr<MemoryBuffer> injectedObj;
    if (injectedObjStart) {
      // Build the MemoryBuffer object from the supplied options.
      injectedObj = std::move(MemoryBuffer::getMemBuffer(
          StringRef(injectedObjStart, injectedObjSize)));
    } else if (!replaceAsmFilenames.empty()) {
      injectedObj = std::move(loadAssemblyAndCompileToObj(
          replaceAsmFilenames, module->getTargetTriple(), buildResult));
    } else {
      injectedObj = std::move(loadObject(replaceObjFilenames, buildResult));
    }

    if (useLLDJIT) {
      std::unique_ptr<StaticObjectLoader> objectLoader(
          new StaticObjectLoader());
      objectLoader->addPreCompiled(module, injectedObj.release());
      // Add the injected object to the execution engine cache
      static_cast<CPUProgram *>(program)->GetExecutionEngine()->setObjectCache(
          objectLoader.release());
    } else {
      orc::LLJIT *LLJIT = program->GetLLJIT();
      if (auto Err = LLJIT->addObjectFile(std::move(injectedObj))) {
        logAllUnhandledErrors(std::move(Err), errs());
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
  using namespace SYCLKernelMetadataAPI;
  auto Kernels = KernelList(module).getList();
  if (Kernels.empty()) {
    auto FSet = CompilationUtils::getKernels(*module);
    for (auto *F : FSet)
      Kernels.push_back(F);
  }
  for (auto *pFunc : Kernels) {
    Function *pWrapperFunc = nullptr;
    auto kimd = SYCLKernelMetadataAPI::KernelInternalMetadataAPI(pFunc);
    if (kimd.KernelWrapper.hasValue())
      pWrapperFunc = kimd.KernelWrapper.get();
    else if (pFunc->hasFnAttribute("kernel_wrapper"))
      pWrapperFunc = module->getFunction(
          pFunc->getFnAttribute("kernel_wrapper").getValueAsString());
    assert(pWrapperFunc && "Always expect a kernel wrapper to be present");
    kernelNames.push_back(pWrapperFunc->getName().str());
  }
  orc::LLJIT *LLJIT = program->GetLLJIT();
  if (!kernelNames.empty()) {
    if (replaceAsmFilenames.empty() && replaceObjFilenames.empty()) {
      if (auto err = LLJIT->addIRModule(orc::ThreadSafeModule(
              program->GetModuleOwner(), std::make_unique<LLVMContext>()))) {
        logAllUnhandledErrors(std::move(err), errs());
        throw Exceptions::CompilerException("Failed to add IR Module");
      }
    }
    CPUProgram *cpuProgram = static_cast<CPUProgram *>(program);
    for (std::string &kernelName : kernelNames)
      (void)cpuProgram->GetPointerToFunction(kernelName);
  } else {
    // There are no kernels to lookup and LLJIT won't be triggered.
    // So we need to compile the module into object buffer.
    if (auto err =
            LLJIT->addObjectFile(m_compiler.SimpleCompile(module, objCache))) {
      logAllUnhandledErrors(std::move(err), errs());
      throw Exceptions::CompilerException("Failed to add object file");
    }
  }
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
