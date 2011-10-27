/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "vectorize.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Special case functions handler dispach
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::checkSpecialCaseFunctionCall(CallInst * CI)
{		
	// Dispatch according to special-case function type
	instProperty specialCaseType = funcProperties->getPropertyGroup(CI, PR_ALL_SPECIAL_CASE_FUNCS);
	switch (specialCaseType) {
		case PR_SC_READ_SAMPLER_F_2D:
			return vectorizeReadSample(CI, true);
			break;
		case PR_SC_READ_SAMPLER_F_3D:
			return vectorizeReadSample(CI, false);
			break;
		case PR_SC_WRITEF_SAMPLER:
			return vectorizeWriteSamplerF(CI);
			break;
		case PR_SC_CI_GAMMA:
			return vectorizeCiGamma(CI);
			break;
		case PR_SC_SELECT_INST:
			return vectorizeSelectFunc(CI);
			break;
		case PR_SC_DOTPROD:
			return vectorizeGeometricFunc(CI, 2, DOT_PRODUCT_FAKE_VECTOR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_DISTANCE:
			return vectorizeGeometricFunc(CI, 2, DISTANCE_FAKE_VECTOR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_FAST_DISTANCE:
			return vectorizeGeometricFunc(CI, 2, FAST_DISTANCE_FAKE_VECTOR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_LENGTH:
			return vectorizeGeometricFunc(CI, 1, LENGTH_FAKE_VECTOR_NAME_PREFIX, true);
			break;
		case PR_SC_GEO_FAST_LENGTH:
			return vectorizeGeometricFunc(CI, 1, FAST_LENGTH_FAKE_VECTOR_NAME_PREFIX, true);
			break;
		case PR_SC_CROSS:
			return vectorizeGeometricFunc(CI, 2, CROSS_PRODUCT_FAKE_VECTOR_NAME_PREFIX, false);
			break;
		case PR_SC_GEO_NORMALIZE:
			return vectorizeGeometricFunc(CI, 1, NORMALIZE_FAKE_VECTOR_NAME_PREFIX, false);
			break;
		case PR_SC_GEO_FAST_NORMALIZE:
			return vectorizeGeometricFunc(CI, 1, FAST_NORMALIZE_FAKE_VECTOR_NAME_PREFIX, false);
			break;
		case PR_SC_FRACT:
			return vectorizeFractFunc(CI);
			break;
		case PR_SC_BOUNDARY_CHECK:
			return vectorizeEarlyExit(CI);
			break;
		default:
			break;
	}
	
	V_UNEXPECTED("No handling for Special case function"); // Should not happen! we only reach this function if the function IS a special case func 
	return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar (fake) read sampler with a vector (still fake) sampler
// Handles both 2D and 3D samplers
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeReadSample(CallInst * CI, bool is2D)
{
	V_PRINT("\tVectorizing a Read-sampler\n");
	
	// Prepare data for transposed sampler
	Value * imagePtr = CI->getOperand(1);
	Value * samplerType = CI->getOperand(2);
	Value * x_input;
	Value * y_input;	
	Value * z_input;	
	bool dummy;
	obtainVectorizedValues(&x_input, &dummy, CI->getOperand(3), CI);
	obtainVectorizedValues(&y_input, &dummy, CI->getOperand(4), CI);
	if (!is2D)
		obtainVectorizedValues(&z_input, &dummy, CI->getOperand(5), CI);
	
	// Find or Create the fake read-sampler function
	Constant * fakeReadSampFunc;
	if (is2D)
	{
		fakeReadSampFunc = CURRENT_MODULE->getFunction(FAKE_VECTOR_READ_IMAGEF_2D_NAME);
	}
	else
	{
		fakeReadSampFunc = CURRENT_MODULE->getFunction(FAKE_VECTOR_READ_IMAGEF_3D_NAME);
	}
	if (!fakeReadSampFunc)
	{
		// Function was not declared yet. First create the needed prototype
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(CI->getOperand(1)->getType()); // image pointer type
		funcArgs.push_back(CI->getOperand(2)->getType()); // sampler-type type
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 
		if (!is2D)
			funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 

		// The return type should have been 4 vectors (R,G,B,A), each of type <[WIDTH] x float>. To emulate this, we use a 
		// single <4 x float> return vector but with 4 following instructions that will break it to 4 different <[WIDTH] x float> vectors.
		FunctionType * readSampFuncType = FunctionType::get(VectorType::get(getFloatTy, 4), funcArgs, false);
		if (is2D)
		{
			fakeReadSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_VECTOR_READ_IMAGEF_2D_NAME, readSampFuncType);
		}
		else
		{
			fakeReadSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_VECTOR_READ_IMAGEF_3D_NAME, readSampFuncType);
		}
		funcProperties->addFakeFunctionToList(cast<Function>(fakeReadSampFunc));
	}
	V_ASSERT(fakeReadSampFunc);
	
	Value * newSamplerCall;
	Value * newFakeOutput[4];
	UndefValue * undefVect = UndefValue::get(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));	
	// will create a single fake sampler
	std::vector<Value *> newArgs;
	newArgs.push_back(imagePtr); // image pointer
	newArgs.push_back(samplerType); // sampler-type
	newArgs.push_back(x_input); // X-coordinate
	newArgs.push_back(y_input); // Y-coordinate
	if (!is2D)
	{
		newArgs.push_back(z_input); // Z-coordinate
	}
	// Create call to fake read sampler
	newSamplerCall = CallInst::Create(fakeReadSampFunc, newArgs.begin(), newArgs.end(), "fake.vector.readsampler", CI);
	funcProperties->duplicateProperties(newSamplerCall, CI);
	
	// Create breakdown of return value (of fake sampler) to [WIDTH] vector
	//   %output.fake.0 = shufflevector <4 x Float> %vector, <4 x Float> undef, <WIDTH x i32><i32 0, i32 0, i32 0, i32 0>
	//   %output.fake.1 = shufflevector <4 x Float> %vector, <4 x Float> undef, <WIDTH x i32><i32 1, i32 1, i32 1, i32 1>
	//   %output.fake.2 = shufflevector <4 x Float> %vector, <4 x Float> undef, <WIDTH x i32><i32 2, i32 2, i32 2, i32 2>
	//   %output.fake.3 = shufflevector <4 x Float> %vector, <4 x Float> undef, <WIDTH x i32><i32 3, i32 3, i32 3, i32 3>
	for (unsigned i = 0; i < 4; i++)
	{
		int indexValues[MAX_SUPPORTED_VECTOR_WIDTH];
		for (unsigned j = 0; j < ARCH_VECTOR_WIDTH; ++j) {
			indexValues[j] = i;
		}
		Constant * shuffleIndex = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, indexValues);
		newFakeOutput[i] = new ShuffleVectorInst(newSamplerCall, undefVect, shuffleIndex, "fake.output.fake", CI);
		funcProperties->duplicateProperties(newFakeOutput[i], CI);
	}
	
	// Vectorize breakdowns of the scalar sampler
	V_ASSERT(CI->getNumUses() <= 4); // Sanity: make sure only the "planted" extractElement instructions really decend from this inst
	for (Value::use_iterator ui = CI->use_begin(), ue = CI->use_end(); ui != ue; ++ui)
	{
		// Extract the accessed index
		Instruction * useInst = dyn_cast<Instruction>(*ui);
		V_ASSERT(useInst);
		V_ASSERT(isa<ExtractElementInst>(useInst));
		Value * scalarIndexVal = cast<ExtractElementInst>(useInst)->getOperand(1);
		V_ASSERT(isa<ConstantInt>(scalarIndexVal));
		uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
		V_ASSERT(scalarIndex < 4);
		// Save new conversions (extractElement -> shuffleVector) in VCM
		createVCMEntryWithVectorValues(useInst, newFakeOutput[scalarIndex]);
		// Mark original breakdowns for removal
		funcProperties->setProperty(useInst, PR_FUNC_PREP_TO_REMOVE);
	}
	
	// Remove original instruction
	removedInsts.insert(CI);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar (fake) write sampler with a vector (still fake) sampler
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeWriteSamplerF(CallInst * CI)
{
	// extract scalar arguments
	Value * scalar_Xcoord = CI->getOperand(2);
	Value * scalar_Ycoord = CI->getOperand(3);

	// Check if coordinates follow horizontal-sequentiality rules. If not - we cannot vectorize this function call!
	if (!funcProperties->getProperty(scalar_Xcoord, PR_TID_VALS_CONSECUTIVE)) //X-axis must have consecutive values
	{
		V_PRINT("\t\tWRITE Sampler vectorization was cancelled - X-axis is not consecutive:" << *scalar_Xcoord << "\n");
		return false; 
	}
	else 
	{
		V_PRINT("\t\tWRITE Sampler: X-axis is marked consecutive:" << *scalar_Xcoord << "\n");		
	}
	
	if (funcProperties->getProperty(scalar_Ycoord, PR_TID_DEPEND)) //Y-axis must have all values the same
	{
		V_PRINT("\t\tWRITE Sampler vectorization was cancelled - Y-axis is not TID-independent:" << *scalar_Ycoord << "\n");
		return false; 
	}
	else 
	{
		V_PRINT("\t\tWRITE Sampler: Y-axis is marked consecutive:" << *scalar_Ycoord << "\n");				
	}
	
	// Prepare all inputs for tranposed sampler
	Value * imagePtr = CI->getOperand(1);
	// Prepare coordinates 
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> x_coords;
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> y_coords;
	bool dummy;
	obtainMultiScalarValues(x_coords, &dummy, scalar_Xcoord, CI);
	obtainMultiScalarValues(y_coords, &dummy, scalar_Ycoord, CI);
	// Parepare colors 
	Value * colors[4];
	obtainVectorizedValues(&colors[0], &dummy, CI->getOperand(4), CI);
	obtainVectorizedValues(&colors[1], &dummy, CI->getOperand(5), CI);
	obtainVectorizedValues(&colors[2], &dummy, CI->getOperand(6), CI);
	obtainVectorizedValues(&colors[3], &dummy, CI->getOperand(7), CI);
	
	// Find or Create the fake write-sampler function
	Constant * fakeWriteSampFunc = CURRENT_MODULE->getFunction(FAKE_VECTOR_WRITE_IMAGEF_NAME);
	if (!fakeWriteSampFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(imagePtr->getType()); // image type
		funcArgs.push_back(getInt32Ty); // X-coordinates type
		funcArgs.push_back(getInt32Ty); // Y-coordinates type
		// R, G, B & A clors 
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); 

		FunctionType * writeSampFuncType = FunctionType::get(getVoidTy, funcArgs, false);
		fakeWriteSampFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_VECTOR_WRITE_IMAGEF_NAME, writeSampFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeWriteSampFunc));
	}
	V_ASSERT(fakeWriteSampFunc);
	
	// create a call to transposed write sampler
	Value * transposedSamplerCall;
	std::vector<Value *> newArgs;
	newArgs.push_back(imagePtr);
	newArgs.push_back(x_coords[0]); // the first value out of a (consecutive) sequence
	newArgs.push_back(y_coords[0]); // all the y-values are the same. pick one...
	for (unsigned color = 0; color < 4; color++)
	{
		newArgs.push_back(colors[color]);
	}

	transposedSamplerCall = CallInst::Create(fakeWriteSampFunc, newArgs.begin(), newArgs.end(), "", CI);
	funcProperties->duplicateProperties(transposedSamplerCall, CI);

	
	// Remove original instruction
	removedInsts.insert(CI);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar (fake) select with a vector (still fake) select
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeSelectFunc(CallInst * CI)
{
	V_PRINT("\t\tSelect function - replacing with transposed function\n");
	
	// Get return type of function
	const Type * retType = CI->getType();
	const Type * maskType = CI->getOperand(3)->getType();
	V_ASSERT(retType == CI->getOperand(1)->getType()); // arguments X&Y have same type as return type
	V_ASSERT(retType == CI->getOperand(2)->getType()); // arguments X&Y have same type as return type
	
	// Obtain proper "fake" vector select function
	std::stringstream fakeFuncStream;
	if (retType == getFloatTy)
	{
		fakeFuncStream << SELECT_FAKE_VECTOR_NAME_PREFIX << "f";
	}
	else
	{
		fakeFuncStream << SELECT_FAKE_VECTOR_NAME_PREFIX << cast<IntegerType>(retType)->getBitWidth();
	}
	
	// Declare or use if already declared, the fake function
	Constant * fakeSelectFunc = CURRENT_MODULE->getFunction(fakeFuncStream.str());
	if (!fakeSelectFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(VectorType::get(retType, ARCH_VECTOR_WIDTH));
		funcArgs.push_back(VectorType::get(retType, ARCH_VECTOR_WIDTH));
		funcArgs.push_back(VectorType::get(maskType, ARCH_VECTOR_WIDTH));
		FunctionType * selectFuncType = FunctionType::get(VectorType::get(retType, ARCH_VECTOR_WIDTH), funcArgs, false);
		fakeSelectFunc = CURRENT_MODULE->getOrInsertFunction(fakeFuncStream.str(), selectFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeSelectFunc));
	}
	V_ASSERT(fakeSelectFunc);
	
	// Prepare all inputs for tranposed function
	Value * vector_args[3]; 
	bool dummy;
	for (unsigned i = 0; i < 3; i++)
	{
		obtainVectorizedValues(&vector_args[i], &dummy, CI->getOperand(i+1), CI);
	}
	
	// create a call to transposed function
	Value * transposedSelectCall;
	std::vector<Value *> newArgs;
	for (unsigned i = 0; i < 3; i++)
	{
		newArgs.push_back(vector_args[i]);			
	}				
	transposedSelectCall = CallInst::Create(fakeSelectFunc, newArgs.begin(), newArgs.end(), "fake.vector.select", CI);
	funcProperties->duplicateProperties(transposedSelectCall, CI);
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(CI, transposedSelectCall);
	
	// Remove original instruction
	removedInsts.insert(CI);
	V_PRINT("\t\tSelect SC - replacement succeeded\n");
	return true;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar (fake) geometric func with a vector (still fake) func
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeGeometricFunc(CallInst * CI, unsigned numArgs, const char *fake_func_name_prefix, bool isReducted)
{
	V_PRINT("\t\tGeometric func - replacing with transposed function\n");
	V_ASSERT(numArgs == 1 || numArgs == 2); // only know geometric funcs take 1 or 2 arguments 
	
	// Find the vector width of the original function
	unsigned funcOperands = CI->getNumOperands() - 1; // all the arguments of the CALL, except for the called function itself...
	unsigned vecWidth = funcOperands/numArgs; 
	
	// Obtain proper "fake" vector geometric function
	std::stringstream fakeFuncStream;
	fakeFuncStream << fake_func_name_prefix << vecWidth;
	
	// Declare or use if already declared, the fake function
	Constant * fakeGeometricFunc = CURRENT_MODULE->getFunction(fakeFuncStream.str());
	if (!fakeGeometricFunc)
	{
		// Function was not declared yet. First create the needed prototype...
		std::vector<const Type *> funcArgs(funcOperands, VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH)); // each argument is now a vector of floats
		const Type * retType = ((!isReducted && vecWidth>1) ? VectorType::get(getFloatTy, vecWidth) : VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));
		FunctionType * geometricFuncType = FunctionType::get(retType, funcArgs, false);
		fakeGeometricFunc = CURRENT_MODULE->getOrInsertFunction(fakeFuncStream.str(), geometricFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeGeometricFunc));
		// Explanation For non-reducted funcs: 
		// The scalar function returned a vector of <VecWidth x float>. Transposed function will generate [VecWidth] vectors
		// of <ARCH_WIDTH x float>. To emulate this, we use a return type of <VecWidth x float> and then use shuffleVector
		// to generate [VecWidth] vectors of <ARCH_WIDTH x float>.
	}
	V_ASSERT(fakeGeometricFunc);
	
	// Prepare all inputs for tranposed function
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> vector_args;
	bool dummy;
	for (unsigned i = 0; i < funcOperands; i++)
	{
		Value *newVal;
		obtainVectorizedValues(&newVal, &dummy, CI->getOperand(i+1), CI);
		vector_args.push_back(newVal);
	}
	
	// create a call to transposed function
	Value * transposedGeometricCall;
	Value * newFakeOutput[4];
	UndefValue * undefVect = UndefValue::get(VectorType::get(getFloatTy, vecWidth));
	
	std::vector<Value *> newArgs;
	for (unsigned i = 0; i < funcOperands; i++)
	{
		newArgs.push_back(vector_args[i]);			
	}				
	transposedGeometricCall = CallInst::Create(fakeGeometricFunc, newArgs.begin(), newArgs.end(), "fake.vector.geometric", CI);
	funcProperties->duplicateProperties(transposedGeometricCall, CI);
	
	// For non-reducted functions, need to generate several output vectors
	if (!isReducted && vecWidth>1)
	{
		// Create breakdown of return value (of fake func) to [VecWidth] vectors
		//   %output.fake.0 = shufflevector <4 x Float> %vector, <4 x Float> undef, <4 x i32><i32 0, i32 0, i32 0, i32 0>
		//   %output.fake.1 = shufflevector <4 x Float> %vector, <4 x Float> undef, <4 x i32><i32 1, i32 1, i32 1, i32 1>
		//   %output.fake.2 = shufflevector <4 x Float> %vector, <4 x Float> undef, <4 x i32><i32 2, i32 2, i32 2, i32 2>
		//   %output.fake.3 = shufflevector <4 x Float> %vector, <4 x Float> undef, <4 x i32><i32 3, i32 3, i32 3, i32 3>
		for (unsigned i = 0; i < vecWidth; i++)
		{
			int indexValues[MAX_SUPPORTED_VECTOR_WIDTH]; 
			for (unsigned j = 0; j < ARCH_VECTOR_WIDTH; ++j) {
				indexValues[j] = i;
			}
			Constant * shuffleIndex = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, indexValues);
			newFakeOutput[i] = new ShuffleVectorInst(transposedGeometricCall, undefVect, shuffleIndex, "fake.output.fake", CI);
			funcProperties->duplicateProperties(newFakeOutput[i], CI);
		}
	}
	
	// If function is not reducted, need to connect the breakdowns to the users of the "extract" breakdowns (of the scalar fake function)
	if (!isReducted && vecWidth>1)
	{
		// Vectorize breakdowns of the scalar func
		V_ASSERT(CI->getNumUses() <= vecWidth); // Sanity: make sure only the "planted" extractElement instructions really decend from this inst
		for (Value::use_iterator ui = CI->use_begin(), ue = CI->use_end(); ui != ue; ++ui)
		{
			// Extract the accessed index
			Instruction * useInst = dyn_cast<Instruction>(*ui);
			V_ASSERT(useInst);
			V_ASSERT(isa<ExtractElementInst>(useInst));
			Value * scalarIndexVal = cast<ExtractElementInst>(useInst)->getOperand(1);
			V_ASSERT(isa<ConstantInt>(scalarIndexVal));
			uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
			V_ASSERT(scalarIndex < vecWidth);
			// Save new conversions (extractElement -> shuffleVector) in VCM
			createVCMEntryWithVectorValues(useInst, newFakeOutput[scalarIndex]);
			// Mark original breakdowns for removal
			funcProperties->setProperty(useInst, PR_FUNC_PREP_TO_REMOVE);
		}
	}
	else
	{
		// Add new value/s to VCM
		createVCMEntryWithVectorValues(CI, transposedGeometricCall);
	}

	// Remove original instruction
	removedInsts.insert(CI);
	V_PRINT("\t\tGeometric - replacement succeeded\n");
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar Fract function with a vector function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeFractFunc(CallInst * CI)
{
	V_PRINT("\t\tFract - replacing with transposed function\n");

	// Get index of vectorized Fract function (of correct width) 
	unsigned index;
	if (ARCH_VECTOR_WIDTH == 4)
	{
		index = 3;
	}
	else if (ARCH_VECTOR_WIDTH == 8)
	{
		index = 4;
	}
	else if (ARCH_VECTOR_WIDTH == 16)
	{
		index = 5;
	}
	else 
	{
		V_UNEXPECTED("met unsupported ARCH width");
		return false; // unexpected vector width
	}

	// Declare or use if already declared, the vector fract function
	const Function * LibFunc = RUNTIME_MODULE->getFunction(fractFuncsList[index]);
	if (!LibFunc)
	{
		V_UNEXPECTED("function not found in runtime module");
		return false;
	}
	Constant * vectorFractFunc = CURRENT_MODULE->getOrInsertFunction(fractFuncsList[index], LibFunc->getFunctionType());
	if (!vectorFractFunc)
	{
		V_UNEXPECTED("failed to generate function in current module");
		return false;
	}
		
	// Prepare all inputs for tranposed function
	Value * vector_arg; 
	bool dummy;
	bool err = obtainVectorizedAndCastedValuesForCall(&vector_arg, &dummy, CI->getOperand(1), CI, LibFunc->getFunctionType()->getParamType(0));
	if (err == false)
		return vectorizeNonVectorizableInst(CI, false); // failed obtaining values. vectorization failed!
	Instruction * dummyAlloca = new AllocaInst(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), "dummy.alloca.for.fract", CI);
	
	// create a call to transposed function
	Value * vectorFractCall;
	std::vector<Value *> newArgs;
	newArgs.push_back(vector_arg);			
	newArgs.push_back(dummyAlloca);			
	vectorFractCall = CallInst::Create(vectorFractFunc, newArgs.begin(), newArgs.end(), "", CI);
	funcProperties->duplicateProperties(vectorFractCall, CI);
	
	// Add new value/s to VCM
	createVCMEntryWithVectorValues(CI, vectorFractCall);
	
	// Remove original instruction
	removedInsts.insert(CI);
	V_PRINT("\t\tFract - replacement succeeded\n");
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the arguments of scalar (fake) early exit with vectorized arguments
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeEarlyExit(CallInst * CI)
{
	V_PRINT("\t\tEarly Exit - replacing with vectorized arguments\n");
	
	// Obtain arguments. Need only the first of each multi-scalar
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> argument0;
	SmallVector<Value*, MAX_SUPPORTED_VECTOR_WIDTH> argument1;
	bool dummy;
	obtainMultiScalarValues(argument0, &dummy, CI->getOperand(1), CI);
	obtainMultiScalarValues(argument1, &dummy, CI->getOperand(2), CI);

	// create a call with the new values
	std::vector<Value *> newArgs;
	newArgs.push_back(argument0[0]);
	newArgs.push_back(argument1[0]);
	Value * newCall = CallInst::Create(CI->getOperand(0), newArgs.begin(), newArgs.end(), "", CI);
	funcProperties->duplicateProperties(newCall, CI);
	
	// Nothing to add to VCM - as this call must have no users!
	
	// Remove original instruction
	removedInsts.insert(CI);
	V_PRINT("\t\tEarly Exit SC - replacement succeeded\n");
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Replace the scalar (fake) ci_gamma with a vector (still fake) func
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectorizeFunction::vectorizeCiGamma(CallInst * CI)
{
	V_PRINT("\tVectorizing a ci_gamma func\n");
	
	// Prepare data for transposed func
	Value * R_input;
	Value * G_input;
	Value * B_input;
	Value * Y_input;	
	bool dummy;
	obtainVectorizedValues(&R_input, &dummy, CI->getOperand(1), CI);
	obtainVectorizedValues(&G_input, &dummy, CI->getOperand(2), CI);
	obtainVectorizedValues(&B_input, &dummy, CI->getOperand(3), CI);
	obtainVectorizedValues(&Y_input, &dummy, CI->getOperand(4), CI);
	
	// Find or Create the fake vector function
	Constant * fakeGammaFunc = CURRENT_MODULE->getFunction(FAKE_VECTOR_CI_GAMMA_NAME);
	if (!fakeGammaFunc)
	{
		// Function was not declared yet. First create the needed prototype
		std::vector<const Type *> funcArgs;
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));  // R
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));  // G
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));  // B
		funcArgs.push_back(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));  // Y		
		FunctionType * gammaFuncType = FunctionType::get(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH), funcArgs, false);
		fakeGammaFunc = CURRENT_MODULE->getOrInsertFunction(FAKE_VECTOR_CI_GAMMA_NAME, gammaFuncType);
		funcProperties->addFakeFunctionToList(cast<Function>(fakeGammaFunc));
		// Return type of scalar was <3 x float>. So for transposed factor 4, it should have been 3 vectors
		// of <4 x float>. If the factor is 16, it should have been 12 vectors of <4 x float>. To emulate this,
		// the return type is <WIDTH x float>, and later we break down to 3 vectors of <WIDTH x float>
	}
	V_ASSERT(fakeGammaFunc);
	
	Value * newGammaCall;
	Value * newFakeOutput[3];
	UndefValue * undefVect = UndefValue::get(VectorType::get(getFloatTy, ARCH_VECTOR_WIDTH));	
	// will create a single fake function
	std::vector<Value *> newArgs;
	newArgs.push_back(R_input); 
	newArgs.push_back(G_input); 
	newArgs.push_back(B_input); 
	newArgs.push_back(Y_input); 
	// Create call to fake gamma func
	newGammaCall = CallInst::Create(fakeGammaFunc, newArgs.begin(), newArgs.end(), "fake.vector.ci.gamma", CI);
	funcProperties->duplicateProperties(newGammaCall, CI);
	
	// Create breakdown of return value (of fake function) to 3 vectors
	//   %output.fake = shufflevector <WIDTH x Float> %vector, <WIDTH x Float> undef, <WIDTH x i32><i32 0, i32 0, i32 0, i32 0>
	//   %output.fake = shufflevector <WIDTH x Float> %vector, <WIDTH x Float> undef, <WIDTH x i32><i32 1, i32 1, i32 1, i32 1>
	//   %output.fake = shufflevector <WIDTH x Float> %vector, <WIDTH x Float> undef, <WIDTH x i32><i32 2, i32 2, i32 2, i32 2>
	for (unsigned i = 0; i < 3; i++)
	{
		int indexValues[MAX_SUPPORTED_VECTOR_WIDTH];
		for (unsigned j = 0; j< MAX_SUPPORTED_VECTOR_WIDTH; ++j)
			indexValues[j] = i;
		Constant * shuffleIndex = createIncrementingConstVectorForShuffles(ARCH_VECTOR_WIDTH, indexValues);
		newFakeOutput[i] = new ShuffleVectorInst(newGammaCall, undefVect, shuffleIndex, "fake.output.fake", CI);
		funcProperties->duplicateProperties(newFakeOutput[i], CI);
	}
	
	// Vectorize breakdowns of the scalar gamma
	V_ASSERT(CI->getNumUses() <= 3); // Sanity: make sure only the "planted" extractElement instructions really decend from this inst
	for (Value::use_iterator ui = CI->use_begin(), ue = CI->use_end(); ui != ue; ++ui)
	{
		// Extract the accessed index
		Instruction * useInst = dyn_cast<Instruction>(*ui);
		V_ASSERT(useInst);
		V_ASSERT(isa<ExtractElementInst>(useInst));
		Value * scalarIndexVal = cast<ExtractElementInst>(useInst)->getOperand(1);
		V_ASSERT(isa<ConstantInt>(scalarIndexVal));
		uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
		V_ASSERT(scalarIndex < 3);
		// Save new conversions (extractElement -> shuffleVector) in VCM
		createVCMEntryWithVectorValues(useInst, newFakeOutput[scalarIndex]);
		// Mark original breakdowns for removal
		funcProperties->setProperty(useInst, PR_FUNC_PREP_TO_REMOVE);
	}
	
	// Remove original instruction
	removedInsts.insert(CI);
	return true;
}
