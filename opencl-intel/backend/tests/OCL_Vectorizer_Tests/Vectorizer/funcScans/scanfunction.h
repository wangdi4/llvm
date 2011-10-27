/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __SCAN_FUNCTION_H__
#define __SCAN_FUNCTION_H__

#include "../common.h"
#include "../specialCaseFuncs.h"

class ScanFunction {
public:
	ScanFunction();
	~ScanFunction();
	void preScalarizeScanFunction(CodeProperties * functionProp);
	void postScalarizeScanFunction();

private:	
	bool PreemptiveScanFunctionCall(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals);
	Value * RootInputArgument(Value * argument, const Type * rootType, Instruction * functionCallInst);
	Value * RootReturnValue(Value * retVal, const Type * rootType, Instruction * functionCallInst);
	Instruction * bitCastValToType(Value * orig, const Type * targetType, Instruction * insertPoint);
	bool markTIDUsers(Instruction * TIDInst);
	bool markDistancesFromTID(Instruction * TIDInst);
	bool preemptiveScanForSettingThreadID(CallInst * callingInst);
	bool preemptiveScanSpecialCaseFunc(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals);
	bool geometricFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals, unsigned numArgs, bool isReducted, geometricListType funcList);
	bool selectFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals);
	bool fractFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals);
	void checkForWorkGroupFuncs(CallInst * callingInst);
	bool isFunctionPseudoDependent(CallInst * CI);

	// Pointer to Function being scalarized
	CodeProperties * funcProperties;	
};		


#endif // __SCAN_FUNCTION_H__


