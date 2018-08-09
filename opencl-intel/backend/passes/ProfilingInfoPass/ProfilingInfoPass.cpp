// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>

#include <list>

using namespace llvm;
using namespace std;

namespace intel {


// Cleans up debug info in the module to be suitable for profiling
//
class ProfilingInfoPass : public ModulePass 
{
public:
    ProfilingInfoPass() 
        : ModulePass(ID)
    {
    }

    bool runOnModule(Module& M);

private:
    // Invoked on each user-implemented function in the module
    //
    void runOnUserFunction(Function* pFunc);

private:
    static char ID; // LLVM pass ID
};

char ProfilingInfoPass::ID = 0;

bool ProfilingInfoPass::runOnModule(Module& M)
{
    // Run runOnUserFunction on all the functions in the module.
    //
    Module::iterator func_iter = M.begin();
    for (; func_iter != M.end(); ++func_iter) {
        runOnUserFunction(&(*func_iter));
    }

    return true;
}


void ProfilingInfoPass::runOnUserFunction(Function* pFunc)
{
    list<Instruction*> instrs_to_remove;

    for (Function::iterator block_iter = pFunc->begin();
         block_iter != pFunc->end(); ++block_iter) {
        for (BasicBlock::iterator instr_iter = block_iter->begin();
             instr_iter != block_iter->end(); ++instr_iter) {
            if (CallInst* call_instr = dyn_cast<CallInst>(instr_iter)) {
                Function* called_func = call_instr->getCalledFunction();
                assert(called_func &&
                       "Unexpected indirect function invocation");
                string funcname = called_func->getName().str();
                if (funcname == "llvm.dbg.declare" ||
                    funcname == "llvm.dbg.value") {
                    instrs_to_remove.push_back(call_instr);
                }
            }
        }
    }

    // Clean up the instructions scheduled for removal
    //
    for (list<Instruction*>::const_iterator i = instrs_to_remove.begin();
         i != instrs_to_remove.end(); ++i) {
        (*i)->eraseFromParent();
    }
}

} // namespace intel {

extern "C"{
  ModulePass* createProfilingInfoPass() {
    return new intel::ProfilingInfoPass();
  }
}
