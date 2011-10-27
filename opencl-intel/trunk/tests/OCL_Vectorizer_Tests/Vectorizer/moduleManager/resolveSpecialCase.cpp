/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "ModuleManager.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Go over list of fake special-case functions. Find all instances, and replace with actual function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveSpecialCaseFunctions(Function *F, CodeProperties * funcProperties)
{
	bool isResolvingPassed = true;
	// Iterate over special-case functions list
	Function * fakeFunction = funcProperties->firstFakeFunctionFromList();
	while (fakeFunction && isResolvingPassed)
	{
		// Find which function this really is, and dispatch its specific resolver
		isResolvingPassed = dispatchSpecialCaseFunc(fakeFunction, F, funcProperties);
		
		if (isResolvingPassed)
		{
			V_ASSERT(fakeFunction->use_empty());
			fakeFunction->eraseFromParent(); // Remove function declaration from module
			funcProperties->clearFakeFunctionFromList(); // clear from the list, so it won't be erased twice if the vectorization is rejected later
		}

		// Go to next special-case function
		fakeFunction = funcProperties->nextFakeFunctionFromList(); // next function from list...
	}
	return isResolvingPassed;
}	


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Go over list of fake special-case functions, and remove them. Used when something
// bad happended, and vectorization failed. The "vectorization-attempted" function is assumed 
// to have been erased already, therefore there should be no uses for any of the fake functions!
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::eraseSpecialCaseFakeFunctions(CodeProperties * funcProperties)
{
	// Remove all the fake function declarations
	Function * fakeFunction = funcProperties->firstFakeFunctionFromList();
	while (fakeFunction)
	{
		if (fakeFunction != (Function *)-1)
		{
			// Remove function declaration from module
			V_ASSERT(fakeFunction->use_empty());
			fakeFunction->eraseFromParent();
		}
		
		// Go to next special-case function
		fakeFunction = funcProperties->nextFakeFunctionFromList(); // next function from list...
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Dispatch the correct resolving handler, according to function which is called
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::dispatchSpecialCaseFunc(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("Dispatching resolve of function: " << specialCaseFunc->getName() << " which has " << specialCaseFunc->getNumUses() << " uses\n");
	while (specialCaseFunc->getNumUses() > 0)
	{
		bool retval;
		// Pick one of the function's users and dispatch the correct handler
		Instruction * callerInst = dyn_cast<Instruction>(*(specialCaseFunc->use_begin()));
		V_ASSERT(callerInst);		
		instProperty specialCaseType = funcProperties->getPropertyGroup(callerInst, PR_ALL_SPECIAL_CASE_FUNCS);
		switch (specialCaseType) {
			case PR_SC_READ_SAMPLER_F_2D:
				retval = resolveReadSamplerCalls(specialCaseFunc, F, funcProperties, true);
				break;
			case PR_SC_READ_SAMPLER_F_3D:
				retval = resolveReadSamplerCalls(specialCaseFunc, F, funcProperties, false);
				break;
			case PR_SC_STREAM_READ_SAMPLER:
				retval = resolveStreamReadSamplerCalls(specialCaseFunc, F, funcProperties);
				break;
			case PR_SC_WRITEF_SAMPLER:
				retval = resolveWriteSamplerCalls(specialCaseFunc, F, funcProperties);
				break;
			case PR_SC_STREAM_WRITE_SAMPLER:
				retval = resolveStreamWriteSamplerCalls(specialCaseFunc, F, funcProperties);
				break;
			case PR_SC_CI_GAMMA:
				retval = resolveCIGammaCalls(specialCaseFunc, F, funcProperties);
				break;
			case PR_SC_SELECT_INST:
				retval = resolveSelectCalls(specialCaseFunc, F, funcProperties);
				break;
			case PR_SC_DOTPROD:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_dot, 2, true);
				break;
			case PR_SC_GEO_DISTANCE:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_distance, 2, true);
				break;
			case PR_SC_GEO_FAST_DISTANCE:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_fast_distance, 2, true);
				break;
			case PR_SC_GEO_LENGTH:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_length, 1, true);
				break;
			case PR_SC_GEO_FAST_LENGTH:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_fast_length, 1, true);
				break;
			case PR_SC_CROSS:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_cross, 2, false);
				break;
			case PR_SC_GEO_NORMALIZE:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_normalize, 1, false);
				break;
			case PR_SC_GEO_FAST_NORMALIZE:
				retval = resolveGeometricFuncCalls(specialCaseFunc, F, funcProperties, geometric_fast_normalize, 1, false);
				break;
			case PR_SC_BOUNDARY_CHECK:
				retval = resolveEarlyExitCalls(specialCaseFunc, F, funcProperties);
				break;
			default:
				V_UNEXPECTED("No handling for Special case function"); // Should not happen! we only reach this function if the function IS a special case func 
				return false;
				break;
		}
	
		// Check for errors
		if (retval == false) return false;
	}
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility function, cast function arguments to the type which the function expects 
/////////////////////////////////////////////////////////////////////////////////////////////////////
Value * VectModuleManager::castArgumentIfNeeded(Value * inputVal, const Type * requiredType, Instruction * insertPoint, CodeProperties * funcProperties)
{
	const Type * sourceType = inputVal->getType();
	if (sourceType != requiredType)
	{
		V_PRINT("\t\tNeed to convert from type: " << *sourceType << "  to type: " << *requiredType << "\n");
		// Check if source Type is a pointer
		if (isa<PointerType>(sourceType))
		{
			V_ASSERT(0); // Currently, pointer (source) types are not known to have casting needs
			return NULL;
		}
		// Else, Check if targetType is a pointer
		else if (isa<PointerType>(requiredType))
		{
			V_ASSERT(cast<PointerType>(requiredType)->getElementType() == sourceType); // pointer must match "wanted" type 
			// create alloca and place data inside.
			Value * refPointer = new AllocaInst(sourceType, "alloca_val", insertPoint);
			V_ASSERT(refPointer->getType() == requiredType);
			new StoreInst(inputVal, refPointer, insertPoint);
			return refPointer;
		}
		// Else, need to cast between types
		else 
		{
			unsigned sourceSize = sourceType->getPrimitiveSizeInBits();
			unsigned targetSize = requiredType->getPrimitiveSizeInBits();
			// Check if both types are of the same size
			if (sourceSize == targetSize)
			{
				// Cast data from sourceType to targetType
				Value * convertedVal = new BitCastInst(inputVal, requiredType, "cast_val", insertPoint);
				return convertedVal;
			}
			// Else (targetType is larger than sourceType), need to zext source value
			else if (targetSize > sourceSize)
			{
				// Bitcast sourceType to an integer of the same size. Zero-extend to targetSize. Bitcast to target type (if needed)
				Value * firstCast;
				if (!inputVal->getType()->isIntegerTy())
				{
					firstCast = new BitCastInst(inputVal, IntegerType::get(funcProperties->context(), sourceSize), "cast_src", insertPoint);
				}
				else
				{
					firstCast = inputVal;
				}
				Value * zextVal = new ZExtInst(firstCast, IntegerType::get(funcProperties->context(), targetSize), "zext_cast", insertPoint);
				if (requiredType == IntegerType::get(funcProperties->context(), targetSize))
				{
					return zextVal;
				}
				else 
				{
					Value * secondCast = new BitCastInst(zextVal, requiredType, "cast_zext", insertPoint);
					return secondCast;
				}				
			}
			else
			{
				// Bitcast sourceType to an integer of the same size. truncate to targetSize. Bitcast to target type (if needed)
				Value * firstCast;
				if (!inputVal->getType()->isIntegerTy())
				{
					firstCast = new BitCastInst(inputVal, IntegerType::get(funcProperties->context(), sourceSize), "cast_src", insertPoint);
				}
				else
				{
					firstCast = inputVal;
				}
				Value * zextVal = new TruncInst(firstCast, IntegerType::get(funcProperties->context(), targetSize), "zext_cast", insertPoint);
				if (requiredType == IntegerType::get(funcProperties->context(), targetSize))
				{
					return zextVal;
				}
				else 
				{
					Value * secondCast = new BitCastInst(zextVal, requiredType, "cast_zext", insertPoint);
					return secondCast;
				}
			}
		}
	}
	else
	{
		return inputVal; // same type as expected, so nothing to do
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve all calls to special-case read-sampler 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveReadSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties, bool is2D)
{
	V_PRINT("\tResolving Read-samplers\n");
	unsigned index;
	if (ARCH_VECTOR_WIDTH == 4)
	{
		index = 3;
	}
	else if (ARCH_VECTOR_WIDTH == 8)
	{
		index = 4;
	}
	else
	{
		V_UNEXPECTED("Trying to vectorize a func with sampler, but no sampler exists for the vector width!\n");
		return false;
	}

	// Obtain the "proper" transposed sampler from Builtin functions module
	Function * LibTranspFunc;
	if (is2D)
	{
		LibTranspFunc = RUNTIME_MODULE->getFunction(transposedread2dList[index]);
	}
	else
	{
		LibTranspFunc = RUNTIME_MODULE->getFunction(transposedread3dList[index]);
	}
	if (!LibTranspFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	Constant * transposedSamplerConst = NULL; // Declare (or find) the function later on in this function
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		
		// Same fake 2D function is used for both Transposed reads and stream reads. So handle only the needed ones 
		if (is2D && !funcProperties->getProperty(callingInst, PR_SC_READ_SAMPLER_F_2D))
			continue;
		
		V_PRINT("\t\tResolving: " << *callingInst << "\n");
		if (!callingInst || !isa<CallInst>(callingInst))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case read sampler?
		}
		
		// Collect all the inputs from the fake call
		Value * imagePtr = callingInst->getOperand(1);
		Value * samplerType = callingInst->getOperand(2);
		Value * x_input = callingInst->getOperand(3);
		Value * y_input = callingInst->getOperand(4);	
		Value * z_input;	
		if (!is2D)
			z_input = callingInst->getOperand(5);
		
		// Prep all the arguments from the fake instruction, to the actual instruction/s
		Instruction * headInsertPoint = F->getEntryBlock().getFirstNonPHI();
		Value * outputPointers[4];

		for (unsigned i = 0; i < 4; i++)
		{
			outputPointers[i] = new AllocaInst(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), NULL, "trnsp.smp.alloca", headInsertPoint);
		}
		
		// May need to resolve the function pointer at this time (generated using the types of the arguments)
		// FIXME: this is a workaround for not materializing LibTranspFunc first.
		if (transposedSamplerConst == NULL)
		{
			std::vector<const Type *> newArgsTys;
			newArgsTys.push_back(imagePtr->getType());
			newArgsTys.push_back(samplerType->getType());
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(2));
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(3));
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(4));
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(5));
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(6));
			newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(7));
			if (!is2D)
				newArgsTys.push_back(LibTranspFunc->getFunctionType()->getParamType(8));
			
			FunctionType *samplerType = FunctionType::get(LibTranspFunc->getReturnType(), newArgsTys, false);
						
			if (is2D)
			{
				transposedSamplerConst = CURRENT_MODULE->getOrInsertFunction(transposedread2dList[index], 
																			 samplerType,
																			 LibTranspFunc->getAttributes());
			}
			else
			{
				transposedSamplerConst = CURRENT_MODULE->getOrInsertFunction(transposedread3dList[index], 
																			 samplerType,
																			 LibTranspFunc->getAttributes());
			}
			if (!transposedSamplerConst)
			{
				V_UNEXPECTED("failed generating function in current module");
				return false;
			}
		}
		
		std::vector<Value *> newArgs;
		newArgs.push_back(imagePtr);
		newArgs.push_back(samplerType);
		newArgs.push_back(castArgumentIfNeeded(x_input, LibTranspFunc->getFunctionType()->getParamType(2), callingInst,funcProperties));
		newArgs.push_back(castArgumentIfNeeded(y_input, LibTranspFunc->getFunctionType()->getParamType(3), callingInst,funcProperties));
		if (!is2D)
			newArgs.push_back(castArgumentIfNeeded(z_input, LibTranspFunc->getFunctionType()->getParamType(4), callingInst,funcProperties));
		
		newArgs.push_back(outputPointers[0]);
		newArgs.push_back(outputPointers[1]);
		newArgs.push_back(outputPointers[2]);
		newArgs.push_back(outputPointers[3]);
		
		// Generate the actual CALL
		CallInst::Create(transposedSamplerConst, newArgs.begin(), newArgs.end(), "", callingInst);
		
		// Vectorize breakdowns of the original sampler
		V_ASSERT(callingInst->getNumUses() == 4); // sanity: only the specially prepared shuffleVector insts may directly inherit from the fake sampler
		Value::use_iterator outputIter = callingInst->use_begin();
		Value::use_iterator outputIterEnd = callingInst->use_end();
		while (outputIter != outputIterEnd)
		{
			ShuffleVectorInst * shuffleInst = dyn_cast<ShuffleVectorInst>(*outputIter);
			++outputIter;
			V_ASSERT(shuffleInst);
			int index = shuffleInst->getMaskValue(0);
			
			// Generate a LOAD and replace with the shuffleVector instruction
			Value *	transpValueLoad = new LoadInst(outputPointers[index], "load.trnsp.val", callingInst);
			shuffleInst->replaceAllUsesWith(transpValueLoad);
			
			// Remove the shuffleVector inst
			shuffleInst->dropAllReferences();
			V_ASSERT(shuffleInst->use_empty());
			funcProperties->setProperty(shuffleInst, PR_INST_IS_REMOVED);
			shuffleInst->eraseFromParent();
		}
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve calls to special-case stream read-sampler 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveStreamReadSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving Stream Read-samplers\n");

	// Obtain the "proper" stream sampler from Builtin functions module
	Function * LibSteamFunc = RUNTIME_MODULE->getFunction(STREAM_READ_IMAGEF_2D_NAME);
	if (!LibSteamFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	Constant * streamSamplerConst = NULL; // Declare (or find) the function later on in this function
	
	// Preparing: obtain pre-prepared useful values: Loop size (get_local_size()), and loop-iteration index
	Value * loopOverallSize = funcProperties->getLoopSizeVal();
	Value * loopIterationIndex = funcProperties->getIterCountInst();
	V_ASSERT(loopOverallSize && loopIterationIndex);

	Value * constIndex0 = ConstantInt::get(getInt32Ty, 0);
	Value * constIndex1 = ConstantInt::get(getInt32Ty, 1);
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		
		// Same fake function may be used for both Transposed reads and stream reads. So handle only the needed ones 
		if (!funcProperties->getProperty(callingInst, PR_SC_STREAM_READ_SAMPLER))
			continue;		
		
		if (!callingInst || !isa<CallInst>(callingInst))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case read sampler?
		}
		
		V_ASSERT(callingInst->getNumUses() == 4); // sanity: only the specially prepared shuffleVector insts may directly inherit from the fake sampler

		// Collect all the inputs from the fake call
		Value * imagePtr = callingInst->getOperand(1);
		Value * samplerType = callingInst->getOperand(2);
		Value * x_input = callingInst->getOperand(3);
		Value * y_input = callingInst->getOperand(4);

		// Generate 4 huge Alloca's for storing all the output RGBA data...
		Instruction * colorAllocas[4];
		Instruction * colorStorage[4];
		unsigned numArrayIndices;
		const Type * dataArray;
		dataArray = ArrayType::get(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), MAX_LOOP_SIZE/ARCH_VECTOR_WIDTH);
		numArrayIndices = 2;
		Value * indices[3] = {constIndex0, constIndex0, constIndex0};
		Instruction * headInsertPoint = F->getEntryBlock().getFirstNonPHI();
		for (unsigned i = 0; i < 4; i++)
		{
			colorAllocas[i] = new AllocaInst(dataArray, NULL, FLOAT_X_WIDTH__ALIGNMENT, "stream.read.alloca", headInsertPoint);
			colorStorage[i] = GetElementPtrInst::CreateInBounds(colorAllocas[i], indices, indices+numArrayIndices , "alloca.address", headInsertPoint);
			if (ARCH_VECTOR_WIDTH != 4)
			{
				// expected storage type is <4 x float>* ,  for bitcast to it.
				colorStorage[i] = CastInst::CreatePointerCast(colorStorage[i], PointerType::get(VectorType::get(getFloatTy, 4), 0), "ptr.cast", headInsertPoint);
			}
		}
		
		// Pre-prepare index values for input to stream sampler
		UndefValue * undefCoord = UndefValue::get(READ_IMAGEF_2D_COORD_TYPE);
		// Generate start vector
		Value * firstXcoord = ExtractElementInst::Create(x_input, constIndex0, "extractX.1", callingInst);
		Value * firstYcoord = ExtractElementInst::Create(y_input, constIndex0, "extractY.1", callingInst);
		Value * start_tmp = InsertElementInst::Create(undefCoord, firstXcoord , constIndex0, "insertX.1", callingInst);
		Value * start = InsertElementInst::Create(start_tmp, firstYcoord , constIndex1, "insertY.1", callingInst);	
		// Generate stride vector
		Value * secondXcoord = ExtractElementInst::Create(x_input, constIndex1, "extractX.2", callingInst);
		Value * secondYcoord = ExtractElementInst::Create(y_input, constIndex1, "extractY.2", callingInst);
		Value * strideX = BinaryOperator::Create(Instruction::FSub ,secondXcoord, firstXcoord, "strideX", callingInst);
		Value * strideY = BinaryOperator::Create(Instruction::FSub ,secondYcoord, firstYcoord, "strideY", callingInst);
		Value * stride_tmp = InsertElementInst::Create(undefCoord, strideX , constIndex0, "stride.1", callingInst);
		Value * stride = InsertElementInst::Create(stride_tmp, strideY , constIndex1, "stride.2", callingInst);	

		
		// prep arguments for calling the stream sampler...
		std::vector<Value *> newArgs;
		newArgs.push_back(imagePtr);
		newArgs.push_back(samplerType);
		newArgs.push_back(castArgumentIfNeeded(start, LibSteamFunc->getFunctionType()->getParamType(2), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(stride, LibSteamFunc->getFunctionType()->getParamType(3), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(loopOverallSize, LibSteamFunc->getFunctionType()->getParamType(4), callingInst, funcProperties));
		newArgs.push_back(colorStorage[0]);
		newArgs.push_back(colorStorage[1]);
		newArgs.push_back(colorStorage[2]);
		newArgs.push_back(colorStorage[3]);

		if (streamSamplerConst == NULL)
		{
			std::vector<const Type *> newArgsTys;
			newArgsTys.push_back(imagePtr->getType());
			newArgsTys.push_back(samplerType->getType());
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(2));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(3));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(4));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(5));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(6));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(7));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(8));
			
			// Generate the actual CALL
			// FIXME: this is a workaround for not materializing LibSteamFunc first.
			//        remove for llvm-2326.
			FunctionType *streamSamplerType = FunctionType::get(LibSteamFunc->getReturnType(), newArgsTys, false);
			streamSamplerConst = CURRENT_MODULE->getOrInsertFunction(STREAM_READ_IMAGEF_2D_NAME,
																	 streamSamplerType,
																	 LibSteamFunc->getAttributes());
			if (!streamSamplerConst)
			{
				V_UNEXPECTED("failed generating function in current module");
				return false;
			}
		}

		// Generate actual stream samler call instruction
		CallInst::Create(streamSamplerConst, newArgs.begin(), newArgs.end(), "", callingInst);
		
		// Vectorize breakdowns of the original sampler
		Value::use_iterator outputIter = callingInst->use_begin();
		Value::use_iterator outputIterEnd = callingInst->use_end();
		while (outputIter != outputIterEnd)
		{
			ShuffleVectorInst * shuffleInst = dyn_cast<ShuffleVectorInst>(*outputIter);
			++outputIter;
			V_ASSERT(shuffleInst);
			int index = shuffleInst->getMaskValue(0);
			int colorIndex = index % 4; // values 0, 4, 8.. are R. values 1, 5, 9.. are G. And so on.
			int dupIndex = index / 4;
			
			// Generate address calculation:
			Instruction * colorPointer;
			Value * constDupIndex = ConstantInt::get(getInt32Ty, dupIndex);
			Value * indices[3] = {constIndex0, loopIterationIndex, constDupIndex};
			colorPointer = GetElementPtrInst::CreateInBounds(colorAllocas[colorIndex], indices, indices+numArrayIndices , "calc.address", shuffleInst);

			// Generate actual load
			Value *	transpValueLoad = new LoadInst(colorPointer, "load.trnsp.val", false, FLOAT_X_WIDTH__ALIGNMENT, shuffleInst);
			shuffleInst->replaceAllUsesWith(transpValueLoad);
			
			// Remove the shuffleVector inst
			shuffleInst->dropAllReferences();
			V_ASSERT(shuffleInst->use_empty());
			funcProperties->setProperty(shuffleInst, PR_INST_IS_REMOVED);
			shuffleInst->eraseFromParent();
		}
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();		
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve calls to special-case write-sampler 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveWriteSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving Write-samplers\n");
	unsigned index;
	if (ARCH_VECTOR_WIDTH == 4)
	{
		index = 3;
	}
	else if (ARCH_VECTOR_WIDTH == 8)
	{
		index = 4;
	}
	else
	{
		V_UNEXPECTED("Trying to vectorize a func with sampler, but no sampler exists for the vector width!\n");
		return false;
	}
	
	// Obtain the "proper" transposed sampler from Builtin functions module
	Function * LibTranspFunc = RUNTIME_MODULE->getFunction(transposedwriteList[index]);
	if (!LibTranspFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}

	Constant * transposedSamplerConst = NULL; // Declare (or find) the function later on in this function
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on current

		// Same fake function may be used for both Transposed writes and stream writes. So handle only the needed ones 
		if (!funcProperties->getProperty(callingInst, PR_SC_WRITEF_SAMPLER))
			continue;		
		
		if (!callingInst || !isa<CallInst>(callingInst))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case read sampler?
		}
		
		// Collect all the inputs from the fake call
		Value * imagePtr = callingInst->getOperand(1);
		Value * x_input = callingInst->getOperand(2);
		Value * y_input = callingInst->getOperand(3);	
		Value * colors[4];
		for (unsigned color = 0; color < 4; color++)
		{
			colors[color] = callingInst->getOperand(4+color);
		}
		
		// May need to resolve the function pointer at this time (generated using the types of the arguments)
		// FIXME: this is a workaround for not materializing LibTranspFunc first.
		if (transposedSamplerConst == NULL)
		{
			std::vector<const Type *> newArgsType;
			newArgsType.push_back(imagePtr->getType());
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(1));
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(2));
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(3));
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(4));
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(5));
			newArgsType.push_back(LibTranspFunc->getFunctionType()->getParamType(6));
			FunctionType *samplerType = FunctionType::get(LibTranspFunc->getReturnType(), newArgsType, false);
			
			// Find (or create) declaration for transposed sampler
			transposedSamplerConst = CURRENT_MODULE->getOrInsertFunction(transposedwriteList[index], 
																		 samplerType,
																		 LibTranspFunc->getAttributes());
			if (!transposedSamplerConst)
			{
				V_UNEXPECTED("failed generating function in current module");
				return false;
			}
			
		}

		// Prep all the arguments from the fake instruction, to the actual instruction/s
		std::vector<Value *> newArgs;
		newArgs.push_back(imagePtr);
		newArgs.push_back(castArgumentIfNeeded(x_input, LibTranspFunc->getFunctionType()->getParamType(1), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(y_input, LibTranspFunc->getFunctionType()->getParamType(2), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(colors[0], LibTranspFunc->getFunctionType()->getParamType(3), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(colors[1], LibTranspFunc->getFunctionType()->getParamType(4), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(colors[2], LibTranspFunc->getFunctionType()->getParamType(5), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(colors[3], LibTranspFunc->getFunctionType()->getParamType(6), callingInst, funcProperties));
		
		// Generate the actual CALL
		CallInst::Create(transposedSamplerConst, newArgs.begin(), newArgs.end(), "", callingInst);
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve calls to special-case stream write-sampler 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveStreamWriteSamplerCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving Stream Write-samplers\n");
	
	// Obtain the "proper" stream sampler from Builtin functions module
	Function * LibSteamFunc = RUNTIME_MODULE->getFunction(STREAM_WRITE_IMAGEF_NAME);
	if (!LibSteamFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	Constant * streamSamplerConst = NULL; // Declare (or find) the function later on in this function
	
	// Preparing: obtain pre-prepared useful values: Loop size (get_local_size()), and loop-iteration index
	Value * loopOverallSize = funcProperties->getLoopSizeVal();
	Instruction * loopIterationIndex = funcProperties->getIterCountInst();
	V_ASSERT(loopOverallSize && loopIterationIndex);
	
	Value * constIndex0 = ConstantInt::get(getInt32Ty, 0);

	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		
		// Same fake function may be used for both Transposed writes and stream writes. So handle only the needed ones 
		if (!funcProperties->getProperty(callingInst, PR_SC_STREAM_WRITE_SAMPLER))
			continue;		
		
		if (!callingInst || !isa<CallInst>(callingInst))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case read sampler?
		}
		
		// Collect only the interesting inputs from the fake call
		unsigned index = 1;
		Value * imagePtr = callingInst->getOperand(index++);
		Value * x_input0 = callingInst->getOperand(index++);
		Value * y_input = callingInst->getOperand(index++);	
		Value * colors[4];
		for (unsigned color = 0; color < 4; color++)
		{
			colors[color] = callingInst->getOperand(index++);
		}
				
		// Generate 4 huge Alloca's for storing all the input RGBA data...
		Instruction * colorAllocas[4];
		Instruction * colorStorage[4];
		unsigned numArrayIndices;
		const Type * dataArray;
		dataArray = ArrayType::get(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), MAX_LOOP_SIZE/ARCH_VECTOR_WIDTH);
		numArrayIndices = 2;
		Instruction * headInsertPoint = F->getEntryBlock().getFirstNonPHI();
		Value * indices[3] = {constIndex0, constIndex0, constIndex0};
		for (unsigned i = 0; i < 4; i++)
		{
			colorAllocas[i] = new AllocaInst(dataArray, NULL, FLOAT_X_WIDTH__ALIGNMENT, "stream.write.alloca", headInsertPoint);
			colorStorage[i] = GetElementPtrInst::CreateInBounds(colorAllocas[i], indices, indices+numArrayIndices , "alloca.address", headInsertPoint);
			if (ARCH_VECTOR_WIDTH != 4)
			{
				// expected storage type is <4 x float>* ,  for bitcast to it.
				colorStorage[i] = CastInst::CreatePointerCast(colorStorage[i], PointerType::get(VectorType::get(getFloatTy, 4), 0), "ptr.cast", headInsertPoint);				
			}
		}
		
		// Store all the input colors in the alloca locations
		for (unsigned colorIndex = 0; colorIndex < 4; colorIndex++)
		{			
			Instruction * storeInsertPoint;
			// Insertion point is right after the value is generated - if its an instruction AND its inside the loop...
			// Otherwise - just place the stores inside the main loop
			if (isa<Instruction>(colors[colorIndex]) && !funcProperties->getProperty(colors[colorIndex], PR_MOVE_TO_LOOP_HEAD))
			{
				V_ASSERT(!funcProperties->getProperty(colors[colorIndex], PR_MOVE_TO_LOOP_TAIL));
				BasicBlock::iterator iter(cast<Instruction>(colors[colorIndex]));
				while (isa<PHINode>(++iter)); // increment insertion point until it is a non PHI instruction
				storeInsertPoint = cast<Instruction>(iter);
			}
			else
			{
				// must place the store INSIDE the main loop, so it will happen multiple times
				if (isa<PHINode>(loopIterationIndex))
				{
					storeInsertPoint = loopIterationIndex->getParent()->getFirstNonPHI();
				}
				else
				{
					BasicBlock::iterator iter(loopIterationIndex);
					++iter;
					storeInsertPoint = cast<Instruction>(iter);
				}
			}

			// Generate address calculation: 
			Value * constDupIndex = ConstantInt::get(getInt32Ty, 0);
			Value * indices[3] = {constIndex0, loopIterationIndex, constDupIndex};
			Instruction * colorPointer = GetElementPtrInst::CreateInBounds(colorAllocas[colorIndex], indices, indices+numArrayIndices , "calc.address", storeInsertPoint);
			
			// Generate the store
			new StoreInst(colors[colorIndex], colorPointer, false, FLOAT_X_WIDTH__ALIGNMENT, storeInsertPoint);
		}
		
		// Arguments are known the have these properties:
		// Y is TID-independent.
		// X is consecutive, and may be it is a Pivot in the main loop .Need to find the original (pre-loop) X value..
		// X can be not Pivot ONLY IF it was pivot before, but since it has no users inside the loop, it was removed from Pivots list
		Value * origX = NULL;
		if (funcProperties->getProperty(x_input0, PR_LOOP_PIVOT))
		{
			V_ASSERT(isa<PHINode>(x_input0)); // All pivots were replaced with PHI nodes
			origX = cast<PHINode>(x_input0)->getIncomingValue(0);
		}
		else
		{
			origX = x_input0;
		}
		V_ASSERT(origX);
		
		
		// prep arguments for calling the strem sampler...
		std::vector<Value *> newArgs;
		newArgs.push_back(imagePtr);
		newArgs.push_back(castArgumentIfNeeded(origX, LibSteamFunc->getFunctionType()->getParamType(1), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(y_input, LibSteamFunc->getFunctionType()->getParamType(2), callingInst, funcProperties));
		newArgs.push_back(castArgumentIfNeeded(loopOverallSize, LibSteamFunc->getFunctionType()->getParamType(3), callingInst, funcProperties));
		newArgs.push_back(colorStorage[0]);
		newArgs.push_back(colorStorage[1]);
		newArgs.push_back(colorStorage[2]);
		newArgs.push_back(colorStorage[3]);
		
		if (streamSamplerConst == NULL)
		{
			std::vector<const Type *> newArgsTys;
			newArgsTys.push_back(imagePtr->getType());
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(1));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(2));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(3));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(4));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(5));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(6));
			newArgsTys.push_back(LibSteamFunc->getFunctionType()->getParamType(7));
			
			// Generate the actual CALL
			// FIXME: this is a workaround for not materializing LibSteamFunc first.
			//        remove for llvm-2326.
			FunctionType *streamSamplerType = FunctionType::get(LibSteamFunc->getReturnType(), newArgsTys, false);
			streamSamplerConst = CURRENT_MODULE->getOrInsertFunction(STREAM_WRITE_IMAGEF_NAME,
																	 streamSamplerType,
																	 LibSteamFunc->getAttributes());
			if (!streamSamplerConst)
			{
				V_UNEXPECTED("failed generating function in current module");
				return false;
			}
		}

		// Generate the actual CALL
		CallInst::Create(streamSamplerConst, newArgs.begin(), newArgs.end(), "", callingInst);
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();		
	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve calls to special-case select 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveSelectCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving Select func calls\n");	
	V_ASSERT(specialCaseFunc->getFunctionType()->getNumParams() == 3); // sanity: select has 3 arguments
	int func_index; // index in the list of transposed function names - index conversion (4->3, 8->4, 16->5) for table lookup
	if (ARCH_VECTOR_WIDTH == 4) func_index = 3;
	else if (ARCH_VECTOR_WIDTH == 8) func_index = 4;
	else if (ARCH_VECTOR_WIDTH == 16) func_index = 5;
	else {
		V_UNEXPECTED("met unsupported ARCH width");
		V_PRINT("\t\tUnsupported arch vector width\n");		
		return false; // Unsupported arch vector width for this transposed function!
	}
	
	// Find the type of the specific select
	const Type * retType = specialCaseFunc->getFunctionType()->getReturnType();
	V_ASSERT(isa<VectorType>(retType));
	const Type * retElementType = cast<VectorType>(retType)->getElementType();
	unsigned funcTypeIndex;
	bool isFloat = false;
	
	if (retElementType == getInt8Ty)
	{
		funcTypeIndex = 0;
	}
	else if (retElementType == getInt16Ty)
	{
		funcTypeIndex = 1;
	}
	else if (retElementType == getInt32Ty)
	{
		funcTypeIndex = 2;
	}
	else if (retElementType == getInt64Ty)
	{
		funcTypeIndex = 3;
	}
	else if (retElementType == getFloatTy)
	{
		funcTypeIndex = 4;
		isFloat = true;
	}
	else
	{
		V_UNEXPECTED("Select of unsupported type!"); // Unsupported type
		return false;
	}
	
	unsigned funcSize = LOG_(ARCH_VECTOR_WIDTH) + 1;  // basically an index conversion (4->3, 8->4, 16->5) for table lookup
	
	const char * selectFuncName = selectFuncsList[funcTypeIndex][funcSize];
	
	// Try to find (or declare) this function locally: Obtain function from Builtin functions module
	Function * LibSelectFunc = RUNTIME_MODULE->getFunction(selectFuncName);
	if (!LibSelectFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	// Find (or create) declaration for select function
	Constant * VectSelectFunc = CURRENT_MODULE->getOrInsertFunction(selectFuncName, 
																   LibSelectFunc->getFunctionType(),
																   LibSelectFunc->getAttributes());
	if (!VectSelectFunc)
	{
		V_UNEXPECTED("failed generating function in current module");
		return false;
	}
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		if (!callingInst || !isa<CallInst>(callingInst) || !funcProperties->getProperty(callingInst, PR_SC_SELECT_INST))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case select?
		}
		
		// Prep all the arguments from the fake instruction, to the actual instruction
		std::vector<Value *> newArgs;
		for (unsigned i = 0; i < 3; i++)
		{
			newArgs.push_back(castArgumentIfNeeded(callingInst->getOperand(i+1), LibSelectFunc->getFunctionType()->getParamType(i), callingInst, funcProperties));
		}
		
		// Generate the actual CALL
		Value * transposedSelectCall = CallInst::Create(VectSelectFunc, newArgs.begin(), newArgs.end(), "", callingInst);
		
		Value * retVal = castArgumentIfNeeded(transposedSelectCall, callingInst->getType(), callingInst, funcProperties);
		
		// Replace users of fake dot_product with real dot_product
		callingInst->replaceAllUsesWith(retVal);
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
		
	}
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve all calls to special-case dot-product 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveGeometricFuncCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties, 
												  geometricListType funcList, unsigned numArgs, bool isReducted)
{
	V_PRINT("\tResolving Geometric function\n");
	V_ASSERT(numArgs == 1 || numArgs == 2); // only know geometric funcs take 1 or 2 arguments 
	
	int func_index; // index in the list of transposed function names - depends on architecture vectorization width
	if (ARCH_VECTOR_WIDTH == 4) func_index = 1;
	else if (ARCH_VECTOR_WIDTH == 8) func_index = 2;
	else if (ARCH_VECTOR_WIDTH == 16) func_index = 3;
	else {
		V_UNEXPECTED("met unsupported ARCH width");
		V_PRINT("\t\tUnsupported arch vector width\n");		
		return false; // Unsupported arch vector width for this transposed function!
	}
	
	// Find the vector width of the specific function
	unsigned funcOperands = specialCaseFunc->getFunctionType()->getNumParams(); 
	unsigned vecWidth = funcOperands/numArgs;
	
	// Find the correct real function to call
	std::string realFuncName = funcList[vecWidth-1][func_index];
	
	// Obtain the "proper" transposed function from Builtin functions module
	Function * LibTranspFunc = RUNTIME_MODULE->getFunction(realFuncName);
	if (!LibTranspFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	// Find (or create) declaration for transposed function
	Constant * transposedGeometricFunc = CURRENT_MODULE->getOrInsertFunction(realFuncName, 
																			 LibTranspFunc->getFunctionType(),
																			 LibTranspFunc->getAttributes());
	if (!transposedGeometricFunc)
	{
		V_UNEXPECTED("failed generating function in current module");
		return false;
	}
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		if (!callingInst || !isa<CallInst>(callingInst))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not an instruction? is not a CALL?
		}
		
		// Prep all the arguments from the fake function, to the actual function
		std::vector<Value *> newArgs;
		for (unsigned i = 0; i < funcOperands; i++)
		{
			newArgs.push_back(castArgumentIfNeeded(callingInst->getOperand(i+1), LibTranspFunc->getFunctionType()->getParamType(i), callingInst, funcProperties));
		}
		
		// For non-reducted functions, the return values are passed by reference at the end of the arguments list. Need to prep Alloca's for them
		SmallVector<Value*, MAX_OCL_VECTOR_WIDTH> outputPointers;
		if (!isReducted) // Also true if the function is scalar-geometric function
		{
			Instruction * headInsertPoint = F->getEntryBlock().getFirstNonPHI();
			for (unsigned i = 0; i < vecWidth; i++)
			{
				outputPointers.push_back(new AllocaInst(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), NULL, "trnsp.alloca", headInsertPoint));
				newArgs.push_back(outputPointers[i]);
			}			
		}
		
		// Generate the actual CALL
		Value * transposedGeometricCall = CallInst::Create(transposedGeometricFunc, newArgs.begin(), newArgs.end(), "", callingInst);
		
		if (!isReducted && vecWidth>1)
		{
			// Vectorize breakdowns of the original function
			V_ASSERT(callingInst->getNumUses() <= vecWidth); // sanity: only the specially prepared shuffleVector insts may directly inherit from the fake func
			Value::use_iterator outputIter = callingInst->use_begin();
			Value::use_iterator outputIterEnd = callingInst->use_end();
			while (outputIter != outputIterEnd)
			{
				ShuffleVectorInst * shuffleInst = dyn_cast<ShuffleVectorInst>(*outputIter);
				++outputIter;
				V_ASSERT(shuffleInst);
				int index = shuffleInst->getMaskValue(0);
				
				// Generate a LOAD and replace with the shuffleVector instruction
				Value *	transpValueLoad = new LoadInst(outputPointers[index], "load.trnsp.val", callingInst);
				shuffleInst->replaceAllUsesWith(transpValueLoad);
				
				// Remove the shuffleVector inst
				shuffleInst->dropAllReferences();
				V_ASSERT(shuffleInst->use_empty());
				funcProperties->setProperty(shuffleInst, PR_INST_IS_REMOVED);
				shuffleInst->eraseFromParent();
			}
		}
		else if (!isReducted)
		{
			// reducted, but for a scalar geometric function
			// Generate a LOAD and replace with the fake func's return value
			Value *	transpValueLoad = new LoadInst(outputPointers[0], "load.trnsp.val", callingInst);
			callingInst->replaceAllUsesWith(transpValueLoad);			
		}
		else
		{
			// cast the return value (if needed) to the "expected" return value
			Value * retVal = castArgumentIfNeeded(transposedGeometricCall, callingInst->getType(), callingInst, funcProperties);
			// Replace users of fake function with real function
			callingInst->replaceAllUsesWith(retVal);
		}
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
	
	}
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve all calls to special-case __ci_gamma 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveCIGammaCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving ci_gamma\n");
	V_ASSERT(ARCH_VECTOR_WIDTH == 4  || ARCH_VECTOR_WIDTH == 8); // Current implementation is dependent on SSE width == 4 or 8
	
	// Obtain the "proper" transposed function from Builtin functions module
	std::string funcName;
	if (ARCH_VECTOR_WIDTH == 4) {
		funcName = CI_GAMMA_TRANSPOSED4_NAME;
	}
	else if (ARCH_VECTOR_WIDTH == 8) {
		funcName = CI_GAMMA_TRANSPOSED8_NAME;
	}
	Function * LibTranspFunc = RUNTIME_MODULE->getFunction(funcName);
	if (!LibTranspFunc)
	{
		V_UNEXPECTED("function not found in runtime module"); // Function doesnt exist?
		return false;
	}
	
	// Find (or create) declaration for transposed function
	Constant * transposedGammaConst = CURRENT_MODULE->getOrInsertFunction(funcName, 
																		  LibTranspFunc->getFunctionType(),
																		  LibTranspFunc->getAttributes());
	if (!transposedGammaConst)
	{
		V_UNEXPECTED("failed generating function in current module");
		return false;
	}
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		
		V_PRINT("\t\tResolving: " << *callingInst << "\n");
		if (!callingInst || !isa<CallInst>(callingInst) || !funcProperties->getProperty(callingInst, PR_SC_CI_GAMMA))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case?
		}
		
		// Collect all the inputs from the fake call
		Value * R_input = callingInst->getOperand(1);
		Value * G_input = callingInst->getOperand(2);
		Value * B_input = callingInst->getOperand(3);
		Value * Y_input = callingInst->getOperand(4);

		// Place-holder for output values
		Value * outputBreadown[3];

		
		// From here on - a split. Implementations for 4-wide and 8-wide are different enough to require separate codes
		if (ARCH_VECTOR_WIDTH == 4)
		{
			// Create undef and constants needed for later usage
			UndefValue * undefVect = UndefValue::get(LibTranspFunc->getReturnType());			
			std::vector<Constant*> vectRvals, vectGvals, vectBvals;
			for (unsigned i = 0; i<4 ; i++)
			{
				vectRvals.push_back(ConstantInt::get(getInt32Ty, i));
				vectGvals.push_back(ConstantInt::get(getInt32Ty, 4+i));
				vectBvals.push_back(ConstantInt::get(getInt32Ty, 8+i));
			}
			Constant * shuffleR = ConstantVector::get(vectRvals);
			Constant * shuffleG = ConstantVector::get(vectGvals);
			Constant * shuffleB = ConstantVector::get(vectBvals);
			
			// Prep arguments from the fake instruction, generate calls, and break-down the retVal
			std::vector<Value *> newArgs;
			newArgs.push_back(castArgumentIfNeeded(R_input, LibTranspFunc->getFunctionType()->getParamType(0), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(G_input, LibTranspFunc->getFunctionType()->getParamType(1), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(B_input, LibTranspFunc->getFunctionType()->getParamType(2), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(Y_input, LibTranspFunc->getFunctionType()->getParamType(3), callingInst,funcProperties));
			
			// Generate the actual CALL
			Value * newGammaCall = CallInst::Create(transposedGammaConst, newArgs.begin(), newArgs.end(), "", callingInst);
			
			// Generate output vectors from return value
			outputBreadown[0] = new ShuffleVectorInst(newGammaCall, undefVect, shuffleR, "outbut.breakdown.R", callingInst);
			outputBreadown[1] = new ShuffleVectorInst(newGammaCall, undefVect, shuffleG, "outbut.breakdown.G", callingInst);
			outputBreadown[2] = new ShuffleVectorInst(newGammaCall, undefVect, shuffleB, "outbut.breakdown.B", callingInst);			
		}
		else if (ARCH_VECTOR_WIDTH == 8)
		{
			// Generate ALLOCAs to spill the output values (as they are passed by reference)
			Instruction * headInsertPoint = F->getEntryBlock().getFirstNonPHI();
			Value * outputPointers[3];
			for (unsigned i = 0; i < 3; i++)
			{
				outputPointers[i] = new AllocaInst(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), NULL, "trnsp.gamma.alloca", headInsertPoint);
			}

			// Prep arguments from the fake instruction, generate calls, and break-down the retVal
			std::vector<Value *> newArgs;
			newArgs.push_back(castArgumentIfNeeded(R_input, LibTranspFunc->getFunctionType()->getParamType(0), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(G_input, LibTranspFunc->getFunctionType()->getParamType(1), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(B_input, LibTranspFunc->getFunctionType()->getParamType(2), callingInst,funcProperties));
			newArgs.push_back(castArgumentIfNeeded(Y_input, LibTranspFunc->getFunctionType()->getParamType(3), callingInst,funcProperties));
			newArgs.push_back(outputPointers[0]);
			newArgs.push_back(outputPointers[1]);
			newArgs.push_back(outputPointers[2]);
			
			// Generate the actual CALL
			CallInst::Create(transposedGammaConst, newArgs.begin(), newArgs.end(), "", callingInst);
			
			// Generate loads of the returned values
			outputBreadown[0] = new LoadInst(outputPointers[0], "load.trnsp.gamma", callingInst);
			outputBreadown[1] = new LoadInst(outputPointers[1], "load.trnsp.gamma", callingInst);
			outputBreadown[2] = new LoadInst(outputPointers[2], "load.trnsp.gamma", callingInst);
		}
		
		// Connect breakdowns of the functions retVal
		V_ASSERT(callingInst->getNumUses() == 3); // sanity: only the specially prepared shuffleVector insts may directly inherit from the fake sampler
		Value::use_iterator outputIter = callingInst->use_begin();
		Value::use_iterator outputIterEnd = callingInst->use_end();
		while (outputIter != outputIterEnd)
		{
			ShuffleVectorInst * shuffleInst = dyn_cast<ShuffleVectorInst>(*outputIter);
			++outputIter;
			V_ASSERT(shuffleInst);
			int index = shuffleInst->getMaskValue(0);
			
			// Find the matching output value needed...
			unsigned matchingIndex = index % 4;
			V_ASSERT(matchingIndex != 3); // Sanity: index 3 is not used in this func!

			shuffleInst->replaceAllUsesWith(outputBreadown[matchingIndex]);
		
			// Remove the shuffleVector inst
			shuffleInst->dropAllReferences();
			V_ASSERT(shuffleInst->use_empty());
			funcProperties->setProperty(shuffleInst, PR_INST_IS_REMOVED);
			shuffleInst->eraseFromParent();
		}
		
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
	}
	V_PRINT("\t\tResolving ci_gamma completed!\n");	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Resolve all calls to early exits 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::resolveEarlyExitCalls(Function * specialCaseFunc, Function *F, CodeProperties * funcProperties)
{
	V_PRINT("\tResolving Early Exits\n");
	
	// Find the properties of the specific early exit situation
	std::string fakeName = specialCaseFunc->getName();
	char exitOnTrue = fakeName.at(EARLY_EXIT_FAKE_NAME_PREFIX.length()) - '0';
	char predicationChar = fakeName.at(EARLY_EXIT_FAKE_NAME_PREFIX.length() + 2);
	V_PRINT("\t\tExitOnTrue? " << (exitOnTrue ? "Yes" : "No") << " prdication:" << predicationChar << " width:" << (int)(fakeName.at(EARLY_EXIT_FAKE_NAME_PREFIX.length() + 4)) << "\n");
	
	// loop over all users
	Value::use_iterator ui = specialCaseFunc->use_begin();
	Value::use_iterator ue = specialCaseFunc->use_end();
	while (ui != ue)
	{
		Instruction * callingInst = dyn_cast<Instruction>(*ui);
		++ui; // move to next iteration before working on corrent
		if (!callingInst || !isa<CallInst>(callingInst) || !funcProperties->getProperty(callingInst, PR_SC_BOUNDARY_CHECK))
		{
			V_UNEXPECTED("user is not an instruction?");
			return false; // Something went wrong here. User is not a function? is not a CALL? was not marked special-case read sampler?
		}
		
		if (!funcProperties->getProperty(callingInst, PR_MOVE_TO_LOOP_HEAD))
		{
			// Early exit was not moved to loop head. This is bad - we cannot continue vectorizing like this. Must abort...
			V_PRINT("\t\tEarly exit was not moved to loop head. Reject vectorizing!\n");
			return false;
		}

		// Extract arguments
		Value * leftArg = callingInst->getOperand(1);
		Value * rightArg = callingInst->getOperand(2);
		
		// Separate the handling for TID-dependent and TID-independent exits (different compares and different branches
		if (!funcProperties->getProperty(callingInst, PR_TID_DEPEND))
		{
			// TID-independent. No boundary check is required. Recreate the original compare, and jump to exit block
			V_PRINT("\t\tTID-independent Early Exit\n");
			// Generate early-exit compare 
			ICmpInst * newCompare = new ICmpInst(callingInst, 
												 (ICmpInst::Predicate)predicationChar, 
												 leftArg,
												 rightArg,
												 "early.exit.compare");
			
			// Split block right after the compare
			BasicBlock * loopHeadStart = newCompare->getParent();
			BasicBlock * loopHeadCont = loopHeadStart->splitBasicBlock(++(BasicBlock::iterator(newCompare)));
			BasicBlock * exitTarget = funcProperties->getExitBlock();
			
			// remove the "ret" which was automatically generated before the split point
			loopHeadStart->getTerminator()->eraseFromParent();
			
			// Prepare arguments for proper branch
			BasicBlock * trueTarget = exitOnTrue ? exitTarget : loopHeadCont;
			BasicBlock * falseTarget = exitOnTrue ? loopHeadCont : exitTarget;
			
			// Generate branch instruction
			BranchInst::Create(trueTarget, falseTarget, newCompare, loopHeadStart);
		}
		else
		{
			// TID-dependent early exit. Those can be handled in 2 ways:
			// 1) Find what subset of the mega-loop should be "iterated" over, and convert the mega-loop
			//    to iterate over this subset
			// 2) If early-exit is true for some part of the mega-loop, just run the whole mega-loop over the scalar code
			//
			// Basically, we prefer option #1 - but to use it, we need to make sure that all references to local_size
			// appear only AFTER the early exit code, as that code will now modify local_size to a (potentially) smaller
			// value (for smaller loop iteration count)
			V_PRINT("\t\tTID-dependent Early Exit\n");
			
			// Check that there is no consumer of local_size between the start of the kernel, and this instruction
			bool canOptimizeEarlyExit = true;
			Value * loopSize = funcProperties->getLoopSizeVal();
			inst_iterator sI = inst_begin(CURRENT_FUNCTION);
			while (true)
			{
				Instruction * currInst = &*sI;
				if (currInst == callingInst) break; // exit from this while-loop
				sI++; 
				for (User::op_iterator i = currInst->op_begin(), e = currInst->op_end(); i != e; ++i) 
				{
					Value *v = *i;
					if (v == loopSize)
					{
						canOptimizeEarlyExit = false;
						break;
					}
				}
				if (!canOptimizeEarlyExit) break;
			}
			
			if (canOptimizeEarlyExit)
			{
				// There is no local_size consumer between kernel's start and this point. So we generate solution #1.
				// The resulting code will look like this:
				//
				//  1) Generate the "remaining" horizontal size (end_of_row - TID)
				//  2) check if "remaining" is smaller than group_size
				//  3) If so - replace the loop size to be the "remaining" value
				//  4) clear the last 2 bits (LOG(WIDTH)) of the loop size, and keep them in a "remainder" var
				//  5) check if loop size is zero or less.
				//  6) if so - jump to exit block
				
				//  7) Divert end-of-code to a new basic block (instead of exit block)
				//  8) compare the "remainder" var to zero
				//  9) is so - jump to exit block. Otherwise - jump to scalar code block
				

				V_PRINT("\t\tUse optimized Early Exit\n");
				BasicBlock * scalarLoopTarget = funcProperties->getScalarLoopBlock();
				BasicBlock * remainderBlock = funcProperties->getRemainderBlock();
				
				// Generate dummy value, to hijack all the local_size users
				const Type * dummyType = PointerType::get(loopSize->getType(), 0);
				Constant * subExpr = ConstantExpr::getIntToPtr(ConstantInt::get(getInt32Ty, APInt(32, 0xdeadbeef)), dummyType);
				Instruction * dummyVal = new LoadInst(subExpr);
				loopSize->replaceAllUsesWith(dummyVal);
				
				bool isRightValueConst;  // One of the 2 values must be TID-independent.
				if (funcProperties->getProperty(leftArg, PR_TID_DEPEND))
				{
					V_ASSERT(!funcProperties->getProperty(rightArg, PR_TID_DEPEND));
					isRightValueConst = true;
				}
				else
				{
					V_ASSERT(funcProperties->getProperty(rightArg, PR_TID_DEPEND));
					isRightValueConst = false;
				}
				Value * idDependentvalue = isRightValueConst ? leftArg : rightArg;
				Value * constantForCalculation = isRightValueConst ? rightArg : leftArg;
				Value * localSize = castArgumentIfNeeded(loopSize, idDependentvalue->getType(), callingInst, funcProperties);
				// Check if bound-check will need to increment constant by 1			
				ICmpInst::Predicate pred = (ICmpInst::Predicate)predicationChar;
				if (pred == CmpInst::ICMP_UGE || pred == CmpInst::ICMP_SGE ||
					pred == CmpInst::ICMP_ULE || pred == CmpInst::ICMP_SLE)
				{
					constantForCalculation = BinaryOperator::Create(Instruction::Add, 
																	constantForCalculation, 
																	ConstantInt::get(constantForCalculation->getType() ,1), 
																	"addOne", 
																	callingInst);
				}
				
				// Generate the "remaining" iterations until the constant value
				Instruction * remainingIters = BinaryOperator::Create(Instruction::Sub, 
																	  constantForCalculation, 
																	  idDependentvalue, 
																	  "suggested_size", 
																	  callingInst);
				
				// compare if suggested is smaller than local_size
				ICmpInst * compareForSelect = new ICmpInst(callingInst, 
														   CmpInst::ICMP_SGT, 
														   remainingIters,
														   localSize,
														   "select.compare");
				
				Instruction * newLocal = SelectInst::Create(compareForSelect, localSize, remainingIters, "select.local.size", callingInst);
				Instruction * newLocalCast = dyn_cast<Instruction>(castArgumentIfNeeded(newLocal, loopSize->getType(), callingInst, funcProperties));
				if (!newLocalCast) return false; // must be an instruction...
				Instruction * remainder = BinaryOperator::Create(Instruction::And, 
														   newLocalCast, 
														   ConstantInt::get(newLocalCast->getType() ,ARCH_VECTOR_WIDTH-1), 
														   "remainder.calc", 
														   callingInst);
				funcProperties->setLoopSizeVal(newLocalCast);

				// Modify all the users of local-size to use the "new" local-size
				dummyVal->replaceAllUsesWith(newLocalCast);
				dummyVal->dropAllReferences();
				V_ASSERT(dummyVal->use_empty());
				delete dummyVal;
				
				// Check if newLocal is smaller than VEC_WIDTH
				ICmpInst * compareForVecWidth = new ICmpInst(callingInst, 
															 CmpInst::ICMP_SLT, 
															 newLocalCast,
															 ConstantInt::get(newLocalCast->getType() ,ARCH_VECTOR_WIDTH),
															 "check.width");
				
				// Split block right after the merge
				BasicBlock * loopHeadStart = compareForVecWidth->getParent();
				BasicBlock * loopHeadCont = loopHeadStart->splitBasicBlock(++(BasicBlock::iterator(compareForVecWidth)));
				BasicBlock * exitTarget = funcProperties->getExitBlock();
				
				// remove the branch which was automatically generated before the split point
				loopHeadStart->getTerminator()->eraseFromParent();
				
				// Generate branch instruction
				BranchInst::Create(scalarLoopTarget, loopHeadCont, compareForVecWidth, loopHeadStart);
				// Attach correct PHI node
				PHINode * countNode = dyn_cast<PHINode>(scalarLoopTarget->begin());
				if (!countNode) return false; // something fishy...
				countNode->addIncoming(ConstantInt::get(countNode->getType() ,0), loopHeadStart);
				
				// Increment the iterations count
				Value * origIter = funcProperties->getIterCountInst();
				Value * newIter = BinaryOperator::Create(Instruction::Add, 
														 origIter, 
														 ConstantInt::get(origIter->getType() ,1), 
														 "iterator.inc", 
														 remainderBlock->getTerminator());
				Value * iterMul = BinaryOperator::Create(Instruction::Shl, 
														 newIter, 
														 ConstantInt::get(origIter->getType() ,LOG_(ARCH_VECTOR_WIDTH)), 
														 "iterator.mul", 
														 remainderBlock->getTerminator());
				Value * newIterCast = castArgumentIfNeeded(iterMul, countNode->getType(), remainderBlock->getTerminator(), funcProperties);

				// Set the iterations-count value at the scalar code to the incremented iterator
				countNode->addIncoming(newIterCast, remainderBlock);
				
				// Compare and branch to scalar code or exit block
				ICmpInst * compareForRemainder = new ICmpInst(remainderBlock->getTerminator(), 
															  CmpInst::ICMP_UGT, 
															  remainder,
															  ConstantInt::get(remainder->getType() ,0),
															  "check.remainder");
				// Replace direct branch with conditional branch
				funcProperties->setProperty(remainderBlock->getTerminator(), PR_INST_IS_REMOVED);
				remainderBlock->getTerminator()->eraseFromParent();
				BranchInst::Create(scalarLoopTarget, exitTarget, compareForRemainder, remainderBlock);			
			}
			else
			{
				return false;
			}
		}
				
		// Remove the fake CALL
		callingInst->dropAllReferences();
		V_ASSERT(callingInst->use_empty());
		funcProperties->setProperty(callingInst, PR_INST_IS_REMOVED);
		callingInst->eraseFromParent();
		
	}
	return true;
}
