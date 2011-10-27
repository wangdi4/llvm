/////////////////////////////////////////////////////////////////////////
// WrapSvmlPass.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

// This pass is used for relaxed functions substitution 

#include "Kernel.h"
#include "CompilationUtils.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Target/TargetData.h>

#include <list>
#include <map>
#include <string>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;

#define SZ_XMM_REG 128

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class WrapSvmlPass : public ModulePass
{
public:
	WrapSvmlPass( LLVMContext *pContext) : ModulePass(ID),m_pLLVMContext(pContext){}

	// doPassInitialization - For this pass, it removes global symbol table
	// entries for primitive types.  These are never used for linking in GCC and
	// they make the output uglier to look at, so we nuke them.
	//
	// Also, initialize instance variables.
	//
	bool runOnModule(Module &M);

protected:
		static char ID; // Pass identification, replacement for typeid

		Module*			m_pModule;
        LLVMContext*	m_pLLVMContext;
        // SVML thunk function pointer creation
        Value *CreateThunkFunction();
        // SVML function call wrapping with thunk function call
        Instruction* WrapSVMLCall(CallInst *pCall, Value *thunkValue, AllocaInst *alloca_buf);
};

char WrapSvmlPass::ID = 0;

ModulePass *createSvmlWrapperPass( LLVMContext *pContext) 
{
	return new WrapSvmlPass( pContext);
}

bool WrapSvmlPass::runOnModule(Module &M)
{
	m_pModule = &M;

    // Prepare thunk function object
    Value* thunkValue = CreateThunkFunction();

	Module::iterator funct_it, func_end;
	for (funct_it = M.begin(), func_end = M.end(); funct_it != func_end; ++funct_it)
	{
		Function* pFunction = &*funct_it;

		// Go through function blocks
		Function::iterator bb_it, bb_end;
		
		AllocaInst *alloca_buf = 0;

		if (!pFunction->isDeclaration()) {
			BasicBlock::iterator inst_it = pFunction->getBasicBlockList().begin()->begin();
			// TODO FIXME - the insertion of AllocaInst before SECOND instruction (unless 1st instruction is 'call'
            // is a workaround. In ideal we should have AllocaInst in the beginning of the function. 
            // However JIT crashes in the latter case
            if (inst_it->getOpcode() != Instruction::Call && pFunction->getBasicBlockList().begin()->size() > 1) {
                inst_it++;
            }
			// allocate save area buffer (alloca_buf = new (int8*)[3];)
			alloca_buf = new AllocaInst(Type::getInt8PtrTy(*m_pLLVMContext), 
						ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 3), "save_buffer", inst_it);
		}

        for (bb_it = pFunction->getBasicBlockList().begin(), 
             bb_end = pFunction->getBasicBlockList().end(); 
             bb_it != bb_end; ++bb_it)
        {
			BasicBlock::iterator inst_it = bb_it->begin();
			while ( inst_it != bb_it->end() )
			{
				if (inst_it->getOpcode() == Instruction::Call)
				{
                    CallInst* pCall = dyn_cast<CallInst>(inst_it);
                    if ( pCall->getCallingConv() == CallingConv::X86_Svml) {
                        // Apply wrapper around SVML calls in order to adjust Win64-prepared stack frame to that of SVML
                        bool isIntegerParam = false;
                        unsigned int sizeOfFloatDoubleVectors = 0; // in bits

                        // Check that there is any integer/pointer parameter or
                        // float/double vector(s) which overflow 4 XMMs
                        for (unsigned int i = 0; i < pCall->getNumArgOperands(); i++ ) {                            
                          const Type * type = pCall->getArgOperand(i)->getType();
                          if (type->isIntOrIntVectorTy() || type->isPointerTy() || type->isArrayTy()) {
                             isIntegerParam = true;
                             break;
                          }
                          if (type->isVectorTy()) {
                            const VectorType *vType = dyn_cast<VectorType>(type);
                            if (vType->getElementType()->isFloatTy() || vType->getElementType()->isDoubleTy()) {
                                sizeOfFloatDoubleVectors += vType->getBitWidth();
                            }
                          }
                        }
                        // The check below assumes that SVML expects to find on stack  
                        // all float/double vectors beyond size of 4 XMMs
                        if (isIntegerParam || sizeOfFloatDoubleVectors > 4*SZ_XMM_REG) {
				            Instruction *newInst = WrapSVMLCall( pCall, thunkValue, alloca_buf);
                            pCall->replaceAllUsesWith(newInst);
                            inst_it = CompilationUtils::removeInstruction(bb_it, inst_it);
                            continue;
                        }
			        }
                }
                ++inst_it;
            }
        }
	}
	return true;
}

// Creates SVML thunk function pointer
Value *WrapSvmlPass::CreateThunkFunction() 
{
        std::vector<const Type *> args;
        args.push_back(Type::getInt8PtrTy(*m_pLLVMContext));
        FunctionType *FnType = FunctionType::get( Type::getVoidTy(*m_pLLVMContext), args, false);
        const char *funcName = "SvmlThunk";
        Value *thunkValue = m_pModule->getOrInsertFunction( funcName, FnType);
        return thunkValue;
}

// Replacement of SVML call with that of SvmlThunk call with a save area
// pointer as a first (induced) parameter. The save area is immediately
// populated with SVML function pointer.
// Parameters: original CALL instruction object, pointer to SVML thunk function
Instruction* WrapSvmlPass::WrapSVMLCall(CallInst *pCall, Value *thunkValue, AllocaInst *alloca_buf) 
{
    
		Function *pCallee = pCall->getCalledFunction();
        
        // Create wrapper parameters and their types
		SmallVector<Value*, 4> params;
        std::vector<const Type*> wrapperTypes;

        /*
        Replace original SVML call with SVML wrapper
        Put SVML function pointer into save area buffer
        Insert pointer to save area buffer as first parameter
        Take all original parameters from SVML call and insert them.
        */


        // populate save area buffer with SVML function pointer
        Value *storeValue = CastInst::Create(Instruction::BitCast, pCallee, 
                Type::getInt8PtrTy(*m_pLLVMContext), "functionPointer", pCall);
        StoreInst *storeInst = new StoreInst(storeValue, alloca_buf, pCall);
        
        // first parameter to SVML thunk wrapper is the pointer to save area buffer
        wrapperTypes.push_back(Type::getInt8PtrTy(*m_pLLVMContext));
        // NOTE !!!! Due to 16-byte alignments issues (we remove first parameter before calling svml function) 
        // we have to insert 1st parameter twice to keep alignment
        wrapperTypes.push_back(Type::getInt8PtrTy(*m_pLLVMContext));
        CastInst* pBitCast = CastInst::Create(Instruction::BitCast, alloca_buf, 
                wrapperTypes[0], "bufferPointer", pCall);
        // put pointer to buffer first
        params.push_back(pBitCast);
        // NOTE !!!! Due to 16-byte alignments issues (we remove first parameter before calling svml function) 
        // we have to insert additional parameter to keep alignment
        params.push_back(pBitCast);

        // put all other parameters
		for(unsigned int i = 0; i < pCall->getNumArgOperands(); i++ )
		{
            wrapperTypes.push_back(pCall->getArgOperand(i)->getType());
            params.push_back(pCall->getArgOperand(i));
		}

        // bit cast of thunk function pointer to function object
        FunctionType *svmlWrapperType = FunctionType::get(pCallee->getReturnType(), wrapperTypes, false);
        thunkValue = CastInst::Create(Instruction::BitCast, thunkValue, 
                                    Type::getInt8PtrTy(*m_pLLVMContext), "thunk_pointer", pCall);
        Value *svmlWrapper = CastInst::Create(Instruction::BitCast, thunkValue, PointerType::get(svmlWrapperType, 0), "tcast", pCall);
        
        // call to thunk function
		CallInst *newCall = CallInst::Create(svmlWrapper, params.begin(), params.end(), "", pCall);
        newCall->setCallingConv(CallingConv::X86_SvmlThunk);
        return newCall;
}

}}}
