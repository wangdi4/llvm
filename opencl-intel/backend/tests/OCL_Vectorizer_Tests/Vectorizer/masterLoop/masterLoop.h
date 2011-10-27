/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __MASTER_LOOP_H__
#define __MASTER_LOOP_H__

#include "../common.h"


/// ********* Scalarization phase specific members ********
class LoopGen {
public:
	LoopGen();
	~LoopGen();
	bool generateLoop(CodeProperties * functionProp);
	BasicBlock * generateScalarLoop(Function * scalarFunc, CodeProperties * functionProp);
private:
	bool createMajorCodeLoop();
	bool markToMoveInstructions(BasicBlock * kernelHead);	
	bool moveMarkedInstsOutOfLoop(BasicBlock * loopHead, BasicBlock * loopTail, SmallVectorImpl<Instruction*> &pivotsList);
	bool createPivotPHINodesAndIncrementors(BasicBlock * loopHead, BasicBlock * kernelHead, BasicBlock * loopTail, SmallVectorImpl<Instruction*> &pivotsList);
	Instruction * addValueIncrementor(Value * origVal, Instruction * PhiNode, Instruction * insertPoint, BasicBlock * loopHead);
	void createCompareAndBranch(BasicBlock * loopHead, BasicBlock * kernelHead, BasicBlock * loopTail);
	void replaceUsersWithPHI(Instruction * pivotInst, Instruction * phiNode);
	
	CodeProperties * funcProperties;
	
};


#endif // __MASTER_LOOP_H__

