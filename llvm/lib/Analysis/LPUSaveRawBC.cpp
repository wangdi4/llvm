#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/raw_ostream.h"
#if 1
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Bitcode/ReaderWriter.h"
#endif

#include "llvm/Analysis/LPUSaveRawBC.h"

using namespace llvm;

namespace llvm {
  void initializeLPUSaveRawBCPass(PassRegistry &);
}

namespace {

struct LPUSaveRawBC: public ImmutablePass {
  static char ID;

  std::string BcData;

  // Constructor
  LPUSaveRawBC() : ImmutablePass(ID) {
    initializeLPUSaveRawBCPass(*PassRegistry::getPassRegistry());
  }

  // Initialization - Pretty much all that we have to work with. Called before
  // any module of function passes. Save away a copy of the raw (unoptimized)
  // IR so we have it if anybody wants it later
  bool doInitialization(Module &M) override {
//    errs() << "LPUSaveRawBC::doInitialization\n";
    // Get the BitCode for the module
    assert(BcData.empty() && "Expected string to be empty - called for multiple modules!");
    {
      raw_string_ostream OS(BcData);
      WriteBitcodeToFile(&M, OS);
    }

    return false;
  }  // doInitialization

#if 0
  bool doFinalization(Module &M) override {
    dumpBC(M);
    return false;
  }
#endif

  const char* getPassName() const override {
    return "LPU Save Raw BitCode";
  }

private:
  void dumpBC(Module &M) {
    assert(!BcData.empty() && "Expected string to be filled by doInitialization!");

    // Create output file
    std::string bcName = M.getName().str() + ".bc";
    std::error_code EC;
    tool_output_file Out(bcName.c_str(), EC, sys::fs::F_None);
    if (EC) {
      errs() << "could not open bitcode file for writing: ";
      errs() << bcName;
      errs() << "\n";
      return;
    }

    // Write the generated bitstream to "Out".
    Out.os().write(BcData.c_str(), BcData.size());

    // Close the file
    Out.os().close();

    if (Out.os().has_error()) {
      errs() << "could not write bitcode file: " << bcName << "\n";
      Out.os().clear_error();
      return;
    }

    Out.keep();
  }

}; // struct LPUSaveRawBC

}  // anonymous namespace


char LPUSaveRawBC::ID = 0;

ImmutablePass *llvm::createLPUSaveRawBCPass() {
  return new LPUSaveRawBC();
}

static void initializePassOnce(PassRegistry &Registry) {
  PassInfo *PI = new PassInfo("LPU Save Raw BC", "lpu-save-raw-bc",
                              &LPUSaveRawBC::ID, nullptr, false, false);
  Registry.registerPass(*PI, true);
}

void llvm::initializeLPUSaveRawBCPass(PassRegistry &Registry) {
  CALL_ONCE_INITIALIZATION(initializePassOnce);
}
