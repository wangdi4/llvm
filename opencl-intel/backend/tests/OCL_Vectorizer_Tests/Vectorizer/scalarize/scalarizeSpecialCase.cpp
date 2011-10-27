/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "scalarize.h"
#include "../specialCaseFuncs.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Special case functions handler dispach
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseFunctionCall(Instruction * I)
{
	CallInst * callerInst = cast<CallInst>(I);
	// dispatch according to kind of special function
	instProperty specialCaseType = funcProperties->getPropertyGroup(I, PR_ALL_SPECIAL_CASE_FUNCS);
	switch (specialCaseType) {
		case PR_SC_READ_SAMPLER_F_2D:
			return markSpecialCaseReadSample(callerInst, true);
			break;
		case PR_SC_READ_SAMPLER_F_3D:
			return markSpecialCaseReadSample(callerInst, false);
			break;
		case PR_SC_WRITEF_SAMPLER:
			return markSpecialCaseWriteSample(callerInst);
			break;
		case PR_SC_CI_GAMMA:
			return markSpecialCaseCIGamma(callerInst);
			break;
		case PR_SC_SELECT_INST:
			return markSpecialCaseSelect(callerInst);
			break;
		case PR_SC_DOTPROD:
			return markSpecialCaseGeometricFunc(callerInst, DOT_PRODUCT_FAKE_SCALAR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_DISTANCE:
			return markSpecialCaseGeometricFunc(callerInst, DISTANCE_FAKE_SCALAR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_FAST_DISTANCE:
			return markSpecialCaseGeometricFunc(callerInst, FAST_DISTANCE_FAKE_SCALAR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_LENGTH:
			return markSpecialCaseGeometricFunc(callerInst, LENGTH_FAKE_SCALAR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_FAST_LENGTH:
			return markSpecialCaseGeometricFunc(callerInst, FAST_LENGTH_FAKE_SCALAR_NAME_PREFIX, true);
			break;
		case PR_SC_CROSS:
			return markSpecialCaseGeometricFunc(callerInst, CROSS_PRODUCT_FAKE_SCALAR_NAME_PREFIX, false);
			break;
		case PR_SC_GEO_NORMALIZE:
			return markSpecialCaseGeometricFunc(callerInst, NORMALIZE_FAKE_SCALAR_NAME_PREFIX, false);
			break;
		case PR_SC_GEO_FAST_NORMALIZE:
			return markSpecialCaseGeometricFunc(callerInst, FAST_NORMALIZE_FAKE_SCALAR_NAME_PREFIX, false);
			break;
		case PR_SC_FRACT:
			return MarkSpecialCaseFract(callerInst);
			break;
		default:
			break;
	}
	
	V_UNEXPECTED("No handling for Special case function"); // Should not happen! we only reach this func if the function IS a special case func
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Vectorization of special-case read sampler. Simply prepare all the arguments and replace
// the original function call with a fake scalar function call.
// Function handles both 2D and 3D samplers
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseReadSample(CallInst * caller, bool is2D)
{
	// Instruction has the following data:   <4xfloat> read_sample(image, sampler, <2/4 x float> coordinates)
	// Need to (sometimes) go thru casts/etc to find the "original" data. Phases:
	// 1) Find the scalar-version of the input coordinates. If they do not exist - create them.
	// 2) Create a fake read sampler, which takes as input the scalar root values. return value is like in the original sampler
	// 3) Create a breakdown of the return value to 4 separate elements, and a buildup back to a vector, and create SCM entry as if this inst was scalarized
	// 4) Save list of input coordinates (to be used in vectorization phase)
	V_PRINT("\t\t\tREAD Sampler. Caller:" << *caller << "\n");
		
	// Find the scalar origins of the coordinates
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	V_ASSERT(rootValues->size() == 2); // expecting 2 values: return value and the coordinates
	Value * retVal = (*rootValues)[0];
	Value * properCoordinates = (*rootValues)[1];
	
	Value * scalarVals[4]; // only 2 needed for 2D, but we set 4 to accomodate 3D as well 
	bool retIsConstant;
	obtainScalarizedValues(scalarVals, &retIsConstant, properCoordinates, caller);
	
	// Find or Create the fake read-sampler function
	Constant * fakeReadSampFunc;
	if (is2D)
	{
		fakeReadSampFunc = CURRENT_MODULE->getFunction(FAKE_SCALAR_READ_IMAGEF_2D_NAME);
	}
	else
	{
		fakeReadSampFunc = CURRENT_MODULE->getFunction(FAKE_SCALAR_READ_IMAGEF_3D_NAME);
	}
	if (!fakeReadSampFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(caller->getOperand(1)->getType()); // image pointer type
		funcArgs.push_back(caller->getOperand(2)->getType()); // sampler-type type
		funcArgs.push_back(getFloatTy); // X-coordinate type
		funcArgs.push_back(getFloatTy); // Y-coordinate type
		if (is2D)
		{
			FunctionType * readSampFuncType = FunctionType::get(READ_IMAGEF_RET_TYPE, funcArgs, false);
			fakeReadSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_SCALAR_READ_IMAGEF_2D_NAME, readSampFuncType);
		}
		else
		{
			funcArgs.push_back(getFloatTy); // Z-coordinate type
			FunctionType * readSampFuncType = FunctionType::get(READ_IMAGEF_RET_TYPE, funcArgs, false);
			fakeReadSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_SCALAR_READ_IMAGEF_3D_NAME, readSampFuncType);
		}
		funcProperties->addFakeFunctionToList(cast<Function>(fakeReadSampFunc));
	}
	V_ASSERT(fakeReadSampFunc);
	
	// Prepare arguments for fake read sampler
	std::vector<Value *> newArgs;
	newArgs.push_back(caller->getOperand(1)); // image pointer
	newArgs.push_back(caller->getOperand(2)); // sampler-type
	newArgs.push_back(scalarVals[0]); // X-coordinate
	newArgs.push_back(scalarVals[1]); // Y-coordinate
	if (!is2D)
		newArgs.push_back(scalarVals[2]); // Z-coordinate
		
	
	// Create call to fake read sampler
	Instruction * fakeReadSampCall = CallInst::Create(fakeReadSampFunc, newArgs.begin(), newArgs.end(), "fake.scalar.readsampler", caller);
	funcProperties->replaceAllInstProperties(caller, fakeReadSampCall);
	funcProperties->setProperty(fakeReadSampCall, PR_NO_SIDE_EFFECT);

	// Create breakdown of return value (of fake sampler) to 4 elements
	//   %scalar0 = extractelement <4 x Type> %vector, i32 0
	//   %scalar1 = extractelement <4 x Type> %vector, i32 1 
	//   %scalar2 = extractelement <4 x Type> %vector, i32 2
	//   %scalar3 = extractelement <4 x Type> %vector, i32 3 
	Value * newExtractInsts[4];
	for (unsigned i = 0; i < 4; i++)
	{
		Value * constIndex = ConstantInt::get(getInt32Ty, i);			
		newExtractInsts[i] = ExtractElementInst::Create(fakeReadSampCall, constIndex, "extract.fake.smp", caller);
	}

	// Check if function's return type was (in the vector function call) in a non-desired type
	if (retVal != caller)
	{
		if (caller->getType() == getVoidTy)
		{
			// return value is passed thru a pointer (first argument)
			V_ASSERT(isa<PointerType>(caller->getOperand(1)->getType())); // first operand really is a pointer
			V_ASSERT(isa<LoadInst>(retVal)); // the actual value is a result of load
			V_ASSERT(cast<LoadInst>(retVal)->getPointerOperand() == caller->getOperand(1)); // the loaded address matches the first arument
		}
		else
		{
			// iterate over all the users of the original return value, until we find the "proper" return value. mark all others for removal		
			V_ASSERT(caller->hasOneUse());
			Value * currentVal = *(caller->use_begin());
			while (currentVal != retVal)
			{
				V_ASSERT(currentVal->hasOneUse());
				funcProperties->setProperty(currentVal, PR_FUNC_PREP_TO_REMOVE);
				currentVal = *(currentVal->use_begin());
			}
		}
		// mark the actual "proper" return value as removal candidate
		funcProperties->setProperty(retVal, PR_FUNC_PREP_TO_REMOVE);			
	}
	
	
	
	// mark the extracts as scalar breakdowns of the sampler value..
	SCMEntry * newSamplerSCMEntry = createEmptySCMEntry(retVal);
	updateSCMEntryWithValues(newSamplerSCMEntry, newExtractInsts, true);

	SCMEntry * newFakeSamplerSCMEntry = createEmptySCMEntry(fakeReadSampCall);
	updateSCMEntryWithValues(newFakeSamplerSCMEntry, newExtractInsts, true);

	// Erase "original" read sampler
	removedInsts.insert(caller);	
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Vectorization of special-case write sampler. Simply prepare all the arguments and replace
// the original function call with a fake scalar function call.
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseWriteSample(CallInst * caller)
{
	// Instruction has the following data:   void write_sample(image, <2 x i32> coords, <4xfloat> color)
	// Need to (sometimes) go thru casts/etc to find the "original" data. Phases:
	// 1) Find the scalar-version of the input coordinates and colors. If they do not exist - create them.
	// 2) Create a fake write sampler, which takes as input the scalar root values.
	// 3) Save list of scalar inputs  (to be used in vectorization phase)
	V_ASSERT(caller->getNumUses() == 0); // sanity check: this function does not return a value..
	
	// Find the scalar origins of the coordinates and colors
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	V_ASSERT(rootValues->size() == 2); // expecting 2 values: the coordinates and the colors
	Value * properCoordinates = (*rootValues)[0];
	Value * properColors = (*rootValues)[1];
		
	Value * scalarCoordinates[2];
	bool retIsConstant;
	obtainScalarizedValues(scalarCoordinates, &retIsConstant, properCoordinates, caller);
	V_PRINT("\t\t\tWRITE Sampler. Scalar coord x:  " << *scalarCoordinates[0] << "\n");
	V_PRINT("\t\t\tWRITE Sampler. Scalar coord y:  " << *scalarCoordinates[1] << "\n");
	
	Value * scalarColors[4];
	obtainScalarizedValues(scalarColors, &retIsConstant, properColors, caller);
	V_PRINT("\t\t\tWRITE Sampler. Scalar color r:  " << *scalarColors[0] << "\n");
	V_PRINT("\t\t\tWRITE Sampler. Scalar color g:  " << *scalarColors[1] << "\n");
	V_PRINT("\t\t\tWRITE Sampler. Scalar color b:  " << *scalarColors[2] << "\n");
	V_PRINT("\t\t\tWRITE Sampler. Scalar color a:  " << *scalarColors[3] << "\n");
	
	// Find or Create the fake write-sampler function
	Constant * fakeWriteSampFunc = CURRENT_MODULE->getFunction(FAKE_SCALAR_WRITE_IMAGEF_NAME);
	if (!fakeWriteSampFunc)
	{
		// Function was not declared yet. Fist create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(caller->getOperand(1)->getType()); // image pointer type
		funcArgs.push_back(getInt32Ty); // X-coordinate type
		funcArgs.push_back(getInt32Ty); // Y-coordinate type
		funcArgs.push_back(getFloatTy); // R-color type
		funcArgs.push_back(getFloatTy); // G-color type
		funcArgs.push_back(getFloatTy); // B-color type
		funcArgs.push_back(getFloatTy); // A-color type
		FunctionType * writeSampFuncType = FunctionType::get(getVoidTy, funcArgs, false);
		fakeWriteSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_SCALAR_WRITE_IMAGEF_NAME, writeSampFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeWriteSampFunc));
	}
	V_ASSERT(fakeWriteSampFunc);
	
	// Prepare arguments for fake write sampler
	std::vector<Value *> newArgs;
	newArgs.push_back(caller->getOperand(1)); // image pointer
	newArgs.push_back(scalarCoordinates[0]); // X-coordinate
	newArgs.push_back(scalarCoordinates[1]); // Y-coordinate
	newArgs.push_back(scalarColors[0]); // R-color
	newArgs.push_back(scalarColors[1]); // G-color
	newArgs.push_back(scalarColors[2]); // B-color
	newArgs.push_back(scalarColors[3]); // A-color
	
	// Create call to fake write sampler
	Instruction * fakeWriteSampCall = CallInst::Create(fakeWriteSampFunc, newArgs.begin(), newArgs.end(), "", caller);
	funcProperties->replaceAllInstProperties(caller, fakeWriteSampCall);
	
	// Erase "original" write sampler
	removedInsts.insert(caller);	
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace calls to select function with call to non-existing function which receives the "root"
// arguments. This is used for simplifying optimizations later on. 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseSelect(CallInst * caller)
{
	// Instruction has the following data:   <N x type> select(<N x type> X, <N x type> Y, <N x intType> Mask)
	// Need to (sometimes) go thru casts/etc to find the "original" data. 
	// The select may also be scalar. For those functions, need to check if the mask was generated as an i1 value
	// which was zext to the needed width. If not - reject the function. Otherwise - replace the zext with sext
	V_PRINT("\t\t\tSpecial-Case Select function detected\n");
	
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	V_ASSERT(rootValues->size() == 4); // expecting return value and 3 inputs
	
	Value * retVal = (*rootValues)[0];
	Value * inputXVal = (*rootValues)[1];
	Value * inputYVal = (*rootValues)[2];
	Value * inputMaskVal = (*rootValues)[3];	
	
	const Type * retType = retVal->getType();
	const Type * maskType = inputMaskVal->getType();
	const Type * scalarValsType;
	const Type * scalarMaskType;
	
	unsigned vecWidth;
	if (!isa<VectorType>(retType)) 
	{
		vecWidth = 1;
		scalarValsType = retType;
		scalarMaskType = maskType;
		// If this is a scalar inst, we must not take the values from rootValues!
		retVal = caller;
		inputXVal = caller->getOperand(1);
		inputYVal = caller->getOperand(2);
		inputMaskVal = caller->getOperand(3);
	}
	else 
	{
		vecWidth = cast<VectorType>(retType)->getNumElements();
		scalarValsType = cast<VectorType>(retType)->getElementType();
		scalarMaskType = cast<VectorType>(maskType)->getElementType();
	}
	
	// If vector width is scalar - need special treatment first
	if (vecWidth == 1)
	{
		// Check if the root of the mask is a sext or zext instruction for i1 origin value
		Instruction * maskInst = dyn_cast<Instruction>(inputMaskVal);
		if (!maskInst || 
			(!isa<ZExtInst>(maskInst) && !isa<SExtInst>(maskInst)) ||
			maskInst->getOperand(0)->getType() != getInt1Ty)
		{
			// unset the SC property from the call instruction and leave
			funcProperties->clearProperty(caller, PR_SPECIAL_CASE_BUILT_IN);
			funcProperties->clearProperty(caller, PR_SC_SELECT_INST);
			return true;
		}
		if (isa<ZExtInst>(maskInst))
		{
			Value * signExtend = new SExtInst(maskInst->getOperand(0), maskInst->getType(), "sign.extend", caller);
			funcProperties->duplicateProperties(signExtend, caller);
			inputMaskVal = signExtend;
		}
	}
	
	// Find the scalar origins of the arguments
	V_ASSERT(vecWidth <= MAX_OCL_VECTOR_WIDTH)
	Value * scalarArgs[3][MAX_OCL_VECTOR_WIDTH];
	if (vecWidth > 1)
	{
		bool retIsConstant0, retIsConstant1, retIsConstant2;
		obtainScalarizedValues(scalarArgs[0], &retIsConstant0, inputXVal, caller);
		obtainScalarizedValues(scalarArgs[1], &retIsConstant1, inputYVal, caller);
		obtainScalarizedValues(scalarArgs[2], &retIsConstant2, inputMaskVal, caller);
		if (retIsConstant0 && retIsConstant1 && retIsConstant2)
		{
			funcProperties->clearProperty(caller, PR_SPECIAL_CASE_BUILT_IN);
			funcProperties->clearProperty(caller, PR_SC_SELECT_INST);
			return true; //dont Scalarize a constant calculation
		}
	}
	else
	{
		// get the scalar operands which are currently used
		scalarArgs[0][0] = inputXVal;
		scalarArgs[1][0] = inputYVal;
		scalarArgs[2][0] = inputMaskVal;
	}
	
	// Obtain proper "fake" select function
	std::stringstream fakeFuncStream;
	if (scalarValsType == getFloatTy)
	{
		fakeFuncStream << SELECT_FAKE_SCALAR_NAME_PREFIX << "f";
	}
	else
	{
		fakeFuncStream << SELECT_FAKE_SCALAR_NAME_PREFIX << cast<IntegerType>(scalarValsType)->getBitWidth();
	}
	
	// Declare or use if already declared, the fake function
	Constant * fakeSelectFunc = CURRENT_MODULE->getFunction(fakeFuncStream.str());
	if (!fakeSelectFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(scalarValsType);
		funcArgs.push_back(scalarValsType);
		funcArgs.push_back(scalarMaskType);
		FunctionType * selectFuncType = FunctionType::get(scalarValsType, funcArgs, false);
		fakeSelectFunc = CURRENT_MODULE->getOrInsertFunction(fakeFuncStream.str(), selectFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeSelectFunc));
	}
	V_ASSERT(fakeSelectFunc);

	unsigned numFuncs = vecWidth;
	// If all instructions do EXACTLY the same thing, throw away most of them...
	if (numFuncs>1 && allValuesAreEqual(scalarArgs[0],vecWidth) && allValuesAreEqual(scalarArgs[1],vecWidth) && allValuesAreEqual(scalarArgs[2],vecWidth))
	{
		numFuncs = 1;
	}

	// Prepare arguments and call the fake function 
	Value * newFuncCalls[MAX_OCL_VECTOR_WIDTH];
	for (unsigned i = 0; i < numFuncs; i++)
	{
		std::vector<Value *> newArgs;
		newArgs.push_back(scalarArgs[0][i]);
		newArgs.push_back(scalarArgs[1][i]);
		newArgs.push_back(scalarArgs[2][i]);
		
		// generate call to fake function
		newFuncCalls[i] = CallInst::Create(fakeSelectFunc, newArgs.begin(), newArgs.end(), "fake.scalar.select", caller);
		funcProperties->duplicateProperties(newFuncCalls[i], caller);
		funcProperties->setProperty(newFuncCalls[i], PR_NO_SIDE_EFFECT);
	}
	
	// If all instructions were the same, only 1 call was created. duplicate its value...
	if (numFuncs == 1 && vecWidth > 1)
	{
		for (unsigned i = 1; i < vecWidth; i++)
		{
			newFuncCalls[i] = newFuncCalls[0]; // duplicate the same call
		}
	}
	
	// Check if function's return type was (in the vector function call) in a non-desired type
	if (retVal != caller)
	{
		if (caller->getType() == getVoidTy)
		{
			// return value is passed thru a pointer (first argument)
			V_ASSERT(isa<PointerType>(caller->getOperand(1)->getType())); // first operand really is a pointer
			V_ASSERT(isa<LoadInst>(retVal)); // the actual value is a result of load
			V_ASSERT(cast<LoadInst>(retVal)->getPointerOperand() == caller->getOperand(1)); // the loaded address matches the first arument
		}
		else
		{
			// iterate over all the users of the original return value, until we find the "proper" return value. mark all others for removal		
			V_ASSERT(caller->hasOneUse());
			Value * currentVal = *(caller->use_begin());
			while (currentVal != retVal)
			{
				V_ASSERT(currentVal->hasOneUse());
				funcProperties->setProperty(currentVal, PR_FUNC_PREP_TO_REMOVE);
				currentVal = *(currentVal->use_begin());
			}
		}
		// mark the actual "proper" return value as removal candidate
		funcProperties->setProperty(retVal, PR_FUNC_PREP_TO_REMOVE);			
	}
	
	if (vecWidth > 1)
	{
		// Add new value/s to SCM
		SCMEntry * newEntry = createEmptySCMEntry(retVal);
		updateSCMEntryWithValues(newEntry, newFuncCalls, true);
	}
	else
	{
		// Replace all users of original CALL with new fake call
		caller->replaceAllUsesWith(newFuncCalls[0]);		
	}
	
	// Remove original instruction
	removedInsts.insert(caller);
	return true;	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace call to geometric func with call to non-existing function which receives the "root"
// arguments. This is used for simplifying optimizations later on. 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseGeometricFunc(CallInst * caller, const char *fake_func_name_prefix, bool isReducted)
{
	// Need to (sometimes) go thru casts/etc to find the "original" data. Phases:
	// 1) Find the scalar-version of the input arguments. If they do not exist - create them.
	// 2) Create a fake func, which takes as input the scalar root values.
	// 3) Save list of scalar inputs  (to be used in vectorization phase)
	V_PRINT("\t\t\tSpecial-Case Geometric function detected\n");
	
	// Find the scalar origins of the arguments
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	unsigned numArguments = rootValues->size() - 1; // reduce the return-value from the amount
	V_ASSERT(numArguments == 1 || numArguments == 2); // Geometric funcs have one or two arguments 
	Value * retVal = (*rootValues)[0];
	
	// Find the vector width of the original instruction
	unsigned vecWidth;
	if (!isa<VectorType>((*rootValues)[1]->getType()))	{
		vecWidth = 1; // this is a geometric calculation of scalars
	}
	else {
		vecWidth = cast<VectorType>((*rootValues)[1]->getType())->getNumElements();
	}
	// Create place-holders for the scalar value pointers
	V_ASSERT(vecWidth <= MAX_OCL_VECTOR_WIDTH);
	Value * scalarArgs[2][MAX_OCL_VECTOR_WIDTH];
	
	if (vecWidth > 1)
	{
		bool dummy;
		for (unsigned i = 0; i < numArguments; i++)
		{
			obtainScalarizedValues(scalarArgs[i], &dummy, (*rootValues)[i+1], caller);
		}
	}
	else
	{
		for (unsigned i = 0; i < numArguments; i++)
		{
			scalarArgs[i][0] = caller->getOperand(i+1);
		}
	}
	
	// Obtain proper "fake" geometric function
	std::stringstream fakeFuncStream;
	fakeFuncStream << fake_func_name_prefix << vecWidth;
	
	// Declare or use if already declared, the fake function
	Constant * fakeGeometricFunc = CURRENT_MODULE->getFunction(fakeFuncStream.str());
	if (!fakeGeometricFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs(vecWidth*numArguments, getFloatTy);
		const Type * retType = (isReducted ? getFloatTy : (*rootValues)[0]->getType());
		FunctionType * geometricFuncType = FunctionType::get(retType, funcArgs, false);
		fakeGeometricFunc = CURRENT_MODULE->getOrInsertFunction(fakeFuncStream.str(), geometricFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeGeometricFunc));
	}
	V_ASSERT(fakeGeometricFunc);
	
	// Prepare arguments for fake function call
	std::vector<Value *> newArgs;
	for (unsigned argument = 0; argument < numArguments; argument++)
	{
		for (unsigned vecItem = 0; vecItem < vecWidth; vecItem++)
		{
			newArgs.push_back(scalarArgs[argument][vecItem]);
		}				
	}
	
	// generate call to fake function
	Instruction * fakeGeometricCall = CallInst::Create(fakeGeometricFunc, newArgs.begin(), newArgs.end(), "fake.scalar.geometric", caller);
	funcProperties->replaceAllInstProperties(caller, fakeGeometricCall);
	funcProperties->setProperty(fakeGeometricCall, PR_NO_SIDE_EFFECT);

	// Connect the return value to the rest of the kernel...
	if (!isReducted && vecWidth > 1)
	{
		// Create breakdown of return value (of fake function) to [vecWidth] elements
		//   %scalar0 = extractelement <4 x Type> %vector, i32 0
		//   %scalar1 = extractelement <4 x Type> %vector, i32 1 
		//   %scalar2 = extractelement <4 x Type> %vector, i32 2
		//   %scalar3 = extractelement <4 x Type> %vector, i32 3 
		Value * newExtractInsts[MAX_OCL_VECTOR_WIDTH];
		for (unsigned i = 0; i < vecWidth; i++)
		{
			Value * constIndex = ConstantInt::get(getInt32Ty, i);			
			newExtractInsts[i] = ExtractElementInst::Create(fakeGeometricCall, constIndex, "extract.fake", caller);
		}
		
		// Check if function's return type was (in the vector function call) in a non-desired type
		if (retVal != caller)
		{
			if (caller->getType() == getVoidTy)
			{
				// return value is passed thru a pointer (first argument)
				V_ASSERT(isa<PointerType>(caller->getOperand(1)->getType())); // first operand really is a pointer
				V_ASSERT(isa<LoadInst>(retVal)); // the actual value is a result of load
				V_ASSERT(cast<LoadInst>(retVal)->getPointerOperand() == caller->getOperand(1)); // the loaded address matches the first arument
			}
			else
			{
				// iterate over all the users of the original return value, until we find the "proper" return value. mark all others for removal		
				V_ASSERT(caller->hasOneUse());
				Value * currentVal = *(caller->use_begin());
				while (currentVal != retVal)
				{
					V_ASSERT(currentVal->hasOneUse());
					funcProperties->setProperty(currentVal, PR_FUNC_PREP_TO_REMOVE);
					currentVal = *(currentVal->use_begin());
				}
			}
			// mark the actual "proper" return value as removal candidate
			funcProperties->setProperty(retVal, PR_FUNC_PREP_TO_REMOVE);			
		}
		
		// mark the extracts as scalar breakdowns of the original function retVal, and also of the fake function's retVal
		SCMEntry * newFuncSCMEntry = createEmptySCMEntry(retVal);
		updateSCMEntryWithValues(newFuncSCMEntry, newExtractInsts, true);
		
		SCMEntry * newFakeFuncSCMEntry = createEmptySCMEntry(fakeGeometricCall);
		updateSCMEntryWithValues(newFakeFuncSCMEntry, newExtractInsts, true);		
	}
	else
	{
		// Return value is a scalar. Just replace all users of original CALL with new fake call
		caller->replaceAllUsesWith(fakeGeometricCall);
	}
	
	// Remove original instruction
	removedInsts.insert(caller);
	
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Check validity of fract instruction vectorization (input pointer must be used as dummy value
// Also, scalarize vector func if needed 
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::MarkSpecialCaseFract(CallInst * caller)
{
	// Check validity: Input pointer must either be used only in this function (and later loaded),
	// or used as a "dummy" - can be for other Fract functions as well...
	// For now - we only support optimization when the pointer is a "dummy" value...
	
	Instruction * pointerVal = dyn_cast<Instruction>(caller->getOperand(2));
	if (!pointerVal)
	{
		// Input is not from an instruction. Maybe func argument? In any case, too risky to optimize away..
		funcProperties->clearProperty(caller, PR_SC_FRACT);
		funcProperties->clearProperty(caller, PR_SPECIAL_CASE_BUILT_IN);
		return true;
	}
	bool validationSuccess = true;
	
	// Check if all uses of the pointer, are only fract functions...
	for (Value::use_iterator ui = pointerVal->use_begin(), ue = pointerVal->use_end(); ui != ue; ++ui)
	{
		Instruction * useInst = dyn_cast<Instruction>(*ui);
		if (!useInst)
		{
			V_UNEXPECTED("User is not an instruction");
			validationSuccess = false;
			break;
		}
		if (!funcProperties->getProperty(useInst, PR_SC_FRACT))
		{
			validationSuccess = false;
			break;
		}
	}
			
	if (!validationSuccess)
	{
		funcProperties->clearProperty(caller, PR_SC_FRACT);
		funcProperties->clearProperty(caller, PR_SPECIAL_CASE_BUILT_IN);
		return true;
	}
	
	// Getting here, validation succeeded. Scalarize function if needed...

	// Find the scalar origins of the arguments
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	V_ASSERT(rootValues->size() == 2);
	
	// Check if root return value is vector or not. if is scalar - meaning func is scalar - no need to scalarize function
	const Type * retType = (*rootValues)[0]->getType();
	if (retType == getFloatTy) return true; // Work is done.
	
	V_ASSERT(isa<VectorType>(retType));
	unsigned width = cast<VectorType>(retType)->getNumElements();
	V_ASSERT(width > 1 && width <= MAX_OCL_VECTOR_WIDTH); // make sure vector width is in an expected size...
	
	// Declare or use if already declared, the scalar fract function
	const Function * LibFunc = RUNTIME_MODULE->getFunction(fractFuncsList[0]);
	if (!LibFunc)
	{
		V_UNEXPECTED("Function not found in runtime module");
		return false;
	}
	Constant * scalarFractFunc = CURRENT_MODULE->getOrInsertFunction(fractFuncsList[0], LibFunc->getFunctionType());
	if (!scalarFractFunc)
	{
		V_UNEXPECTED("Function could not be added to current module");
		return false;
	}
	
	// Obtain root scalar values, for first argument
	Value * scalarArgs[MAX_OCL_VECTOR_WIDTH];
	bool dummy;
	obtainScalarizedValues(scalarArgs, &dummy, (*rootValues)[1], caller);

	// First generate an alloca for dummy vals
	Instruction * dummyAlloca = new AllocaInst(getFloatTy, "dummy.alloca.for.scalar.fract", caller);
	
	// Generate scalar func calls
	Value * newFuncCalls[MAX_OCL_VECTOR_WIDTH];
	for (unsigned i = 0; i< width; i++)
	{
		std::vector<Value *> newArgs;
		newArgs.push_back(scalarArgs[i]);
		newArgs.push_back(dummyAlloca);
		
		newFuncCalls[i] = CallInst::Create(scalarFractFunc, newArgs.begin(), newArgs.end(), "", caller);
		funcProperties->duplicateProperties(newFuncCalls[i], caller);
	}
	
	// Add new value/s to SCM
	SCMEntry * newEntry = createEmptySCMEntry((*rootValues)[0]);
	updateSCMEntryWithValues(newEntry, newFuncCalls, true);
	
	// Remove original instruction
	removedInsts.insert(caller);
	
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Special case ci_gamma instruction. Prepre all the arguments, and replace call with fake function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScalarizeFunction::markSpecialCaseCIGamma(CallInst * caller)
{
	// Instruction has the following data:   <4xfloat> __ci_gamma_scalar_SPI(<3 x float> rgb, float y)
	// Need to (sometimes) go thru casts/etc to find the "original" data. Phases:
	// 1) Find the scalar-version of the inputs rgb. If they do not exist - create them.
	// 2) Create a fake gamma func, which takes as input the scalar root values. return value is like in the original func
	// 3) Create a breakdown of the return value to 4 separate elements, and a buildup back to a vector, and create SCM entry as if this inst was scalarized
	// 4) Save list of input coordinates (to be used in vectorization phase)
	V_PRINT("\t\t\tci_gamma function. Caller:" << *caller << "\n");
	
	// Find the scalar origins of rgb
	SmallVectorImpl<Value *> * rootValues = funcProperties->getScalarizableList(caller);
	V_ASSERT(rootValues != NULL);
	V_ASSERT(rootValues->size() == 2); // expecting 2 values: return value, and rgb vector 
	Value * retVal = (*rootValues)[0];
	Value * properRGB = (*rootValues)[1];
	
	Value * scalarRGBVals[3];
	bool dummy;
	obtainScalarizedValues(scalarRGBVals, &dummy, properRGB, caller);

	// Find or Create the fake ci_gamma function
	Constant * fakeGammaFunc = CURRENT_MODULE->getFunction(FAKE_SCALAR_CI_GAMMA_NAME);
	if (!fakeGammaFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(getFloatTy); // R-coordinate type
		funcArgs.push_back(getFloatTy); // G-coordinate type
		funcArgs.push_back(getFloatTy); // B-coordinate type
		funcArgs.push_back(getFloatTy); // Y-coordinate type
		FunctionType * gammaFuncType = FunctionType::get(CI_GAMMA_SCALAR_RETVAL_TYPE, funcArgs, false);
		fakeGammaFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_SCALAR_CI_GAMMA_NAME, gammaFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeGammaFunc));
	}
	V_ASSERT(fakeGammaFunc);
	
	// Prepare arguments for fake ci_gamma
	std::vector<Value *> newArgs;
	newArgs.push_back(scalarRGBVals[0]); // R
	newArgs.push_back(scalarRGBVals[1]); // G
	newArgs.push_back(scalarRGBVals[2]); // B
	newArgs.push_back(caller->getOperand(2)); // Y
	
	// Create call to fake ci_gamma
	Instruction * fakeGammaCall = CallInst::Create(fakeGammaFunc, newArgs.begin(), newArgs.end(), "fake.ci.gamma", caller);
	funcProperties->replaceAllInstProperties(caller, fakeGammaCall);
	funcProperties->setProperty(fakeGammaCall, PR_NO_SIDE_EFFECT);
	
	// Create breakdown of return value (of fake function) to 3 elements
	//   %scalar0 = extractelement <3 x Type> %vector, i32 0
	//   %scalar1 = extractelement <3 x Type> %vector, i32 1 
	//   %scalar2 = extractelement <3 x Type> %vector, i32 2
	Value * newExtractInsts[3];
	for (unsigned i = 0; i < 3; i++)
	{
		Value * constIndex = ConstantInt::get(getInt32Ty, i);			
		newExtractInsts[i] = ExtractElementInst::Create(fakeGammaCall, constIndex, "extract.fake.gamma", caller);
	}
	
	// Check if function's return type was (in the orig function call) in a non-desired type
	if (retVal != caller)
	{
		if (caller->getType() == getVoidTy)
		{
			// return value is passed thru a pointer (first argument)
			V_ASSERT(isa<PointerType>(caller->getOperand(1)->getType())); // first operand really is a pointer
			V_ASSERT(isa<LoadInst>(retVal)); // the actual value is a result of load
			V_ASSERT(cast<LoadInst>(retVal)->getPointerOperand() == caller->getOperand(1)); // the loaded address matches the first arument
		}
		else
		{
			// iterate over all the users of the original return value, until we find the "proper" return value. mark all others for removal		
			V_ASSERT(caller->hasOneUse());
			Value * currentVal = *(caller->use_begin());
			while (currentVal != retVal)
			{
				V_ASSERT(currentVal->hasOneUse());
				funcProperties->setProperty(currentVal, PR_FUNC_PREP_TO_REMOVE);
				currentVal = *(currentVal->use_begin());
			}
		}
		// mark the actual "proper" return value as removal candidate
		funcProperties->setProperty(retVal, PR_FUNC_PREP_TO_REMOVE);			
	}
	
	
	
	// mark the extracts as scalar breakdowns of the sampler value..
	SCMEntry * newGammaSCMEntry = createEmptySCMEntry(retVal);
	updateSCMEntryWithValues(newGammaSCMEntry, newExtractInsts, true);
	
	SCMEntry * newFakeGammaSCMEntry = createEmptySCMEntry(fakeGammaCall);
	updateSCMEntryWithValues(newFakeGammaSCMEntry, newExtractInsts, true);
	
	// Erase "original" read sampler
	removedInsts.insert(caller);	
	
	return true;
}

