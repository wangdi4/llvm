// Copyright (c) 2016 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "BuiltInAttributesImport.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>

using namespace llvm;
namespace intel {

  char BIAttrImport::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BIAttrImport, "builtin-attr-import", "Import built-in attributes", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BIAttrImport, "builtin-attr-import", "Import built-in attributes", false, true)

  bool BIAttrImport::runOnModule(Module &M) {
    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();

    if (BLI.getBuiltinModules().empty()) {
      // If there are no built-in libraries, then nothing can be imported.
      return false;
    }

    bool ret = false;
    for(Function & userF: M) {
      // Look for such a function in in the built-in libraries
      for(Module const * module : BLI.getBuiltinModules()) {
        Function * biF = module->getFunction(userF.getName());
        if(!biF) continue;
        // Copy attributes from the found built-in
        userF.setAttributes(biF->getAttributes());
        ret = true;
      }
    }

    return ret;
  }
} // namespace intel

extern "C" llvm::ModulePass* createBuiltInAttributesImportPass() {
  return new intel::BIAttrImport();
}
