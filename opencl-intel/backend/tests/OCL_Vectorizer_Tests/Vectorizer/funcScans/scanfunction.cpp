/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "scanfunction.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// ScanFunction object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScanFunction::ScanFunction():
	funcProperties(NULL) // clear it here, as a method for asserting that "preScalarizeScanFunction" is called before "postScalarizeScanFunction"
{
	V_PRINT("ScanFunction constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ScanFunction object destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
ScanFunction::~ScanFunction()
{
	V_PRINT("ScanFunction destructor\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface function, for initiating scanning of the code PRIOR to scalarization
// This scan is used for rooting input arguments of built-in and special-case functions
// Also used for detecting TID setter instructions, Alloca's ,RET instructions, and illegal
// code sequences (which should cause vectorizer to reject function)
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScanFunction::preScalarizeScanFunction(CodeProperties * functionProp)
{
	funcProperties = functionProp; // Set the function properties to *this* function
	unsigned numOfExits = 0; // does the function have more than one return points?
	V_PRINT("\t@@@ Pre-scanning function\n");
	// First check for sanity, that function returns void
	if (CURRENT_FUNCTION->getFunctionType()->getReturnType() != getVoidTy)
	{
		funcProperties->setFuncProperty(ERROR__FUNC_HAS_UNSUPPORTED_SEQ);
		V_PRINT("\t   Kernel has a non-void return type! cannot be vectorized!\n");
		return;
	}
	
	
	// Iterate over the entire function
	inst_iterator sI = inst_begin(CURRENT_FUNCTION);
	inst_iterator sE = inst_end(CURRENT_FUNCTION);
	while (sI != sE)
	{
		Instruction * currInst = &*sI;

		switch (currInst->getOpcode())
		{
			case Instruction::Call :
			{
				V_PRINT("\t\t@@@ Pre-scanning a CALL\n");
				CallInst * CI = cast<CallInst>(currInst);
				if (preemptiveScanForSettingThreadID(CI))
				{
					// instruction is a TID setter. Was marked accordingly already - no need to analyze it further
					break;
				}
				// CALL instructions are scanned, to find their inputs and output (which may be converted/truncated/etc..-
				// so we want to find the *original* value - before it was converted).
				SmallVector<Value *, 4> * rootVals = new SmallVector<Value *, 4>; // Permanent hold. if used, responsibility of funcProperties class to release in the end
				if (PreemptiveScanFunctionCall(CI, *rootVals))
				{
					// Add the new root values to the function calls map
					funcProperties->setScalarizableFunc(CI, rootVals);
					funcProperties->setProperty(CI, PR_SCALARIZABLE_BUILT_IN);
				}
				else if (preemptiveScanSpecialCaseFunc(CI, *rootVals))
				{
					// Add the new root values to the function calls map
					funcProperties->setScalarizableFunc(CI, rootVals);
					funcProperties->setProperty(CI, PR_SPECIAL_CASE_BUILT_IN);
				}
				else
				{
					delete rootVals;
					checkForWorkGroupFuncs(CI); // if function call is barrier or async operation - need to mark it in the function properties
					
					// Check if instruction may fall under speudo-dependency. If it receives pointer arguments - it should be treated as pseudo TID dependent
					if (isFunctionPseudoDependent(CI))
					{
						funcProperties->addToPseudoDependentList(CI);
					}
				}				
				break;
			}		
			case Instruction::Alloca :
			{
				V_PRINT("\t\t@@@ Pre-scanning an ALLOCA\n");
				// Alloca intructions provide separate allocations for separate work items.
				// - treat Alloca as PseudoDependent
				funcProperties->addToPseudoDependentList(currInst);
				break;
			}
			case Instruction::Ret :
			{
				funcProperties->setRetInst(currInst);
				if (++numOfExits > 1)
				{
					funcProperties->setFuncProperty(FUNC_HAS_MULTIPLE_EXITS);
					V_UNEXPECTED("Multiple returns from function. Unsupported!");
				}
				break;
			}
			default :
				break;
		}

		sI++;
	}		
	V_PRINT("\t@@@ Done Pre-scanning function\n");
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface function, for initiating scanning of the code AFTER the scalarization
// This scan is used for marking all the TID-dependent instructions, and marking
// instructions whose (soon to be) vectored values are known to have an equal distance
// (also known as induction variables)
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScanFunction::postScalarizeScanFunction()
{
	V_ASSERT(funcProperties != NULL); // make sure the preScalarize.. was called before this function
	V_PRINT("@@@ Start Post-scanning Function. Mark TID setters\n");
	// Mark TID-dependent instructions
	Instruction * TIDSetter = funcProperties->firstTIDSetterFromList();
	while (TIDSetter)
	{
		if (!funcProperties->getProperty(TIDSetter, PR_INST_IS_REMOVED))
		{			
			funcProperties->setProperty(TIDSetter, PR_TID_DEPEND);
			markTIDUsers(TIDSetter);
			markDistancesFromTID(TIDSetter);
		}
		TIDSetter = funcProperties->nextTIDSetterFromList();
	}

	V_PRINT("@@@ Start Marking PseudoDependent deps\n");
	// Mark PseudoDependent instructions as if they were TID-dependent (don't check for sequentials etc)
	TIDSetter = funcProperties->firstPseudoDependentFromList();
	while (TIDSetter)
	{
		if (!funcProperties->getProperty(TIDSetter, PR_INST_IS_REMOVED))
		{			
			funcProperties->setProperty(TIDSetter, PR_TID_DEPEND);
			markTIDUsers(TIDSetter);
		}
		TIDSetter = funcProperties->nextPseudoDependentFromList();
	}
	
	V_PRINT("@@@ Done Post-scanning function\n");
	funcProperties = NULL; // clear the function properties (for calling the scanner on the next function)
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Validate a CALL instruction - to see if it can be scalarized. Trace the roots of all the argumets
// and return value - in case there are type casts and dereferencing by pointers
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::PreemptiveScanFunctionCall(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals)
{
	// Look for the CALLed function in the func_names hash 
	Function * vectorFunc = callingInst->getCalledFunction();
	std::string vectorFuncName = vectorFunc->getName();
	V_PRINT("\tRoot function scanning for function: " << vectorFuncName << "\n");
	unsigned vectorWidth = 0;
	VFH::hashEntry * foundFunction = VFH::findVectorFunctionInHash(vectorFuncName, &vectorWidth);
	
	if (!foundFunction)
	{
		V_PRINT("\t\tNot found in hash!\n");
		return false;
	}
	V_ASSERT(vectorWidth > 1);
	
	// Vector function was found in hash. Now find the function prototype of the scalar function
	const char * scalarFuncName = foundFunction->funcs[0]; // name of the scalar function
	const Function *scalarFunc = RUNTIME_MODULE->getFunction(scalarFuncName);
	if (!scalarFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Functions should exist in runtime module, if exist in hash...
		return false;
	}
	
	// Generate a "desired" function type for vectorized function, by expanding all arguments to vector width
	const FunctionType * scalarFuncType = scalarFunc->getFunctionType();
	const FunctionType * vectorFuncType = vectorFunc->getFunctionType();
	unsigned numInputParams = scalarFuncType->getNumParams();
	const Type * desiredRetValType;
	std::vector<const Type *> desiredArgsTypes;
	V_ASSERT(scalarFuncType->getReturnType() != getVoidTy); // function hash does not support null-returning functions
	desiredRetValType = VectorType::get(scalarFuncType->getReturnType(), vectorWidth);
	for (unsigned i = 0; i < numInputParams; ++i)
	{
		V_ASSERT(foundFunction->funcArgs[i + 1] != VFH::T_NONE); // no more arguments? must be an error in function hash
		if (foundFunction->funcArgs[i + 1] == VFH::T_STATIC) 
		{
			desiredArgsTypes.push_back(scalarFuncType->getParamType(i));
		}
		else
		{
			desiredArgsTypes.push_back(VectorType::get(scalarFuncType->getParamType(i), vectorWidth));
		}
	}
	FunctionType * desiredVectorFuncType = FunctionType::get(desiredRetValType, desiredArgsTypes, false);
	
	// Check if desired function type really fits actual vector function type
	bool isVectorFuncInDesiredType = (desiredVectorFuncType == vectorFuncType);
	if (!isVectorFuncInDesiredType) 
	{
		V_PRINT("\t\tFunction prototype is different than desired.\n");
	}
	else
	{
		V_PRINT("\t\tFunction prototype fits desired exactly.\n");
	}
	
	// if function has desired structure, just fill the output vector and exit
	if (isVectorFuncInDesiredType)
	{
		rootVals.push_back(callingInst); // return value is the call itself
		for (unsigned i = 0; i < numInputParams; ++i)
		{
			rootVals.push_back(callingInst->getOperand(i+1)); // push in the values
		}
		return true;
	}
	
	
	// Getting here, the function did not have the desired type. Need to find the desired root values...
	unsigned currentArgumentIndex = 1; // index of "current" operand. Operand 0 is the called function itself 
	
	// First, obtain the root Return value
	Value * actualRetValue = callingInst;
	if (actualRetValue->getType() == getVoidTy)
	{
		// vector func return void, meaning return value is pushed as first function argument.
		actualRetValue = callingInst->getOperand(1);
		currentArgumentIndex++;
	}
	Value * rootRetVal = RootReturnValue(actualRetValue, desiredRetValType, callingInst);
	if (!rootRetVal) return false; // something went wrong, when rooting the return value
	rootVals.push_back(rootRetVal); // return value is the call itself
	
	// Second, obtain the input arguments
	for (unsigned i = 0; i < numInputParams; ++i)
	{
		Value *rootInputArg = RootInputArgument(callingInst->getOperand(currentArgumentIndex), desiredArgsTypes[i], callingInst);
		if (!rootInputArg) return false; // something went wrong, when rooting the argument
		rootVals.push_back(rootInputArg); // push in the values
		currentArgumentIndex++;
	}	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if the shuffle instruction is simply a vector extension of its input vector
/////////////////////////////////////////////////////////////////////////////////////////////////////
static Value * isShuffleExtend(Instruction * inst, const Type * realType)
{
	if (!isa<VectorType>(realType)) return NULL;
	ShuffleVectorInst * shuff = dyn_cast<ShuffleVectorInst>(inst);
	if (!shuff) return NULL;
	// Only assumption that must be true, is that the "proper" input is in the first vector input.
	// and the first shuffle values (locations) are the ordered components of that input.
	// No assumptions are made on the second input, or the trailing shuffled elements
	unsigned width = shuff->getType()->getNumElements();
	unsigned realWidth = cast<VectorType>(realType)->getNumElements();
	if (realWidth >= width) return NULL;
	
	for (unsigned i = 0; i < realWidth; ++i)
	{
		int maskValue = shuff->getMaskValue(i);
		if (maskValue != i) return NULL;
	}
	return shuff->getOperand(0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Follow thru a function input argument, until finding the root (where its type matches the "expected" type)
/////////////////////////////////////////////////////////////////////////////////////////////////////
Value * ScanFunction::RootInputArgument(Value * argument, const Type * rootType, Instruction * functionCallInst)
{
	if (argument->getType() == rootType) return argument; // no need for rooting. the argument is of the correct type.
	if (isa<PointerType>(argument->getType()))
	{
		Value * retVal = NULL;
		// expecting to find the origin of the pointer as an alloca instruction with 2 users: a store (of the original value) and the CALL inst
		AllocaInst * allocator = dyn_cast<AllocaInst>(argument);
		if (!allocator) return NULL; // argument is not a result of an Alloca
		if (allocator->isArrayAllocation()) return NULL; // alloca was done for an array of elements - not a single one
		if (allocator->getAllocatedType() != rootType) return NULL; // the alloca was done for the wrong type
		if (!allocator->hasNUses(2)) return NULL; // alloca has more (or less?) than 2 uses. Unexpected.
		
		for (Value::use_iterator i = allocator->use_begin(), e = allocator->use_end(); i != e; ++i)
		{
			if (Instruction *inst = dyn_cast<Instruction>(*i)) 
			{
				if (isa<StoreInst>(inst))
				{
					retVal = inst->getOperand(0);
					if (retVal->getType() != rootType) return NULL; // the store uses an unexpected type!
				}
				else if (isa<CallInst>(inst))
				{
					if (inst != functionCallInst) return NULL; // reached a different call instruction
				}
				else
				{
					return NULL; // Unsupported instruction as user of the pointer
				}
			}	
			else
			{
				return NULL; //  usage is not an instruction. Wierd...
			}
		}
		V_ASSERT(retVal != NULL);
		return retVal;
	}
	else
	{
		Value * currVal = argument;
		Instruction * inst;
		// Climb up over use-def chain, until the value's root is found
		for (;;)
		{
			if (currVal->getType() == rootType) return currVal; // found the root value!	
			if (inst = dyn_cast<Instruction>(currVal))
			{
				switch (inst->getOpcode())
				{
					case Instruction::BitCast :
					case Instruction::ZExt :
						// All bitcasts and ZEXTs are allowed
						currVal = inst->getOperand(0);
						break;
					case Instruction::ExtractElement :
						// ExtractElement is allowed in a single (wierd) case: ExtractElement <1 x Type>, 0
						currVal = inst->getOperand(0);
						if (cast<VectorType>(currVal->getType())->getNumElements() != 1) return NULL;
						break;
					case Instruction::ShuffleVector :
						// ShuffleVector may have been used as a sort of wierd "extend" 
						currVal = isShuffleExtend(inst, rootType);
						if (!currVal) return NULL;
						break;
					default:
						return NULL; // Unsupported instruction type
				}
			}
			else
			{
				unsigned sourceSize = currVal->getType()->getPrimitiveSizeInBits();
				unsigned targetSize = rootType->getPrimitiveSizeInBits();
				
				if (Constant * constVal = dyn_cast<Constant>(currVal))
				{
					// if value is a constant, cast it to the required type
					// Check if both types are of the same size
					if (sourceSize == targetSize)
					{
						currVal = ConstantExpr::getBitCast(constVal, rootType);
					}
					else
					{
						currVal = ConstantExpr::getBitCast(constVal, IntegerType::get(funcProperties->context(), sourceSize));
						currVal = ConstantExpr::getIntegerCast(cast<Constant>(currVal), IntegerType::get(funcProperties->context(), targetSize), false);
						if (currVal->getType() != rootType)
						{
							currVal = ConstantExpr::getBitCast(cast<Constant>(currVal), rootType);
						}						
					}
				}
				else
				{
					Instruction * insertPoint = &*(inst_begin(CURRENT_FUNCTION));
					// Value may be an input argument, or global of some sort. Cast it at the head of the function to the required type
					currVal = bitCastValToType(currVal, rootType, insertPoint);
				}
				
				return currVal;
			}
		}
		V_ASSERT(0); // Should never get here...
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if the shuffle instruction is simply a vector truncation of its input vector
/////////////////////////////////////////////////////////////////////////////////////////////////////
static bool isShuffleTruncate(Instruction * inst)
{
	ShuffleVectorInst * shuff = dyn_cast<ShuffleVectorInst>(inst);
	if (!shuff) return false;
	if (!isa<UndefValue>(shuff->getOperand(1))) return false;
	unsigned width = shuff->getType()->getNumElements();
	
	for (unsigned i = 0; i < width; i++)
	{
		int maskValue = shuff->getMaskValue(i);
		if (maskValue != i) return false;
	}
			
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Follow thru a function return argument, until its type matches the "expected" type
/////////////////////////////////////////////////////////////////////////////////////////////////////
Value * ScanFunction::RootReturnValue(Value * retVal, const Type * rootType, Instruction * functionCallInst)
{
	if (retVal->getType() == rootType) return retVal; // no need for rooting. the return value is of the right type
	if (isa<PointerType>(retVal->getType()))
	{
		// RetVal is a pointer. It was made by an Alloca instruction, which should have 2 uses: the Call Inst, and a load (of the root value)
		Value * rootRetVal = NULL;
		AllocaInst * allocator = dyn_cast<AllocaInst>(retVal);
		if (!allocator) return NULL; // argument was not a result of an Alloca. Can't scalarize.
		if (allocator->isArrayAllocation()) return NULL; // alloca is not sized as a single element. Unexpected for scalarizing.
		if (allocator->getAllocatedType() != rootType) return NULL; // the alloca was done for the wrong type
		if (!allocator->hasNUses(2)) return NULL; // alloca has more (or less?) than 2 uses. Unexpected.
		
		for (Value::use_iterator i = allocator->use_begin(), e = allocator->use_end(); i != e; ++i)
		{
			if (Instruction *inst = dyn_cast<Instruction>(*i)) 
			{
				if (isa<LoadInst>(inst))
				{
					rootRetVal = inst;
					if (rootRetVal->getType() != rootType) return NULL; // the load uses an unexpected type!
				}
				else if (isa<CallInst>(inst))
				{
					if (inst != functionCallInst) return NULL; // reached a different call instruction
				}
				else
				{
					return NULL; // Unsupported instruction as user of the pointer
				}
			}	
			else
			{
				return NULL; //  usage is not an instruction. Wierd...
			}
		}
		V_ASSERT(rootRetVal != NULL);
		return rootRetVal;
	}
	else
	{
		Instruction * retInst = dyn_cast<Instruction>(retVal);
		if (!retInst) return NULL; // retVal is not an instruction. Unexpected...
		V_ASSERT(retInst == functionCallInst); 
		
		// For the scalarization of the function to work, the return type needs to be exactly as expected, and not casted
		// due to ABI etc.. So, we add a casting inst/s from the func's return value, to the needed type. Afterwards,
		// we connect all the func's consumers to the new value. If some consumers are, by themselves, casts to the
		// "proper" type, we eliminate them on the way.
		
		// Collect all the convert(cast\trunc) instructions which are eminating from the retInst
		SmallPtrSet<Instruction *, 8> interimInstsList, middleConvertInstList, edgeConvertInstList;
		interimInstsList.insert(retInst);
		
		// For each entry in interimList, scan all the decendents. 
		while (!interimInstsList.empty())
		{
			// Extract interim value
			Instruction * interimToTest = *(interimInstsList.begin());
			interimInstsList.erase(interimToTest);
		
			// Add all the convert decendents to interimList. Decide if inst is an edge convert (has non-convert decendents)
			bool isEdge = false;
			for (Value::use_iterator i = interimToTest->use_begin(), e = interimToTest->use_end(); i != e; ++i)
			{
				Instruction * userInst = dyn_cast<Instruction>(i);
				if (userInst == NULL) return NULL; // unexpected user - not an instruction

				if (isa<BitCastInst>(userInst) || isa<TruncInst>(userInst) || isShuffleTruncate(userInst))
				{
					interimInstsList.insert(userInst);
				}
				else
				{
					isEdge = true;
				}
			}
			
			// Place this instruction into proper convert list
			if (isEdge)
			{
				edgeConvertInstList.insert(interimToTest);
			}
			else
			{
				middleConvertInstList.insert(interimToTest);
			}			
		}
		
		// Mark for removal all the convert instructions which are not direct parents of consuming instructions (any non-convert inst)
		for (SmallPtrSet<Instruction *, 8>::iterator i = middleConvertInstList.begin(), e = middleConvertInstList.end(); i != e; ++i)
		{
			Instruction * inst = *i;
			if (inst != retInst)
			{
				// careful not to remove the retInst, even if all its decendents are convert insts....
				inst->dropAllReferences();
				funcProperties->setProperty(inst, PR_FUNC_PREP_TO_REMOVE);				
			}
		}		
		
		// Generate dummy with "proper" return value
		const Type * ptrType = PointerType::get(rootType, 0);
		Constant * subExpr = ConstantExpr::getIntToPtr(ConstantInt::get(getInt32Ty, APInt(32, 0xdeadbeef)), ptrType);
		Instruction * dummyInst = new LoadInst(subExpr);
		
		// For each remaining convert, replace all its users with the dummy, or a cast from the dummy value. Mark convert for removal
		for (SmallPtrSet<Instruction *, 8>::iterator i = edgeConvertInstList.begin(), e = edgeConvertInstList.end(); i != e; ++i)
		{
			Instruction * edge = *i;
			Instruction * convertedVal;
			
			if (edge != retInst)
			{
				if (edge->getType() == rootType)
				{
					convertedVal = dummyInst;
				}
				else
				{
					convertedVal = bitCastValToType(dummyInst, edge->getType(), edge);
				}
				// Reconnect all the insts's users with the dummy (or a conversion of it), and erase the edge
				edge->replaceAllUsesWith(convertedVal);
				edge->dropAllReferences();
				funcProperties->setProperty(edge, PR_FUNC_PREP_TO_REMOVE);
			}
			else
			{
				// dealing with the retInst itself
				V_ASSERT(edge->getType() != rootType);
				// Generate a convertion (from dummy to needed type) right after the func call
				BasicBlock::iterator nextInst(functionCallInst);
				++nextInst;  // Set nextInst to point to the instruction after the function call
				convertedVal = bitCastValToType(dummyInst, edge->getType(), nextInst);
				edge->replaceAllUsesWith(convertedVal);
			}
		}		
		
		// Replace dummy with a cast from retInst to the proper type
		BasicBlock::iterator nextInst(functionCallInst);
		++nextInst;  // Set nextInst to point to the instruction after the function call
		Instruction * newCast = bitCastValToType(retInst, rootType, nextInst);
		dummyInst->replaceAllUsesWith(newCast);
		delete dummyInst;
		
		return newCast;
	}	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate type-conversion and place in given location (either before or after it)
/////////////////////////////////////////////////////////////////////////////////////////////////////
Instruction * ScanFunction::bitCastValToType(Value * orig, const Type * targetType, Instruction * insertPoint)
{
	const Type * currType = orig->getType();
	unsigned currSize = currType->getPrimitiveSizeInBits();
	unsigned rootSize = targetType->getPrimitiveSizeInBits();
	Instruction * newCast;

	if (currSize == rootSize)
	{
		// just bitcast from one to the other
		newCast = new BitCastInst(orig, targetType, "cast_val", insertPoint);
	}
	else if (currSize < rootSize)
	{
		// cast to integer
		Instruction * firstCast = new BitCastInst(orig, IntegerType::get(funcProperties->context(), currSize), "cast1", insertPoint);
		// Zero-extend
		Instruction * zextVal = new ZExtInst(firstCast, IntegerType::get(funcProperties->context(), rootSize), "zext_cast", insertPoint);
		// cast to target
		newCast = new BitCastInst(zextVal, targetType, "cast_val", insertPoint);
	}
	else
	{
		// cast to integer
		Instruction * firstCast = new BitCastInst(orig, IntegerType::get(funcProperties->context(), currSize), "cast1", insertPoint);
		// Truncate
		Instruction * trunc1 = new TruncInst(firstCast, IntegerType::get(funcProperties->context(), rootSize), "trunc1", insertPoint);
		// cast to target
		newCast = new BitCastInst(trunc1, targetType, "cast_val", insertPoint);
	}
	
	return newCast;
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Iterate thru def-use map from TID setter instruction, and mark all the TID users as such
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::markTIDUsers(Instruction * TIDInst)
{
	// Storage containing all the instructions which need to be checked (for their users, who are also TID users)
	SmallPtrSet<Value *, ESTIMATED_INST_NUM> valuesToVisit;
	Value * current = TIDInst;

	// Loop over all instructions which require marking as TID (and checking their users)
	while (true) {		
		// For all instructions and basicblock, find and add their decendents (if unmarked)		
		if (isa<Instruction>(current))
		{
			for (Value::use_iterator ui = current->use_begin(), ue = current->use_end(); ui != ue; ++ui)
			{
				if (Instruction * useInst = dyn_cast<Instruction>(*ui))
				{
					// If user is not marked as TID user, add it to the instructions list. we will visit it later.
					if (!funcProperties->getProperty(useInst, PR_TID_DEPEND))
					{
						valuesToVisit.insert(useInst);
					}
				}
				else
				{
					// Instruction users who are not instructions???
					V_UNEXPECTED("Users are not instructions");
				}
			}
			
			// For Branch instructions, add their decending basic blocks		
			if (BranchInst * BI = dyn_cast<BranchInst>(current))
			{
				for (unsigned j = 0; j < BI->getNumSuccessors(); j++)
				{
					valuesToVisit.insert(BI->getSuccessor(j));
				}
			}
		}
		else if (isa<BasicBlock>(current))
		{
			for (Value::use_iterator ui = current->use_begin(), ue = current->use_end(); ui != ue; ++ui)
			{
				// For basic blocks, only descending PHI nodes are to be marked as TID-dependent
				if (PHINode * useInst = dyn_cast<PHINode>(*ui))
				{
					// If user is not marked as TID user, add it to the instructions list. we will visit it later.
					if (!funcProperties->getProperty(useInst, PR_TID_DEPEND))
					{
						valuesToVisit.insert(useInst);
					}
				}
			}
		}
		else
		{
			// Only instructions and basic blocks should be in this loop.
			V_UNEXPECTED("Expecting only instructions and basic blocks!\n");
			return false;
		}
		

		// Now select the next instruction/BB out of the instructions list, and mark it as TID-dependent
		if (valuesToVisit.empty()) break; // if list is empty - exit main loop
		V_ASSERT(valuesToVisit.begin() != valuesToVisit.end());
		current = cast<Value>(*valuesToVisit.begin());
		valuesToVisit.erase(current);
		if (isa<Instruction>(current))
		{
			funcProperties->setProperty(current, PR_TID_DEPEND);
			V_PRINT("\t@@@ TID dependent inst: " << *current << "\n");
		}
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Iterate thru def-use map from TID setter instruction, and mark instructions whose vectorized
// values are going to be consecutive (like the TID setter) or with equal distances
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::markDistancesFromTID(Instruction * TIDInst)
{
	SmallVector<Instruction *, 8> Level1Insts, Level2Insts, *currLevelInsts, *nextLevelInsts, *tmp;
	funcProperties->setProperty(TIDInst, PR_TID_VALS_CONSECUTIVE);
	V_PRINT("\t### inst is consecutive :" << *TIDInst << "\n");
	
	currLevelInsts = &Level1Insts;
	nextLevelInsts = &Level2Insts;
	currLevelInsts->push_back(TIDInst);
	// Go BFS-wise into the def-use graph, marking instructions known to have consecutive values, or equal-distance values

	// Iterate until the current Level is empty
	while (currLevelInsts->size() > 0)
	{
		// Iterate over all instructions in nextLevelInsts list
		while (currLevelInsts->size() > 0)
		{
			Instruction * localRoot = currLevelInsts->pop_back_val();
			V_PRINT("\t#   localRoot being scanned :" << *localRoot << "\n");
			bool is_consecutive = funcProperties->getProperty(localRoot, PR_TID_VALS_CONSECUTIVE);
			// Sanity check: Insts in the list have EITHER consecutive vals, OR equal distance vals.
			V_ASSERT((!is_consecutive && funcProperties->getProperty(localRoot, PR_TID_VALS_EQUAL_DIST))||
					 (is_consecutive && !funcProperties->getProperty(localRoot, PR_TID_VALS_EQUAL_DIST)));
			
			// Iterate over all users. Mark qualified users and add them to nextLevelInsts list
			for (Value::use_iterator ui = localRoot->use_begin(), ue = localRoot->use_end(); ui != ue; ++ui)
			{
				Instruction * useInst = dyn_cast<Instruction>(*ui);
				V_ASSERT(funcProperties->getProperty(useInst, PR_TID_DEPEND)); // sanity check. Is definately TID dependent...
				if (!useInst) continue; // Can there be a user that is not an instruction??
				V_PRINT("\t##  decendant being scanned :" << *useInst << "\n");
				// If instruction was marked already - no need to check it again...
				if (funcProperties->getProperty(useInst, PR_TID_VALS_CONSECUTIVE) ||
					funcProperties->getProperty(useInst, PR_TID_VALS_EQUAL_DIST))
				{
					continue;
				}

				switch (useInst->getOpcode())
				{
					case Instruction::Add :
					case Instruction::Sub :
					case Instruction::FAdd :
					case Instruction::FSub :
					{
						Value * op0 = useInst->getOperand(0);
						Value * op1 = useInst->getOperand(1);
						V_ASSERT(op0 == localRoot || op1 == localRoot);
						// If one of the operands is a constant (the other must be localRoot) - this inst keeps the property
						if (!funcProperties->getProperty(op0, PR_TID_DEPEND) ||
							!funcProperties->getProperty(op1, PR_TID_DEPEND))
						{
							// This add/sub works on a constant (not TID dependent) value. So the new value keeps the properties of its father
							funcProperties->setProperty(useInst, is_consecutive? 
														PR_TID_VALS_CONSECUTIVE : 
														PR_TID_VALS_EQUAL_DIST);
							if (is_consecutive)
							{
								V_PRINT("\t### ADD/SUB consecutive :" << *useInst << "\n");
							}
							else
							{
								V_PRINT("\t### ADD/SUB Equal-dist :" << *useInst << "\n");
							}
							nextLevelInsts->push_back(useInst);
						}
						// If both operands are equal-dist or consecutive - the result is eual-dist
						else if ((funcProperties->getProperty(op0, PR_TID_VALS_CONSECUTIVE) || 
								  funcProperties->getProperty(op0, PR_TID_VALS_EQUAL_DIST) )   &&
								 (funcProperties->getProperty(op1, PR_TID_VALS_CONSECUTIVE) ||
								  funcProperties->getProperty(op1, PR_TID_VALS_EQUAL_DIST)))
						{
							funcProperties->setProperty(useInst, PR_TID_VALS_EQUAL_DIST);
							V_PRINT("\t### ADD/SUB Equal-dist :" << *useInst << "\n");
							nextLevelInsts->push_back(useInst);						
						}
						break;
					}
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
					{
						// Casts keep the values as is
						funcProperties->setProperty(useInst, is_consecutive? 
													PR_TID_VALS_CONSECUTIVE : 
													PR_TID_VALS_EQUAL_DIST);
						if (is_consecutive)
						{
							V_PRINT("\t### CAST is consecutive :" << *useInst << "\n")
						}
						else
						{
							V_PRINT("\t### CAST is Equal-dist :" << *useInst << "\n");
						}
						nextLevelInsts->push_back(useInst);
						break;
					}
					case Instruction::Trunc :
					{
						// Trunc may cause the loss of valuable bits in the value. So allow only consecutives for truncs to width over 4 bits
						if (is_consecutive)
						{
							const Type * destType = cast<CastInst>(useInst)->getDestTy();
							const IntegerType * intType = dyn_cast<IntegerType>(destType);
							if (intType && (intType->getBitWidth() >= 4))
							{
								funcProperties->setProperty(useInst, PR_TID_VALS_CONSECUTIVE);
								V_PRINT("\t### TRUNC is consecutive :" << *useInst << "\n");
								nextLevelInsts->push_back(useInst);
							}
						}
						break;
					}
					case Instruction::Mul :
					case Instruction::FMul :
					case Instruction::Shl :
					{
						// Multiplication by constant moves consec or equal-dist values to be equal-dist
						Value * op0 = useInst->getOperand(0);
						Value * op1 = useInst->getOperand(1);
						V_ASSERT(op0 == localRoot || op1 == localRoot);
						if (!funcProperties->getProperty(op0, PR_TID_DEPEND) ||
							!funcProperties->getProperty(op1, PR_TID_DEPEND))
						{
							// This mul/shl works on a constant (not TID dependent) value. so the new val is equal-dist
							funcProperties->setProperty(useInst, PR_TID_VALS_EQUAL_DIST);
							V_PRINT("\t### MUL is Equal-dist :" << *useInst << "\n");
							nextLevelInsts->push_back(useInst);
						}
						break;
					}
					case Instruction::GetElementPtr :
					{
						// Consecutive values which are used as array indexes, cause the addresses to be consecutive pointers
						GetElementPtrInst * getElemInst = cast<GetElementPtrInst>(useInst);
						Value * pointerOp = getElemInst->getPointerOperand();
						// The pointer must not be TID dependent. Also there must be exactly one index (which is the localRoot inst)
						if ((!funcProperties->getProperty(pointerOp, PR_TID_DEPEND)) &&
							(getElemInst->getNumIndices() == 1))
						{
							V_ASSERT(getElemInst->getOperand(1) == localRoot); // sanity check
							// only if localRoot is consecutive, the pointers are consecutive
							if (funcProperties->getProperty(localRoot, PR_TID_VALS_CONSECUTIVE))
							{
								funcProperties->setProperty(getElemInst, PR_PTRS_CONSECUTIVE);
								V_PRINT("\t### inst has consecutive pointers :" << *getElemInst << "\n");
							}
							// Don't add this inst to list. No distance can be derived from its decendents
						}
						break;
					}
					case Instruction::Call :
					{
						// Some special-case functions can be treated safely as having a known behavior
						if (funcProperties->getProperty(useInst, PR_SC_DOTPROD))
						{
							// Dot product includes multiplications and additions. so if one side is constant, the result is equal-dist
							
							// input arguments of the fake dot-product should be scalars. So left (or right) half of them needs to be constant 
							unsigned funcOperands = useInst->getNumOperands() - 1; // all the arguments of the CALL, except for the called function itself...
							V_ASSERT(funcOperands%2 == 0); // sanity: fake-scalar dot product must have even amount of inputs!
							V_ASSERT(funcOperands > 0 && funcOperands <= 8); // sanity: should have 2, 4, 6 or 8 operands.
							unsigned vecWidth = funcOperands/2;
							
							bool isLeftConstant=true, isRightConstant=true, isLegal=true;;
							for (unsigned i = 0; i< vecWidth; i++)
							{
								Value * leftOp = useInst->getOperand(i+1);
								Value * rightOp = useInst->getOperand(i+1+vecWidth);
								
								// Scan all the input arguments. Should have at least half constant, and the rest equal-dist...
								if (funcProperties->getProperty(leftOp, PR_TID_DEPEND))
								{
									isLeftConstant = false;
									if (!funcProperties->getProperty(leftOp, PR_TID_VALS_CONSECUTIVE) && !funcProperties->getProperty(leftOp, PR_TID_VALS_EQUAL_DIST))
									{
										isLegal = false;
										break; // Failed. Can exit the loop...
									}
								}
								if (funcProperties->getProperty(rightOp, PR_TID_DEPEND))
								{
									isRightConstant = false;
									if (!funcProperties->getProperty(rightOp, PR_TID_VALS_CONSECUTIVE) && !funcProperties->getProperty(leftOp, PR_TID_VALS_EQUAL_DIST))
									{
										isLegal = false;
										break; // Failed. Can exit the loop...
									}
								}
							}
							V_ASSERT(!(isLeftConstant && isRightConstant)); // Sanity can't be that ALL the arguments are constants...
							
							if (isLeftConstant || isRightConstant)
							{
								funcProperties->setProperty(useInst, PR_TID_VALS_EQUAL_DIST);
								V_PRINT("\t### CALL is Equal-dist :" << *useInst << "\n");
								nextLevelInsts->push_back(useInst);							
							}
						}
						break;
					}
				}			
			}		
		}
		
		// move to next depth level
		tmp = currLevelInsts;
		currLevelInsts = nextLevelInsts;
		nextLevelInsts = tmp;
		V_PRINT("\t@@@ Switching lists. New has " << currLevelInsts->size() << " entries\n");
	}
	
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if CALL instruction corresponds to get_global_id or get_local_id, and mark it accordingly
// return TRUE if it is so.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::preemptiveScanForSettingThreadID(CallInst * callingInst)
{	
	std::string funcName = callingInst->getCalledFunction()->getName();
	if (funcName != GET_GID_NAME && funcName != GET_LID_NAME) return false; // not a get_***_id function
	
	Value * inputValue = callingInst->getOperand(1);
	if (!isa<ConstantInt>(inputValue))
	{
		// Getting TID of non-constant dimension is not supported by vectorizer!
		funcProperties->setFuncProperty(ERROR__FUNC_HAS_UNSUPPORTED_SEQ);
		return true;
	}
	// Need to check which constant is provided to function.
	if (!(cast<ConstantInt>(inputValue)->equalsInt(0)))
	{
		// TID is from different dimension 
		// Sanity: Should be of a supported dimension...
		V_ASSERT(cast<ConstantInt>(inputValue)->equalsInt(1) || cast<ConstantInt>(inputValue)->equalsInt(2));
		return true;
	}
	// Asking for dimesion 0 - need to mark for vectorizing sequentially
	// Optimization: Follow tid setter, maybe it is immediately casted to different type (in the same block)
	Instruction * instToMark = callingInst;
	BasicBlock * callerBlock = callingInst->getParent();
	while (instToMark->hasOneUse())
	{
		V_ASSERT(isa<Instruction>(instToMark->use_begin()));
		if (cast<Instruction>(instToMark->use_begin())->isCast() &&
			!isa<BitCastInst>(instToMark->use_begin()) && 
			cast<Instruction>(instToMark->use_begin())->getParent() == callerBlock)
		{
			// single user who is cast instruction. follow down to it, to be marked as TID setter
			instToMark = cast<Instruction>(instToMark->use_begin());
		}
		else
		{
			break; // dont want to loop forever over this instruction...
		}
	}
	funcProperties->setProperty(instToMark, PR_OBTAIN_CL_INDEX);		
	funcProperties->addToTIDSettersList(instToMark);
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Validate a CALL instruction - to see if it is a special-case function (Samplers, etc). if so,
// find all the argumets - in case there are type casts and dereferencing by pointers
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::preemptiveScanSpecialCaseFunc(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals)
{
	std::string funcName = callingInst->getCalledFunction()->getName();
	
	if (funcName == READ_IMAGEF_2D_NAME)
	{
		V_PRINT("\t\t\tPre-scanning found special-case read sampler 2D\n");
		Value * inputCoordinates = callingInst->getOperand(3);
		if (!inputCoordinates) return false;			
		// Find the scalar origins of the coordinates
		Value * properCoordinates = RootInputArgument(inputCoordinates, READ_IMAGEF_2D_COORD_TYPE, callingInst);
		Value * properRetVal = RootReturnValue(callingInst, READ_IMAGEF_RET_TYPE, callingInst);
		if (!properCoordinates || !properRetVal) return false; // failed to find root values
		rootVals.push_back(properRetVal); // put the proper return value in the rootVals vector
		rootVals.push_back(properCoordinates); // put the proper arguments in the rootVals vector
		funcProperties->setProperty(callingInst, PR_SC_READ_SAMPLER_F_2D);		
		V_PRINT("\t\t\tPre-scanning special-case succeeded\n");
		return true;
	}
	else if (funcName == READ_IMAGEF_3D_NAME)
	{
		V_PRINT("\t\t\tPre-scanning found special-case read sampler 3D\n");
		Value * inputCoordinates = callingInst->getOperand(3);
		if (!inputCoordinates) return false;			
		// Find the scalar origins of the coordinates
		Value * properCoordinates = RootInputArgument(inputCoordinates, READ_IMAGEF_3D_COORD_TYPE, callingInst);
		Value * properRetVal = RootReturnValue(callingInst, READ_IMAGEF_RET_TYPE, callingInst);
		if (!properCoordinates || !properRetVal) return false; // failed to find root values
		rootVals.push_back(properRetVal); // put the proper return value in the rootVals vector
		rootVals.push_back(properCoordinates); // put the proper arguments in the rootVals vector
		funcProperties->setProperty(callingInst, PR_SC_READ_SAMPLER_F_3D);		
		V_PRINT("\t\t\tPre-scanning special-case succeeded\n");
		return true;
	}
	else if (funcName == WRITE_IMAGEF_NAME)
	{
		V_PRINT("\t\t\tPre-scanning found special-case write sampler\n");
		Value * inputCoordinates = callingInst->getOperand(2);
		Value * inputColors = callingInst->getOperand(3);
		if (!inputCoordinates || !inputColors) return false;
		// Find the scalar origins of the coordinates
		Value * properCoordinates = RootInputArgument(inputCoordinates, WRITE_IMAGEF_COORD_TYPE, callingInst);
		if (!properCoordinates) return false; // failed to find coordinates source
		Value * properColors = RootInputArgument(inputColors, WRITE_IMAGEF_COLORS_TYPE, callingInst);
		if (!properColors) return false; // failed to find colors source
		// put the proper arguments in the rootVals vector
		rootVals.push_back(properCoordinates);
		rootVals.push_back(properColors);
		funcProperties->setProperty(callingInst, PR_SC_WRITEF_SAMPLER);		
		V_PRINT("\t\t\tPre-scanning special-case succeeded\n");
		return true;
	}
	else if (funcName == CI_GAMMA_SCALAR_NAME)
	{
		V_PRINT("\t\t\tPre-scanning found special-case ci_gamma function\n");
		if (ARCH_VECTOR_WIDTH != 4 && ARCH_VECTOR_WIDTH != 8) return false;
		Value * inputRGB = callingInst->getOperand(1);
		Value * inputY = callingInst->getOperand(2);
		if (!inputRGB || !inputY) return false;
		if (inputY->getType() != CI_GAMMA_SCALAR_INPUT_Y_TYPE) return false; // scalar input MUST be of correct type!
		// Find the scalar origins of the arguments and retVal
		Value * properRGB = RootInputArgument(inputRGB, CI_GAMMA_SCALAR_INPUT_RGB_TYPE, callingInst);
		Value *properRetVal = RootReturnValue(callingInst, CI_GAMMA_SCALAR_RETVAL_TYPE, callingInst);
		if (!properRGB || !properRetVal) return false; // failed to find root values
		rootVals.push_back(properRetVal);
		rootVals.push_back(properRGB);
		funcProperties->setProperty(callingInst, PR_SC_CI_GAMMA);		
		V_PRINT("\t\t\tPre-scanning special-case ci_gamma succeeded\n");
		return true;
	}
	
	else if (funcName.compare(0, SELECT_NAME_PREFIX.length(), SELECT_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case select\n");
		if (!selectFunctionPrepArgs(callingInst, rootVals))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_SELECT_INST);		
		return true;
	}	
	else if (funcName.compare(0, DOT_PRODUCT_NAME_PREFIX.length(), DOT_PRODUCT_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case dot product\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 2, true, geometric_dot))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_DOTPROD);		
		return true;
	}
	else if (funcName.compare(0, DISTANCE_NAME_PREFIX.length(), DISTANCE_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case distance\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 2, true, geometric_distance))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_DISTANCE);		
		return true;
	}
	else if (funcName.compare(0, FAST_DISTANCE_NAME_PREFIX.length(), FAST_DISTANCE_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case fast_distance\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 2, true, geometric_fast_distance))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_FAST_DISTANCE);		
		return true;
	}		
	else if (funcName.compare(0, LENGTH_NAME_PREFIX.length(), LENGTH_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case length\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 1, true, geometric_length))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_LENGTH);		
		return true;
	}		
	else if (funcName.compare(0, FAST_LENGTH_NAME_PREFIX.length(), FAST_LENGTH_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case fast_length\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 1, true, geometric_fast_length))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_FAST_LENGTH);		
		return true;
	}
	else if (funcName.compare(0, CROSS_PRODUCT_NAME_PREFIX.length(), CROSS_PRODUCT_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case cross product\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 2, false, geometric_cross))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_CROSS);		
		return true;
	}		
	else if (funcName.compare(0, NORMALIZE_NAME_PREFIX.length(), NORMALIZE_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case normalize\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 1, false, geometric_normalize))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_NORMALIZE);		
		return true;
	}
	else if (funcName.compare(0, FAST_NORMALIZE_NAME_PREFIX.length(), FAST_NORMALIZE_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case fast_normalize\n");
		if (!geometricFunctionPrepArgs(callingInst, rootVals, 1, false, geometric_fast_normalize))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_GEO_FAST_NORMALIZE);		
		return true;
	}
	else if (funcName.compare(0, FRACT_NAME_PREFIX.length(), FRACT_NAME_PREFIX) == 0)
	{
		V_PRINT("\t\t\tPre-scanning found special-case fract function\n");
		if (!fractFunctionPrepArgs(callingInst, rootVals))
		{
			return false;
		}
		funcProperties->setProperty(callingInst, PR_SC_FRACT);		
		return true;
	}
	
	// Reaching here, function does not correspond to a special case. Leave.
	return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// For geometric functions (part of special funcs) - obtain and save root input arguments
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::geometricFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals, unsigned numArgs, bool isReducted, geometricListType funcList)
{
	std::string funcName = callingInst->getCalledFunction()->getName();
	
	// Try to find the actual function in the names list
	unsigned width;
	for (width = 0; width < GEOMETRIC_WIDTHS; width++)
	{
		if (funcName.compare(funcList[width][0]) == 0)
			break;
	}
	if (width == GEOMETRIC_WIDTHS) return false;  // false alarm: this function does not appear in our lists	
	V_PRINT("\t\t\tVector Width found: " << width+1 << "\n");
	
	const Type * expectedType;
	if (width > 0)
	{
		expectedType = VectorType::get(getFloatTy, width+1); // find the "proper" (un-casted) origins of the input arguments
	}
	else
	{
		// if vector width is 1, its not really a vector type...
		expectedType = getFloatTy;
	}

	// For non-reducted functions, the return value is the same type as the inputs
	const Type * expectedReturnType = (isReducted ? getFloatTy : expectedType);

	// Root the return value
	Value * rootRetVal = RootReturnValue(callingInst, expectedReturnType, callingInst);
	rootVals.push_back(rootRetVal);
	
	// Root the input arguments
	for (unsigned i = 0; i < numArgs; i++)
	{
		Value * inputArgument = callingInst->getOperand(i + 1);
		Value * properArgument = RootInputArgument(inputArgument, expectedType, callingInst);
		if (!properArgument) 
		{
			V_PRINT("\t\t\tPre-scanning failed to find root argument " << i << "\n");
			rootVals.clear(); // Empty whatever was pushed into rootVals
			return false; // failed to find coordinates source
		}
		// put the proper arguments in the rootVals vector
		rootVals.push_back(properArgument);
	}
	V_PRINT("\t\t\tPre-scanning special-case succeeded\n");
	return true;			
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// For select functions (part of special funcs) - obtain and save root input arguments
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::selectFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals)
{
	std::string funcName = callingInst->getCalledFunction()->getName();

	// Analyze function name, to extract types and vector width
	unsigned vectorWidth;
	unsigned integerWidth = getSelectType(funcName, &vectorWidth);
	if (integerWidth == 0)
	{
		return false; // function was not found. 
	}
	V_ASSERT(vectorWidth > 0 && (vectorWidth < 5 || vectorWidth == 8 || vectorWidth == 16)); // sanity - vector width

	// Check if function really exists - this assumes the "__select_" prefix is ONLY used for proper select functions in the runtime module!!
	Function * LibFunc = RUNTIME_MODULE->getFunction(funcName);
	if (!LibFunc)
	{
		return false; // Function doesnt exist
	}
	
	const Type * expectedXYType;
	const Type * singleXYElementType;
	const Type * expectedMaskType;
	const Type * singleMaskElementType;
	
	if (integerWidth == 100)
	{
		// width 100 is codename for Float type
		singleXYElementType = getFloatTy;
		singleMaskElementType = getInt32Ty;
	}
	else
	{
		singleMaskElementType = singleXYElementType = IntegerType::get(funcProperties->context(), integerWidth);
	}
	
	if (vectorWidth == 1)
	{
		expectedXYType = singleXYElementType;
		expectedMaskType = singleMaskElementType;
	}
	else
	{
		expectedXYType = VectorType::get(singleXYElementType, vectorWidth);
		expectedMaskType = VectorType::get(singleMaskElementType, vectorWidth);
	}
	
	
	// Obtain root values and return value. 
	unsigned currentArgumentIndex = 1; // index of "current" operand. Operand 0 is the called function itself 
	
	// First, obtain the root Return value
	Value * actualRetValue = callingInst;
	if (actualRetValue->getType() == getVoidTy)
	{
		// func returns void, meaning return value is pushed as first function argument.
		actualRetValue = callingInst->getOperand(1);
		currentArgumentIndex++;
	}
	Value * rootRetVal = RootReturnValue(actualRetValue, expectedXYType, callingInst);
	if (!rootRetVal) return false; // something went wrong, when rooting the return value
	
	// Second, obtain the input arguments
	Value * rootXArgument = RootInputArgument(callingInst->getOperand(currentArgumentIndex++), expectedXYType, callingInst);
	Value * rootYArgument = RootInputArgument(callingInst->getOperand(currentArgumentIndex++), expectedXYType, callingInst);
	Value * rootMaskArgument = RootInputArgument(callingInst->getOperand(currentArgumentIndex++), expectedMaskType, callingInst);
	if (!rootXArgument || !rootYArgument || !rootMaskArgument) return false; // something went wrong, when rooting the arguments
	
	// Fill the output vector
	rootVals.push_back(rootRetVal); // return value is the call itself
	rootVals.push_back(rootXArgument); // return value is the call itself
	rootVals.push_back(rootYArgument); // return value is the call itself
	rootVals.push_back(rootMaskArgument); // return value is the call itself
			 
	V_PRINT("\t\t\tPre-scanning select succeeded\n");
	return true;			
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// For fract functions (part of special funcs) - check if function is supported - meaning it appears
// in the SC list, and root its values. Later, we also check if it's pointer input is a scratch
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::fractFunctionPrepArgs(CallInst * callingInst, SmallVectorImpl<Value *> &rootVals)
{
	std::string funcName = callingInst->getCalledFunction()->getName();
	
	// Analyze function name, to extract vector width
	unsigned index;
	for (index = 0; index < SUPPORTED_WIDTHS; index++)
	{
		if (funcName.compare(fractFuncsList[index]) == 0)
			break;
	}
	if (index == SUPPORTED_WIDTHS)
	{
		V_PRINT("\t\tFract function does not match any supported functions!\n");
		return false;  // false alarm: this function does not appear in our lists	
	}
	unsigned index_to_width[SUPPORTED_WIDTHS] = {1, 2, 3, 4, 8, 16};
	unsigned width = index_to_width[index];
	
	// Check if function really exists
	Function * LibFunc = RUNTIME_MODULE->getFunction(funcName);
	if (!LibFunc)
	{
		V_UNEXPECTED("function not found in runtime module");
		return false; // Function doesnt exist
	}

	const Type * expectedType;
	if (width == 1)
	{
		expectedType = getFloatTy;
	}
	else
	{
		expectedType = VectorType::get(getFloatTy, width);
	}
	
	// Obtain the root Return value
	Value * actualRetValue = callingInst;
	Value * rootRetVal = RootReturnValue(actualRetValue, expectedType, callingInst);
	if (!rootRetVal) 
	{
		V_PRINT("\t\tFailed to root return value!\n");
		return false; // something went wrong, when rooting the return value
	}
	
	// Obtain the (first) input argument
	Value * rootArgument = RootInputArgument(callingInst->getOperand(1), expectedType, callingInst);
	if (!rootArgument) 
	{
		V_PRINT("\t\tFailed to root first input argument!\n");
		return false; // something went wrong, when rooting the argument value
	}
	
	// Fill the output vector
	rootVals.push_back(rootRetVal); 
	rootVals.push_back(rootArgument);
	
	V_PRINT("\t\t\tPre-scanning Fract succeeded\n");
	return true;			
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check for functions which work on work-group granularity, and may impact correctness in a vectorized kernel
/////////////////////////////////////////////////////////////////////////////////////////////////////
void ScanFunction::checkForWorkGroupFuncs(CallInst * callingInst)
{
	std::string calledFuncName = callingInst->getCalledFunction()->getName();

	if (calledFuncName == BARRIER_FUNC_NAME)
	{
		funcProperties->setFuncProperty(FUNC_CONTAINS_BARRIER);
	}
	else if (calledFuncName.substr(0, std::string(WG_FUNCS_NAME_PREFIX).size()) ==  WG_FUNCS_NAME_PREFIX)
	{
		funcProperties->setFuncProperty(FUNC_CONTAINS_WG_SYNC_OP);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if function in question must be treated as pseudo TID-dependent
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanFunction::isFunctionPseudoDependent(CallInst * CI)
{
	bool hasPointers = false;
	// Check if the function has any input pointers
	for (User::op_iterator oI = CI->op_begin(), oE = CI->op_end(); oI != oE; ++oI)
	{
		Value * val = *oI;
		if (isa<FunctionType>(val->getType())) continue; // skip the function pointer argument...
		if (isa<PointerType>(val->getType()))
		{
			hasPointers = true;
			break;  // no need to keep scanning
		}
	}		
	if (!hasPointers) return false; // function receives no pointers
	
	std::string funcName = CI->getCalledFunction()->getName();
	Function *LibFunc = RUNTIME_MODULE->getFunction(funcName);

	if (LibFunc == NULL) return true; // its not a builtin function, so we know nothing about it. 
	
	// Search whether the function name appears in the list of "safe" functions which receive pointer inputs
	unsigned safeFuncsIndex = 0;
	while (noSideEffectFuncs[safeFuncsIndex] != NULL)
	{
		std::string currFunc(noSideEffectFuncs[safeFuncsIndex++]);
		if (funcName.compare(0, currFunc.length(), currFunc) == 0) return false; // found a match! this function is safe
	}
	return true; // function not appearing in safe list.	
}



