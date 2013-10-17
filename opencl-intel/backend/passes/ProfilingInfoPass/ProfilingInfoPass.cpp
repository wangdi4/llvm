/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ProfilingInfoPass.cpp

\*****************************************************************************/

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Constants.h>
#include <llvm/Instructions.h>
#include <llvm/DerivedTypes.h>

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