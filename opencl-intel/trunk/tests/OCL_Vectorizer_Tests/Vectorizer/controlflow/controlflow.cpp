/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "controlflow.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// HandleControlFlow object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
HandleControlFlow::HandleControlFlow()
{
	V_PRINT("HandleControlFlow constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// HandleControlFlow object constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
HandleControlFlow::~HandleControlFlow()
{
	V_PRINT("HandleControlFlow destructor\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface function - Scan function for supported control flow structures
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool HandleControlFlow::analyzeControlFlow(CodeProperties * functionProp)
{
	funcProperties = functionProp;
	if (CURRENT_FUNCTION->size() == 1) return true; // there is no control flow - nothing to do...
	
	// Detect boundary checks with early exits
	scanForBoundaryChecks();
	
	// Flatten if-then-else blocks
	scanFunctionForIfThenElse();
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Scan function for if-then-else blocks and try to replace with "select" insts
/////////////////////////////////////////////////////////////////////////////////////////////////////
void HandleControlFlow::scanFunctionForIfThenElse()
{	
	Function::iterator iter = CURRENT_FUNCTION->begin();
	while (iter != CURRENT_FUNCTION->end()) 
	{
		BasicBlock * BB = &*iter;
		bool success = mergeIfThenElseBlock(BB, 0);
		if (!success) ++iter; // this block was no longer an if-then-else head. move on.
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Recursive function, drill down from given basic-block to find if-then-else block and replace
// with "select" inst if possible
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool HandleControlFlow::mergeIfThenElseBlock(BasicBlock * headBB, unsigned nestingLevel)
{
	if (nestingLevel > IF_NESTING_LEVEL_LIMIT)
		return false; // nesting went too deep into the rabbit hole. Stop it here.
	BasicBlock * thenBlock = NULL, * elseBlock = NULL;
	BasicBlock * trueBlock, * falseBlock;
	BasicBlock * joinBlock = NULL;
	Value * branchCondition;
	
	// First start with the discovery phase
	
	// Head block: must end with a conditional 2-way branch
	TerminatorInst * headTerminator = headBB->getTerminator();
	BranchInst * headBranch = dyn_cast<BranchInst>(headTerminator);
	if (!headBranch || !headBranch->isConditional())
		return false;
	
	// Obtain the two targets, and check if they are then/else (no matter which is which) or they are then/join
	branchCondition = headBranch->getCondition();
	trueBlock = headBranch->getSuccessor(0);
	falseBlock = headBranch->getSuccessor(1);
	
	// Check the then/else blocks. maybe one of them is the join block.
	if (trueBlock->getSinglePredecessor() == headBB)
	{
		thenBlock = trueBlock;
	}
	else
	{
		joinBlock = trueBlock; // block does not have a single predecessor. it is suspected to be a join block.
	}

	if (falseBlock->getSinglePredecessor() == headBB)
	{
		elseBlock = falseBlock;
	}
	else
	{
		// else block does not have a single predecessor. it is suspected to be a join block.
		if (joinBlock == NULL)
		{
			joinBlock = falseBlock;
		}
		else
		{
			return false; // both then and else blocks are suspected to be join block. This is unexpected structure. Reject structure
		}
	}

	// Check the structure of the blocks
	V_ASSERT (thenBlock || elseBlock);
	if (thenBlock)
	{
		// Check the block's terminator. If it is not a direct jump - there may be a nested if-then-else block
		TerminatorInst * thenTerminator = thenBlock->getTerminator();
		BranchInst * thenBranch = dyn_cast<BranchInst>(thenTerminator);
		if (!thenBranch || !thenBranch->isUnconditional())
		{
			// block does not end in unconditional branch. Start a new nested if-then-else block check
			bool isNestedIfElse = mergeIfThenElseBlock(thenBlock, nestingLevel+1);
			if (!isNestedIfElse) return false;
			// try again to detect the branch instruction
			thenTerminator = thenBlock->getTerminator();
			thenBranch = dyn_cast<BranchInst>(thenTerminator);
			if (!thenBranch || !thenBranch->isUnconditional()) return false;
		}
		BasicBlock * successor = thenBranch->getSuccessor(0);
		if (joinBlock == NULL)
		{
			joinBlock = successor;
		}
		else
		{
			if (joinBlock != successor)
				return false; // structure does not corrsepond to expected if/then/else
		}
		
		// Check that all instructions are legal for then/else block!
		bool isLegalInstructions = validateNoSideEffect(thenBlock);
		if (!isLegalInstructions) return false; // encountered unsupported instructions in the then block
	}

	if (elseBlock)
	{
		// Check the block's terminator. If it is not a direct jump - there may be a nested if-then-else block
		TerminatorInst * elseTerminator = elseBlock->getTerminator();
		BranchInst * elseBranch = dyn_cast<BranchInst>(elseTerminator);
		if (!elseBranch || !elseBranch->isUnconditional())
		{
			// block does not end in unconditional branch. Start a new nested if-then-else block check
			bool isNestedIfElse = mergeIfThenElseBlock(elseBlock, nestingLevel+1);
			if (!isNestedIfElse) return false;
			// try again to detect the branch instruction
			elseTerminator = elseBlock->getTerminator();
			elseBranch = dyn_cast<BranchInst>(elseTerminator);
			if (!elseBranch || !elseBranch->isUnconditional()) return false;
		}
		if (elseBranch->getSuccessor(0) != joinBlock) return false; // structure does not corrsepond to expected if/then/else
		
		// Check that all instructions are legal for then/else block!
		bool isLegalInstructions = validateNoSideEffect(elseBlock);
		if (!isLegalInstructions) return false; // encountered unsupported instructions in the else block
	}
	
	// check validity of join block
	V_ASSERT(joinBlock);
	pred_iterator PI = pred_begin(joinBlock), E = pred_end(joinBlock);
	unsigned preds = 0;
	while (PI != E)
	{
		preds++;
		++PI;
	}
	V_ASSERT(preds >= 2); // must have at least 2 predecessors (if/then or then/else)
	if (preds != 2) return false; // There are more predecessors then expected for if/then/else
	// check that join block actually has PHI instruction/s
	BasicBlock::iterator iterPHI = joinBlock->begin();
	if (!isa<PHINode>(iterPHI)) return false; // data is not merged with PHI node. Unexpected...
	
	
	V_PRINT("Detected a full if-then-else, starting in basic block " << headBB->getName() << "\n");	
	// Getting here - the discovery phase is over. The code is legal for if/then/else flattening.
	
	// move instructions from then/else blocks to head block
	if (thenBlock)
	{
		BasicBlock::iterator i = thenBlock->begin();
		while (!isa<TerminatorInst>(i)) {
			Instruction * inst = &*i;
			++i;
			inst->removeFromParent();
			inst->insertBefore(headTerminator);
		}
	}
	if (elseBlock)
	{
		BasicBlock::iterator i = elseBlock->begin();
		while (!isa<TerminatorInst>(i)) {
			Instruction * inst = &*i;
			++i;
			inst->removeFromParent();
			inst->insertBefore(headTerminator);
		}
	}
	// insert "select" instructions to replace PHI nodes
	if (trueBlock == joinBlock) trueBlock = headBB;
	if (falseBlock == joinBlock) falseBlock = headBB;
	while (isa<PHINode>(iterPHI))
	{
		PHINode * node = cast<PHINode>(iterPHI);
		++iterPHI; // already step to next PHI node (if there is...)		
		if (node->getNumIncomingValues() != 2) return false; // a PHI node of unexpected edges. Reject.
		Value *thenValue, *elseValue;
		BasicBlock * incomingBlock0 = node->getIncomingBlock(0);
		BasicBlock * incomingBlock1 = node->getIncomingBlock(1);
		if (incomingBlock0 == trueBlock)
		{
			if (incomingBlock1 != falseBlock) return false;
			thenValue = node->getIncomingValue(0);
			elseValue = node->getIncomingValue(1);
		}
		else
		{
			if (incomingBlock0 != falseBlock || incomingBlock1 != trueBlock) return false;
			thenValue = node->getIncomingValue(1);
			elseValue = node->getIncomingValue(0);
		}
		
		// Create new select instruction, and replace PHI node
		SelectInst * newSelect = SelectInst::Create(branchCondition, thenValue, elseValue, "replacePHI", headTerminator);
		newSelect->takeName(node);
		funcProperties->setProperty(node, PR_INST_IS_REMOVED);
		funcProperties->replaceAllInstProperties(node, newSelect);
		node->replaceAllUsesWith(newSelect);
		node->dropAllReferences();	
		V_ASSERT(node->use_empty());
		node->eraseFromParent();
	}
	
	// replace coditional branch with unconditional branch
	BranchInst::Create(joinBlock, headTerminator);
	funcProperties->setProperty(headTerminator, PR_INST_IS_REMOVED);
	headTerminator->dropAllReferences();
	headTerminator->eraseFromParent();
	
	// erase the then/else blocks
	if (thenBlock)
		DeleteDeadBlock(thenBlock);
	if (elseBlock)
		DeleteDeadBlock(elseBlock);
		
	// merge the join block into the head block
	return MergeBlockIntoPredecessor(joinBlock);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check in given basic block, if all instructions have no side-effects.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool HandleControlFlow::validateNoSideEffect(BasicBlock * BB)
{
	BasicBlock::iterator iter = BB->begin();
	BasicBlock::iterator iterEnd = BB->end();
	while (iter != iterEnd)
	{
		Instruction * inst = &*iter;
		
		// Only need to check instructions which we didnt explicitly mark as "safe"
		if (!funcProperties->getProperty(inst, PR_NO_SIDE_EFFECT))
		{
			switch (inst->getOpcode())
			{
				case Instruction::Br :
					// end of basic block
					break;
					
					// Should allow calling "safe" builtin functions.
				case Instruction::Call :
				{	
					bool retVal = validateCallHasNoSideEffect(cast<CallInst>(inst));
					if (!retVal) return false;
					break;
				}	
					// A list of all allowed instructions
				case Instruction::Add :
				case Instruction::Sub :
				case Instruction::Mul :
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
				case Instruction::BitCast :	
					
				case Instruction::GetElementPtr :
					
				case Instruction::ICmp :
				case Instruction::FCmp :
				case Instruction::Select :				
				case Instruction::ExtractElement :
				case Instruction::InsertElement :
				case Instruction::ShuffleVector :
				case Instruction::ExtractValue :
				case Instruction::InsertValue :
				case Instruction::Alloca :
					break;
					
				default :
					V_PRINT("\t\tInstruction: " << inst->getName() <<"... Considered as having side-effect...\n");
					return false;
			}
		}
		++iter;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if function in questions is known to have no side effect:
// Function must be a built-in function, which receives no pointer inputs
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool HandleControlFlow::validateCallHasNoSideEffect(CallInst * inst)
{
	bool hasPointer = false;
	std::string funcName = inst->getCalledFunction()->getName();
	Function *LibFunc = RUNTIME_MODULE->getFunction(funcName);
	if (LibFunc == NULL)
	{
		V_PRINT("\t\tFunction: " << funcName << " Not found in runtime lib. Considered as having side-effect...\n");
		return false;
	}
	else
	{
		V_PRINT("\t\tFunction: " << funcName << " was found in runtime lib\n");
	}
	
	// Function was found in runtime module - now check that it receives no pointers as inputs
	bool hasPointers = false;
	const FunctionType * funcType = LibFunc->getFunctionType();
	unsigned numInputParams = funcType->getNumParams();
	for (unsigned i = 0; i < numInputParams; ++i)
	{
		if (isa<PointerType>(inst->getOperand(i+1)->getType()))
		{
			hasPointers = true;
			break; // no need to keep searching for pointers...
		}
	}
	if (hasPointer)
	{
		// Search whether the function name appears in the list of "safe" functions which receive pointer inputs
		unsigned safeFuncsIndex = 0;
		while (noSideEffectFuncs[safeFuncsIndex] != NULL)
		{
			std::string currFunc(noSideEffectFuncs[safeFuncsIndex++]);
			if (funcName.compare(0, currFunc.length(), currFunc) == 0) return true; // found a match!
		}
		V_PRINT("\t\tFunction: " << funcName << " is passed a pointer, but does not appear in safe functions list. Considered as having side-effect...\n");
		return false; // function not appearing in safe list.
	}
	
	// Getting here, no function arguments are pointers. function has no side effect	
	return true;
}

