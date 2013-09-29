
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
VectorizerRuntimeLib("runtimelib",
                  cl::desc("Runtime declarations (bitCode) library, to be used by vectorizer"),
                  cl::value_desc("filename"), cl::init(""));

static cl::opt<std::string>
VectorizerServices("runtime",
                  cl::desc("Runtime services type (ocl/dx)"),
                  cl::value_desc("runtime_type"), cl::init("ocl"));

static cl::opt<bool>
MicPasses("mic-passes",
                 cl::desc("Include optimization passes specific for MIC achitecture."));


extern "C" {
extern void* createVolcanoOpenclRuntimeSupport(const Module *runtimeModule,
                                        unsigned packetizationWidth);
extern void* createDXRuntimeSupport(const Module *runtimeModule,
                                        unsigned packetizationWidth);
extern void* createAppleOpenclRuntimeSupport(const Module *runtimeModule,
                                        unsigned packetizationWidth);
extern void* createBuiltInImportPass(Module* pRTModule);

extern void* createSpirMaterializer();

extern void* createObfuscation();

}

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
    intel::initializeRedundantPhiNodePass(Registry);
    intel::initializeGroupBuiltinPass(Registry);
    intel::initializeBarrierInFunctionPass(Registry);
    intel::initializeRemoveDuplicationBarrierPass(Registry);
    intel::initializeSplitBBonBarrierPass(Registry);
    intel::initializeBarrierPass(Registry);
    intel::initializeWIRelatedValuePass(Registry);
    intel::initializeDataPerBarrierPass(Registry);
    intel::initializeDataPerValuePass(Registry);
    intel::initializeDataPerInternalFunctionPass(Registry);
    intel::initializePreventDivCrashesPass(Registry);
    intel::initializeShuffleCallToInstPass(Registry);
    intel::initializeInstToFuncCallPass(Registry);
    intel::initializeModuleCleanupPass(Registry);
    intel::initializeAddImplicitArgsPass(Registry);
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
    intel::initializeDetectFuncPtrCallsPass(Registry);
    intel::initializeResolveWICallPass(Registry);
    intel::initializeCloneBlockInvokeFuncToKernelPass(Registry);
    intel::initializeResolveBlockToStaticCallPass(Registry);
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

  if (VectorizerRuntimeLib != "") {

    runtimeModule.reset(llvm::ParseIRFile(VectorizerRuntimeLib, Err, context));

    if (runtimeModule.get() == 0) {
      errs() << "Runtime file error!\n";
      return;
    }
  }
  else {
    runtimeModule.reset(new Module("empty", context));
  }

  // Generate runtimeSupport object, to be used as input for vectorizer
  if (VectorizerServices == "ocl") {
    createVolcanoOpenclRuntimeSupport(runtimeModule.release(), 4);
  } else if (VectorizerServices == "dx") {
    createDXRuntimeSupport(runtimeModule.release(), 4);
  } else if (VectorizerServices == "apple") {
    createAppleOpenclRuntimeSupport(runtimeModule.release(), 4);
  } else {
    errs()<<"Unknown runtime services \""<<VectorizerServices<<"\"\n";
  }

}

int main(int argc, char **argv) {
  RegisterPluginInit(InitOCLOpt);
  RegisterPluginInitPasses(InitOCLPasses);
  return mainImp(argc, argv);
}
