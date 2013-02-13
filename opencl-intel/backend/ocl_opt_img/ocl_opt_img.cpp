
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

extern "C" 
{
    extern void* createBuiltInImportPass(Module* pRTModule);
}

void initializeOCLPasses(PassRegistry &Registry)
{
    intel::initializeShuffleCallToInstPass(Registry);
    intel::initializeBIImportPass(Registry);
}

void InitOCLOpt(llvm::LLVMContext& context)
{
    //---=== Pre Command Line Initialization ===---
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeOCLPasses(Registry);
}

int main(int argc, char **argv) 
{
    RegisterPluginInit(InitOCLOpt);
    return mainImp(argc, argv);
}