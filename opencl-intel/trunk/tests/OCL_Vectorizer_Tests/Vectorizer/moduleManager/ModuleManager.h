/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __MODULEMANAGER_H__
#define __MODULEMANAGER_H__

#include "../common.h"
#include "../scalarize/scalarize.h"
#include "../vectorize/vectorize.h"
#include "../controlflow/controlflow.h"
#include "../funcScans/scanfunction.h"
#include "../masterLoop/masterLoop.h"


/***
 * This Class manages the Module as a whole. Sends functions for scalarization/vetctorization/etc..
 ***/

class VectModuleManager {
public:
	// Constructor. Receives pointer to Module 
	VectModuleManager(Module *targetModule, const Module *runtimeMod, unsigned archVectorWidth);
	~VectModuleManager();
	bool runOnModule(SmallVectorImpl<Function*> &functionsList);
	void getVectorizedFunctionsPointers(SmallVectorImpl<Function*> &vectorFunctions);
	void getVectorizedFunctionsWidths(SmallVectorImpl<int> &vectorMaxWidths);
		
private:
	VectModuleManager(); // Do not implement
	bool createConvertedFunction(Function * F);
	void removeDeadCode(Function * F, CodeProperties * funcProperties);
	void prepareKernelForLoop(Function * scalarFunc, CodeProperties * funcProperties);
	void addVectorWidthCheck(Function * scalarFunc, CodeProperties * funcProperties);
#if defined(VERIFY_FUNCS)
	bool checkFunctionInBuiltin(const char * name);
	void VerifyFunctionsListsAreUpToDate();
#endif	
	
	// Special case functions handling
	bool resolveSpecialCaseFunctions(Function *F, CodeProperties * funcProperties);
	void eraseSpecialCaseFakeFunctions(CodeProperties * funcProperties);
	bool dispatchSpecialCaseFunc(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveReadSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties, bool is2D);
	bool resolveStreamReadSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveWriteSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveStreamWriteSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveSelectCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveGeometricFuncCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties, 
								   geometricListType funcList, unsigned numArgs, bool isReducted);
	bool resolveCIGammaCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	bool resolveEarlyExitCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties);
	Value * castArgumentIfNeeded(Value * inputVal, const Type * requiredType, Instruction * insertPoint, CodeProperties * funcProperties);
	
	bool singleVisit;
	
	SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> vectoredKernelsList;
	SmallVector<int,       ESTIMATED_NUM_OF_FUNCTIONS> vectoredKernelsMaxWidth;
	
	bool isModuleModified;

	// Pointer to current module
	Module * currentModule;

	// Pointer to runtime module (which holds the OCL builtin functions)
	const Module * runtimeModule;

	// Used vector width 
	unsigned m_archVectorWidth;
	
	// ControlFlow, Vectorizer and scalarizer objects
	HandleControlFlow controlFlowObject;
	ScalarizeFunction scalObject;
	VectorizeFunction vectObject;
	ScanFunction scanObject;
	LoopGen loopObject;
};

#endif // __MODULEMANAGER_H__
