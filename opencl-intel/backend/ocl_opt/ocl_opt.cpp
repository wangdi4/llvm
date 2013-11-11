
#include "llvm/Module.h"
#include "llvm/PassRegistry.h"
#include "llvm/PassManager.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/opt.h"
#include "InitializePasses.h"

int mainImp(int argc, char **argv);

using namespace llvm;

static cl::opt<std::string>
RuntimeLib("runtimelib",
                  cl::desc("Runtime declarations (bitCode) library"),
                  cl::value_desc("filename"), cl::init(""));

static cl::opt<std::string>
RuntimeServices("runtime",
                  cl::desc("Runtime services type (ocl/dx/apple)"),
                  cl::value_desc("runtime_type"), cl::init("ocl"));

static cl::opt<bool>
MicPasses("mic-passes",
                 cl::desc("Include optimization passes specific for MIC achitecture."));


extern "C" Pass* createBuiltinLibInfoPass(Module* pRTModule, std::string type);

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
    intel::initializeShuffleCallToInstPass(Registry);
    intel::initializeInstToFuncCallPass(Registry);
    intel::initializeModuleCleanupPass(Registry);
    intel::initializeAddImplicitArgsPass(Registry);
    intel::initializeOclFunctionAttrsPass(Registry);
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
    intel::initializeLinearIdResolverPass(Registry);
    intel::initializePrepareKernelArgsPass(Registry);
    intel::initializeReduceAlignmentPass(Registry);
    intel::initializeDetectFuncPtrCallsPass(Registry);
    intel::initializeResolveWICallPass(Registry);
    intel::initializeCloneBlockInvokeFuncToKernelPass(Registry);
    intel::initializeResolveBlockToStaticCallPass(Registry);
    intel::initializeDetectRecursionPass(Registry);
    intel::initializeDebugInfoPassPass(Registry);
}


void InitOCLOpt(llvm::LLVMContext& context)
{
    //---=== Pre Command Line Initialization ===---
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeOCLPasses(Registry);
}

void InitOCLPasses( llvm::LLVMContext& context, llvm::PassManager& passMgr )
{
  //---=== Post Command Line Initialization ===---
  llvm::SMDiagnostic Err;
  // *** Vectorizer initializations
  // Obtain the runtime module (either from input, or generate empty ones)
  std::auto_ptr<llvm::Module> runtimeModule;

  if (RuntimeLib != "") {

    runtimeModule.reset(llvm::ParseIRFile(RuntimeLib, Err, context));

    if (runtimeModule.get() == 0) {
      errs() << "Runtime file error!\n";
      return;
    }
  }
  else {
    runtimeModule.reset(new Module("empty", context));
  }

  //Always add the BuiltinLibInfo Pass to the Pass Manager
  passMgr.add(createBuiltinLibInfoPass(runtimeModule.release(), RuntimeServices));
}

int main(int argc, char **argv) {
  RegisterPluginInit(InitOCLOpt);
  RegisterPluginInitPasses(InitOCLPasses);
  return mainImp(argc, argv);
}
