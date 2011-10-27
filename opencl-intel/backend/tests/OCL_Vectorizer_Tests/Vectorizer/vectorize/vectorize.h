/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __VECTORIZE_H__
#define __VECTORIZE_H__

#include "../common.h"
#include "../specialCaseFuncs.h"

/// ********* Vectorization phase specific members ********
class VectorizeFunction {
public:
	VectorizeFunction();
	~VectorizeFunction();
	bool vectorizeFunction(CodeProperties * functionProp);
private:
	
	// vectorization functions for different instruction types
	bool vectorizeInstruction(Instruction * I);
	bool vectorizeBinOpInst(Instruction * I, bool supportsWrap);
	bool vectorizeCastInst(Instruction * I);
	bool vectorizeNonVectorizableInst(Instruction * I, bool mustDuplicate);
	bool vectorizeCallInst(Instruction * I);
	bool vectorizeCompare(Instruction * I);
	bool vectorizeSelectInst(Instruction * I);
	bool vectorizeExtractElement(Instruction * I);
	bool vectorizeAllocaInst(Instruction * I);
	bool vectorizeLoadInst(Instruction * I);
	bool vectorizeStoreInst(Instruction * I);
	bool vectorizeGetElementPtrInst(Instruction * I);
	bool vectorizeBranch(Instruction * I);
	bool vectorizePHI(Instruction * I);
	bool vectorizeRetInst(Instruction * I);

	// Utility functions
	Constant * createIncrementingConstVectorForShuffles(unsigned width, int * values);
	bool generateSequentialIndices(Instruction * I);	
	Instruction * findInsertPoint(Value * val);
	void obtainVectorizedValues(Value ** retValue, bool * retIsConstant, Value * origValue, Instruction * origInst);
	bool obtainVectorizedAndCastedValuesForCall(Value ** retValue, bool * retIsConstant, Value * origValue, Instruction * origInst, const Type * targetType);
	Value * fixCastedReturnTypes(CallInst * newCall, const Type * targetType);
	void obtainMultiScalarValues(SmallVectorImpl<Value*> &retValues, bool * retIsConstant, Value * origValue, Instruction * origInst);
	bool obtainNonVectorizedValue(Value * origValue, Value ** retValue);
	bool existVectorizedOrConstantValues(Value * origValue, bool * isScalarRemoved);	
	bool checkPossibilityToUseOriginalInstruction(Instruction * origInst);
	bool useOriginalConstantInstruction(Instruction * origInst);
	
	// Special case functions handling
	bool checkSpecialCaseFunctionCall(CallInst * CI);
	bool vectorizeReadSample(CallInst * CI, bool is2D);
	bool vectorizeWriteSamplerF(CallInst * CI);
	bool vectorizeSelectFunc(CallInst * CI);
	bool vectorizeGeometricFunc(CallInst * CI, unsigned numArgs, const char *fake_func_name_prefix, bool isReducted);
	bool vectorizeFractFunc(CallInst * CI);
	bool vectorizeEarlyExit(CallInst * CI);
	bool vectorizeCiGamma(CallInst * CI);
	
	// Pointer to current functions' properties
	CodeProperties * funcProperties;
	
	// Stack containing all the removed instructions per function. After vectorizing is complete, they are actually removed.
	SmallPtrSet<Instruction *, ESTIMATED_INST_NUM> removedInsts;
	
	
	// The VCM (vector conversions map). Per each value - map of its vectorized and multi-scalar counterparts
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef struct VCMEntry
	{
		Value * vectorValue;
		Value * multiScalarValues[MAX_SUPPORTED_VECTOR_WIDTH];
		bool isScalarRemoved;
	} VCMEntry;
	DenseMap<Value *, VCMEntry *> VCM;
	// VCM service functions
	void createVCMEntryWithVectorValues(Instruction * origInst, Value * vectoredValue);
	void createVCMEntryWithMultiScalarValues(Instruction * origInst, SmallVectorImpl<Value*> &multiScalarValues);
	void createVCMEntryWithVectorAndMultiScalarValues(Instruction * origInst, Value * vectoredValue, SmallVectorImpl<Value*> &multiScalarValues);	
	// VCM allocation funcs
	VCMEntry * allocateNewVCMEntry();
	void releaseAllVCMEntries();
	VCMEntry * VCMAllocationArray;
	unsigned VCMArrayLocation;
	SmallVector<VCMEntry *, 4> VCMArrays;		
	 
	
	// DRL (Deferred resolution handling). Create dummy values (in a "fake" VCM), which can be resolved after passing over all the code
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void createDummyVectorVal(Value * origValue, Value ** vectorVal);
	void createDummyMultiScalarVals(Value * origValue, SmallVectorImpl<Value*> &multiScalarVals);
	bool resolveDeferredInstructions();
	DenseMap<Value *, VCMEntry *> DeferredResMap;

	// The BAG (Broadcast Arguments and Globals) map. Per each value - map to its broadcast.
	////////////////////////////////////////////////////////////////////////////////////////
	DenseMap<Value *, Value *> BAG;
};



#endif // __VECTORIZE_H__

