/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "vectorize.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for providing vectorized values (ie alrerady converted values) to be used as inputs
// to "currently" converted instructions. 
// If value requires preparation (found, but not vectorized) - prepare it first
// If value is not found - it is in a following block. Provide a dummy value and mark for deferred resolution
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::obtainVectorizedValues(Value ** retValue, bool * retIsConstant, Value * origValue, Instruction * origInst)
{	
	if (isa<Instruction>(origValue))
	{
		*retIsConstant = false; // value is an instruction - not a constant
		if (VCM.count(origValue))
		{
			// Entry is found in VCM
			VCMEntry * foundEntry = VCM[origValue];
			if (foundEntry->vectorValue != NULL)
			{
				*retValue = foundEntry->vectorValue;
			}
			else
			{
				// Vectored value does not exist - create it
				if (foundEntry->isScalarRemoved == false)
				{
					// Original value is kept, so just need to broadcast it
					//    %temp   = insertelement <4 x Type> undef  , Type %value, i32 0
					//    %vector = shufflevector <4 x Type> %temp, <4 x Type> %undef, <4 x i32> <i32 0, i32 0, i32 0, i32 0>
					UndefValue * undefVect = UndefValue::get(VectorType::get(origValue->getType(), ARCH_VECTOR_WIDTH));
					Constant * constIndex = ConstantInt::get(getInt32Ty, 0);
					Value * insertInst = InsertElementInst::Create(undefVect, origValue, constIndex, "temp");
					funcProperties->duplicateProperties(insertInst, origValue);
					Constant * zeroConst32Vector = ConstantVector::get(std::vector<Constant*>(ARCH_VECTOR_WIDTH, constIndex));	
					Value * shuffleInst = new ShuffleVectorInst(insertInst, undefVect, zeroConst32Vector , "vector");
					funcProperties->duplicateProperties(shuffleInst, origValue);
					cast<Instruction>(insertInst)->insertAfter(findInsertPoint(origValue));
					cast<Instruction>(shuffleInst)->insertAfter(cast<Instruction>(insertInst));	
					*retValue = shuffleInst; // Add to return structure
					foundEntry->vectorValue = shuffleInst; // Add to VCM
				}
				else
				{
					// Cannot use original value. Must assemble the multi-scalars
					//   %temp.vect.0 = insertelement <4 x type> undef       , type %scalar.0, i32 0
					//   %temp.vect.1 = insertelement <4 x type> %indx.vect.0, type %scalar.1, i32 1
					//   %temp.vect.2 = insertelement <4 x type> %indx.vect.1, type %scalar.2, i32 2
					//   %temp.vect.3 = insertelement <4 x type> %indx.vect.2, type %scalar.3, i32 3
					V_ASSERT(foundEntry->multiScalarValues[0] != NULL);
					V_ASSERT(foundEntry->multiScalarValues[ARCH_VECTOR_WIDTH-1] != NULL);
					UndefValue * undefVect = UndefValue::get(VectorType::get(origValue->getType(), ARCH_VECTOR_WIDTH));
					
					Instruction * insertPoint = cast<Instruction>(foundEntry->multiScalarValues[ARCH_VECTOR_WIDTH - 1]);
					Value * prevResult = undefVect; // start "building" with an empty vector
					for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
					{
						Value * constIndex = ConstantInt::get(getInt32Ty, i);
						Value * newInst = InsertElementInst::Create(prevResult, foundEntry->multiScalarValues[i] , constIndex, "temp.vect");
						funcProperties->duplicateProperties(newInst, origValue);
						V_ASSERT(newInst != NULL);
						cast<Instruction>(newInst)->insertAfter(findInsertPoint(insertPoint));
						prevResult = newInst;
						insertPoint = cast<Instruction>(newInst);
					}
					foundEntry->vectorValue = insertPoint;
					*retValue = insertPoint;
				}
			}
		}
		else 
		{
			V_ASSERT(origInst != NULL);
			// Entry was not found in VCM. Means it will be defined in a following basic block
			createDummyVectorVal(origValue, retValue);
		}
	}
	else 
	{		
		// Value must be a global/constant/argument/undef
		if (origInst && isa<CallInst>(origInst) && isa<PointerType>(origValue->getType()))
		{
			*retIsConstant = false; // in function calls, pointers may never be considered "constant" 
		}
		else
		{
			*retIsConstant = true; // value is a constant. 
		}
		Value * broadcastedVal;
		// Check if this value is a "proper" constant value, or an Undef
		if (isa<Constant>(origValue))
		{
			// Create a broadcasted constant (no need to make an instruction for this)
			std::vector<Constant *> vectorVal(ARCH_VECTOR_WIDTH, cast<Constant>(origValue));
			broadcastedVal = ConstantVector::get(vectorVal);
		}
		else
		{
			// Check in BAG (broadcast arguments and globals) whether we have this value already 
			if (BAG.count(origValue))
			{
				// Value was already broadcasted. use it
				broadcastedVal = BAG[origValue];
				V_ASSERT(broadcastedVal != NULL);
			}
			else
			{
				// Need to broadcast the value
				//    %temp   = insertelement <4 x Type> undef  , Type %value, i32 0
				//    %vector = shufflevector <4 x Type> %temp, <4 x Type> %undef, <4 x i32> <i32 0, i32 0, i32 0, i32 0>
				UndefValue * undefVect = UndefValue::get(VectorType::get(origValue->getType(), ARCH_VECTOR_WIDTH));
#ifndef SNOWLEOPRD
				Constant * constIndex = ConstantInt::get(getInt32Ty, 0);
				Value * insertInst = InsertElementInst::Create(undefVect, origValue, constIndex, "temp");
#else
				Value * insertInst = InsertElementInst::Create(undefVect, origValue, (unsigned)0, "temp");
#endif
				funcProperties->duplicateProperties(insertInst, origValue);
				Constant * zeroConst32Vector = ConstantVector::get(std::vector<Constant*>(ARCH_VECTOR_WIDTH, constIndex));	
				broadcastedVal = new ShuffleVectorInst(insertInst, undefVect, zeroConst32Vector , "vector");
				funcProperties->duplicateProperties(broadcastedVal, origValue);
				// Place instructions in start of function
				Instruction * firstInstructionInFunc = &*(inst_begin(CURRENT_FUNCTION));
				cast<Instruction>(insertInst)->insertBefore(firstInstructionInFunc);
				cast<Instruction>(broadcastedVal)->insertAfter(cast<Instruction>(insertInst));	
				// Add broadcast to BAG
				BAG.insert(std::pair<Value*,Value*>(origValue, broadcastedVal));
			}
		}
		
		// Put broadcasted constant in returned structure
		*retValue = broadcastedVal;
	}
	V_PRINT("\t\tObtained vectorized value(s) of type: " << *((*retValue)->getType()) << "\n");
	
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for providing vectorized values (ie alrerady converted values) to be used as inputs
// to vectorized call instructions. May also require to cast the types of the vectorized data. 
// If value requires preparation (found, but not vectorized) - prepare it first
// If value is not found - it is in a following block. Provide a dummy value and mark for deferred resolution
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::obtainVectorizedAndCastedValuesForCall(Value ** retValue, bool * retIsConstant, Value * origValue, Instruction * origInst, const Type * targetType)
{
	obtainVectorizedValues(retValue, retIsConstant, origValue, origInst); // Do the "normal" value vectorization
	// Check if type casting is required
	const Type * sourceType = (*retValue)->getType();
	if (sourceType != targetType)
	{
		V_PRINT("\t\tNeed to convert from type: " << *sourceType << "  to type: " << *targetType << "\n");
		Value * convertedVal;
		// Check if source Type is a pointer
		if (isa<PointerType>(sourceType))
		{
			V_ASSERT(0); // Currently, pointer (source) types are not known to have casting needs
		}
		// Else, Check if targetType is a pointer
		else if (isa<PointerType>(targetType))
		{
			V_ASSERT(cast<PointerType>(targetType)->getElementType() == sourceType); // pointer must match "wanted" type 
			// create alloca and place data inside.
			convertedVal = new AllocaInst(sourceType, "alloca_val", origInst);
			funcProperties->duplicateProperties(convertedVal, origInst);
			V_ASSERT(convertedVal->getType() == targetType);
			StoreInst * newSI = new StoreInst(*retValue, convertedVal, origInst);
			funcProperties->duplicateProperties(newSI, origInst);
		}
		// Else, need to cast between types
		else 
		{
			unsigned sourceSize = sourceType->getPrimitiveSizeInBits();
			unsigned targetSize = targetType->getPrimitiveSizeInBits();
			// Check if both types are of the same size
			if (sourceSize == targetSize)
			{
				// Cast data from sourceType to targetType
				convertedVal = new BitCastInst(*retValue, targetType, "cast_val", origInst);
				funcProperties->duplicateProperties(convertedVal, origInst);
			}
			// Else (targetType is larger than sourceType), need to zext source value
			else if (targetSize > sourceSize)
			{
				// Bitcast sourceType to an integer of the same size. Zero-extend to targetSize. Bitcast to target type (if needed)
				Value * firstCast = new BitCastInst(*retValue, IntegerType::get(funcProperties->context(), sourceSize), "cast_src", origInst);
				funcProperties->duplicateProperties(firstCast, origInst);
				Value * zextVal = new ZExtInst(firstCast, IntegerType::get(funcProperties->context(), targetSize), "zext_cast", origInst);
				funcProperties->duplicateProperties(zextVal, origInst);
				if (targetType == IntegerType::get(funcProperties->context(), targetSize))
				{
					convertedVal = zextVal;
				}
				else 
				{
					convertedVal = new BitCastInst(zextVal, targetType, "cast_zext", origInst);
					funcProperties->duplicateProperties(convertedVal, origValue);
				}
			}
			else
			{
				// targetType is smaller than sourceType.Not quite sure what kind of trimming to do. Fail for now...
				return false;
			}
		}
		// Save the "final" values (after conversion) as the values returned to the caller (to be sent to the function call).
		*retValue = convertedVal;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for transforming a vectored functions' return value from whatever the function defines,
// to the "needed" vectored type.
/////////////////////////////////////////////////////////////////////////////////////////////////////
Value * VectorizeFunction::fixCastedReturnTypes(CallInst * newCall, const Type * targetType)
{
	const Type * sourceType = newCall->getType();
	if (sourceType == targetType)
		return newCall;
	
	// Getting here, a cast is needed...
	unsigned sourceSize = sourceType->getPrimitiveSizeInBits();
	unsigned targetSize = targetType->getPrimitiveSizeInBits();
	
	if (sourceSize == targetSize)
	{
		// Simply cast sourceType to targetType
		Instruction * retCast = new BitCastInst(newCall, targetType, "cast_res");
		funcProperties->duplicateProperties(retCast, newCall);
		retCast->insertAfter(newCall);
		return retCast;
	}
	else 
	{
		// need to trim the output to the appropriate size
		V_ASSERT(sourceSize > targetSize);
		Instruction * sourceAsInt;
		if (sourceType != IntegerType::get(funcProperties->context(), sourceSize))
		{
			// first bitcast source to integer
			sourceAsInt = new BitCastInst(newCall, IntegerType::get(funcProperties->context(), sourceSize), "prep_res");
			funcProperties->duplicateProperties(sourceAsInt, newCall);
			sourceAsInt->insertAfter(newCall);
		}
		else 
		{
			sourceAsInt = newCall;
		}
		Instruction * truncVal = new TruncInst(sourceAsInt, IntegerType::get(funcProperties->context(), targetSize), "trunc_res");
		funcProperties->duplicateProperties(truncVal, sourceAsInt);
		truncVal->insertAfter(sourceAsInt);
		if (targetType != IntegerType::get(funcProperties->context(), targetSize))
		{
			Instruction * finalVal = new BitCastInst(truncVal, targetType, "cast_res");
			funcProperties->duplicateProperties(finalVal, truncVal);
			finalVal->insertAfter(truncVal);
			return finalVal;
		}
		else
		{
			return truncVal;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for providing multiple scalar values to be used as inputs to "currently" converted instructions. 
// If value is not an instruction - use "as is" for all instances 
// If value has the original instruction not removed - use it for all instances
// If value only exists in vector form - break it down to scalar elements for usage
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::obtainMultiScalarValues(SmallVectorImpl<Value*> &retValues, bool * retIsConstant, Value * origValue, Instruction * origInst)
{
	if (!isa<Instruction>(origValue))
	{
		if (isa<CallInst>(origInst) && isa<PointerType>(origValue->getType()))
		{
			*retIsConstant = false; // in function calls, pointers may never be considered "constant" 
		}
		else
		{
			*retIsConstant = true; // value is a constant. 
		}
		for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
			retValues.push_back(origValue);
	}
	else
	{
		if (VCM.count(origValue))
		{
			// Entry is found in VCM
			VCMEntry * foundEntry = VCM[origValue];
			if (foundEntry->isScalarRemoved == false)
			{
				// Use the scalar value multiple times
				V_ASSERT(foundEntry->multiScalarValues[0] == NULL); // no need for it to exist
				for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
					retValues.push_back(origValue);
				if (isa<CallInst>(origInst) && isa<PointerType>(origValue->getType()))
				{
					*retIsConstant = false; // in function calls, pointers may never be considered "constant" 
				}
				else
				{
					*retIsConstant = true; // value is a constant. 
				}
			}
			else 
			{
				*retIsConstant = false; // value was modified
				// Must either find multi-scalar values, or break down the vector value
				if (foundEntry->multiScalarValues[0] != NULL)
				{
					// found pre-prepared multi-scalar values
					for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
						retValues.push_back(foundEntry->multiScalarValues[i]);
				}
				else
				{
					// Failed to find multi-scalar values. Need to break down the vectorized value instead.
					//     %extract0 = extractelement <4 x Type> %vector, i32 0
					//     %extract1 = extractelement <4 x Type> %vector, i32 1
					//     %extract2 = extractelement <4 x Type> %vector, i32 2
					//     %extract3 = extractelement <4 x Type> %vector, i32 3
					V_ASSERT(foundEntry->vectorValue != NULL);
					Instruction * insertPoint = findInsertPoint(cast<Instruction>(foundEntry->vectorValue));
					for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
					{
						Value * constIndex = ConstantInt::get(getInt32Ty, i);
						retValues.push_back(ExtractElementInst::Create(foundEntry->vectorValue, constIndex, "extract"));
						funcProperties->duplicateProperties(retValues[i], foundEntry->vectorValue);
						cast<Instruction>(retValues[i])->insertAfter(insertPoint);
						insertPoint = cast<Instruction>(retValues[i]);
						foundEntry->multiScalarValues[i] = retValues[i]; // Also add back new values to VCM
					}
				}
			}
		}
		else
		{
			V_ASSERT(origInst != NULL);
			*retIsConstant = false; 
			// Entry was not found in VCM. Means it will be defined in a following basic block
			createDummyMultiScalarVals(origValue, retValues);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for providing a non-vectorized value (after it was perhaps broadcasted and removed)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::obtainNonVectorizedValue(Value * origValue, Value ** retValue)
{
	if (!isa<Instruction>(origValue))
	{
		*retValue = origValue;
		return true;
	}
	if (funcProperties->getProperty(origValue, PR_TID_DEPEND))
	{
		// Somehow, a TID-dependent value was requested as not vectorized. Wierd.
		V_UNEXPECTED("TID-dependent value was requested as not vectorized");
		return false;
	}
	if (!VCM.count(origValue))
	{
		// Value was not declared yet. For now, we do not support this.
		V_UNEXPECTED("Value was not declared yet");
		return false;
	}
	VCMEntry * foundEntry = VCM[origValue];
	if (foundEntry->isScalarRemoved == false)
	{
		// Original value is still out there, can be used...
		*retValue = origValue;
		return true;
	}
	else if (foundEntry->multiScalarValues[0] != NULL)
	{
		// Was replaced by multi-scalars. They should all be the same value. Use the first one...
		*retValue = foundEntry->multiScalarValues[0];
		return true;		
	}
	else
	{
		V_ASSERT(foundEntry->vectorValue != NULL)
		// Was replaced by vector. Should be a broadcast. Use the first element...
		V_ASSERT(isa<VectorType>(foundEntry->vectorValue->getType()));
		Value * constIndex = ConstantInt::get(getInt32Ty, 0);
		Instruction * extract = ExtractElementInst::Create(foundEntry->vectorValue, constIndex, "extractFromBroadcast");
		funcProperties->duplicateProperties(extract, foundEntry->vectorValue);
		extract->insertAfter(findInsertPoint(foundEntry->vectorValue));
		*retValue = extract;
		return true;		
	}
}
	

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Return TRUE if value appears in VCM (= was vectorized), and appears in vector form, or is a 
// constant. Also return whether the scalar was removed or not (just for TRUE retvals)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::existVectorizedOrConstantValues(Value * origValue, bool * isScalarRemoved)
{
	if (isa<Instruction>(origValue))
	{
		if (VCM.count(origValue))
		{
			// Entry is found in VCM
			VCMEntry * foundEntry = VCM[origValue];
			if (foundEntry->vectorValue != NULL)
			{
				// Vecotred value is found 
				*isScalarRemoved = foundEntry->isScalarRemoved;
				return true;
			}
		}
	}
	else
	{
		// not an instruction. both scalar and vector values are usable
		*isScalarRemoved = false;
		return true;
	}
	return false;				
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if instruction's input have not been vectorized already
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::checkPossibilityToUseOriginalInstruction(Instruction * origInst)
{
	bool allInputsScalarExist = true;
	bool isRemoved;
	for (User::op_iterator i = origInst->op_begin(), e = origInst->op_end(); i != e; ++i) 
	{
		Value *v = *i;
		if (existVectorizedOrConstantValues(v, &isRemoved) && isRemoved)
		{
			// at least one input value was already vectorized. must vectorize entire instruction...
			allInputsScalarExist = false;
			break;
		}
	}
	if (!allInputsScalarExist) return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for using the existing instruction without modifying it (updates VCM accordingly)
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::useOriginalConstantInstruction(Instruction * origInst)
{
	V_PRINT("\t\tNot Duplicated Instruction\n");
	V_ASSERT(VCM.count(origInst) == 0); //  should not be appearing in VCM already
	VCMEntry * newEntry = allocateNewVCMEntry();
	newEntry->isScalarRemoved = false;
	VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for creating VCM entry and filling it with vectored values (original is removed)  
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::createVCMEntryWithVectorValues(Instruction * origInst, Value * vectoredValue)
{
	V_ASSERT(VCM.count(origInst) == 0); //  should not be appearing in VCM already
	VCMEntry * newEntry = allocateNewVCMEntry();
	newEntry->isScalarRemoved = true;
	newEntry->vectorValue = vectoredValue;
	VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for creating VCM entry and filling it with multiple-scalar values (original is removed)  
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::createVCMEntryWithMultiScalarValues(Instruction * origInst, SmallVectorImpl<Value*> &multiScalarValues)
{
	V_ASSERT(VCM.count(origInst) == 0); //  should not be appearing in VCM already
	VCMEntry * newEntry = allocateNewVCMEntry();
	newEntry->isScalarRemoved = true;
	for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
		newEntry->multiScalarValues[i] = multiScalarValues[i];
	VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for creating VCM entry and filling it with both vector values and 
// multiple-scalar values (original is removed)  
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::createVCMEntryWithVectorAndMultiScalarValues(Instruction * origInst, Value * vectoredValue, SmallVectorImpl<Value*> &multiScalarValues)
{
	V_ASSERT(VCM.count(origInst) == 0); //  should not be appearing in VCM already
	VCMEntry * newEntry = allocateNewVCMEntry();
	newEntry->isScalarRemoved = true;
	newEntry->vectorValue = vectoredValue;
	for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
		newEntry->multiScalarValues[i] = multiScalarValues[i];
	VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));	
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for allocating a clean VCM entry  
/////////////////////////////////////////////////////////////////////////////////////////////////////
VectorizeFunction::VCMEntry* VectorizeFunction::allocateNewVCMEntry()
{
	// If index of next free VCMEntry is over the array size, create a new array
	if (VCMArrayLocation == ESTIMATED_INST_NUM)
	{
		// Create new VCMAllocationArray, push it to the vector of arrays, and set free index to 0
		VCMAllocationArray = new VCMEntry[ESTIMATED_INST_NUM];
		VCMArrays.push_back(VCMAllocationArray);
		VCMArrayLocation = 0;
	}
	VCMEntry * newEntry = &(VCMAllocationArray[VCMArrayLocation++]);
	// Make sure the entry is clean
	newEntry->vectorValue = NULL;
	newEntry->multiScalarValues[0] = NULL;
	return newEntry;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Method for releasing allocations of VCM entries  
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::releaseAllVCMEntries()
{
	V_ASSERT(VCMArrays.size() > 0); // At least one buffer is allocated
	while (VCMArrays.size() > 1)
	{
		// If there are additional allocated entry Arrays, release all of them (leave only the first)
		VCMEntry * popEntry = VCMArrays.pop_back_val();
		delete[] popEntry;
	}
	VCMAllocationArray = VCMArrays[0];  // set the "current" array pointer to the only remaining array
	VCMArrayLocation = 0;  // set the free-entry index to 0
}





/////////////////////////////////////////////////////////////////////////////////////////////////////
// Given a value, find the instruction after which this value is declared, and further instructions 
// can be placed. This is an "issue" when the location found is in the "middle" of PHI instructions
// in which case the return location must be after all the PHI insts are done
/////////////////////////////////////////////////////////////////////////////////////////////////////
Instruction * VectorizeFunction::findInsertPoint(Value * val)
{
	if (isa<Instruction>(val))
	{
		if (!isa<PHINode>(val))
		{
			return cast<Instruction>(val);
		}
		// Find last PHI instruction in the basic block
		BasicBlock * instBB = cast<Instruction>(val)->getParent();
		BasicBlock::iterator iter = instBB->begin();
		Instruction * current = &*iter;
		while(true)
		{
			++iter;
			if (!isa<PHINode>(iter))
				break;
			current = &*iter;
		}
		return current;
	}
	else 
	{
		return &*(inst_begin(CURRENT_FUNCTION));
	}
}







/////////////////////////////////////////////////////////////////////////////////////////////////////
// Create dummy values (to be replaced by "real" values after function vectorizing is complete) 
// All dummy values are Load instructions.
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::createDummyVectorVal(Value * origValue, Value ** vectorVal)
{
	V_ASSERT(!isa<VectorType>(origValue->getType()));
	
	VCMEntry * dummyEntry;
	
	// First, try to find if the needed dummy values already exist
	if (DeferredResMap.count(origValue))
	{
		dummyEntry = DeferredResMap[origValue];
		if (dummyEntry->vectorValue != NULL)
		{
			// Dummy values already exist. use them..
			V_PRINT("\t\tFound existing Dummy Vector value/s \n");
			*vectorVal = dummyEntry->vectorValue;
			return;
		}
	}
	else
	{
		dummyEntry = allocateNewVCMEntry();
	}
	
	// Create the dummy values and place them in VCMEntry
	const Type * vectorType = VectorType::get(origValue->getType(), ARCH_VECTOR_WIDTH);
	const Type * ptrType = PointerType::get(vectorType, 0);
	V_PRINT("\t\tCreate Dummy Vector value/s (of type " << *vectorType << ")\n");
	Constant * subExpr = ConstantExpr::getIntToPtr(ConstantInt::get(getInt32Ty,APInt(32, 0xdeadbeef)), ptrType);

	dummyEntry->vectorValue = new LoadInst(subExpr);
	*vectorVal = dummyEntry->vectorValue;
	funcProperties->duplicateProperties(*vectorVal, origValue);
	
	// Insert into deferred resolution map/list
	DeferredResMap.insert(std::pair<Value *, VCMEntry *>(origValue, dummyEntry));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Create dummy values (to be replaced by "real" values after function vectorizing is complete) 
// All dummy values are Load instructions.
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectorizeFunction::createDummyMultiScalarVals(Value * origValue, SmallVectorImpl<Value*> &multiScalarVals)
{
	VCMEntry * dummyEntry;// = allocateNewVCMEntry();
	
	// First, try to find if the needed dummy values already exist
	if (DeferredResMap.count(origValue))
	{
		dummyEntry = DeferredResMap[origValue];
		if (dummyEntry->multiScalarValues[0] != NULL)
		{
			// Dummy values already exist. use them..
			V_PRINT("\t\tFound existing Dummy Multi-scalar value/s \n");
			for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
				multiScalarVals.push_back(dummyEntry->multiScalarValues[i]);
			return;
		}
	}
	else
	{
		dummyEntry = allocateNewVCMEntry();
	}
	
	// Create the dummy values and place them in VCMEntry
	const Type * ptrType = PointerType::get(origValue->getType(), 0);
	V_PRINT("\t\tCreate Dummy value/s (of type " << *(origValue->getType()) << ")\n");
	Constant * subExpr = ConstantExpr::getIntToPtr(ConstantInt::get(getInt32Ty,APInt(32, 0xdeadbeef)), ptrType);

	for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; i++)
	{
		dummyEntry->multiScalarValues[i] = new LoadInst(subExpr);
		multiScalarVals.push_back(dummyEntry->multiScalarValues[i]);
		funcProperties->duplicateProperties(multiScalarVals[i], origValue);
	}
	
	// Insert into deferred resolution map/list
	DeferredResMap.insert(std::pair<Value *, VCMEntry *>(origValue, dummyEntry));
}





/////////////////////////////////////////////////////////////////////////////////////////////////////
// Called after finishing the first pass over the entire function, this method will replace all
// dummy values with the actual needed (vectorized) values.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::resolveDeferredInstructions()
{
	// Iterate over the deferred insts list, and resolve them
	for (DenseMap<Value *, VCMEntry *>::iterator currIter = DeferredResMap.begin(), endIter = DeferredResMap.end();
		 currIter != endIter; ++currIter)
	{
		Value * origVal = currIter->first;
		VCMEntry * dummyEntry = currIter->second;
		V_PRINT("\tDRL Going to fix value of orig inst: " << *origVal << "\n");
		
		if (!VCM.count(origVal))
		{
			// The target instruction is not in VCM. Not sure how this could happen
			V_UNEXPECTED("The target instruction is not in VCM");
			return false;
		}
		
		// check (in the dummy values) if vectorized values are required
		if (dummyEntry->vectorValue != NULL)
		{
			Value * resolvedVal;
			bool dummyBool;
			// Plcing "NULL" as the "origInst" value in obtainVectorizedValues should be safe, as long as we know there is a VCM entry...
			obtainVectorizedValues(&resolvedVal, &dummyBool, origVal, NULL); 
			
			dummyEntry->vectorValue->replaceAllUsesWith(resolvedVal);
			V_ASSERT(cast<Instruction>(dummyEntry->vectorValue)->use_empty());
			delete cast<Instruction>(dummyEntry->vectorValue); // Deleted and not "erased from parent" - because dummy was never insert to function!
		}
		
		// check (in the dummy values) if multi scalar values are required
		if (dummyEntry->multiScalarValues[0] != NULL)
		{
			SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> resolvedVals;
			bool dummyBool;
			// Plcing "NULL" as the "origInst" value in obtainMultiScalarValues should be safe, as long as we know there is a VCM entry...
			obtainMultiScalarValues(resolvedVals, &dummyBool, origVal, NULL); 
			
			// Replace dummy values with proper values
			for (unsigned i = 0; i < ARCH_VECTOR_WIDTH; ++i)
			{
				dummyEntry->multiScalarValues[i]->replaceAllUsesWith(resolvedVals[i]);
				V_ASSERT(cast<Instruction>(dummyEntry->multiScalarValues[i])->use_empty());
				delete cast<Instruction>(dummyEntry->multiScalarValues[i]); // Deleted and not "erased from parent" - because dummy was never insert to function!
			}
		}		
	}
	return true;
}








