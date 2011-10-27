/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "scalarize.h"
#ifdef CF_EXPERIMENT
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Target/TargetData.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ScalarizeFunction object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScalarizeFunction::ScalarizeFunction()
{
	// Initialize SCM buffers and allocation
	SCMAllocationArray = new SCMEntry[ESTIMATED_INST_NUM]; // Create an array of SCMEntrys
	SCMArrays.push_back(SCMAllocationArray); // Place the new array in the vector of SCM arrays
	SCMArrayLocation = 0; // set index of free-entry to 0
	
	V_PRINT("ScalarizeFunction constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ScalarizeFunction object destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScalarizeFunction::~ScalarizeFunction()
{
	releaseAllSCMEntries(); // Erase all the SCMEntry arrays (except for the first)
	delete[] SCMAllocationArray; // Erase the first SCMEntry array as well 
	V_PRINT("ScalarizeFunction destructor\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// main Method for scalarizing a function. Iterate over each instruction in the kernel and send for
// sclarization.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeFunction(CodeProperties * functionProp)
{
	funcProperties = functionProp;
	
	bool retVal = true;
	V_PRINT("\nStart scalarizing function: " << CURRENT_FUNCTION->getName() << "\n");
	
	// Prepare data structures for scalarizing a new function (may still have data left from previous function scalarization)
	removedInsts.clear();  // Clear list of instructions pending removal (may not have been cleared, if previous scalarization failed)
	SCM.clear();  // Clear all entries in SCM map
	releaseAllSCMEntries(); // Erase all the SCMEntry arrays (except for the first)
	DRL.clear();	// Clear list of DRL entries, which may have remained from previous failed scalarization
	numOfKernelinstructions = 0; // clear counter of number of instructions
	

	// Scalarization. Iterate over all the instructions.
	// Always hold the iterator at the instruction following the one being scalarized (so the iterator will 
	// "skip" any instructions that are going to be added in the scalarization work)
	inst_iterator sI = inst_begin(CURRENT_FUNCTION);
	inst_iterator sE = inst_end(CURRENT_FUNCTION);
	while (sI != sE)
	{
		Instruction * currInst = &*sI;
		sI++; // Move iterator to next instruction BEFORE scalarizing current instruction
		if (!scalarizeInstruction(currInst))
		{
			retVal = false;
			break; // If scalarization failed - reject this function
		}
	}

	if (retVal == true)
	{
		// Resolved DRL entries
		resolveDeferredInstructions();
		
		// Iterate over removed insts and delete them
		SmallPtrSet<Instruction *, ESTIMATED_INST_NUM>::iterator ri = removedInsts.begin(), re = removedInsts.end();
		SmallPtrSet<Instruction *, ESTIMATED_INST_NUM>::iterator index = ri;
		while (index != re)
		{
			Instruction * nextInst = *index; // obtain the instruction to be deleted
			index++; // Iterate to next instruction before deleting the current one
			funcProperties->setProperty(nextInst, PR_INST_IS_REMOVED);
			nextInst->dropAllReferences();
		}
		index = ri; //start-over, now safe to really delete
		while (index != re)
		{
			Instruction * nextInst = *index; // obtain the instruction to be deleted
			index++; // Iterate to next instruction before deleting the current one
			V_ASSERT(nextInst->use_empty());
			nextInst->eraseFromParent();
		}
		V_PRINT("\nCompleted scalarizing function: " << CURRENT_FUNCTION->getName() << "\n");	
	}
	else
	{
		V_PRINT("\nFailed scalarizing function: " << CURRENT_FUNCTION->getName() << "\n");
	}
	
	// return vectorized kernel width
	return retVal;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////
// main Method for validating and scalarizing instruction. A sort of dispatch table.  
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeInstruction(Instruction * I)
{
    V_PRINT("\tScalarizing Instruction: " << *I << "\n");
	
	// Check if number of instructions does not exceed threshold for rejection
	numOfKernelinstructions++;
	if (numOfKernelinstructions > MAX_ACCEPTABLE_KERNEL_SIZE)
	{
		V_PRINT("\tNumber of kernel instructions exceeds threshold (" << MAX_ACCEPTABLE_KERNEL_SIZE << "). Rejecting!\n");
		return false;
	}
	
	if (funcProperties->getProperty(I, PR_FUNC_PREP_TO_REMOVE))
	{
		V_PRINT("\tInstruction is a CALL cast supporter. Being removed..\n");
		removedInsts.insert(I);
		return true;		
	}
	
	switch (I->getOpcode())
	{
		case Instruction::Ret :
			return scalarizeRetInst(I);
		case Instruction::Br :
			// Branch instruction cannot have vector inputs
			return true;
			
		case Instruction::Add :
		case Instruction::Sub :
		case Instruction::Mul :
			return scalarizeBinOpAndCmpOperations(I, false, true);
			
		case Instruction::FAdd :
		case Instruction::FSub :
		case Instruction::FMul :
		case Instruction::UDiv :
		case Instruction::SDiv :
		case Instruction::FDiv :
		case Instruction::URem :
		case Instruction::SRem :
		case Instruction::FRem :
		case Instruction::Shl :
		case Instruction::LShr :
		case Instruction::AShr :
		case Instruction::And :
		case Instruction::Or :
		case Instruction::Xor :
			return scalarizeBinOpAndCmpOperations(I, false, false);

		case Instruction::ICmp :
		case Instruction::FCmp :
			return scalarizeBinOpAndCmpOperations(I, true, false);

		case Instruction::Trunc :
		case Instruction::ZExt :
		case Instruction::SExt :
		case Instruction::FPToUI :
		case Instruction::FPToSI :
		case Instruction::UIToFP :
		case Instruction::SIToFP :
		case Instruction::FPTrunc :
		case Instruction::FPExt :
		case Instruction::PtrToInt :
		case Instruction::IntToPtr :
			return scalarizeCastInstructions(I);
		case Instruction::BitCast :	
			return scalarizeBitCastInst(I);
						
		case Instruction::Alloca :
			return processNonScalarizableInst(I);
			
		case Instruction::Load :
			return processNonScalarizableInst(I);
		case Instruction::Store :
			return scalarizeStoreInst(I);
			
		case Instruction::GetElementPtr :
			// GetElementPtr does not work on vectors
			return true;
			
		case Instruction::PHI :
			return scalarizePHI(I);
			
		case Instruction::Call :
			return scalarizeCallInst(I);
			
		case Instruction::Select :
			return scalarizeSelectInst(I);
			
		case Instruction::ExtractElement :
			return scalarizeExtractElement(I);
		case Instruction::InsertElement :
			return scalarizeInsertElement(I);
		case Instruction::ShuffleVector :
			return scalarizeShuffleVector(I);
			
		case Instruction::ExtractValue :
			// ExtractValue may create a new vector value. But it cannot be scalarized
			return processNonScalarizableInst(I);
			
		case Instruction::InsertValue :
			// InsertValue may use a vector as input. Can't be scalarized, but need to make sure the value is ready
			return scalarizeInsertValue(I);
			
			// The remaining instructions are not supported by the optimizer
		default :
			V_PRINT("\tUnhandled instruction ("<< I->getOpcodeName() << "). Aborting function.\n");
			return false;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Instructions which cannot be scalarized reach this function. Sometimes they may have a vector
// value, so we create empty SCM entries for them if needed.
// If input arguments need to be resolved - do this BEFORE calling this function!
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::processNonScalarizableInst(Instruction * I)
{
	V_PRINT("\t\tNon-scalarizable Instruction\n");
	if (isa<VectorType>(I->getType()))
		createEmptySCMEntry(I);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing a vector instruction with scalar instructions
// 1) switch-case to different instruction classifications
// 2) Prepare all input arguments
// 3) Create the converted instructions
// 4) Update the scalar conversions map (SCM)
// 5) remove the original instruction, from the function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeBinOpAndCmpOperations(Instruction * I, bool isCompare, bool supportsWrap)
{
	V_PRINT("\t\tConvert " << I->getOpcodeName() << " to scalars\n");
	if (!isa<VectorType>(I->getType()))
		return true;
	SCMEntry * newEntry = createEmptySCMEntry(I);
	
	unsigned numElements = cast<VectorType>(I->getType())->getNumElements();
	V_ASSERT(numElements <= MAX_OCL_VECTOR_WIDTH);

	bool has_NSW = false;
	bool has_NUW = false;
	if (supportsWrap)
	{
		has_NSW = cast<BinaryOperator>(I)->hasNoSignedWrap();
		has_NUW = cast<BinaryOperator>(I)->hasNoUnsignedWrap();
	}
	
	Value * newScalarizedInsts[numElements];

	V_ASSERT(I->getNumOperands() == 2); // Sanity checks
	V_ASSERT(isa<VectorType>(I->getOperand(0)->getType())); 
	V_ASSERT(isa<VectorType>(I->getOperand(1)->getType())); 
	V_ASSERT((cast<VectorType>(I->getOperand(0)->getType()))->getNumElements() == numElements); 
	V_ASSERT((cast<VectorType>(I->getOperand(1)->getType()))->getNumElements() == numElements); 
	Value * operand0[MAX_OCL_VECTOR_WIDTH];
	Value * operand1[MAX_OCL_VECTOR_WIDTH];
	bool op0IsConst, op1IsConst;
	obtainScalarizedValues(operand0, &op0IsConst, I->getOperand(0), I);
	obtainScalarizedValues(operand1, &op1IsConst, I->getOperand(1), I);
	if (op0IsConst && op1IsConst)
		return true; //dont Scalarize a constant calculation

	// Create new instructions
	for (unsigned dup = 0; dup < numElements; dup++)
	{
		if (isCompare)
		{
			newScalarizedInsts[dup] = CmpInst::Create(cast<CmpInst>(I)->getOpcode(),
													  cast<CmpInst>(I)->getPredicate(),
													  operand0[dup],
													  operand1[dup],
													  I->getName(),
													  I
													  );
		}
		else
		{
			newScalarizedInsts[dup] = BinaryOperator::Create(cast<BinaryOperator>(I)->getOpcode(),
															 operand0[dup],
															 operand1[dup],
															 I->getName(),
															 I
															 );
			if (has_NSW) cast<BinaryOperator>(newScalarizedInsts[dup])->setHasNoSignedWrap();
			if (has_NUW) cast<BinaryOperator>(newScalarizedInsts[dup])->setHasNoUnsignedWrap();
		}
	}

	// If all instructions do EXACTLY the same thing, throw away most of them...
	if (allValuesAreEqual(operand0,numElements) && allValuesAreEqual(operand1,numElements))
	{
		pasteFirstValue(newScalarizedInsts, numElements);
	}
	
	// Add new value/s to SCM
	updateSCMEntryWithValues(newEntry, newScalarizedInsts, true);
	// Remove original instruction
	removedInsts.insert(I);
	V_PRINT("\t  Finished scalarizing inst\n");
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing cast instruction which work on vectors, with several
// scalar counterparts
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeCastInstructions(Instruction * I)
{
	V_PRINT("\t\tCast inst " << I->getOpcodeName() << "\n");
	V_ASSERT(I->getNumOperands() == 1); // Sanity checks

	if (!isa<VectorType>(I->getType()))
		return true;

	SCMEntry * newEntry = createEmptySCMEntry(I);
			
	// Obtain number of elements of vector
	unsigned numElements = cast<VectorType>(I->getType())->getNumElements();
	V_ASSERT(numElements <= MAX_OCL_VECTOR_WIDTH);
	
	// Sanity: make sure number of input elements = number of output elements
	V_ASSERT(isa<VectorType>(I->getOperand(0)->getType()));
	V_ASSERT(cast<VectorType>(I->getOperand(0)->getType())->getNumElements() == numElements);
	
	// Obtain type, which ever scalar cast will cast-to
	const Type * scalarDestType = cast<VectorType>(I->getType())->getElementType();
	
	Value * newScalarizedInsts[MAX_OCL_VECTOR_WIDTH];
	Value * operand0[MAX_OCL_VECTOR_WIDTH];
	bool op0IsConst;

	obtainScalarizedValues(operand0, &op0IsConst, I->getOperand(0), I);
	if (op0IsConst) 
		return true; //dont Scalarize a constant calculation
	for (unsigned duplicates = 0; duplicates < numElements; duplicates++)
	{
		newScalarizedInsts[duplicates] = CastInst::Create(
														  cast<CastInst>(I)->getOpcode(),
														  operand0[duplicates],
														  scalarDestType,
														  I->getName(),
														  I
														  );
	}

	// If all instructions do EXACTLY the same thing, throw away most of them...
	if (allValuesAreEqual(operand0,numElements))
	{
		pasteFirstValue(newScalarizedInsts, numElements);
	}

	// Add new value/s to SCM
	updateSCMEntryWithValues(newEntry, newScalarizedInsts, true);
	// Remove original instruction
	removedInsts.insert(I);
	V_PRINT("\t  Finished scalarizing cast inst\n");
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for handling bitCast instructions
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeBitCastInst(Instruction * I)
{
	// We cannot scalarize bitCast insts (input and output may have different vector widths!)
	V_PRINT("\t\tBitCast inst \n");
	
	if (isa<VectorType>(I->getType()))
		createEmptySCMEntry(I);
	if (isa<VectorType>(I->getOperand(0)->getType()))
		return obtainVectorValueWhichMightBeScalarized(I->getOperand(0));
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// RET instructions cannot be "scalarized". If they have a vector - meaning the return value
// of the function is vector - the only thing that can be done is to make sure the vector 
// value exists (as we may have scalarized it...)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeRetInst(Instruction * I)
{
	V_PRINT("\t\tRet Instruction\n");
	Value * retVal = cast<ReturnInst>(I)->getReturnValue();
	if (retVal != NULL && isa<VectorType>(retVal->getType()))
	{
		V_PRINT("\t\t\tRet with a vector return-value.\n");
		return obtainVectorValueWhichMightBeScalarized(retVal);
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Store instructions may receive vector inputs. It may be possible to scalarize these instructions,
// but such an optimization is deferred for now.
// Only thing to be done is make sure the vector value exists (as we may have scalarized it...)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeStoreInst(Instruction * I)
{
	V_PRINT("\t\tStore Instruction\n");
	Value * storeVal = cast<StoreInst>(I)->getOperand(0);
	if (isa<VectorType>(storeVal->getType()))
	{
		V_PRINT("\t\t\ttStore a vector value.\n");
		return obtainVectorValueWhichMightBeScalarized(storeVal);
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// PHI instruction may have vector inputs (and therefore a vector output).
// If so - we can break the instruction to multiple PHI instructions of scalar values
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizePHI(Instruction * I)
{
	V_PRINT("\t\tPHI Instruction\n");
	const Type * retValType = I->getType();
	
	if (!isa<VectorType>(retValType))
		return true;

	SCMEntry * newEntry = createEmptySCMEntry(I);

	// Need to break the PHI node to multiple PHI nodes, each dealing with a scalar input
	V_PRINT("\t\t\tPHI with a vector value.\n");	
	PHINode * origInst = cast<PHINode>(I);
	const Type * scalarType = cast<VectorType>(retValType)->getElementType();
	unsigned numElements = cast<VectorType>(retValType)->getNumElements();
	V_ASSERT(numElements <= MAX_OCL_VECTOR_WIDTH);
	
	// Get number of incoming values
	unsigned numValues = origInst->getNumIncomingValues();
		
	// Create new (empty) PHI nodes, and place them.
	Value * newScalarizedPHI[MAX_OCL_VECTOR_WIDTH];
	for (unsigned i = 0; i < numElements; i++)
	{
		newScalarizedPHI[i] = PHINode::Create(scalarType, origInst->getName(), origInst);
		cast<PHINode>(newScalarizedPHI[i])->reserveOperandSpace(numValues);
	}
		
	// Iterate over incoming values in vector PHI, and fill scalar PHI's accordingly
	Value * operand[MAX_OCL_VECTOR_WIDTH];
	bool allValuesAreTheSame = true;
	bool dummyVal;
	for (unsigned j = 0; j < numValues; j++)
	{
		obtainScalarizedValues(operand, &dummyVal, origInst->getIncomingValue(j), I);
		// Check if all input value are the same..
		if (allValuesAreTheSame && !allValuesAreEqual(operand,numElements)) allValuesAreTheSame = false;
			
		for (unsigned i = 0; i < numElements; i++)
		{
			cast<PHINode>(newScalarizedPHI[i])->addIncoming(operand[i], origInst->getIncomingBlock(j));
		}
	}

	// If all instructions do EXACTLY the same thing, throw away most of them...
	if (allValuesAreTheSame)
	{
		pasteFirstValue(newScalarizedPHI, numElements);
	}

	// Add new value/s to SCM
	updateSCMEntryWithValues(newEntry, newScalarizedPHI, true);
		
	// Remove original instruction
	removedInsts.insert(I);
	return true;		
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Call instructions may have any mix of scalar and vector inputs and output.
// Somal Call instructions may be scalarized - if they can be found in the functions hash table.
// If they cannot be scalarized - need to make sure all the vector inputs were not replaced
// with scalar counterparts.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeCallInst(Instruction * I)
{
	V_PRINT("\t\tCall Instruction to function: " << cast<CallInst>(I)->getCalledFunction()->getName() << "\n");
	CallInst * CI = cast<CallInst>(I);

	if (funcProperties->getProperty(CI, PR_SCALARIZABLE_BUILT_IN))
	{
		// Look for the Call instruction in the scalarizable Calls map.
		SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(CI);
		V_ASSERT(rootValues != NULL);
		Function * vectorFunc = CI->getCalledFunction();
		std::string vectorFuncName = vectorFunc->getName(); 
		unsigned vectorWidth = 0;
		VFH::hashEntry * foundFunction = VFH::findVectorFunctionInHash(vectorFuncName, &vectorWidth);
		V_ASSERT(foundFunction != NULL); // we already found this function before. Should still be there...
		V_ASSERT(vectorWidth <= MAX_OCL_VECTOR_WIDTH);
		const char * scalarFuncName = foundFunction->funcs[0]; // extract name of scalar function
		const Function *LibFunc = RUNTIME_MODULE->getFunction(scalarFuncName);
		V_ASSERT(LibFunc != NULL); // we already found this function before. Must still be found...
				
		V_PRINT("\t\t\tPrototype of scalarized function is:" << *(LibFunc->getFunctionType()) << "\n");
		
		// Go over all inputs, and convert the needed ones to vectors
		std::vector<Value *> newArgs[MAX_OCL_VECTOR_WIDTH];
		bool isAllConstantInputs = true;
		bool isAllInputsTheSame = true;
		unsigned argIndex = 1; // skip arg0, as it is the return value
		
		for (; argIndex < 4; argIndex++)
		{
			if (foundFunction->funcArgs[argIndex] == VFH::T_NONE) break; // no more arguments
			Value * operand[MAX_OCL_VECTOR_WIDTH];
			bool opIsConst;
			V_ASSERT(LibFunc->getFunctionType()->getParamType(argIndex-1) != NULL);
			if (foundFunction->funcArgs[argIndex] != VFH::T_STATIC)
			{
				obtainScalarizedValues(operand, &opIsConst, (*rootValues)[argIndex], I);
				// Check if all input value are the same..
				if (isAllInputsTheSame && !allValuesAreEqual(operand,vectorWidth)) isAllInputsTheSame = false;
				
				for (unsigned duplicates = 0; duplicates < vectorWidth; duplicates++)
					newArgs[duplicates].push_back(operand[duplicates]);
			}
			else 
			{
				V_ASSERT(LibFunc->getFunctionType()->getParamType(argIndex-1) == I->getOperand(argIndex)->getType());
				opIsConst = !isa<Instruction>(I->getOperand(argIndex));
				for (unsigned duplicates = 0; duplicates < vectorWidth; duplicates++)
					newArgs[duplicates].push_back(I->getOperand(argIndex));
			}
			isAllConstantInputs &= opIsConst;
		}
		V_ASSERT(argIndex > 1); // should have at least one input argument
		// If all inputs are constants - don't vectorize this call instruction
		if (isAllConstantInputs)
		{
			V_PRINT("\t\t\tAll inputs are constants. Not scalarizing!\n");			
			return true; //dont Scalarize a constant calculation
		}
		
		// Find (or create) declaration for newly called function
		V_ASSERT(!CURRENT_MODULE->getFunction(scalarFuncName) || LibFunc->getFunctionType() == CURRENT_MODULE->getFunction(scalarFuncName)->getFunctionType());
		Constant * scalarFunctionConst = CURRENT_MODULE->getOrInsertFunction(scalarFuncName, LibFunc->getFunctionType());
		if (!scalarFunctionConst)
		{
			V_UNEXPECTED("Failed generating function in current module");
			return false;
		}
		// Create new instruction/s
		Value * newFuncCalls[MAX_OCL_VECTOR_WIDTH];

		if (isAllInputsTheSame)
		{
			newFuncCalls[0] = CallInst::Create(scalarFunctionConst, newArgs[0].begin(), newArgs[0].end(), I->getName(), I);
			for (unsigned duplicates = 1; duplicates < vectorWidth; duplicates++)
			{
				newFuncCalls[duplicates] = newFuncCalls[0];
			}
		}
		else
		{
			for (unsigned duplicates = 0; duplicates < vectorWidth; duplicates++)
			{		
				newFuncCalls[duplicates] = CallInst::Create(scalarFunctionConst, newArgs[duplicates].begin(), newArgs[duplicates].end(), I->getName(), I);
			}
		}
				
		// Check if function's return type was (in the vector function call) in a non-desired type
		if ((*rootValues)[0] != I)
		{
			if (I->getType() == getVoidTy)
			{
				// return value is passed thru a pointer (first argument)
				V_ASSERT(isa<PointerType>(I->getOperand(1)->getType())); // first operand really is a pointer
				V_ASSERT(isa<LoadInst>((*rootValues)[0])); // the actual value is a result of load
				V_ASSERT(cast<LoadInst>((*rootValues)[0])->getPointerOperand() == I->getOperand(1)); // the loaded address matches the first arument
			}
			else
			{
				// iterate over all the users of the original return value, until we find the "proper" return value. mark all others for removal		
				V_ASSERT(I->hasOneUse());
				Value * currentVal = *(I->use_begin());
				while (currentVal != (*rootValues)[0])
				{
					V_ASSERT(currentVal->hasOneUse());
					funcProperties->setProperty(currentVal, PR_FUNC_PREP_TO_REMOVE);
					currentVal = *(currentVal->use_begin());
				}
			}
			// mark the actual "proper" return value as removal candidate
			funcProperties->setProperty((*rootValues)[0], PR_FUNC_PREP_TO_REMOVE);			
		}
		
		
		// Add new value/s to SCM
		SCMEntry * newEntry = createEmptySCMEntry((*rootValues)[0]);
		updateSCMEntryWithValues(newEntry, newFuncCalls, true);
		
		// Remove original instruction
		removedInsts.insert(I);
		return true;
	}
	V_PRINT("\t\t\tDid not find function in root values!\n");
	
	// Check if maybe this function call falls under special cases.
	if (funcProperties->getProperty(CI, PR_SPECIAL_CASE_BUILT_IN))
	{
		return markSpecialCaseFunctionCall(I);		
	}
	V_PRINT("\t\t\tDid not find function in special cases!\n");

	// Getting here, the function was not modified. Need to scan all inputs 
	// and make sure any vectors found are available
	
	if (isa<VectorType>(I->getType()))
		createEmptySCMEntry(I);
	
	unsigned numOperands = CI->getNumOperands();
	for (unsigned i = 1; i < numOperands; i++)
	{
		Value * operand = CI->getOperand(i);
		if (isa<VectorType>(operand->getType()))
		{
			if (!obtainVectorValueWhichMightBeScalarized(operand))
				return false; // Obtaining the value failed for some reason. Abort this function
		}
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Select instruction can have vector inputs/output only in theory. Current LLVM implementation
// Does not support it yet. Check and assert for any chance it is enabled in the future
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeSelectInst(Instruction * I)
{
    V_PRINT("\t\tSelect Instruction\n");
	const Type * retValType = I->getType();
	//V_ASSERT(!isa<VectorType>(retValType));
	
	if (!isa<VectorType>(retValType))
		return true;
	
	SCMEntry * newEntry = createEmptySCMEntry(I);
	
	// Need to break the Vector select to multiple scalar selects
	V_PRINT("\t\t\tSelect with a vector value!\n");	
	V_ASSERT(I->getNumOperands() == 3); // Sanity checks
		
		
	SelectInst * origInst = cast <SelectInst>(I);
	unsigned numElements = cast<VectorType>(retValType)->getNumElements();
	V_ASSERT(numElements <= MAX_OCL_VECTOR_WIDTH);
		
	Value* cond = origInst->getCondition();
	Value* trueVal = origInst->getTrueValue();
	Value* falseVal = origInst->getFalseValue();
	V_ASSERT(isa<VectorType>(trueVal->getType())); 
	V_ASSERT(isa<VectorType>(falseVal->getType())); 
	V_ASSERT((cast<VectorType>(trueVal->getType()))->getNumElements() == numElements); 
	V_ASSERT((cast<VectorType>(falseVal->getType()))->getNumElements() == numElements); 
		
	Value * scalarCond[MAX_OCL_VECTOR_WIDTH];
	Value * scalarTrueVal[MAX_OCL_VECTOR_WIDTH];
	Value * scalarFalseVal[MAX_OCL_VECTOR_WIDTH];
	bool dummy;
		
	obtainScalarizedValues(scalarTrueVal, &dummy, trueVal, I);
	obtainScalarizedValues(scalarFalseVal, &dummy, falseVal, I);
		
	// Need to check if the condition itself is vector or scalar
	if (isa<VectorType>(cond->getType()))
	{
		obtainScalarizedValues(scalarCond, &dummy, cond, I);
	}
	else
	{
		for (unsigned i = 0; i < numElements; i++)
		{
			scalarCond[i] = cond;
		}
	}
		
	// Create the new scalar instructions, and place them in the function
	Value *newScalarizedInsts[MAX_OCL_VECTOR_WIDTH];
	std::string instName = I->getName();
	for (unsigned duplicates = 0; duplicates < numElements; duplicates++)
	{
	// Small optimization: the scalarization may cause some scalar sub-selects to be redundant (input == output).
		if (scalarTrueVal[duplicates] != scalarFalseVal[duplicates])
		{
			newScalarizedInsts[duplicates] = SelectInst::Create(
																scalarCond[duplicates],
																scalarTrueVal[duplicates],
																scalarFalseVal[duplicates],
																instName,
																I
																);
		}
		else
		{
			newScalarizedInsts[duplicates] = scalarTrueVal[duplicates]; // just "connect" the destination value to the input
		}
	}

	// If all instructions do EXACTLY the same thing, throw away most of them...
	if (allValuesAreEqual(scalarCond,numElements) && allValuesAreEqual(scalarTrueVal,numElements) && allValuesAreEqual(scalarFalseVal,numElements))
	{
		pasteFirstValue(newScalarizedInsts, numElements);
	}

	// Add new value/s to SCM
	updateSCMEntryWithValues(newEntry, newScalarizedInsts, true);
	// Remove original instruction
	removedInsts.insert(I);
 
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// InsertValue instructions cannot be scalarized, but may operate with a vector value
// Only thing to be done is make sure the vector value exists (as we may have scalarized it...)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeInsertValue(Instruction * I)
{
    V_PRINT("\t\tInsertValue Instruction\n");
	Value * insertedValue = cast<InsertValueInst>(I)->getInsertedValueOperand();
	if (isa<VectorType>(insertedValue->getType()))
		return obtainVectorValueWhichMightBeScalarized(insertedValue);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// VCmp instructions (VICmp and VFCmp) are compare instructions that work only on vector inputs.
// they differ from "normal" compare by emitting an output which is the same type as the input
// Although there is no scalar inst for doing the same thing, we can achieve the same result
// by performing a "normal" scalar compare, followed by a SExt opertion.  
// For now - this optimization is not implemented. We just check that both vector inputs still exist 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeVectorCmp(Instruction * I)
{
    V_PRINT("\t\tVector Compare Instruction\n");
	createEmptySCMEntry(I);
	if (!obtainVectorValueWhichMightBeScalarized(I->getOperand(0)))
		return false;
	return obtainVectorValueWhichMightBeScalarized(I->getOperand(1));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtractElement instructions do not require scalarization per-se, but may still need handling:
//    If the vector is found in SCM (so it has scalar counterparts already)- we can remove this inst, 
//    and change all of its users to use "our" scalarized value instead
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeExtractElement(Instruction * I)
{
    V_PRINT("\t\tExtractElement Instruction\n");
	Value * vectorValue = cast<ExtractElementInst>(I)->getOperand(0);
	Value * scalarIndexVal = cast<ExtractElementInst>(I)->getOperand(1);
	// If the index is not a constant - we cannot statically remove this inst
	if(!isa<ConstantInt>(scalarIndexVal))
		return obtainVectorValueWhichMightBeScalarized(vectorValue);

	Value * operand[MAX_OCL_VECTOR_WIDTH];
	bool dummy;
	obtainScalarizedValues(operand, &dummy, vectorValue, I);
	
	uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
	V_ASSERT(operand[scalarIndex] != NULL);
	// Replace all users of this inst, with the extracted scalar value
	I->replaceAllUsesWith(operand[scalarIndex]);
	// Remove original instruction
	removedInsts.insert(I);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// InsertElement instructions actually "build" a new vector from an existing vector.
// When encountering this, we should create a new SCM value for the new vector, and copy
// most of the SCM elements from the old SCM entry (plus the new one)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeInsertElement(Instruction * I)
{
    V_PRINT("\t\tInsertElement Instruction\n");
	SCMEntry * newEntry = createEmptySCMEntry(I);
	
	Value * sourceVectorValue = cast<InsertElementInst>(I)->getOperand(0);
	V_ASSERT(isa<VectorType>(sourceVectorValue->getType()));
	Value * sourceScalarValue = cast<InsertElementInst>(I)->getOperand(1);
	Value * scalarIndexVal = cast<InsertElementInst>(I)->getOperand(2);

	// If the index is not a constant - we cannot statically remove this inst
	if(!isa<ConstantInt>(scalarIndexVal))
		return obtainVectorValueWhichMightBeScalarized(sourceVectorValue);
	
	V_ASSERT(isa<ConstantInt>(scalarIndexVal));
	uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
	V_ASSERT(scalarIndex <= newEntry->numElements);

	// Array for storing the scalar values
	Value * scalarValues[MAX_OCL_VECTOR_WIDTH];
	
	if (isa<UndefValue>(sourceVectorValue))
	{
		V_PRINT("\t\t\tBase vector is undef\n");
		UndefValue * undef = UndefValue::get(cast<VectorType>(sourceVectorValue->getType())->getElementType());
		// fill new entry with UNDEFs and new value
		for (unsigned j = 0; j<MAX_OCL_VECTOR_WIDTH; j++)
			scalarValues[j] = undef;
		scalarValues[scalarIndex] = sourceScalarValue;
	}
	else
	{
		bool dummy;
		// Obtain the scalar values of the input vector
		obtainScalarizedValues(scalarValues, &dummy, sourceVectorValue, I);
		// Add the new element
		scalarValues[scalarIndex] = sourceScalarValue;
	}

	updateSCMEntryWithValues(newEntry, scalarValues, true);		
	
	// Remove original instruction
	removedInsts.insert(I);

	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// SuffleVector instructions "build" a new vector from reorganizing existing vectors.
// When encountering this, we should create a new SCM value for the new vector, and copy
// the SCM elements from the old SCM entries
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::scalarizeShuffleVector(Instruction * I)
{
    V_PRINT("\t\tShuffleVector Instruction\n");
	SCMEntry * newEntry = createEmptySCMEntry(I);
	ShuffleVectorInst * SVI = cast<ShuffleVectorInst>(I);
	unsigned width = SVI->getType()->getNumElements();
	Value * sourceVector0Value = SVI->getOperand(0);
	Value * sourceVector1Value = SVI->getOperand(1);
	unsigned sourceVectorWidth = cast<VectorType>(sourceVector0Value->getType())->getNumElements();
	V_ASSERT(sourceVectorWidth == cast<VectorType>(sourceVector1Value->getType())->getNumElements());
	
	Value * allValues[MAX_OCL_VECTOR_WIDTH * 2] = {NULL};
	
	SCMEntry * source0Entry = isScalarizedValue(sourceVector0Value);
	SCMEntry * source1Entry = isScalarizedValue(sourceVector1Value);
	
	// If both sources are not in SCM or undef, don't scalarize the shuffleVector
	if (!source0Entry && !isa<UndefValue>(sourceVector0Value) && 
		!source1Entry && !isa<UndefValue>(sourceVector1Value))
	{
		if (!obtainVectorValueWhichMightBeScalarized(sourceVector0Value))
			return false;
		return obtainVectorValueWhichMightBeScalarized(sourceVector1Value);		
	}
	
	// Obtain values from the first vector
	if (!isa<UndefValue>(sourceVector0Value))
	{
		Value * operand[MAX_OCL_VECTOR_WIDTH];
		bool dummy;
		obtainScalarizedValues(operand, &dummy, I->getOperand(0), I);
		for (unsigned i = 0; i < sourceVectorWidth; i++)
		{
			V_ASSERT(operand[i] != NULL);
			allValues[i] = operand[i];
		}
	}
	else
	{
		Value * undefElem = UndefValue::get(cast<VectorType>(sourceVector0Value->getType())->getElementType());
		for (unsigned i = 0; i < sourceVectorWidth; i++)
		{
			allValues[i] = undefElem;
		}
	}
	
	
	// Obtain values from the second vector
	if (!isa<UndefValue>(sourceVector1Value))
	{
		Value * operand[MAX_OCL_VECTOR_WIDTH];
		bool dummy;
		obtainScalarizedValues(operand, &dummy, I->getOperand(1), I);
		for (unsigned i = 0; i < sourceVectorWidth; i++)
		{
			V_ASSERT(operand[i] != NULL);
			allValues[i+sourceVectorWidth] = operand[i];
		}
	}	
	else
	{
		Value * undefElem = UndefValue::get(cast<VectorType>(sourceVector1Value->getType())->getElementType());
		for (unsigned i = 0; i < sourceVectorWidth; i++)
		{
			allValues[i+sourceVectorWidth] = undefElem;
		}
	}
	
	// create the new values order
	Value * newVector[MAX_OCL_VECTOR_WIDTH];
	for (unsigned i = 0; i < width; i++)
	{
		int maskValue = SVI->getMaskValue(i);
		if (maskValue >= 0)
		{
			newVector[i] = allValues[maskValue];
			V_PRINT("\t\t\tNew Vector Value " << i << " Comes from index: " << SVI->getMaskValue(i) << "\n");
		}
		else 
		{
			newVector[i] = UndefValue::get(allValues[0]->getType());
			V_PRINT("\t\t\tNew Vector Value " << i << " is undef\n");			
		}

	}
	
	// Create the new SCM entry
	updateSCMEntryWithValues(newEntry, newVector, true);
	
	// Remove original instruction
	removedInsts.insert(I);
	
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// This function takes a vector value, and returns the scalarized "breakdown" of that value  
// The return value of this func is "false" only if the breakdown did not exist and was created now (instructions only)
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScalarizeFunction::obtainScalarizedValues(Value * retValues[], bool * retIsConstant, Value * origValue, Instruction * origInst)
{
	if (!isa<VectorType>(origValue->getType()))
		V_PRINT("\nERROR: Encountered type: " << *(origValue->getType()) << " of Value: " << *origValue << "\n");
	V_ASSERT(isa<VectorType>(origValue->getType()));
	V_PRINT("\t\t\tObtaining scalar value... " << *origValue << "\n");
	unsigned width = cast<VectorType>(origValue->getType())->getNumElements();
	
	if (isa<Instruction>(origValue))
		*retIsConstant = false;
	else 
		*retIsConstant = true;
	
	// Lookup value in SCM (may contain constants as well, if they are function arguments)
	SCMEntry * currEntry = isScalarizedValue(origValue);
	if (currEntry)
	{
		V_PRINT("\t\t\tFound existing entry in lookup of " << origValue->getName() << "\n");
		V_ASSERT(width == currEntry->numElements);
		for (unsigned i = 0; i < width; i++)
		{
			V_ASSERT(currEntry->scalarValues[i] != NULL);
			retValues[i] = currEntry->scalarValues[i];
		}
	}
	else if (isa<UndefValue>(origValue))
	{
		// undefVal can also be scalarized. Break it to element-sized undefs
		V_PRINT("\t\t\tUndefVal constant\n");
		Value * undefElement = UndefValue::get(cast<VectorType>(origValue->getType())->getElementType());
		for (unsigned i = 0; i < width; i++)
		{
			retValues[i] = undefElement;
		}			
	}
	else if (Constant * vectorConst = dyn_cast<Constant>(origValue))
	{
		// Value is a "proper" constant. Need to break it down to scalar constant.
		V_PRINT("\t\t\tProper constant: " <<  *vectorConst << "\n");
		V_ASSERT(isa<VectorType>(vectorConst->getType()));
		
		if (isa<ConstantVector>(vectorConst))
		{
			SmallVector<Constant*, MAX_OCL_VECTOR_WIDTH> elements;
			vectorConst->getVectorElements(elements);
			V_PRINT("\t\t\tConstant element #0: " <<  *elements[0] << "\n");
			V_ASSERT(elements.size() == width);
			for (unsigned i = 0; i < width; i++)
			{
				retValues[i] = elements[i];
			}
		}
		else
		{
			V_PRINT("\t\t\tConstant is (probably) an expression: " <<  *vectorConst << "\n");
#ifdef CF_EXPERIMENT			
			ConstantExpr *ce = dyn_cast<ConstantExpr>(vectorConst);
			if (ce)
			{
				TargetData td = TargetData(CURRENT_MODULE);
				// Neet to over-cast the constant with "extract element"
				for (unsigned i = 0; i < width; i++)
				{
					Constant *elem = ConstantExpr::getExtractElement(vectorConst, ConstantInt::get(getInt32Ty, i));
					if (isa<ConstantExpr>(elem))
					{
						elem = ConstantFoldConstantExpression(cast<ConstantExpr>(elem), &td);
					}
					retValues[i] = elem;
				}				
			}
			else
#endif				
			{
				// Neet to over-cast the constant with "extract element"
				for (unsigned i = 0; i < width; i++)
				{
					retValues[i] = ConstantExpr::getExtractElement(vectorConst, ConstantInt::get(getInt32Ty, i));
				}			
			}
		}
	}
	else if (isa<Instruction>(origValue) && !SCM.count(origValue))
	{
		// Entry was not found in SCM at all. Means it will be defined in a following basic block. use dummy vals for now
		V_PRINT("\t\t\t*** Not found. Setting DRL. Is entry exist?" << SCM.count(origValue) << "\n");
		const Type * dummyType = PointerType::get(cast<VectorType>(origValue->getType())->getElementType(), 0);
		DRLEntry newDRLEntry;
		newDRLEntry.unresolvedInst = origValue;
		for (unsigned i = 0; i < width; i++)
		{
			Constant * subExpr = ConstantExpr::getIntToPtr(ConstantInt::get(getInt32Ty, APInt(32, 0xdeadbeef)), dummyType);
			retValues[i] = new LoadInst(subExpr);
			newDRLEntry.dummyVals[i] = retValues[i];
		}
		DRL.push_back(newDRLEntry); // This basically COPIES "newDRLEntry", so no worry about it being destroyed right afterwards.
	}
	else
	{
		V_PRINT("\t\t\tCreating scalar conversion for " << origValue->getName() << "\n");
		// Value was not converted to scalars yet. Create a scalar values and place in SCM
		// Need to break down the vector:
		//   %scalar0 = extractelement <4 x Type> %vector, i32 0
		//   %scalar1 = extractelement <4 x Type> %vector, i32 1 
		//   %scalar2 = extractelement <4 x Type> %vector, i32 2
		//   %scalar3 = extractelement <4 x Type> %vector, i32 3 
		Instruction * locationInst;
		if (isa<Instruction>(origValue))
			locationInst = cast<Instruction>(origValue);
		else 			
			locationInst = &*(inst_begin(CURRENT_FUNCTION)); // The first instruction in the function
		
		V_ASSERT(locationInst->getParent() != NULL);
		V_ASSERT(locationInst->getParent()->getParent() != NULL);
		
		for (unsigned i = 0; i < width; i++)
		{
			Value * constIndex = ConstantInt::get(getInt32Ty, i);
			retValues[i] = ExtractElementInst::Create(origValue, constIndex, "scalar");
			cast<Instruction>(retValues[i])->insertAfter(locationInst);
			locationInst = cast<Instruction>(retValues[i]);
			V_PRINT("\t\t\tAdded scalar conversion inst:" << *(retValues[i]) << "\n");
		}
		
		SCMEntry * newEntry = createEmptySCMEntry(origValue);
		updateSCMEntryWithValues(newEntry, retValues, false);
	}	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is called when the (given) vector value is needed, but might no longer be available.
// This can happen if the vector value was replaced with scalar values. If such a thing occured,
// we need to re-assemble the vector value.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::obtainVectorValueWhichMightBeScalarized(Value * vectorVal)
{
	if (isa<UndefValue>(vectorVal))
		return true;

	// ONLY IF the value appears in the SCM - there is a chance it was removed. 
	if (SCM.count(vectorVal))
	{
		SCMEntry * valueEntry = SCM[vectorVal];
		if (valueEntry->isOriginalVectorRemoved == false)
			return true; // value was not removed.
		
		V_PRINT("\t\t\tTrying to use a removed value. Reassembling it...\n");
		// Getting here - value was removed. Need to reassemble it to vector...
		//   %temp.vect.0 = insertelement <4 x type> undef       , type %scalar.0, i32 0
		//   %temp.vect.1 = insertelement <4 x type> %indx.vect.0, type %scalar.1, i32 1
		//   %temp.vect.2 = insertelement <4 x type> %indx.vect.1, type %scalar.2, i32 2
		//   %temp.vect.3 = insertelement <4 x type> %indx.vect.2, type %scalar.3, i32 3
		V_ASSERT(valueEntry->numElements == cast<VectorType>(vectorVal->getType())->getNumElements());
		Instruction * currentLoc = dyn_cast<Instruction>(vectorVal);
		if (!currentLoc)
		{
			// NULL means the value is not an instruction. So place the location marker at the start of the function
			currentLoc = &*(inst_begin(CURRENT_FUNCTION)); // The first instruction in the function
		}
		V_ASSERT(currentLoc != NULL);
		if (isa<PHINode>(currentLoc))
		{
			currentLoc = currentLoc->getParent()->getFirstNonPHI();
		}
		V_ASSERT(currentLoc->getParent() != NULL);
		V_ASSERT(currentLoc->getParent()->getParent() != NULL);
		
		Value * prevVector = UndefValue::get(vectorVal->getType());
		for (unsigned i = 0; i < valueEntry->numElements; i++)
		{
			V_ASSERT(valueEntry->scalarValues[i] != NULL);
			Value * constIndex = ConstantInt::get(getInt32Ty, i);
			prevVector = InsertElementInst::Create(prevVector, valueEntry->scalarValues[i], constIndex, "temp.vect", currentLoc);
			V_PRINT("\t\t\tCreated vector assembly inst:" << *prevVector << "\n");
		}
		SCMEntry * newEntry = createEmptySCMEntry(prevVector); // need a non-scalarized entry to represent this vector..
		updateSCMEntryWithValues(newEntry, valueEntry->scalarValues, false);		
		// Now need to replace the uses of "vectorVal" with the new vector (in this instruction)...
		vectorVal->replaceAllUsesWith(prevVector);
	}
	return true;
}

	
/////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is called to create a new SCM entry. If entry already exists - return it instead
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScalarizeFunction::SCMEntry * ScalarizeFunction::createEmptySCMEntry(Value * origValue)
{
	V_ASSERT(!isa<UndefValue>(origValue));
	if (SCM.count(origValue)) return SCM[origValue]; 
	V_ASSERT(isa<VectorType>(origValue->getType()));
	unsigned width = cast<VectorType>(origValue->getType())->getNumElements();
	
	// If index of next free SCMEntry is over the array size, create a new array
	if (SCMArrayLocation == ESTIMATED_INST_NUM)
	{
		// Create new SCMAllocationArray, push it to the vector of arrays, and set free index to 0
		SCMAllocationArray = new SCMEntry[ESTIMATED_INST_NUM];
		SCMArrays.push_back(SCMAllocationArray);
		SCMArrayLocation = 0;
	}
	// Allocate the new entry, and increment the free-element index
	SCMEntry * newEntry = &(SCMAllocationArray[SCMArrayLocation++]);

	// Set all primary data in entry
	newEntry->scalarValues[0] = NULL;
	newEntry->isOriginalVectorRemoved = false;
	newEntry->numElements = width;
	
	// Insert new entry to SCM map
	SCM.insert(std::pair<Value *, SCMEntry *>(origValue, newEntry));
	
	return newEntry;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is called to updte values in SCM entry.
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScalarizeFunction::updateSCMEntryWithValues(ScalarizeFunction::SCMEntry * entry, 
												 Value * scalarValues[], 
												 bool isOrigValueRemoved)
{	
	entry->isOriginalVectorRemoved = isOrigValueRemoved;
	
	for (unsigned i = 0; i < entry->numElements; i++)
	{
		V_ASSERT(scalarValues[i] != NULL);
		entry->scalarValues[i] = scalarValues[i];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This function returns an SCM entry if it exists and has scalarized values. otherwise return NULL.
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScalarizeFunction::SCMEntry * ScalarizeFunction::isScalarizedValue(Value * origValue)
{
	if (SCM.count(origValue))
	{
		SCMEntry * currEntry = SCM[origValue];
		if (currEntry->scalarValues[0] != NULL)
			return currEntry;
	}	
	// Getting here, value is either not in SCM, or was not scalarized
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for releasing all allocations of SCM entries  
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScalarizeFunction::releaseAllSCMEntries()
{
	V_ASSERT(SCMArrays.size() > 0); // At least one buffer is allocated at all times
	while (SCMArrays.size() > 1)
	{
		// If there are additional allocated entry Arrays, release all of them (leave only the first)
		SCMEntry * popEntry = SCMArrays.pop_back_val();
		delete[] popEntry;
	}
	SCMAllocationArray = SCMArrays[0]; // set the "current" array pointer to the only remaining array
	SCMArrayLocation = 0; // set the free-entry index to 0
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve any deferred instructions (if DRL is not empty)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::resolveDeferredInstructions ()
{
	unsigned DRLElements = DRL.size();
	while (DRLElements > 0)
	{
		DRLEntry current = DRL[DRLElements - 1];
		V_PRINT("\tDRL Going to fix value of orig inst: " << *current.unresolvedInst << "\n");
		unsigned width = cast<VectorType>(current.unresolvedInst->getType())->getNumElements();
		
		SCMEntry * currentInst;
		currentInst = createEmptySCMEntry(current.unresolvedInst);			
		
		if (currentInst->scalarValues[0] == NULL)
		{
			V_PRINT("\t\tInst was not scalarized yet, Scalarizing now...\n");
			// This instruction was not scalarized. Create a scalar values and place in SCM.
			// Need to break down the vector:
			//   %scalar0 = extractelement <4 x Type> %vector, i32 0
			//   %scalar1 = extractelement <4 x Type> %vector, i32 1 
			//   %scalar2 = extractelement <4 x Type> %vector, i32 2
			//   %scalar3 = extractelement <4 x Type> %vector, i32 3 
			Value *newInsts[width];
			for (unsigned i = 0; i < width; i++)
			{
				Value * constIndex = ConstantInt::get(getInt32Ty, i);			
				newInsts[i] = ExtractElementInst::Create(current.unresolvedInst, constIndex, "scalar");
			}
			
			updateSCMEntryWithValues(currentInst, newInsts, false);
			
			// Place new insts in code 
			Instruction * currInst = cast<Instruction>(current.unresolvedInst);
			for (unsigned i = 0; i < width; i++)
			{
				cast<Instruction>(newInsts[i])->insertAfter(currInst);
				currInst = cast<Instruction>(newInsts[i]);
			}
		}
		
		for (unsigned i = 0; i < width; ++i)
		{
			V_PRINT("\t\tConnecting to consumer #" << i << "  " << *(current.dummyVals[i]) <<  "...");
			current.dummyVals[i]->replaceAllUsesWith(currentInst->scalarValues[i]);
			V_ASSERT(cast<Instruction>(current.dummyVals[i])->use_empty());
			delete cast<Instruction>(current.dummyVals[i]); // Deleted and not "erased from parent" because this dummy was never inserted into the function...
			V_PRINT(" ...\n");
		}
		DRLElements--;
	}
	DRL.clear();
	return true;
}

