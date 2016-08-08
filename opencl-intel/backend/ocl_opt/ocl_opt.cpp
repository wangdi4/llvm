#include "llvm/IR/Module.h"
#include "llvm/PassRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ExecutionEngine/ObjectMemoryBuffer.h"
#include <llvm/Bitcode/ReaderWriter.h>
#include "llvm/opt.h"
#include "InitializePasses.h"
#include "CPUDetect.h"

#include <memory>

int mainImp(int argc, char **argv);

using namespace llvm;

static cl::list<std::string>
RuntimeLib(cl::CommaSeparated, "runtimelib",
                  cl::desc("Runtime declarations (bitCode) libraries (comma separated)"),
                  cl::value_desc("filename1,filename2"));

static cl::opt<std::string>
RuntimeServices("runtime",
                  cl::desc("Runtime services type (ocl/dx/apple/rs)"),
                  cl::value_desc("runtime_type"), cl::init("ocl"));

extern "C" Pass* createBuiltinLibInfoPass(
  SmallVector<Module*, 2> builtinsList,
  SmallVector<MemoryBuffer*, 2> builtinsBufferList,
  std::string type);
extern "C" Pass* createBuiltInImportPass(const char* CPUName);

void initializeOCLPasses(PassRegistry &Registry)
{
    intel::initializePhiCanonPass(Registry);
    intel::initializePredicatorPass(Registry);
    intel::initializeWIAnalysisPass(Registry);
    intel::initializeScalarizeFunctionPass(Registry);
    intel::initializeSimplifyGEPPass(Registry);
    intel::initializePacketizeFunctionPass(Registry);
    intel::initializeX86ResolverPass(Registry);
    intel::initializeMICResolverPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeSpecialCaseBuiltinResolverPass(Registry);
    intel::initializeAppleWIDepPrePacketizationPassPass(Registry);
    intel::initializeOCLBuiltinPreVectorizationPassPass(Registry);
    intel::initializeCLWGLoopCreatorPass(Registry);
    intel::initializeCLWGLoopBoundariesPass(Registry);
    intel::initializeCLStreamSamplerPass(Registry);
    intel::initializeKernelAnalysisPass(Registry);
    intel::initializeIRInjectModulePass(Registry);
    intel::initializenameByInstTypePass(Registry);
    intel::initializeDuplicateCalledKernelsPass(Registry);
    intel::initializeRedundantPhiNodePass(Registry);
    intel::initializeGroupBuiltinPass(Registry);
    intel::initializeBarrierInFunctionPass(Registry);
    intel::initializeRemoveDuplicationBarrierPass(Registry);
    intel::initializeSplitBBonBarrierPass(Registry);
    intel::initializeBarrierPass(Registry);
    intel::initializeWIRelatedValuePass(Registry);
    intel::initializeDataPerBarrierPass(Registry);
    intel::initializeDataPerValuePass(Registry);
    intel::initializePreventDivCrashesPass(Registry);
    intel::initializeBuiltinCallToInstPass(Registry);
    intel::initializeInstToFuncCallPass(Registry);
    intel::initializeModuleCleanupPass(Registry);
    intel::initializeAddImplicitArgsPass(Registry);
    intel::initializeOclFunctionAttrsPass(Registry);
    intel::initializeOclSyncFunctionAttrsPass(Registry);
    intel::initializeBuiltinLibInfoPass(Registry);
    intel::initializeLocalBuffersWrapperPass(Registry);
    intel::initializeLocalBuffersWithDebugWrapperPass(Registry);
    intel::initializeRelaxedPassPass(Registry);
    intel::initializeShiftZeroUpperBitsPass(Registry);
    intel::initializePrefetchPass(Registry);
    intel::initializeBIImportPass(Registry);
    intel::initializeGenericAddressStaticResolutionPass(Registry);
    intel::initializeGenericAddressDynamicResolutionPass(Registry);
    intel::initializeSpirMaterializerPass(Registry);
    intel::initializeObfuscationPass(Registry);
    intel::initializeSubGroupAdaptationPass(Registry);
    intel::initializeLinearIdResolverPass(Registry);
    intel::initializePrepareKernelArgsPass(Registry);
    intel::initializeReduceAlignmentPass(Registry);
    intel::initializeDetectFuncPtrCallsPass(Registry);
    intel::initializeResolveWICallPass(Registry);
    intel::initializeCloneBlockInvokeFuncToKernelPass(Registry);
    intel::initializeResolveBlockToStaticCallPass(Registry);
    intel::initializeDetectRecursionPass(Registry);
    intel::initializeDebugInfoPassPass(Registry);
    intel::initializeSmartGVNPass(Registry);
    intel::initializeDeduceMaxWGDimPass(Registry);
    intel::initializeRenderscriptVectorizerPass(Registry);
    intel::initializeSinCosFoldPass(Registry);
    intel::initializePreLegalizeBoolsPass(Registry);
    intel::initializeOCLAliasAnalysisPass(Registry);
    intel::initializeSPIR20BlocksToObjCBlocksPass(Registry);
    intel::initializePrintfArgumentsPromotionPass(Registry);
}


void InitOCLOpt(llvm::LLVMContext& context)
{
    //---=== Pre Command Line Initialization ===---
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeOCLPasses(Registry);
}

static void GenerateEmptyModuleAndBuffer(Module* &module, MemoryBuffer* &buffer, llvm::LLVMContext& context)
{
  module = new Module("empty", context);
  // serialize to LLVM bitcode
  llvm::SmallVector<char, 1024>* moduleBuffer = new llvm::SmallVector<char, 1024>();
  llvm::raw_svector_ostream ir_ostream(*moduleBuffer);
  llvm::WriteBitcodeToFile(module, ir_ostream);
  buffer = new ObjectMemoryBuffer(std::move(*moduleBuffer));
}

extern "C" llvm::ImmutablePass * createImplicitArgsAnalysisPass(LLVMContext *C);
void InitOCLPasses( llvm::LLVMContext& context, llvm::legacy::PassManager& passMgr )
{
  //---=== Post Command Line Initialization ===---
  // *** Vectorizer initializations
  // Obtain the runtime modules (either from input, or generate empty ones)
  llvm::SmallVector<llvm::Module*, 2> runtimeModuleList;
  llvm::SmallVector<MemoryBuffer*, 2> runtimeBufferList;

  if (RuntimeLib.size() != 0) {
    for (unsigned i = 0; i != RuntimeLib.size(); ++i)
    {
      if (RuntimeLib[i] == "") {
        Module* empty = nullptr;
        MemoryBuffer* emptyBuffer = nullptr;
        GenerateEmptyModuleAndBuffer(empty, emptyBuffer, context);
        runtimeModuleList.push_back(empty);
        runtimeBufferList.push_back(emptyBuffer);
      }
      else {
        llvm::SMDiagnostic Err;
        std::unique_ptr<llvm::Module> runtimeModule = llvm::getLazyIRFileModule(RuntimeLib[i], Err, context);
        if (!runtimeModule) {
          errs() << "Runtime error reading IR from \"" << RuntimeLib[i] << "\":\n";
          Err.print("Error: ", errs());
          exit(1);
        }
        runtimeModuleList.push_back(runtimeModule.release());
        llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> rtlBufferOrErr = llvm::MemoryBuffer::getFile(RuntimeLib[i]);
        if (!rtlBufferOrErr)
        {
          errs() << "Runtime error reading file from \"" << RuntimeLib[i] << "\":\n";
          errs() << "Error: " << rtlBufferOrErr.getError().message();
          exit(1);
        }
        runtimeBufferList.push_back(rtlBufferOrErr.get().release());
      }
    }
  }
  else {
    Module* empty = nullptr;
    MemoryBuffer* emptyBuffer = nullptr;
    GenerateEmptyModuleAndBuffer(empty, emptyBuffer, context);
    runtimeModuleList.push_back(empty);
    runtimeBufferList.push_back(emptyBuffer);
  }

  //Always add the BuiltinLibInfo Pass to the Pass Manager
  passMgr.add(createBuiltinLibInfoPass(runtimeModuleList, runtimeBufferList, RuntimeServices));
  passMgr.add(createImplicitArgsAnalysisPass(&context));

  Intel::CPUId cpuId = Intel::OpenCL::DeviceBackend::Utils::CPUDetect::GetInstance()->GetCPUId();
  passMgr.add(createBuiltInImportPass(cpuId.GetCPUPrefix()));
}

int main(int argc, char **argv) {
  RegisterPluginInit(InitOCLOpt);
  RegisterPluginInitPasses(InitOCLPasses);
  return mainImp(argc, argv);
}
