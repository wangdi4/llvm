#include "llvm/IR/Module.h"
#include "llvm/PassRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/opt.h"
#include "InitializePasses.h"

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

extern "C" Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> builtinsList, std::string type);

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
    intel::initializeAVX512ResolverPass(Registry);
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
    intel::initializeBlockToFuncPtrPass(Registry);
    intel::initializeChannelPipeTransformationPass(Registry);
    intel::initializePipeSupportPass(Registry);
    intel::initializeFMASplitterPass(Registry);
}


void InitOCLOpt(llvm::LLVMContext& context)
{
    //---=== Pre Command Line Initialization ===---
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeOCLPasses(Registry);
}

extern "C" llvm::ImmutablePass * createImplicitArgsAnalysisPass(LLVMContext *C);
void InitOCLPasses( llvm::LLVMContext& context, llvm::legacy::PassManager& passMgr )
{
  //---=== Post Command Line Initialization ===---
  // *** Vectorizer initializations
  // Obtain the runtime modules (either from input, or generate empty ones)
  llvm::SmallVector<llvm::Module*, 2> runtimeModuleList;

  if (RuntimeLib.size() != 0) {
    for (unsigned i = 0; i != RuntimeLib.size(); ++i)
    {
      if (RuntimeLib[i] == "") {
        runtimeModuleList.push_back(new Module("empty", context));
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
      }
    }
  }
  else {
    runtimeModuleList.push_back(new Module("empty", context));
  }

  //Always add the BuiltinLibInfo Pass to the Pass Manager
  passMgr.add(createBuiltinLibInfoPass(runtimeModuleList, RuntimeServices));
  passMgr.add(createImplicitArgsAnalysisPass(&context));
}

int main(int argc, char **argv) {
  RegisterPluginInit(InitOCLOpt);
  RegisterPluginInitPasses(InitOCLPasses);
  return mainImp(argc, argv);
}
