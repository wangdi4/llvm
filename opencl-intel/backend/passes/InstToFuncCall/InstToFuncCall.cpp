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

#include "InstToFuncCall.h"
#include "OCLPassSupport.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;

extern "C" {
    /// @brief Creates new InstToFuncCall module pass
    /// @returns new InstToFuncCall module pass
    void* createInstToFuncCallPass(bool isV16Supported) {
        return new intel::InstToFuncCall(isV16Supported);
    }
}


namespace intel{

    char InstToFuncCall::ID = 0;

    OCL_INITIALIZE_PASS(InstToFuncCall, "inst-to-func-call", "Replaces LLVM IR instructions with calls to functions", false, false)

    InstToFuncCall::InstToFuncCall(bool isV16Supported) : ModulePass(ID), m_I2F(isV16Supported) {}

    /// Replaces instruction 'II' with call to function 'funcName' which has a
    /// calling convention 'CC'.
    void InstToFuncCall::replaceInstWithCall(Function *func,
        BasicBlock::iterator &II, const char* funcName, CallingConv::ID CC)
    {
        Instruction *inst = &*II;
        // Get number of operands.
        unsigned int numOperands = inst->getNumOperands();

        std::vector<Value*> args;  // arguments
        std::vector<Type*>  types; // type of args
        for (unsigned int i = 0; i < numOperands; ++i)
        {
            Value *Op = inst->getOperand(i);
            args.push_back(Op);
            types.push_back(Op->getType());
        }

        FunctionType *proto = FunctionType::get(inst->getType(), types, false);
        Constant* new_f = func->getParent()->getOrInsertFunction(funcName, proto);
        CallInst* call = CallInst::Create(new_f, ArrayRef<Value*>(args), "call_conv");
        call->setCallingConv(CC);
        llvm::ReplaceInstWithInst(inst->getParent()->getInstList(), II,call);
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
                    replaceInstWithCall(&(*fn), it, LV->first, LV->second);
                    changed = true;
                }
            }

        }

        return changed;
    }

    ModulePass *createInstToFuncCallPass(bool isV16Supported) { return new InstToFuncCall(isV16Supported); }

    } // namespace

