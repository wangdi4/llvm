#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Threading.h"

#include "llvm/Bitcode/CSASaveRawBC.h"

using namespace llvm;

char CSASaveRawBC::ID = 0;

std::string CSASaveRawBC::BcData;

static once_flag save_raw_bc_init_flag;

static cl::opt<bool>
DumpRawBc("csa-dump-raw-bc", cl::Hidden,
           cl::desc("CSA Specific: Dump raw bitcode to file"),
           cl::init(false));

static void initializePassOnce(PassRegistry &Registry) {
  PassInfo *PI =
    new PassInfo("CSA Save Raw BC",  // name
                 "csa-save-raw-bc",  // arg
                 &CSASaveRawBC::ID,  // pointer to ID
                 nullptr,            // normal ctor
                 true,               // only looks at CFG
                 true);              // is analysis pass
  Registry.registerPass(*PI, true);
}

namespace llvm {

ImmutablePass *createCSASaveRawBCPass() {
  return new CSASaveRawBC();
}

void initializeCSASaveRawBCPass(PassRegistry &Registry) {
  llvm::call_once(save_raw_bc_init_flag, initializePassOnce, std::ref(Registry));
}

CSASaveRawBC::CSASaveRawBC() : ImmutablePass(ID) {
  initializeCSASaveRawBCPass(*PassRegistry::getPassRegistry());
}

bool CSASaveRawBC::doInitialization(Module &M) {

  // Only supported by CSA. It would be better to only add the pass if the
  // target is CSA, but I don't seem to have access to that information in
  // the appropriate places
  if (0 != M.getTargetTriple().compare("csa")) {
/*
    errs() << "Not an CSA module - ";
    errs() << M.getTargetTriple();
    errs() << "\n";
*/
    return false;
  }

  //  errs() << "CSASaveRawBC::doInitialization\n";

  // Multiple instances of the analyzer are created by LLVM. We
  // only need to save the raw IR once. If the string is already
  // filled, we're done
  if (! BcData.empty()) {
    return false;
  }

  // Fetch the raw IR
  {
    raw_string_ostream OS(BcData);
    WriteBitcodeToFile(&M, OS);
  }

  // If asked, dump the serialized IR to a file
  if (DumpRawBc) {
    dumpBC(M.getName());
  }

    return false;
}

// Dump the raw IR to a .bc file
void CSASaveRawBC::dumpBC(StringRef modName) {

  // Generate the name for the file by appending ".bc" to the
  // file name. So we'll get something like foo.cpp.bc
  std::string bcName = modName.str() + ".bc";

  // Get the raw IR serialized as a bitcode file
  const std::string &data = getRawBC();
  if (data.empty()) {
    errs() << "serialized IR not available. " << bcName << " not created\n";
    return;
  }

  // Create output file
  std::error_code EC;
  tool_output_file Out(bcName.c_str(), EC, sys::fs::F_None);
  if (EC) {
    errs() << "could not open bitcode file for writing: ";
    errs() << bcName;
    errs() << "\n";
    return;
  }

  // Write the generated bitstream to the file
  Out.os().write(data.c_str(), data.size());
  Out.os().close();

  if (Out.os().has_error()) {
    errs() << "could not write bitcode file: " << bcName << "\n";
    Out.os().clear_error();
    return;
  }

  Out.keep();
}

const std::string &CSASaveRawBC::getRawBC() const {
  assert(!BcData.empty() && "Expected string to be filled by doInitialization for an CSA module!");

  return BcData;
}

} // namespace llvm
