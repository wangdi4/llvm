/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "vectorize.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////
// VectorizeFunction object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
VectorizeFunction::VectorizeFunction()
{
	// VCM buffer allocation
	VCMAllocationArray = new VCMEntry[ESTIMATED_INST_NUM];
	VCMArrayLocation = 0;
	VCMArrays.push_back(VCMAllocationArray);

	V_PRINT("VectorizeFunction constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// VectorizeFunction object destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
VectorizeFunction::~VectorizeFunction()
{
	releaseAllVCMEntries();  // Erase all the VCMEntry arrays (except for the first)
	delete[] VCMAllocationArray;
	
	V_PRINT("VectorizeFunction destructor\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main method for vectorizing a function. It is responsible for iterating over all
// the instructions and sending them for Vectorization. Afterwards, it called the DR handling.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeFunction(CodeProperties * functionProp)
{	
	funcProperties = functionProp;

	bool retVal = true;
	
	// Prepare data structures for vectorizing a new function (may still have data left from previous function vectorization)
	removedInsts.clear();   // Clear list of instructions pending removal (may not have been cleared, if previous scalarization failed)
	VCM.clear();   // Clear all entries in VCM map
	BAG.clear();   // Clear all entries in BAG map
	DeferredResMap.clear();  // Clear list of DRL entries, which may have remained from previous failed vectorization
	releaseAllVCMEntries();  // Erase all the VCMEntry arrays (except for the first)

	// Vectorization. Iterate over all the instructions.
	// Always hold the iterator at the instruction following the one being vectorized (so the iterator will 
	// "skip" any instructions that are going to be added in the vectorization work
	inst_iterator vI = inst_begin(CURRENT_FUNCTION);
	inst_iterator vE = inst_end(CURRENT_FUNCTION);
	while (vI != vE)
	{
		Instruction * currInst = &*vI;
		vI++; // Move iterator to next instruction BEFORE vectorizing current instruction
		if (!vectorizeInstruction(currInst))
		{
			retVal = false;
			break; // If vectorization of instruction failed - reject this function
		}
	}
	
	// Iterate over deferred resolution instructions
	if (retVal) 
		retVal = resolveDeferredInstructions();

	if (retVal)
	{
		// Iterate over removed insts and delete them
		SmallPtrSet<Instruction *, ESTIMATED_INST_NUM>::iterator ri = removedInsts.begin();
		SmallPtrSet<Instruction *, ESTIMATED_INST_NUM>::iterator re = removedInsts.end();
		SmallPtrSet<Instruction *, ESTIMATED_INST_NUM>::iterator index = ri;
		for (ri = index; ri != re; ++ri)
		{
			funcProperties->setProperty(*ri, PR_INST_IS_REMOVED);
			(*ri)->dropAllReferences();
		}
		for (ri = index, re = removedInsts.end(); ri != re; ++ri)
		{
			V_ASSERT((*ri)->use_empty());
			(*ri)->eraseFromParent();
		}
		V_PRINT("\nCompleted vectorizing function: " << CURRENT_FUNCTION->getName() << " - Width: " << ARCH_VECTOR_WIDTH << "\n");
	}
	else
	{
		V_PRINT("\nFailed vectorizing function: " << CURRENT_FUNCTION->getName() << "\n");
	}

	// return vectorized kernel width
	return retVal;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main method for vectorizing an instruction. It is responsible for the following:
// 1) Find the required form of conversion
// 2) Prepare all input arguments
// 3) Create the converted instructions
// 4) Update the vector conversions map (VCM)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeInstruction(Instruction * I)
{
	V_PRINT("\tNext instruction to vectorize: " << *I << "\n");
	if (funcProperties->getProperty(I, PR_FUNC_PREP_TO_REMOVE))
	{
		V_PRINT("\t\tInstruction is marked for removal. Being removed..\n");
		removedInsts.insert(I);
		return true;		
	}
	if (funcProperties->getProperty(I, PR_OBTAIN_CL_INDEX))
	{
		V_PRINT("\t\tVectorizing TID creation Instruction: " << I->getName() << "\n");
		return generateSequentialIndices(I);
	}
	if (!funcProperties->getProperty(I, PR_TID_DEPEND) && !funcProperties->getProperty(I, PR_SPECIAL_CASE_BUILT_IN))
	{
		if (checkPossibilityToUseOriginalInstruction(I))
		{
			V_PRINT("\t\tInstruction is TID-independent. Will try not to vectorize it\n");
			return useOriginalConstantInstruction(I);
		}
	}
	
	switch (I->getOpcode())
	{
		case Instruction::Add :
		case Instruction::Sub :
		case Instruction::Mul :
			return vectorizeBinOpInst(I, true);
			
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
			return vectorizeBinOpInst(I, false);

		case Instruction::ICmp :
		case Instruction::FCmp :
			return vectorizeCompare(I);

		case Instruction::Trunc :
		case Instruction::ZExt :
		case Instruction::SExt :
		case Instruction::FPTrunc :
		case Instruction::FPExt :
		case Instruction::PtrToInt :
		case Instruction::IntToPtr :
		case Instruction::UIToFP :
		case Instruction::FPToUI :
		case Instruction::FPToSI :
		case Instruction::SIToFP :
			return vectorizeCastInst(I);

		case Instruction::BitCast :
			return vectorizeNonVectorizableInst(I, false);

		case Instruction::ShuffleVector :			
		case Instruction::ExtractValue :
		case Instruction::InsertValue :
			return vectorizeNonVectorizableInst(I, false);

		case Instruction::InsertElement :
			return vectorizeNonVectorizableInst(I, false);
			
		case Instruction::ExtractElement :
			return vectorizeExtractElement(I);

		case Instruction::Select :
			return vectorizeSelectInst(I);
			
		case Instruction::Alloca :
			return vectorizeAllocaInst(I);

		case Instruction::Load :
			return vectorizeLoadInst(I);
			
		case Instruction::Store :
			return vectorizeStoreInst(I);
			
		case Instruction::GetElementPtr :
			return vectorizeGetElementPtrInst(I);
			
		case Instruction::Call :
			return vectorizeCallInst(I);
						
		case Instruction::Br :
			return vectorizeBranch(I);
			
		case Instruction::PHI :
			return vectorizePHI(I);
			
		case Instruction::Ret :
			return vectorizeRetInst(I);
			
			// Unsupported instructions
		default :
			V_UNEXPECTED("Unsupported instruction: " << I->getName() );
			return false;
	}
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing a scalar instruction with multiple scalar instructions
// 1) clone the original instruction (ARCH_VECTOR_WIDTH times)
// 2) Prepare all input arguments
// 3) Replace arguments in cloned instructions, with prepared arguments
// 4) Update the vector conversions map (VCM)
// 5) remove (not delete) the original instruction, from the function
// Note: if mustDuplicate if false, and all the inputs are constants, the instruction will not be duplicated
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeNonVectorizableInst(Instruction * I, bool mustDuplicate)
{
	V_PRINT("\t\tNon-Vectorizable Instruction\n");
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> duplicateInsts;
	
	// Create cloned instructions
	for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
		duplicateInsts.push_back(I->clone());
	
	// Replace operands in duplicates
	unsigned numOperands = I->getNumOperands();
	unsigned firstOp = 0;
	bool allOperandsAreConstants = true;
	// if instruction is a function call, the first operand is the called function. No need to look it up in VCM...
	if (isa<CallInst>(I))
	{
		firstOp = 1;
		for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
		{
			cast<Instruction>(duplicateInsts[i])->setOperand(0, I->getOperand(0));
		}		
	}
	for (unsigned op = firstOp; op < numOperands; op++)
	{
		SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> multiOperands;
		bool operandIsConst;
		obtainMultiScalarValues(multiOperands, &operandIsConst, I->getOperand(op), I);
		allOperandsAreConstants = allOperandsAreConstants && operandIsConst;
		for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
		{
			cast<Instruction>(duplicateInsts[i])->setOperand(op, multiOperands[i]);
		}
	}
	// If all operands are consts - duplicating the original instruction is a waste.
	if (!mustDuplicate && allOperandsAreConstants)
	{
		for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
			delete duplicateInsts[i];
		return useOriginalConstantInstruction(I);
	}
	// Add new value/s to VCM
	createVCMEntryWithMultiScalarValues(I, duplicateInsts);
	std::string instName = I->getName();
	// Add new instructions into function
	for (unsigned duplicates = 0; duplicates < ARCH_VECTOR_WIDTH; duplicates++)
	{	
		duplicateInsts[duplicates]->setName(instName); // "Steal" the name of the original instruction (will automatically get an attached index)
		cast<Instruction>(duplicateInsts[duplicates])->insertBefore(I);
		funcProperties->duplicateProperties(duplicateInsts[duplicates], I);
	}
	// Remove original instruction
	removedInsts.insert(I);
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing a scalar BinOp instruction with vectored instruction/s
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeBinOpInst(Instruction * I, bool supportsWrap)
{
	V_PRINT("\t\tBinaryOp Instruction\n");
	V_ASSERT(I->getNumOperands() == 2); // Sanity check

	const Type * origInstType = I->getType();
	// If instruction's return type is already vector or more than 32 bits - do not vectorize
	if (isa<VectorType>(origInstType))
		return vectorizeNonVectorizableInst(I, false);

	bool has_NSW = false;
	bool has_NUW = false;
	if (supportsWrap)
	{
		has_NSW = cast<BinaryOperator>(I)->hasNoSignedWrap();
		has_NUW = cast<BinaryOperator>(I)->hasNoUnsignedWrap();
	}
	
	Value * newVectorInst;	
	Value * operand0;
	Value * operand1;
	bool op0IsConst, op1IsConst;
	obtainVectorizedValues(&operand0, &op0IsConst, I->getOperand(0), I);
	obtainVectorizedValues(&operand1, &op1IsConst, I->getOperand(1), I);
	if (op0IsConst && op1IsConst) 
		return useOriginalConstantInstruction(I);
	newVectorInst = BinaryOperator::Create(
										   cast<BinaryOperator>(I)->getOpcode(),
										   operand0,
										   operand1,
										   I->getName(),
										   I
										   );
	if (has_NSW) cast<BinaryOperator>(newVectorInst)->setHasNoSignedWrap();
	if (has_NUW) cast<BinaryOperator>(newVectorInst)->setHasNoUnsignedWrap();
	funcProperties->duplicateProperties(newVectorInst, I);
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(I, newVectorInst);
	// Remove original instruction
	removedInsts.insert(I);
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing a scalar Cast instruction with vectored instruction/s
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeCastInst(Instruction * I)
{
	V_PRINT("\t\tVectorizable Instruction\n");
	V_ASSERT(I->getNumOperands() == 1); // Sanity check
	const Type * origInstType = I->getType();

	// Most cast instructions are not vectorized - as Apple's back-end does not support mahy vectorized casts
	// The only known "supported" cast is Sext and Zext from i1 types to i32 and i64 integers
	if (!isa<SExtInst>(I) && !isa<ZExtInst>(I))
		return vectorizeNonVectorizableInst(I, false);
	const Type * inputType = I->getOperand(0)->getType();
	if (!isa<IntegerType>(inputType) || cast<IntegerType>(inputType)->getBitWidth() != 1 || cast<IntegerType>(origInstType)->getBitWidth() < 32)
		return vectorizeNonVectorizableInst(I, false);
	
	// Getting here, this cast inst IS vectorizable
	
	// If instruction's return type is already vector or more than 32 bits - do not vectorize
	if (isa<VectorType>(origInstType))
		return vectorizeNonVectorizableInst(I, false);
	
	Value * newVectorInst;
	
	const Type * targetDestType = VectorType::get(origInstType, ARCH_VECTOR_WIDTH);
	Value * operand0;
	bool op0IsConst;
	obtainVectorizedValues(&operand0, &op0IsConst, I->getOperand(0), I);
	if (op0IsConst) 
		return useOriginalConstantInstruction(I);
	newVectorInst = CastInst::Create(
									 cast<CastInst>(I)->getOpcode(),
									 operand0,
									 targetDestType,
									 I->getName(),
									 I
									 );
	
	funcProperties->duplicateProperties(newVectorInst, I);
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(I, newVectorInst);
	// Remove original instruction
	removedInsts.insert(I);
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for replacing function calls with multiple calls, or finding
// a known vector replacement for the called function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeCallInst(Instruction * I)
{
	V_PRINT("\t\tCall Instruction: " << cast<CallInst>(I)->getCalledFunction()->getName() << "\n");
//	V_ASSERT(ARCH_VECTOR_WIDTH < 16); // This is an NYI alert. No handling for 16-wide yet (as return vals are placed as first argument)
	CallInst * CI = cast<CallInst>(I);

	// First sttempt to see if this is a special-case function
	if (funcProperties->getProperty(CI, PR_SPECIAL_CASE_BUILT_IN))
	{
		// If vectorization of the special-case function failed, we must abort vectorization of the function - as it calls a fake function
		return checkSpecialCaseFunctionCall(CI);
	}

	/* Look for the function in the func_names hash */
	Function * scalarFunc = CI->getCalledFunction();
	std::string scalarFuncName = scalarFunc->getName(); 
	
	VFH::hashEntry * foundFunction = VFH::findScalarFunctionInHash(scalarFuncName);
	if (foundFunction != NULL)
	{
		const char * vectorFuncName = foundFunction->funcs[LOG_(ARCH_VECTOR_WIDTH)];
		// Found the function in the hash!
		V_PRINT("\t\t\tFound function ("<< scalarFuncName <<") in hash. Will convert to: "<< vectorFuncName << "\n");
		// Get new decl out of module.
		const Function *LibFunc = RUNTIME_MODULE->getFunction(vectorFuncName);
		V_ASSERT(LibFunc != NULL); // In debug mode, we don't want to allow a mismatch between the hash and the runtime module!
		if (LibFunc)
		{
			V_PRINT("\t\t\tPrototype of vectorized function is:" << *(LibFunc->getFunctionType()) << "\n");
			
			// Go over all inputs, and convert the needed ones to vectors
			std::vector<Value *> newArgs;
			bool isAllConstantInputs = true;
			unsigned argIndex = 1; // skip arg0, as it is the return value
			
			for (; argIndex < 4; argIndex++)
			{
				if (foundFunction->funcArgs[argIndex] == VFH::T_NONE) break; // no more arguments
				V_ASSERT(foundFunction->funcArgs[argIndex] != VFH::T_STATIC); // should not have non-vectorized inputs
				Value * operand;
				bool opIsConst;
				const Type * neededType = LibFunc->getFunctionType()->getParamType(argIndex-1);
				V_ASSERT(neededType != NULL);
				bool err = obtainVectorizedAndCastedValuesForCall(&operand, &opIsConst, I->getOperand(argIndex), I, neededType);
				if (err == false)
					return vectorizeNonVectorizableInst(I, false); // failed obtaining values. vectorization failed!
				isAllConstantInputs &= opIsConst;
				newArgs.push_back(operand);
			}
			V_ASSERT(argIndex > 1); // should have at least one input argument
			// If all inputs are constants - don't vectorize this call instruction
			if (isAllConstantInputs)
				return useOriginalConstantInstruction(I);		
			
			// Find (or create) declaration for newly called function
			V_ASSERT(!CURRENT_MODULE->getFunction(vectorFuncName) || LibFunc->getFunctionType() == CURRENT_MODULE->getFunction(vectorFuncName)->getFunctionType());
			Constant * vectFunctionConst = CURRENT_MODULE->getOrInsertFunction(vectorFuncName, LibFunc->getFunctionType());
			if (!vectFunctionConst)
			{
				V_UNEXPECTED("failed generating function in current module");
				return false;
			}
			
			// Create new instruction/s
			Value * newFuncCall;
			Value * newCall = CallInst::Create(vectFunctionConst, newArgs.begin(), newArgs.end());
			newCall->setName(I->getName()); // "Steal" the name of the original instruction (will automatically get an attached index)
			cast<Instruction>(newCall)->insertBefore(I);
			newFuncCall = fixCastedReturnTypes(cast<CallInst>(newCall), VectorType::get(I->getType(), ARCH_VECTOR_WIDTH));
			funcProperties->duplicateProperties(newCall, I);
			
			// Add new value/s to VCM
			createVCMEntryWithVectorValues(I, newFuncCall);
			
			// Remove original instruction
			removedInsts.insert(I);
			return true;
		}
	}	
	
	// Getting here, vectorization of the function call failed
	return vectorizeNonVectorizableInst(I, false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper method for generating a constant vector of values, to be used as ShuffleVector inputs 
/////////////////////////////////////////////////////////////////////////////////////////////////////
Constant * VectorizeFunction::createIncrementingConstVectorForShuffles(unsigned width, int * values)
{
	std::vector<Constant*> pre_vect;
	for (unsigned i = 0; i< width; i++)
	{
		if (values[i] >= 0)
		{
			pre_vect.push_back(ConstantInt::get(getInt32Ty, values[i]));
		}
		else
		{
			pre_vect.push_back(UndefValue::get(getInt32Ty));
		}
	}
	return ConstantVector::get(pre_vect);
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method handles ExtractElement instructions. Basically they are modified from extracting a 
// single element out of a vector, into a transpose of a value.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeExtractElement(Instruction * I)
{
	V_PRINT("\t\tExtractElement Instruction\n");
	Value * vectorValue = cast<ExtractElementInst>(I)->getOperand(0);
	unsigned vectorValueWidth = cast<VectorType>(vectorValue->getType())->getNumElements();
	Value * scalarIndexVal = cast<ExtractElementInst>(I)->getOperand(1);
	V_ASSERT(isa<VectorType>(vectorValue->getType()));
	
	// For transposing, Make sure the index is a constant, and the vector width is 2 or more
	if (isa<ConstantInt>(scalarIndexVal) && vectorValueWidth > 1)
	{
		uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();

		// The input should have been converted from 1 vector to several vectors.
		SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> inputOperands;
		bool operandIsConst;
		obtainMultiScalarValues(inputOperands, &operandIsConst, vectorValue, I);
		V_ASSERT(inputOperands[ARCH_VECTOR_WIDTH - 1] != NULL); // must have exactly ARCH_VECTOR_WIDTH vectors!
		V_ASSERT(isa<VectorType>(inputOperands[0]->getType()));
		unsigned inputVectorWidth = cast<VectorType>(inputOperands[0]->getType())->getNumElements();
		Value * shuffleMerge;

		// Make the transpose optimization only while arch vector is 4, and the usr vector is equal or smaller than that.
		if ((ARCH_VECTOR_WIDTH == 4) && (inputVectorWidth <= ARCH_VECTOR_WIDTH)/*(inputVectorWidth % 2 == 0)  && (I->getType() != getInt8Ty*/)
		{
			// Check if this inst is a simple broadcast
			bool isBroadcast = true;
			for (unsigned i = 1; i < ARCH_VECTOR_WIDTH; i++)
			{
				if (inputOperands[i] != inputOperands[i-1])
				{
					isBroadcast = false;
					break; // not a broadcast. no need to keep checking.
				}
			}
			
			if (isBroadcast)
			{
				// All values are the same (broadcast): a very simple transpose can be used
				UndefValue * undefVect = UndefValue::get(inputOperands[0]->getType());
				int b_sequence[4 /*ARCH_VECTOR_WIDTH*/];
				for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; ++i) 
					b_sequence[i] = scalarIndex;
				Constant * broadcast = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, b_sequence);
				shuffleMerge = new ShuffleVectorInst(inputOperands[0], undefVect, broadcast, "shuffleBroadcast", I);
				funcProperties->duplicateProperties(shuffleMerge, I);					
			}
			else
			{
				// If inputVectorWidth is smaller than ARCH_VECTOR_WIDTH, we'll need to extend the input vectors first
				if (inputVectorWidth < ARCH_VECTOR_WIDTH)
				{
					// Create the constant sequence for extending. Will look like (0, 1, undef, undef).
					int extender_sequence[4]; /* ==ARCH_VECTOR_WIDTH */
					for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; ++i)
					{
						if (i < inputVectorWidth) {
							extender_sequence[i] = i;
						} 
						else {
							extender_sequence[i] = -1;
						}
					}

					Constant * vectExtend = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, extender_sequence);
					UndefValue * undefVect = UndefValue::get(inputOperands[0]->getType());
					// Replace all the original input operands with their extended versions
					for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
					{
						inputOperands[i] = new ShuffleVectorInst(inputOperands[i], undefVect, vectExtend , "extend_vec", I);						
						funcProperties->duplicateProperties(inputOperands[i], I);
					}
				}
				
				// replace the extractElement with a sequence (or similar):
				//    %vector.tmp0 = shufflevector <4 x Type> %inp0, <4 x Type> %inp1, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
				//    %vector.tmp2 = shufflevector <4 x Type> %inp0, <4 x Type> %inp1, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
				//    %vector = shufflevector <4 x Type> %vector.tmp0, <4 x Type> %vector.tmp2, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
				
				// Create the indices for the shuffle instructions
				bool isIndexEven = (scalarIndex % 2 == 0);
				int evenIndex = isIndexEven ? scalarIndex : scalarIndex - 1;
				int sequence[4 /*ARCH_VECTOR_WIDTH*/] = {evenIndex, evenIndex + 1, ARCH_VECTOR_WIDTH + evenIndex, ARCH_VECTOR_WIDTH + evenIndex + 1};
				Constant * vectIndexer0 = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, sequence);
				int even_sequence[4 /*ARCH_VECTOR_WIDTH*/] = {0, 2, 4, 6};
				int odd_sequence[4 /*ARCH_VECTOR_WIDTH*/] = {1, 3, 5, 7};
				Constant * vectIndexer1 = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, isIndexEven ? even_sequence : odd_sequence);

				// Create the shuffleVecotr instructions
				Value * shuffle0 = new ShuffleVectorInst(inputOperands[0], inputOperands[1], vectIndexer0 , "shuffle0", I);
				Value * shuffle1 = new ShuffleVectorInst(inputOperands[2], inputOperands[3], vectIndexer0 , "shuffle1", I);
				shuffleMerge = new ShuffleVectorInst(shuffle0, shuffle1, vectIndexer1 , "shuffleMerge", I);
				funcProperties->duplicateProperties(shuffle0, I);
				funcProperties->duplicateProperties(shuffle1, I);
				funcProperties->duplicateProperties(shuffleMerge, I);
			}
		}
		else
		{
			// This is a fallback solution, when the size of VEC_WIDTH does not have a specialized solution.
			return vectorizeNonVectorizableInst(I, false);
		}
				
		// Add new value/s to VCM
		createVCMEntryWithVectorValues(I, shuffleMerge);

		// Remove original instruction
		removedInsts.insert(I);
		return true;
	}
	return vectorizeNonVectorizableInst(I, false);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method handles Compare instructions. Handeling depends on the usage of the result.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeCompare(Instruction * I)
{
	V_PRINT("\t\tCompare Instruction\n");
	const Type * origInstType = I->getType();
	std::string instName = I->getName();
	V_ASSERT(I->getNumOperands() == 2); // Sanity check
	
	// If instruction's return type is already vector or is a pointer - don't vectorize
	if (isa<VectorType>(origInstType) || isa<PointerType>(origInstType))
		return vectorizeNonVectorizableInst(I, false);
	
	// Create the vector values
	Value * newVectorInst;
	Value * operand0;
	Value * operand1;
	bool op0Const, op1Const;
	obtainVectorizedValues(&operand0, &op0Const, I->getOperand(0), I);
	obtainVectorizedValues(&operand1, &op1Const, I->getOperand(1), I);
	if (op0Const && op1Const)
		return useOriginalConstantInstruction(I); 		// all operands are consts. no need to do any vectorization work.
	newVectorInst = CmpInst::Create(
									cast<CmpInst>(I)->getOpcode(),
									cast<CmpInst>(I)->getPredicate(),
									operand0,
									operand1
									);
	newVectorInst->setName(instName);
	cast<Instruction>(newVectorInst)->insertBefore(I);
	funcProperties->duplicateProperties(newVectorInst, I);
	
	// Add all new instructions to VCM
	createVCMEntryWithVectorValues(I, newVectorInst);	
	
	// Remove original instruction
	removedInsts.insert(I);
	return true;	
}

	

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method handles Select instructions. Basically they are modified to the "select builtin function 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeSelectInst(Instruction * I)
{
	SelectInst * SI = cast<SelectInst>(I);
	V_PRINT("\t\tSelect Instruction\n");
	
	// First classify which "select" is needed: according to ARCH_WIDTH and result type
	const Type * retValType = I->getType();
	unsigned funcTypeIndex;
	bool isFloat = false;

// *** Types which are marked out have no corresponding sext <i1 x 4> to <type x 4> conversion instruction in LLVM 	
//	if (retValType == getInt8Ty)
//	{
//		funcTypeIndex = 0;
//	}
//	if (retValType == getInt16Ty)
//	{
//		funcTypeIndex = 1;
//	}
	if (retValType == getInt32Ty)
	{
		funcTypeIndex = 2;
	}
	else if (retValType == getInt64Ty)
	{
		funcTypeIndex = 3;
	}
	else if (retValType == getFloatTy)
	{
		funcTypeIndex = 4;
		isFloat = true;
	}
	else
	{
		// Return type is unsupported for vectorization (maybe even its a vector already?).
		return vectorizeNonVectorizableInst(I, false);
	}
	V_ASSERT(ARCH_VECTOR_WIDTH >= 4); // funcSize calculation below requires ARCH_VECTOR_WIDTH to be 4 or more 
	unsigned funcSize = LOG_(ARCH_VECTOR_WIDTH) + 1;  // basically an index conversion (4->3, 8->4, 16->5) for table lookup
	
	const char * selectFuncName = selectFuncsList[funcTypeIndex][funcSize];
	
	// Try to find (or declare) this function locally: Obtain function from Builtin functions module
	Function * LibSelectFunc = RUNTIME_MODULE->getFunction(selectFuncName);
	if (!LibSelectFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return vectorizeNonVectorizableInst(I, false);
	}
	
	// Find (or create) declaration for select function
	Constant * VectSelectFunc = CURRENT_MODULE->getOrInsertFunction(selectFuncName, 
																   LibSelectFunc->getFunctionType(),
																   LibSelectFunc->getAttributes());
	if (!VectSelectFunc)
	{
		V_UNEXPECTED("failed generating function in current module");
		return vectorizeNonVectorizableInst(I, false);
	}

	
	// Obtain vectorized true/false arguments
	Value * trueVal = SI->getTrueValue();
	Value * falseVal = SI->getFalseValue();
	Value * trueOperand;
	Value * falseOperand;
	const Type * argumentsNeededType = LibSelectFunc->getFunctionType()->getParamType(0); // get actual type used by select function
	V_ASSERT(argumentsNeededType != NULL);
	V_ASSERT(argumentsNeededType == LibSelectFunc->getFunctionType()->getParamType(1)); // must have same needed type for true and false values!
	bool err1, err2;
	bool isTrueConst, isFalseConst;
	err1 = obtainVectorizedAndCastedValuesForCall(&falseOperand, &isFalseConst, falseVal, I, argumentsNeededType);
	err2 = obtainVectorizedAndCastedValuesForCall(&trueOperand, &isTrueConst, trueVal, I, argumentsNeededType);
	if (err1 == false || err2 == false)
		return vectorizeNonVectorizableInst(I, false); // failed obtaining values. vectorization failed!				

	
	// Obtain and transform boolean vector argument to integer vector values
	Value * conditionVal = SI->getCondition();
	Value * conditionalOperand;
	bool isCondConst;
	obtainVectorizedValues(&conditionalOperand, &isCondConst, conditionVal, I);				
	if (isCondConst && isTrueConst && isFalseConst)
		return useOriginalConstantInstruction(I);
	// sign-extend the boolean vector to integer vector
	const Type * extendedType = VectorType::get(isFloat ? getInt32Ty : retValType, ARCH_VECTOR_WIDTH);
	Value * vectorExtend;
	vectorExtend = new SExtInst(conditionalOperand, extendedType, "boolean.extend", I);
	funcProperties->duplicateProperties(vectorExtend, I);		

	// Add new instruction to VCM so we can use it as input to function obtainVectorizedAndCastedValuesForCall
	createVCMEntryWithVectorValues(cast<Instruction>(vectorExtend), vectorExtend);
	// Obtained the "fixed" version of the conditional (bitcasted to whatever the function requires)
	const Type * conditionalNeededType = LibSelectFunc->getFunctionType()->getParamType(2);
	V_ASSERT(conditionalNeededType != NULL);
	Value * fixedConditionalOperand;
	bool err3, dummy;
	err3 = obtainVectorizedAndCastedValuesForCall(&fixedConditionalOperand, &dummy, vectorExtend, I, conditionalNeededType);

	// Prepare arguments to be used in function call/s, and create the new CALL instruction/s
	std::vector<Value *> newArgs;
	Value * newFuncCall;
	newArgs.push_back(falseOperand);
	newArgs.push_back(trueOperand);
	newArgs.push_back(fixedConditionalOperand);
	
	Value * newCall = CallInst::Create(VectSelectFunc, newArgs.begin(), newArgs.end(), "vectored.select", I);
	newFuncCall = fixCastedReturnTypes(cast<CallInst>(newCall), VectorType::get(retValType, ARCH_VECTOR_WIDTH));
	funcProperties->duplicateProperties(newFuncCall, I);
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(I, newFuncCall);
	
	// Remove original instruction
	removedInsts.insert(I);
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing Alloca insts
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeAllocaInst(Instruction * I)
{
	V_PRINT("\t\tAlloca Instruction\n");
	return vectorizeNonVectorizableInst(I, true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing Load insts
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeLoadInst(Instruction * I)
{
	V_PRINT("\t\tLoad Instruction\n");
	Value * ptrOperand = cast<LoadInst>(I)->getPointerOperand();
	// If loading a non vector/struct/array first-class value, and addresses are consecutive, optimize!
	if (funcProperties->getProperty(ptrOperand, PR_PTRS_CONSECUTIVE))
	{
		const Type * retType = I->getType();
		if (retType->isFloatingPointTy() || retType->isIntegerTy())
		{
			Value * newLoadInst;
			// Obtain the input addresses.
			SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> inpAddress;
			bool dummy;
			obtainMultiScalarValues(inpAddress, &dummy,ptrOperand , I);	
			
			const PointerType * inPtr = dyn_cast<PointerType>(inpAddress[0]->getType());
			V_ASSERT(inPtr); 
			// BitCast the "scalar" pointer to a "vector" pointer
			const Type * elementType = inPtr->getElementType();
			const Type * vectorElementType = VectorType::get(elementType, ARCH_VECTOR_WIDTH);
			PointerType * vectorInPtr = PointerType::get(vectorElementType, inPtr->getAddressSpace());
			Value * bitCastPtr = new BitCastInst(inpAddress[0], vectorInPtr, "ptrTypeCast", I);
			funcProperties->duplicateProperties(bitCastPtr, I);
			
			// Calculate which alignment can be assumed (alignment of original type)
			unsigned alignment = elementType->getPrimitiveSizeInBits() / 8; // divide by 8 to get number of bytes
			if (alignment == 0) alignment = 1; // if previous check failed, put "default" alignment
			V_ASSERT(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8);
			
			// Create a "vectorized" load
			newLoadInst = new LoadInst(bitCastPtr, I->getName(), false, alignment, I);
			funcProperties->duplicateProperties(newLoadInst, I);
			
			// Add new value/s to VCM
			createVCMEntryWithVectorValues(I, newLoadInst);
			
			// Remove original instruction
			removedInsts.insert(I);
			return true;
		}
	}
	return vectorizeNonVectorizableInst(I, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing Store insts
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeStoreInst(Instruction * I)
{
	V_PRINT("\t\tStore Instruction\n");
	// If addresses are consecutive, storing a non vector/struct/array first-class value, and data
	// is already organized inside vectors - optimize!
	Value * addrVal = cast<StoreInst>(I)->getPointerOperand();
	if (funcProperties->getProperty(addrVal, PR_PTRS_CONSECUTIVE))
	{
		Value * storedVal = I->getOperand(0);
		const Type * storeValType = storedVal->getType();
		if (storeValType->isFloatingPointTy() || storeValType->isIntegerTy())
		{
			Value * newStoreInst;
			SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> inpAddress;
			Value * inpValue;
			bool dummy;
			// Obtain the input addresses.
			obtainMultiScalarValues(inpAddress, &dummy, addrVal , I);	
			// obtain the input calues
			obtainVectorizedValues(&inpValue, &dummy, storedVal, I);
			
			const PointerType * inPtr = dyn_cast<PointerType>(inpAddress[0]->getType());
			V_ASSERT(inPtr); 
			// BitCast the "scalar" pointer to a "vector" pointer
			const Type * elementType = inPtr->getElementType();
			const Type * vectorElementType = VectorType::get(elementType, ARCH_VECTOR_WIDTH);
			PointerType * vectorInPtr = PointerType::get(vectorElementType, inPtr->getAddressSpace());
			Value * bitCastPtr = new BitCastInst(inpAddress[0], vectorInPtr, "ptrTypeCast", I);
			funcProperties->duplicateProperties(bitCastPtr, I);
			
			// Calculate which alignment can be assumed (alignment of original type)
			unsigned alignment = elementType->getPrimitiveSizeInBits() / 8; // divide by 8 to get number of bytes
			if (alignment == 0) alignment = 1; // if previous check failed, put "default" alignment
			V_ASSERT(alignment == 1 || alignment == 2 || alignment == 4 || alignment == 8);					
			
			// Create a "vectorized" store
			newStoreInst = new StoreInst(inpValue, bitCastPtr, false, alignment, I);
			funcProperties->duplicateProperties(newStoreInst, I);
			
			// Add new value/s to VCM
			createVCMEntryWithVectorValues(I, newStoreInst);
			
			// Remove original instruction
			removedInsts.insert(I);
			return true;
		}			
	}
	return vectorizeNonVectorizableInst(I, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing GetElementPtr insts
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeGetElementPtrInst(Instruction * I)
{
	V_PRINT("\t\tGetElementPtr Instruction\n");
	return vectorizeNonVectorizableInst(I, false);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing branches.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeBranch(Instruction * I)
{
	if (funcProperties->getProperty(I, PR_TID_DEPEND))
	{
		// Branch is dependent on TID. Can't vectorize (now...)
		V_PRINT("\tUnhandled Branch (TID dependent!). Rejecting.\n");
		return false;
	}
	BranchInst * BI = cast<BranchInst>(I);
	
	// If the branch is conditional, must make sure we didnt erase the condition which we needed.
	if (BI->isConditional())
	{
		Value * nonVectorizedCondition;
		if (!obtainNonVectorizedValue(BI->getCondition(), &nonVectorizedCondition))
			return false; // failed to obtain non-vectorized condition. Abort.
		BI->setCondition(nonVectorizedCondition);
	}

	V_PRINT("\t\tBranch Instruction\n");
	return useOriginalConstantInstruction(I);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for vectorizing PHI instructions. 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizePHI(Instruction * I)
{	
	V_PRINT("\t\tPHI Instruction\n");
	PHINode * PI = cast<PHINode>(I);
	unsigned numValues = PI->getNumIncomingValues();
	const Type * retType = PI->getType();
	
	// If PHI's return type is already a vector or a pointer - don't vectorize
	if (isa<VectorType>(retType) || isa<PointerType>(retType))
		return vectorizeNonVectorizableInst(I, false);
	
	const Type * vectorPHIType = VectorType::get(retType, ARCH_VECTOR_WIDTH);
	
	// Create new PHI node
	Value * newPHINode;
	newPHINode = PHINode::Create(vectorPHIType, "vectorPHI", I);
	funcProperties->duplicateProperties(newPHINode, I);

	// Obtain vectorized versions of incoming values, and place in new PHI node
	for (unsigned inputNum = 0; inputNum < numValues; inputNum++)
	{
		Value * origVal = PI->getIncomingValue(inputNum);
		BasicBlock * origBlock = PI->getIncomingBlock(inputNum);
		bool dummy;
		Value * newValue;
		obtainVectorizedValues(&newValue, &dummy, origVal, I);
		// Place vector values in PHI node/s
		cast<PHINode>(newPHINode)->addIncoming(newValue, origBlock);
	}
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(I, newPHINode);
	
	// Remove original instruction
	removedInsts.insert(I);
	return true;	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method is responsible for handling RET instructions. Must not return values!
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeRetInst(Instruction * I)
{
	V_PRINT("\t\tRet Instruction\n");
	if (cast<ReturnInst>(I)->getReturnValue() != NULL)
	{
		V_PRINT("\tRet with a return-value is not supported. Rejecting.\n");
		return false;
	}
	return useOriginalConstantInstruction(I);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This method generates the instructions for creating vectored indices (x, x+1, x+2, x+3)
// Orig instruction:
//   %indx.0 = call i32 get_global_id(i32 0) ;; Or some other inst which is derived from this one
// Add below it:
//   %broadcast1 = insertelement <4 x i32> undef, i32 %indx.0, i32 0
//   %broadcast2 = shufflevector <4 x i32> %broadcast1, undef, <4 x i32> zeroinitializer
//   %indx.vector = add <4 x i32> %broadcast2, <4 x i32> <0, 1, 2, 3>
// ***** The last inst is multiplied by the amount of vectors in the complete vectorization width
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::generateSequentialIndices(Instruction * I)
{	
	V_PRINT("\t\tFollowing get_global/local_id, make sequential indices Instructions\n");
	// Prepare: Obtain the used type of the ID, abd make a vector for it
	const Type * usedType = I->getType();
	V_ASSERT(!isa<VectorType>(usedType));
	const VectorType * vectorIndexType = VectorType::get(usedType, ARCH_VECTOR_WIDTH);
	UndefValue * undefVect = UndefValue::get(vectorIndexType);
	// Generate the broadcasting of the original ID
	Constant * constIndex = ConstantInt::get(getInt32Ty, 0);
	Value * insertInst = InsertElementInst::Create(undefVect, I, constIndex, "broadcast1");
	funcProperties->duplicateProperties(insertInst, I);
	Constant * zeroConst32Vector = ConstantVector::get(std::vector<Constant*>(ARCH_VECTOR_WIDTH, constIndex));	
	Value * shuffleInst = new ShuffleVectorInst(insertInst, undefVect, zeroConst32Vector , "broadcast2");
	funcProperties->duplicateProperties(shuffleInst, I);
	cast<Instruction>(insertInst)->insertAfter(I);
	cast<Instruction>(shuffleInst)->insertAfter(cast<Instruction>(insertInst));	

	// Generate the constant vectors
	std::vector<Constant*> constList; // separate constants to be used in constant vectors
	Instruction::BinaryOps addOperation;
	if (usedType->isIntegerTy())
	{
		addOperation = Instruction::Add;
		uint64_t constVal = 0;
		for (unsigned j=0; j < ARCH_VECTOR_WIDTH; ++j)
		{
			constList.push_back(ConstantInt::get(usedType, constVal++));
		}
	}
	else if (usedType->isFloatingPointTy())
	{
		addOperation = Instruction::FAdd;
		double constVal = 0.0f;
		for (unsigned j=0; j < ARCH_VECTOR_WIDTH; ++j)
		{
			constList.push_back(ConstantFP::get(usedType, constVal++));
		}
	}
	else 
	{
		V_UNEXPECTED("unsupported type for generating sequential indices");
		return false;
	}
	
	// Generate the TID vectors
	Instruction * current = cast<Instruction>(shuffleInst);
	Value * vectorIndex; // Instruction with the "complete" index vectors
	vectorIndex = BinaryOperator::Create(addOperation ,shuffleInst, ConstantVector::get(vectorIndexType, constList));
	funcProperties->duplicateProperties(vectorIndex, I);
	cast<Instruction>(vectorIndex)->insertAfter(current);
	current = cast<Instruction>(vectorIndex);
	
	// register the new converted values.  
	createVCMEntryWithVectorValues(I, vectorIndex);	
	return true;	
}


