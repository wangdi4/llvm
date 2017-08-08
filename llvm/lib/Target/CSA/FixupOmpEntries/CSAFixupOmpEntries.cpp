//===- CSAFixupOmpEntries.cpp - Fixup OpenMP offloading entries -===//
//
//===----------------------------------------------------------------===//
//
// When we have anything other than a simple module, the offloading
// entries that OpenMP is emmitting are getting killed by the dead
// globals elimination pass. To prevent this, we need to change the
// linkage from "internal" to "global". This pass searches for global
// variables that look like their part of the OpenMP offloading array
// and changes their linkage. It's a stand-alone pass that the OpenMP
// offloading plugin will invoke using the opt tool.
//
//===----------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "csa-fixup-omp-entries"

using namespace llvm;

namespace {

struct CSAFixupOmpEntries : public ModulePass {
  static char ID;

  CSAFixupOmpEntries() : ModulePass{ID} {
    //    initializeCSAFixupOmpEntriesPass(*PassRegistry::getPassRegistry());
  }


  bool runOnModule(Module &M) override;

  StringRef getPassName() const override {
    fprintf(stderr, "CSAFixupOmpEntries::getPassName\n");
    return "CSA fix linkage of OpenMP offloading entries";
  }
};

}  // namespace

char CSAFixupOmpEntries::ID = 0;


static RegisterPass<CSAFixupOmpEntries>
    X("csa-fixup-omp-entries",
      "Fixup OpenMP entries",
      false,
      false);

bool CSAFixupOmpEntries::runOnModule(Module &M) {

  DEBUG(
        dbgs() << "CSAFixupOmpEntries::runModule\n"
  );

  bool Changed = false;
  bool inEntriesSection = false;

  for (Module::global_iterator GVI = M.global_begin(), E = M.global_end();
       GVI != E; ) {

    GlobalVariable *GV = &*GVI++;

    DEBUG(
          dbgs() << "Found GlobalVariable " << GV->getName() << "\n"
    );

    // The entry starts with a variable in the .omp_offloading.entries
    // section. All of the names we're interested start with the string
    // ".omp_offloading" - ".omp_offloading.entry*",
    // ".omp_offloading.entry_name*"

    if (GV->hasSection()) {
      StringRef sectionName = GV->getSection();
      inEntriesSection = (sectionName == ".omp_offloading.entries");
    }

    if (inEntriesSection) {
      StringRef name = GV->getName();
      if (! name.startswith(".omp_offloading.")) {
        inEntriesSection = false;
      } else {    
        // If the linkage type is internal fix it
        if (GV->hasInternalLinkage()) {
          DEBUG(
                dbgs() << "Converting " << GV->getName() <<
                          " to global linkage\n"
          );
          GV->setLinkage(GlobalValue::ExternalLinkage);
          Changed = true;
        }
      }
    }

    // Set the linkage type to external so it's not discarded
  }

  return Changed;
}
