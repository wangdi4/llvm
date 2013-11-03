
#include "llvm/IR/Module.h"
#include "llvm/PassRegistry.h"
#include "llvm/PassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
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

extern "C" Pass* createBuiltinLibInfoPass(Module* pRTModule, std::string type);

void initializeOCLPasses(PassRegistry &Registry)
{
    intel::initializeShuffleCallToInstPass(Registry);
    intel::initializeBIImportPass(Registry);
    intel::initializeBuiltinLibInfoPass(Registry);
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
  passMgr.add(createBuiltinLibInfoPass(runtimeModule.release(), ""));
}

int main(int argc, char **argv) 
{
    RegisterPluginInit(InitOCLOpt);
    RegisterPluginInitPasses(InitOCLPasses);
    return mainImp(argc, argv);
}