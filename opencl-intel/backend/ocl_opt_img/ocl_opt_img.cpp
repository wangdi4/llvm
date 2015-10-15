
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

static cl::list<std::string>
RuntimeLib(cl::CommaSeparated, "runtimelib",
                  cl::desc("Runtime declarations (bitCode) libraries (comma separated)"),
                  cl::value_desc("filename1,filename2"));

// BIImport pass resolves svml calls of "shared" functions:
// if CPU is l9, __ocl_svml_shared_acos1f to be changed to __ocl_svml_l9_acos1f
// so, BIImport must be initialized by the name of CPU architecture (l9, for example)
static cl::opt<std::string>
arch("arch",
            cl::desc("CPU architecture name for svml library"),
            cl::value_desc("CPU architecture type string"), cl::init(""));

extern "C" Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> builtinsList, std::string type);
extern "C" Pass* createBuiltInImportPass(const char* CPUName);

void initializeOCLPasses(PassRegistry &Registry)
{
    intel::initializeBuiltinCallToInstPass(Registry);
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
  // Obtain the runtime modules (either from input, or generate empty ones)
  llvm::SmallVector<Module*, 2> runtimeModuleList;

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
  passMgr.add(createBuiltinLibInfoPass(runtimeModuleList, ""));
  //add the BIImport Pass to the Pass Manager
  passMgr.add(createBuiltInImportPass(arch.c_str()));
}

int main(int argc, char **argv) 
{
    RegisterPluginInit(InitOCLOpt);
    RegisterPluginInitPasses(InitOCLPasses);
    return mainImp(argc, argv);
}
