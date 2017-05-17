// Copyright (c) 2015 Intel Corporation
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
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include <string>

namespace intel {

/// @brief This pass replaces sub-group built-ins with appropriate IR sequences
/// It goes over all sub-group built-in declarations in a module in order
/// and substitutes with a call to work-group built-in or a constant.
class SubGroupAdaptation : public llvm::ModulePass {

public:
  static char ID;

  SubGroupAdaptation() : ModulePass(ID){};

  virtual llvm::StringRef getPassName() const { return "SubGroupAdaptation"; }

  virtual bool runOnModule(llvm::Module &M);

private:
  llvm::Module *m_pModule;

  llvm::LLVMContext *m_pLLVMContext;

  llvm::IntegerType *m_pSizeT;

  void replaceFunction(llvm::Function *oldFunc, std::string newFuncName);
  void replaceWithConst(llvm::Function *oldFunc, unsigned constInt,
                        bool isSigned);

  // Generate sub_group_broudcast body
  void defineSubGroupBroadcast(llvm::Function *pFunc);

  // Helper for WI function call generation.
  // Generates a call to WI function upon its name and dimension index
  llvm::CallInst *getWICall(llvm::BasicBlock *pAtEnd, char const *instName,
                            std::string funcName, unsigned dimIdx);
};
}
