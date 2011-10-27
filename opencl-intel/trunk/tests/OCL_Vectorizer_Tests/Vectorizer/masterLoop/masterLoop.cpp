/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "masterLoop.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// LoopGen object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
LoopGen::LoopGen()
{
	V_PRINT("LoopGen constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// LoopGen object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
LoopGen::~LoopGen()
{
	V_PRINT("LoopGen destructor\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Main method for generating a "master loop" which iterates many work-items.
// Return false if operation failed (and therefore vectorization of this function should be aborted)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool LoopGen::generateLoop(CodeProperties * functionProp)
{
	funcProperties = functionProp;
	return createMajorCodeLoop();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate actual loop code, including TID count managment
// Loop structure:
//
//	head:
//		setTID = (get TID, +1, +2, +3)
//		br code
//	code:
//		actualTID = PHI (setTID, entry), (addTID, head)
//		...
//		... (Actual kernel code)
//		...
//		addTID = actualTID + (4,5,6,7)
//		cmp addTID.x to MAX_LOOP_VAL
//		branch code or end
//	end:
//		Return
//
// 
// Additionally, move all the Alloca insts from the kernel's head block, to the loop head block
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool LoopGen::createMajorCodeLoop()
{
	bool retVal;
	
	// hold the kernel's first basicblock
	BasicBlock * kernelHead = &(CURRENT_FUNCTION->getEntryBlock());

	// Mark all the instructions which should move 
	retVal = markToMoveInstructions(kernelHead);
	if (!retVal) 
	{
		V_PRINT("\tLoop creation failed (during instruction marking)\n");
		return false;
	}
	
	// Create loop head and tail blocks, and place in function
	BasicBlock * loopHead = BasicBlock::Create(funcProperties->context(), "entry", CURRENT_FUNCTION, kernelHead);
	BasicBlock * loopTail = BasicBlock::Create(funcProperties->context(), "loopTail", CURRENT_FUNCTION);
	
	// place ret instruction in end block
	Instruction * newRetInst = ReturnInst::Create(funcProperties->context(), NULL, loopTail);
	
	// put direct branch instruction in end of head block
	BranchInst::Create(kernelHead, loopHead);
	
	// Maintain list of instructions who are TID setting pivots: they are moved to head block, but 
	// all their uses are in the code blocks. They all must be incremented per each major master-loop
	SmallVector<Instruction*, 8> tidGenPivots; 	
	
	retVal = moveMarkedInstsOutOfLoop(loopHead, loopTail, tidGenPivots);
	if (!retVal) 
	{
		V_PRINT("\tLoop creation failed (during instruction moving)\n");
		return false;
	}
	
	// create PHI nodes at the start of the code block, incrementors at the end, and replace all uses of the TID pivots
	retVal = createPivotPHINodesAndIncrementors(loopHead, kernelHead, loopTail, tidGenPivots);
	
	// update the pointer to the ret instruction
	funcProperties->setRetInst(newRetInst);
	
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Go linearly over all the instructions in the kernel. Mark for moving to the loop
// head instructions that follow one of these rules
// 1) Instruction is Alloca, and is located in the kernel's entry block
// 2) Instruction is not TID-dependent, and all its predecessors are also marked to move (except for terminators)
// 3) Instruction is TID setter. If not all predecessors are marked to move - must abort the loop generation
// 4) Instruction is TID-dependent with sequential or equal-distance values, and all predecessors are marked to move
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool LoopGen::markToMoveInstructions(BasicBlock * kernelHead)
{
	// Iterate linearly over all instructions in the kernel
	inst_iterator sI = inst_begin(CURRENT_FUNCTION);
	inst_iterator sE = inst_end(CURRENT_FUNCTION);
	while (sI != sE)
	{
		Instruction * currInst = &*sI;
		sI++; // Move iterator to next instruction

		if (isa<TerminatorInst>(currInst) || isa<PHINode>(currInst)) continue; // No handling for terminators and PHI nodes
		BasicBlock * currBlock = currInst->getParent();
		
		if ((isa<AllocaInst>(currInst) && currBlock == kernelHead &&
			 !isa<Instruction>(cast<AllocaInst>(currInst)->getArraySize())) ||					// Inst is alloca of constant size in entry block
			funcProperties->getProperty(currInst, PR_OBTAIN_CL_INDEX) ||						// or inst is TID setter
			funcProperties->getProperty(currInst, PR_TID_VALS_CONSECUTIVE) ||					// or inst has consecutive values
			(	funcProperties->getProperty(currInst, PR_TID_VALS_EQUAL_DIST) && 
				isa<VectorType>(currInst->getType()) &&
				cast<VectorType>(currInst->getType())->getNumElements() > 1) ||					// or inst has equal-distance values (vector of 2 or more elements)
			(!funcProperties->getProperty(currInst, PR_TID_DEPEND) && currBlock == kernelHead))	// or inst does not depend on TID (in entry block)
		{
			// Check if all predecessors are marked for moving
			bool isLegalToMove = true;
			for (User::op_iterator i = currInst->op_begin(), e = currInst->op_end(); i != e; ++i) 
			{
				Value *v = *i;
				if (isa<Instruction>(v) && !funcProperties->getProperty(v, PR_MOVE_TO_LOOP_HEAD))
				{
					isLegalToMove = false;
					V_PRINT("\tDon't mark (partial predecessors mark).");		
					break; // failed. no need to check the remaining arguments...
				}
			}
			
			// For TID setters, if isLegalToMove is FALSE - we must abort the loop creation!
			if (!isLegalToMove && funcProperties->getProperty(currInst, PR_OBTAIN_CL_INDEX)) return false;
			
			if (isLegalToMove)
			{
				funcProperties->setProperty(currInst, PR_MOVE_TO_LOOP_HEAD);	
				V_PRINT("\tMark for Moving.");		
			}
		}
		else if (isa<CallInst>(currInst) && funcProperties->getProperty(currInst, PR_SC_READ_SAMPLER_F_2D))
		{
			// Check if instruction is special-case read-sampler. Some of these can be transferred to the outer loop and replaced by stream samplers
			// To be legal for moving, all inputs must be marked for moving
			bool isLegalToMove = true;
			for (User::op_iterator i = currInst->op_begin(), e = currInst->op_end(); i != e; ++i) 
			{
				Value *v = *i;
				if (isa<Instruction>(v) && !funcProperties->getProperty(v, PR_MOVE_TO_LOOP_HEAD))
				{
					isLegalToMove = false;
					V_PRINT("\tDon't mark (partial predecessors mark).");		
					break; // failed. no need to check the remaining arguments...
				}
			}
			if (isLegalToMove)
			{
				// Moving a read-sampler: Change it's property signature from "read sampler" to "stream read sampler"
				funcProperties->clearProperty(currInst, PR_SC_READ_SAMPLER_F_2D);	
				funcProperties->setProperty(currInst, PR_SC_STREAM_READ_SAMPLER);
				funcProperties->setProperty(currInst, PR_MOVE_TO_LOOP_HEAD);
				V_PRINT("\t*STREAM-READ* Moving.");				
			}
		}
		else if (isa<CallInst>(currInst) && funcProperties->getProperty(currInst, PR_SC_WRITEF_SAMPLER))
		{
			// Check if instruction is special-case write-sampler. Some of these can be transferred to the end
			// of the loop and replaced by stream samplers
			// To be legal for moving, it must not have been moved to loop head, and the X argument must be a pivot..
			// as this check cannot be done now, we tentatively mark for moving to loop tail. If we find later
			// that the pivot rule is not enforced - we back away from moving the inst
			funcProperties->clearProperty(currInst, PR_SC_WRITEF_SAMPLER);	
			funcProperties->setProperty(currInst, PR_SC_STREAM_WRITE_SAMPLER);
			funcProperties->setProperty(currInst, PR_MOVE_TO_LOOP_TAIL);
			V_PRINT("\t*STREAM-WRITE* Tentatively Moving to tail");							
		}		
		else
		{
			V_PRINT("\tDon't mark.");
		}
		V_PRINT(" Consq?" << funcProperties->getProperty(currInst, PR_TID_VALS_CONSECUTIVE));
		V_PRINT(" EqualDist?" << funcProperties->getProperty(currInst, PR_TID_VALS_EQUAL_DIST));
		V_PRINT(" " << *currInst << "\n");		
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Go linearly over all the instructions, and move all the marked ones to the loop head or tail.
// Any TID-dependent moved instruction (to head) which has decendents which are not marked - is registered as a pivot
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool LoopGen::moveMarkedInstsOutOfLoop(BasicBlock * loopHead, BasicBlock * loopTail, SmallVectorImpl<Instruction*> &pivotsList)
{
	Instruction * headMovePosition = loopHead->getTerminator();
	Instruction * tailMovePosition = loopTail->getTerminator();
	V_ASSERT(headMovePosition != NULL);

	// Iterate linearly over all instructions in the kernel
	inst_iterator sI = inst_begin(CURRENT_FUNCTION);
	inst_iterator sE = inst_end(CURRENT_FUNCTION);
	while (sI != sE)
	{
		Instruction * currInst = &*sI;
		sI++; // Move iterator to next instruction
		
		if (funcProperties->getProperty(currInst, PR_MOVE_TO_LOOP_HEAD))
		{
			currInst->moveBefore(headMovePosition);
			// For TID-dependent insts, check if all decendents are marked for move. Otherwise this is a pivot inst
			if (funcProperties->getProperty(currInst, PR_TID_DEPEND) && 
				!isa<AllocaInst>(currInst) &&
				!funcProperties->getProperty(currInst, PR_SC_STREAM_READ_SAMPLER))
			{
				for (Value::use_iterator uI = currInst->use_begin(), uE = currInst->use_end(); uI != uE; ++uI)
				{
					Instruction *decendInst = dyn_cast<Instruction>(*uI);
					if (decendInst && !funcProperties->getProperty(decendInst, PR_MOVE_TO_LOOP_HEAD)) 
					{
						V_ASSERT(!isa<BitCastInst>(currInst)); // Must not have a bitCast pivot! incrementing this value has no meaning! 
						pivotsList.push_back(currInst);
						funcProperties->setProperty(currInst, PR_LOOP_PIVOT);
						break;
					}		
				}
			}
		}
		else if (funcProperties->getProperty(currInst, PR_MOVE_TO_LOOP_TAIL))
		{
			// Need to check that the instruction's X-coordinate is a pivot. If not - back out from moving this inst
			if (!funcProperties->getProperty(currInst->getOperand(2), PR_LOOP_PIVOT))
			{
				funcProperties->clearProperty(currInst, PR_SC_STREAM_WRITE_SAMPLER);	
				funcProperties->clearProperty(currInst, PR_MOVE_TO_LOOP_TAIL);	
				funcProperties->setProperty(currInst, PR_SC_WRITEF_SAMPLER);
				V_PRINT("\t*STREAM-WRITE* ABORTED Moving to tail\n");
			}
			else
			{
				currInst->moveBefore(tailMovePosition);
				V_PRINT("\t*STREAM-WRITE* Moving to tail\n");
			}
		}
	}	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Create PHI nodes which will be used instead of the TID pivots
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool LoopGen::createPivotPHINodesAndIncrementors(BasicBlock * loopHead, BasicBlock * kernelHead, BasicBlock * loopTail, SmallVectorImpl<Instruction*> &pivotsList)
{
	Instruction * PhiInsertPoint = kernelHead->getFirstNonPHI();
	Instruction * incrementInsertPoint = funcProperties->getRetInst();
	unsigned num_pivots = pivotsList.size();

	// Per pivot...
	for (unsigned i = 0; i < num_pivots; i++)
	{		
		// Create and place PHI node in start of loop code	
		Instruction * currPivot = pivotsList[i];

		// First check if the Pivot is really needed. After moving stuff to the head and tail of the loop, maybe somethinh is no longer useful?
		bool pivotNeeded = false;
		for (Value::use_iterator uI = currPivot->use_begin(), uE = currPivot->use_end(); uI != uE; ++uI)
		{
			Instruction *decendInst = dyn_cast<Instruction>(*uI);
			if (decendInst && decendInst->getParent() != loopHead && decendInst->getParent() != loopTail)
			{
				// there is a user which is not in the loop head or tail
				pivotNeeded = true;
				break; // no need to continue looking...
			}
		}			
		if (!pivotNeeded) 
		{
			V_PRINT("\t\tPivot " << *currPivot << " is no longer needed (no users in loop). Removing.\n");
			funcProperties->clearProperty(currPivot, PR_LOOP_PIVOT);
			continue;
		}

		PHINode * newNode = PHINode::Create(currPivot->getType(), currPivot->getName(), PhiInsertPoint);
		funcProperties->duplicateProperties(newNode, currPivot);
		newNode->reserveOperandSpace(2); // Loop PHI Nodes have 2 incoming basic blocks

		// replace all users of the original value, with using the PHI node
		replaceUsersWithPHI(currPivot, newNode);
		
		// Add incrememntor at end of loop code
		Instruction * incrementor = addValueIncrementor(currPivot, newNode, incrementInsertPoint, loopHead);
		if (!incrementor) return false;

		V_PRINT("\t\tIncrementor: " << *incrementor << "\n");
		
		// Fill PHI node with incoming values
		newNode->addIncoming(currPivot, currPivot->getParent()); // first incoming is from the loop header
		newNode->addIncoming(incrementor, incrementor->getParent()); // second is an increment of that value		
	}
	
	// Create compare-and-branch at end of code
	createCompareAndBranch(loopHead, kernelHead, loopTail);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Create (and place) incrementor for values of several types
/////////////////////////////////////////////////////////////////////////////////////////////////////
Instruction * LoopGen::addValueIncrementor(Value * origVal, Instruction * PhiNode, Instruction * insertPoint, BasicBlock * loopHead)
{
	const Type * adderType = origVal->getType();
	Instruction::BinaryOps addOperation = Instruction::Add;
	Instruction * addInst;
	Value * incrementValue = NULL;
	
	// The value to increment by is ARCH_VECTOR_WIDTH for the popular case, but different value for the EUQAL_DIST instructions
	if (!funcProperties->getProperty(origVal, PR_TID_VALS_EQUAL_DIST))
	{
		if (isa<VectorType>(adderType))
		{
			const Type * baseType = cast<VectorType>(adderType)->getElementType();
			if (baseType->isIntegerTy())
			{
				// Type is vector of integers
				Constant * constAdder = ConstantInt::get(baseType, ARCH_VECTOR_WIDTH);
				incrementValue = ConstantVector::get(std::vector<Constant*>(cast<VectorType>(adderType)->getNumElements(), constAdder));
				addOperation = Instruction::Add;
			}
			else if (baseType->isFloatingPointTy())
			{
				// Type is vector of FPs
				Constant * constAdder = ConstantFP::get(baseType, (double)(ARCH_VECTOR_WIDTH));
				incrementValue = ConstantVector::get(std::vector<Constant*>(cast<VectorType>(adderType)->getNumElements(), constAdder));
				addOperation = Instruction::FAdd;
			}
			else
			{
				// no support for other types!
				V_UNEXPECTED("unsupported vector type");
				return NULL;
			}
		}
		else if (adderType->isIntegerTy())
		{
			// Type is integer
			incrementValue = ConstantInt::get(adderType, ARCH_VECTOR_WIDTH);
			addOperation = Instruction::Add;
		}
		else if (adderType->isFloatingPointTy())
		{
			// Type is FP
			incrementValue = ConstantFP::get(adderType, (double)(ARCH_VECTOR_WIDTH));
			addOperation = Instruction::FAdd;
		}
		else
		{
			V_PRINT("Error! Trying to increment this value:" << *origVal << "\n");
			// no support for other types!
			V_UNEXPECTED("unsupported type");
			return NULL;
		}
		V_ASSERT(incrementValue);
	}
	else
	{
		// Instruction is equal-dist, so simple incrememntor won't work. Need to increment by the dist value.
		V_ASSERT(isa<VectorType>(adderType));
		unsigned numElements = cast<VectorType>(adderType)->getNumElements();
		const Type * baseType = cast<VectorType>(adderType)->getElementType();
		
		// Dist is a calculated by subtracting value.x from value.y, and broadcasted into a vector
		UndefValue * undefVect = UndefValue::get(adderType);
		Constant * const32Vector_0 = ConstantVector::get(VectorType::get(getInt32Ty, numElements), 
														 std::vector<Constant*>(numElements, ConstantInt::get(getInt32Ty, 0)));
		Constant * const32Vector_1 = ConstantVector::get(VectorType::get(getInt32Ty, numElements), 
														 std::vector<Constant*>(numElements, ConstantInt::get(getInt32Ty, 1)));
		// Broadcast value.y
		Value * BroadcastY = new ShuffleVectorInst(origVal, undefVect, const32Vector_1 , "broadcast.val1", loopHead->getTerminator());
		// Broadcast value.x
		Value * BroadcastX = new ShuffleVectorInst(origVal, undefVect, const32Vector_0 , "broadcast.val0", loopHead->getTerminator());		
		// Subtract BroadcastX from BroadcastY - to get the dist vector
		Instruction::BinaryOps subOperation = Instruction::Sub;
		Instruction::BinaryOps mulOperation = Instruction::Mul;
		Constant * vectorOfOverallWidth;
		if (baseType->isFloatingPointTy())
		{
			addOperation = Instruction::FAdd;
			subOperation = Instruction::FSub;
			mulOperation = Instruction::FMul;
			vectorOfOverallWidth = ConstantVector::get(VectorType::get(baseType, numElements), 
													   std::vector<Constant*>(numElements, ConstantFP::get(baseType, (double)ARCH_VECTOR_WIDTH)));					
		}
		else
		{			
			vectorOfOverallWidth = ConstantVector::get(VectorType::get(baseType, numElements), 
													   std::vector<Constant*>(numElements, ConstantInt::get(baseType, ARCH_VECTOR_WIDTH)));		
		}
		
		Value * subVals = BinaryOperator::Create(subOperation ,BroadcastY, BroadcastX, "get.delta.prep", loopHead->getTerminator());
		incrementValue = BinaryOperator::Create(mulOperation ,subVals, vectorOfOverallWidth, "get.delta", loopHead->getTerminator());
	}		
	
	
	// Create incrementor
	addInst = BinaryOperator::Create(addOperation ,PhiNode, incrementValue, "main.loop.add", insertPoint);
	
	return addInst;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace all the users of the pivot value, with the incremented value
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LoopGen::replaceUsersWithPHI(Instruction * pivotInst, Instruction * phiNode)
{
	V_PRINT("\tpivotInst to replace its users: " << *pivotInst);
	V_PRINT(" is it consecutive:" <<funcProperties->getProperty(pivotInst, PR_TID_VALS_CONSECUTIVE));
	V_PRINT(" is it equal-dist:" <<funcProperties->getProperty(pivotInst, PR_TID_VALS_EQUAL_DIST) << "\n");
	
	SmallVector<Instruction*, 16> instsList;
	
	// Iterate over users list. Replace every user which is not in the loop header by itself
	Value::use_iterator iter = pivotInst->use_begin();
	Value::use_iterator endIter = pivotInst->use_end();
	while (iter != endIter)
	{
		Instruction *inst = dyn_cast<Instruction>(*iter);
		++iter; // Increment iterator BEFORE replacing the uses - because this inst will no longer be a user! 
		if (inst != NULL && !funcProperties->getProperty(inst, PR_MOVE_TO_LOOP_HEAD))
		{
			instsList.push_back(inst);
		}
	}
	
	for (unsigned i = 0; i < instsList.size(); ++i)
	{
		instsList[i]->replaceUsesOfWith(pivotInst, phiNode);
	}	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Create (and place) a compare-and-branch of loop iterations 
/////////////////////////////////////////////////////////////////////////////////////////////////////
void LoopGen::createCompareAndBranch(BasicBlock * loopHead, BasicBlock * kernelHead, BasicBlock * loopTail)
{
	// create a new "simple" integer counter
	V_PRINT("\tCreate compare-and-branch\n");
	Instruction * headInsertPoint = loopHead->getTerminator();
	Instruction * endOfLoop = funcProperties->getRetInst(); // Get the end of the loop code
	Value * constZero = ConstantInt::get(getInt32Ty, 0);
	Value * constOne = ConstantInt::get(getInt32Ty, 1);
		
	// Create PHI node for counter
	PHINode * newNode = PHINode::Create(getInt32Ty, "counter", kernelHead->getFirstNonPHI());
	newNode->reserveOperandSpace(2); // Loop PHI Nodes have 2 incoming basic blocks
	funcProperties->setIterCountInst(newNode);
	
	// Add incrememntor at end of loop code
	Instruction * pivotIncrementor = BinaryOperator::Create(Instruction::Add ,newNode, constOne, "main.loop.add", endOfLoop);
	
	// Fill PHI node with incoming values
	newNode->addIncoming(constZero, loopHead); // first incoming is zero value (if coming from loop head)
	newNode->addIncoming(pivotIncrementor, pivotIncrementor->getParent()); // second is an increment of that value
	
	// Create "get_local_size()" value
	Function *libLocalSizeFunc = RUNTIME_MODULE->getFunction(GET_LOCAL_SIZE);
	Value * localSizeFunc = CURRENT_MODULE->getOrInsertFunction(GET_LOCAL_SIZE, libLocalSizeFunc->getFunctionType(), libLocalSizeFunc->getAttributes());
	Instruction * loopSize = CallInst::Create(localSizeFunc, constZero, "local.size", loopHead->getFirstNonPHI());
	V_ASSERT(localSizeFunc != NULL);
	funcProperties->setLoopSizeVal(loopSize);
	
	// Create maximum value (get_local_size()/ARCH_VECTOR_WIDTH)
	if (loopSize->getType() != getInt32Ty)
		loopSize = CastInst::CreateIntegerCast(loopSize, getInt32Ty, false, "cast.size", headInsertPoint);
	Value * constWidth = ConstantInt::get(getInt32Ty, LOG_(ARCH_VECTOR_WIDTH));
	Instruction * maxVal = BinaryOperator::Create(Instruction::AShr ,loopSize, constWidth, "max.val", headInsertPoint);		

	// compare "if-equal to max value..."
	Instruction * compare = new ICmpInst(endOfLoop, CmpInst::ICMP_UGE, pivotIncrementor, maxVal, "cmp.to.max");
	
	// remove the "ret" from the loop code
	BasicBlock * blockOfReturn = endOfLoop->getParent();
	funcProperties->setProperty(endOfLoop, PR_INST_IS_REMOVED);
	endOfLoop->eraseFromParent();
	
	// add new branch at end of code (if true - to end of kernel, if false - to start of loop)
	BranchInst::Create(loopTail, kernelHead, compare, blockOfReturn);
}





/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface function for early-exits: generate a loop around the scalar code of the kernel
// and place at the bottom of the vectorized kernel
/////////////////////////////////////////////////////////////////////////////////////////////////////
BasicBlock * LoopGen::generateScalarLoop(Function * scalarFunc, CodeProperties * functionProp)
{
	// The vectorizer may or may not have created a master loop. funcProperties might not be set yet.
	funcProperties = functionProp;
	V_PRINT("\tStart Generating scalar-loop\n");
	
	// First, create a basic block at the end of the vectorized kernel, to be the loop's head
	BasicBlock * loopHead = BasicBlock::Create(funcProperties->context(), "scalarLoop", funcProperties->currentFunction);
	
	// Clone the scalar function 
	BasicBlock * scalarStartBlock = NULL;
	{
		// Create denseMap of function arguments
		DenseMap<const Value *, Value *> valueMap;
		Function::const_arg_iterator scalarIter = scalarFunc->arg_begin();
		Function::const_arg_iterator scalarIterEnd = scalarFunc->arg_end();
		Function::arg_iterator vectorIter = funcProperties->currentFunction->arg_begin();
		Function::arg_iterator vectorIterEnd = funcProperties->currentFunction->arg_end();
		for (; scalarIter != scalarIterEnd; ++scalarIter, ++vectorIter)
		{
			valueMap[scalarIter] = vectorIter;
		}
		V_ASSERT(vectorIter == vectorIterEnd); // same argument list for both functions!
		
		// create a list for return values
		SmallVector<ReturnInst*, 2> returns;
		
		// Do actual cloning work
		CloneFunctionInto(funcProperties->currentFunction, scalarFunc, valueMap, returns, "Scalar_func");
		
		// Get hold of the entry to the scalar section in the vectorized function...
		scalarStartBlock = dyn_cast<BasicBlock>(valueMap[scalarFunc->begin()]);
	}
	V_ASSERT(scalarStartBlock);

	V_PRINT("\t\t successfully cloned scalar code\n");
	
	// How code will look like:
	// The new entry block (of the scalar code) will have a PHI for getting its local_id. Can be 0 or another
	// value (assumed less than local_size). That will be used as initial incrementor.
	// Then, get_global_id is taken, and added by current incrementor value
	
	// Create loop tail block (where counters are incremented)
	BasicBlock * loopTail = BasicBlock::Create(funcProperties->context(), "scalarLoopTail", funcProperties->currentFunction);

	// Create end-block (where function exits)
	BasicBlock * loopExit = BasicBlock::Create(funcProperties->context(), "scalarLoopExit", funcProperties->currentFunction);
		
	// place ret instruction in exit block
	ReturnInst::Create(funcProperties->context(), NULL, loopExit);
	// place temporary ret instruction in head block
	Instruction * tmpRet = ReturnInst::Create(funcProperties->context(), NULL, loopHead);

	// Obtain a counter (thru PHI node) and a global_id which is incremented by initial counter
	// No need to generate a local_id, because according to interface, that will always return 0 (like our counter)
	// This must be the ONLY phi node in this basic block: early exits will modify this PHI node!
	Function * getLidFunc = RUNTIME_MODULE->getFunction(GET_LID_NAME);
	const Type * countersType = getLidFunc->getReturnType();
	PHINode * inputCounter = PHINode::Create(countersType, "local_id_ref", loopHead->getFirstNonPHI());

	// Create casted "get_local_size()" value (at start of loopHead block)
	Value *loopSize = funcProperties->getLoopSizeVal();
	Value * castedLoopSize = loopSize;

	if (loopSize->getType() != countersType)
	{
		castedLoopSize = CastInst::CreateIntegerCast(loopSize, countersType, false, "cast", loopHead->getFirstNonPHI());
	}

	// At end of loopHead block, compare LocalID to zero and exit if needed
	Instruction * compareFirst = new ICmpInst(*loopHead, 
										 CmpInst::ICMP_EQ, 
										 inputCounter, 
										 castedLoopSize, 
										 "cmp.to.max");

	// Branch to exit block or scalar loop start
	BranchInst::Create(loopExit, scalarStartBlock, compareFirst, loopHead);
	tmpRet->eraseFromParent();
	
	Function * getGidFunc = CURRENT_MODULE->getFunction(GET_GID_NAME);
	PHINode * GidValPhi = NULL;
	if (getGidFunc)
	{
		// generate call
		CallInst * getGidCall = CallInst::Create(getGidFunc, 
												 ConstantInt::get(getGidFunc->getFunctionType()->getParamType(0), 0), 
												 "global_id_ref", 
												 loopHead->getTerminator());
		Instruction * castedInputCounter = CastInst::CreateIntegerCast(inputCounter, getGidCall->getType(), false, "global_id_ref.cast", loopHead->getTerminator());
		BinaryOperator * incrementedGid = BinaryOperator::Create(Instruction::Add, getGidCall, castedInputCounter, "base_gid", loopHead->getTerminator());
		
		// Create PHI node for GID counter
		GidValPhi = PHINode::Create(incrementedGid->getType(), "GlobalID", scalarStartBlock->getFirstNonPHI());
		GidValPhi->reserveOperandSpace(2); // Loop PHI Nodes have 2 incoming basic blocks
		
		// Add incrememntor at end of loop code
		Constant * constOne =  ConstantInt::get(incrementedGid->getType(), 1);
		Instruction * GidIncrementor = BinaryOperator::Create(Instruction::Add ,GidValPhi, constOne, "gid_increment", loopTail);
		
		// Fill PHI node with incoming values
		GidValPhi->addIncoming(incrementedGid, loopHead); // first incoming is GID
		GidValPhi->addIncoming(GidIncrementor, loopTail); // second is an increment of that value
	}
	
	// Create PHI node for iterations counter
	PHINode * iteratorPhi = PHINode::Create(countersType, "LocalID", scalarStartBlock->getFirstNonPHI());
	iteratorPhi->reserveOperandSpace(2); // Loop PHI Nodes have 2 incoming basic blocks
	
	// Add incrememntor at end of loop code
	Instruction * incrementor = BinaryOperator::Create(Instruction::Add ,iteratorPhi, ConstantInt::get(countersType, 1), "lid_increment", loopTail);
	
	// Fill PHI node with incoming values
	iteratorPhi->addIncoming(inputCounter, loopHead); // first incoming is input from main kernel code (if coming from loop head)
	iteratorPhi->addIncoming(incrementor, loopTail); // second is an increment of that value
	
	// compare "if-equal to max value..."
	Instruction * compare = new ICmpInst(*loopTail, 
										 CmpInst::ICMP_EQ, 
										 incrementor, 
										 castedLoopSize, 
										 "cmp.to.max");
	
	// add new branch at end of code (if true - to end of kernel, if false - to start of loop)
	BranchInst::Create(loopExit, scalarStartBlock, compare, loopTail);
	
	V_PRINT("\t\t successfully generated iterators\n");
		
	// Scan scalar code, and replace all uses of TID with PHI nodes. Also replace RET with jump to loop Tail	
	Function::iterator iterBB = Function::iterator(scalarStartBlock);
	while (cast<BasicBlock>(iterBB) != loopTail)
	{
		BasicBlock::iterator iterInst = iterBB->begin();
		BasicBlock::iterator iterInstEnd = iterBB->end();
		while (iterInst != iterInstEnd)
		{
			Instruction * currInst = cast<Instruction>(iterInst);		
			++iterInst; // Already move to next inst iterator, as this inst might move..
			
			if (CallInst * CI = dyn_cast<CallInst>(currInst))
			{
				std::string funcName = CI->getCalledFunction()->getName();
				if ((funcName == GET_GID_NAME) && isa<ConstantInt>(CI->getOperand(1)) && cast<ConstantInt>(CI->getOperand(1))->equalsInt(0))
				{
					// Found GID access. Divert all users to our incremented GID
					V_ASSERT(GidValPhi); // should have created this phi...
					CI->replaceAllUsesWith(GidValPhi);
				}
				else if ((funcName == GET_LID_NAME) && isa<ConstantInt>(CI->getOperand(1)) && cast<ConstantInt>(CI->getOperand(1))->equalsInt(0))
				{
					// Found LID access. Divert all users to our incrementor
					CI->replaceAllUsesWith(iteratorPhi);
				}
			}
			else if (isa<ReturnInst>(currInst))
			{
				// Remove the ret instruction and replace with branch to too tail
				BasicBlock * blockOfReturn = currInst->getParent();
				funcProperties->setProperty(currInst, PR_INST_IS_REMOVED);
				currInst->eraseFromParent();
				// add direct branch to loop tail
				BranchInst::Create(loopTail, blockOfReturn);
				
			}
		}
		++iterBB; // move to next basic block
	}
	V_PRINT("\t\t completed connecting code to iterators\n");
	
	return loopHead;
}

