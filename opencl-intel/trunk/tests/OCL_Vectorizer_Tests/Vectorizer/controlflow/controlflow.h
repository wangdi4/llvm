/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __CONTROL_FLOW_H__
#define __CONTROL_FLOW_H__

#include "../common.h"
#include "../specialCaseFuncs.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

// Define a nesting level limit, so as not to drown in a loop, thinking it is a nested if-then-else structure...
#define IF_NESTING_LEVEL_LIMIT 20 


/// ********* Scalarization phase specific members ********
class HandleControlFlow {
public:
	HandleControlFlow();
	~HandleControlFlow();
	bool analyzeControlFlow(CodeProperties * functionProp);
private:
	void scanFunctionForIfThenElse();
	bool validateNoSideEffect(BasicBlock * BB);
	bool mergeIfThenElseBlock(BasicBlock * headBB, unsigned nestingLevel);
	bool validateCallHasNoSideEffect(CallInst * inst);

	// Early-exit detection and handling
	void scanForBoundaryChecks();
	bool findAndCollapseEarlyExit(BasicBlock * kernelExitBlock);
	bool searchUpConditional(Value * rootCondition, bool isEarlyExitOnTrueSize, SmallVectorImpl<ICmpInst*> &conditionalsList, SmallVectorImpl<Instruction*> &instsToRemove);
	void replaceEarlyExit(ICmpInst * compare, bool earlyExitIsTrue);
		
	// Pointer to current functions' properties
	CodeProperties * funcProperties;
	
};
	

#endif // __CONTROL_FLOW_H__

