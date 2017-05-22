/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"
#include "moduleManager/ModuleManager.h"



class Vectorizer : public ModulePass {
public:
	static char ID;
	Vectorizer(const Module * m, unsigned archVectorWidth) : ModulePass((intptr_t)&ID),runtimeModule(m) 
	{
		numOfKernels = 0;
		isModuleVectorized = false;
		m_archVectorWidth = archVectorWidth;
		V_INIT_PRINT;
		V_PRINT("Vectorizer Constructor\n");
	}
	~Vectorizer() 
	{
		V_PRINT("Vectorizer Destructor\n"); 
		V_DESTROY_PRINT;
	}
	virtual llvm::StringRef getPassName() const {
		return "Intel OpenCL Vectorizer";
	}
	virtual bool runOnModule(Module &M);
	
	// Inform about usage/mofication/dependency of this pass
	virtual void getAnalysisUsage(AnalysisUsage &AU) const {}
	
	// Functions for filling data structures with functions info
	int getVectorizerFunctions(SmallVectorImpl<Function*> &Functions);
	int getVectorizerWidths(SmallVectorImpl<int> &MaxWidths);

private:
	Vectorizer(); // Do not implement

	SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> scalarFuncsList; // holds all the "original" (scalar) functions

	// Lists for holding the vectorized kernels data, after vectorization (for runtime query)
	SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> targetFunctionsList;
	SmallVector<int,       ESTIMATED_NUM_OF_FUNCTIONS> targetFunctionsMaxWidth;
	
	// Pointer to runtime module (which holds the OCL builtin functions)
	const Module * runtimeModule;

	// Number of kernels in current module
	unsigned numOfKernels;
	// Was current module vectorized
	bool isModuleVectorized;
	// Used vector width 
	unsigned m_archVectorWidth;
};


#endif // __MAIN_H__

