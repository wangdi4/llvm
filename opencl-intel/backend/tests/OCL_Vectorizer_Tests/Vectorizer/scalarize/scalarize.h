/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __SCALARIZE_H__
#define __SCALARIZE_H__

#include "../common.h"

/// ********* Scalarization phase specific members ********
class ScalarizeFunction {
public:
	ScalarizeFunction();
	~ScalarizeFunction();
	bool scalarizeFunction(CodeProperties * functionProp);
private:
	// Scalarization functions for different instruction types
	bool scalarizeInstruction(Instruction * I);
	bool processNonScalarizableInst(Instruction * I);
	bool scalarizeBinOpAndCmpOperations(Instruction * I, bool isCompare, bool supportsWrap);
	bool scalarizeCastInstructions(Instruction * I);
	bool scalarizeBitCastInst(Instruction * I);
	bool scalarizeRetInst(Instruction * I);
	bool scalarizeStoreInst(Instruction * I);
	bool scalarizePHI(Instruction * I);
	bool scalarizeCallInst(Instruction * I);
	bool scalarizeSelectInst(Instruction * I);
	bool scalarizeExtractElement(Instruction * I);
	bool scalarizeInsertElement(Instruction * I);
	bool scalarizeShuffleVector(Instruction * I);
	bool scalarizeInsertValue(Instruction * I);
	bool scalarizeVectorCmp(Instruction * I);

	// Scalarization Utility functions
	void obtainScalarizedValues(Value * retValues[], bool * retIsConstant, Value * origValue, Instruction * origInst);
	bool obtainVectorValueWhichMightBeScalarized(Value * vectorVal);
	bool resolveDeferredInstructions();	
	
	// Special-case scalarization handling
	bool markSpecialCaseFunctionCall(Instruction * I);
	bool markSpecialCaseReadSample(CallInst * caller, bool is2D);
	bool markSpecialCaseWriteSample(CallInst * caller);
	bool markSpecialCaseSelect(CallInst * caller);
	bool markSpecialCaseGeometricFunc(CallInst * caller, const char *fake_func_name_prefix, bool isReducted);
	bool MarkSpecialCaseFract(CallInst * caller);
	bool markSpecialCaseCIGamma(CallInst * caller);
	
	// Pointer to current functions' properties
	CodeProperties * funcProperties;
	
	// Stack containing all the removed instructions per function. After scalarization is complete, they are actually removed.
	SmallPtrSet<Instruction *, ESTIMATED_INST_NUM> removedInsts;
		
	// Count number of instructions in the kernel
	unsigned numOfKernelinstructions;	
	
	// The SCM (scalar conversions map). Per each value - map of its scalar elements
	////////////////////////////////////////////////////////////////////////////////
	typedef struct SCMEntry
	{
		Value * scalarValues[MAX_OCL_VECTOR_WIDTH];
		short numElements; 
		bool isOriginalVectorRemoved;
	} SCMEntry;
	DenseMap<Value *, SCMEntry *> SCM;
	
	SCMEntry * createEmptySCMEntry(Value * origValue);
	void updateSCMEntryWithValues(SCMEntry * entry, Value * scalarValues[], bool isOrigValueRemoved);	
	SCMEntry * isScalarizedValue(Value * origValue);
	void releaseAllSCMEntries();
	
	SCMEntry * SCMAllocationArray;        	// An array of available SCMEntry's
	unsigned SCMArrayLocation;				// Index, in "SCMAllocationArray", of next free SCMEntry
	SmallVector<SCMEntry *, 4> SCMArrays;	// Vector containing all the "SCMAllocationArray" arrays which were allocated

	
	
	// The DRL (Deferred resolution list). 
	///////////////////////////////////////
	typedef struct DRLEntry
	{
		Value * unresolvedInst;
		Value * dummyVals[MAX_OCL_VECTOR_WIDTH];
	} DRLEntry;
	SmallVector<DRLEntry, 4> DRL;
	
	
};		


// Utility function - just check if all the values in an array are the same
static bool allValuesAreEqual(Value * list[], unsigned size)
{
	Value * first = list[0];
	for (unsigned i = 1; i < size; i++)
	{
		if (list[i] != first) return false;
	}
	return true;
}

// Utility function - just paste the first value from an array, to the entire array
static void pasteFirstValue(Value * list[], unsigned size)
{
	Value * first = list[0];
	for (unsigned i = 1; i < size; i++)
	{
		list[i] = first;
	}
}



#endif // __SCALARIZE_H__


