/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"

namespace intel {

	using namespace llvm;

	class SubGroupAdaptation : public ModulePass {

	public:

		static char ID;

		SubGroupAdaptation() : ModulePass(ID) { };

		virtual const char *getPassName() const {
			return "SubGroupAdaptation";
		}

		virtual bool runOnModule(Module &M);

	private:

		Module *m_pModule;

		LLVMContext *m_pLLVMContext;

		IntegerType *m_pSizeT;

		void replaceFunction(Function *oldFunc, std::string newFuncName);
		void replaceWithConst(Function *oldFunc, unsigned constInt, bool isSigned);
		void replaceSubGroupBroadcast(Function *pFunc);
		// Helper for WI function call generation.
		// Generates a call to WI function upon its name and dimension index
		CallInst *getWICall(BasicBlock *pAtEnd, char const* twine, std::string funcName, unsigned dimIdx);
	};
}