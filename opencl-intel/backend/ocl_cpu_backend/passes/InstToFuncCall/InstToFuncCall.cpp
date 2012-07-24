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

File Name:  InstToFuncCall.cpp

\*****************************************************************************/
#include "InstToFuncCall.h"

#include <llvm/Instructions.h>
#include <llvm/Constants.h>

using namespace llvm;

extern "C" {
  /// @brief Creates new InstToFuncCall module pass
  /// @returns new InstToFuncCall module pass
  void* createInstToFuncCallPass(bool isMic) {
    return new Intel::OpenCL::DeviceBackend::InstToFuncCall(isMic);
  }
}

/// Register pass to for opt
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::InstToFuncCall> InstToFuncCallPass("inst-to-func-call", "Replaces LLVM IR instructions with calls to functions.");


namespace Intel { namespace OpenCL { namespace DeviceBackend {

    char InstToFuncCall::ID = 0;

    InstToFuncCall::InstToFuncCall(bool isMic) : ModulePass(ID), m_I2F(isMic) {}

    /// Replaces instruction 'inst' with call to function 'funcName' which has a
	/// calling convention 'CC'.
    void InstToFuncCall::replaceInstWithCall(Function *func,
        Instruction* inst, const char* funcName, CallingConv::ID CC)
    {

		Value *Op0 = inst->getOperand(0);

		std::vector<Value*>      args (1, Op0); // arguments
		std::vector<Type*> types(1, Op0->getType()); // type of args

		FunctionType *proto = FunctionType::get(inst->getType(), types, false);
		Constant* new_f = func->getParent()->getOrInsertFunction(funcName, proto);
		CallInst* call = CallInst::Create(new_f, ArrayRef<Value*>(args), "call_conv", inst);
		call->setCallingConv(CC);
		// replace all users with new function call, DCE will take care of it
		inst->replaceAllUsesWith(call);
	}

    bool InstToFuncCall::runOnModule(Module &M)
    {
        bool changed = false;

        // for each function

        Module::FunctionListType &FL = M.getFunctionList();
        for (Module::iterator fn = FL.begin(), fne = FL.end(); fn != fne; ++fn)
        {
            // for each bb
            for (Function::iterator bb = fn->begin(), bbe = fn->end(); bb != bbe; ++bb) {
                // for each instruction
                for(BasicBlock::iterator it = bb->begin(), e = bb->end(); it != e ; ++it) {
                    // See if a mapping exists for replacing this instruction class
                    const Inst2FunctionLookup::LookupValue *LV = m_I2F[*it];
                    if (0 == LV) {
                        continue;
                    }
                    replaceInstWithCall(&(*fn), &(*it), LV->first, LV->second);
                    changed = true;
                }
            }

        }

        return changed;
    }

    ModulePass *createInstToFuncCallPass(bool isMic) { return new InstToFuncCall(isMic); }

    }}} // namespace

