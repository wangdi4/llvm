/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "ModuleManager.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
VectModuleManager::VectModuleManager(Module *M, const Module *runtimeMod, unsigned archVectorWidth):
	singleVisit(false), 
	isModuleModified(false),
	currentModule(M),
	runtimeModule(runtimeMod),
	controlFlowObject(),
	scalObject(),
	vectObject(),
	scanObject(),
	loopObject(),
	m_archVectorWidth(archVectorWidth)
{
	V_PRINT("Module manager constructor\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
VectModuleManager::~VectModuleManager()
{
	V_PRINT("Module manager destructor\n");
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Given a list of functions, send each for vectorization
// This function returns FALSE, if module was not modified at all
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::runOnModule(SmallVectorImpl<Function*> &functionsList)
{
	V_ASSERT(singleVisit == false); // Validate that this function is only called ONCE (per ModuleManager object).
	singleVisit = true;
	
#if defined(VERIFY_FUNCS)
	// Go over list of built-in functions, making sure they all really exist
	VerifyFunctionsListsAreUpToDate();
#endif	

	// Iterate over kernels list
	unsigned numFuncs = functionsList.size();
	for (unsigned i = 0; i < numFuncs; i++)
	{
		Function * scalarFunc = functionsList[i];
		
		// Send scalar kernel for vectorization process
		createConvertedFunction(scalarFunc);
	}	
	
	// Sanity: check that lists of vectorized kernels are complete
	V_ASSERT(vectoredKernelsList.size() == numFuncs);
	V_ASSERT(vectoredKernelsMaxWidth.size() == numFuncs);
	
	V_DUMP_MODULE(currentModule); // Print entire module
	return true;
}






/////////////////////////////////////////////////////////////////////////////////////////////////////
// Per kernel, try to vectorize (by invoking the controlFlow handler, scalarizer and vectorizer),
// The function may be called recursively, in which case it creates new scalarize/vectorize
// objects.
// This function returns TRUE if vectorization of the function was successful
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::createConvertedFunction(Function * F)
{
	bool retVal = true;
	
	// Clone the original function
	const Twine VName = "__" + F->getName() + "_Vectorized";
	Function *newVectoredFunc = Function::Create(F->getFunctionType(),
												 F->getLinkage(), 
												 VName, currentModule);
	
	DenseMap<const Value*, Value*> ValueMap;
	
	// Loop over the arguments, copying the names of the mapped arguments over...
	Function::arg_iterator DestI = newVectoredFunc->arg_begin();
	for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
		 I != E; ++I)
		if (ValueMap.count(I) == 0) {		// Is this argument preserved?
			DestI->setName(I->getName()); // Copy the name over...
			ValueMap[I] = DestI++;				// Add mapping to ValueMap
		}
	
	SmallVector<ReturnInst*, 8> Returns;	// Ignore returns cloned.
	CloneFunctionInto(newVectoredFunc, F, ValueMap, Returns);
	
	// Preform actual code conversions, plus check-points for aborting the vectorization
	CodeProperties functionProperties(currentModule, runtimeModule, newVectoredFunc, m_archVectorWidth);
	CodeProperties * funcProperties = &functionProperties; // keep as pointer, to align with the rest of the classes
	V_ASSERT(MAX_LOOP_SIZE >= ARCH_VECTOR_WIDTH);
	V_PRINT("\nStarting function: " << newVectoredFunc->getName() << "\n");
	V_DUMP(newVectoredFunc);
	
	do {
		// Pre-scalarizer function scanning: detect inputs to functions, TID setting instructions, WG-granularity functions, barriers, etc..
		V_PRINT("\nStart pre-scalarization phase...\n");
		scanObject.preScalarizeScanFunction(funcProperties);
		V_PRINT("\nCompleted pre-scalarization phase\n");
		
		// Check point: Check if function is valid for vectorizing
		if (funcProperties->getFuncProperty(ERROR__FUNC_HAS_UNSUPPORTED_SEQ) ||
			funcProperties->getFuncProperty(FUNC_CONTAINS_BARRIER) ||
			funcProperties->getFuncProperty(FUNC_CONTAINS_WG_SYNC_OP) ||
			funcProperties->getFuncProperty(FUNC_HAS_MULTIPLE_EXITS))
		{
			retVal = false;
			V_PRINT("Function contains illegal (unsupported by vectorizer) code. Aborting vectorization!\n");
			break;
		}
		
		// Scalarize function
		V_PRINT("\nStart scalarizing phase...\n");
		retVal = scalObject.scalarizeFunction(funcProperties);
		V_PRINT("\nCompleted scalarizing phase\n");
		
		// Check point: Check if scalarization was successful
		if (!retVal) break;
		
		// Remove dead code (to simplify vectorization)
		V_PRINT("\nStart Dead code removal phase...\n");
		removeDeadCode(newVectoredFunc, funcProperties);
		V_PRINT("\nCompleted Dead code removal phase\n");
		V_DUMP(newVectoredFunc);
		
		// Post scalarization scanning: mark TID-dependent insts, and their "distance": will their vectorized values be sequential, etc..
		V_PRINT("\nStart post-scalarization scanning phase...\n");
		scanObject.postScalarizeScanFunction();
		V_PRINT("\nCompleted post-scalarization scanning phase\n");
		
		// Control flow handling: Flatten small if-then-else clauses to "select" instructions, detect boundary checks
		V_PRINT("\nStart control flow handling phase...\n");
		retVal = controlFlowObject.analyzeControlFlow(funcProperties);
		V_PRINT("\nCompleted control flow handling phase\n");
		
		// Check point: Check if control-flow analysis was successful
		if (!retVal) break;
		V_DUMP(newVectoredFunc);
		
		// Vectorize function
		V_PRINT("\nStart vectorizing phase...\n");
		retVal = vectObject.vectorizeFunction(funcProperties);
		V_PRINT("\nCompleted vectorizing phase\n");
		V_DUMP(newVectoredFunc);

		// Check point: Check if vectorization was successful
		if (!retVal) break;
		
		// Genarate master loop
		V_PRINT("\nStart adding Master loop phase...\n");
		retVal = loopObject.generateLoop(funcProperties);
		V_PRINT("\nCompleted adding Master loop phase\n");
		V_DUMP(newVectoredFunc);
		
		// Check point: Check if vectorization/mega-loop was successful
		if (!retVal) break;

		V_PRINT("\nStart prepping kernel for early exits phase...\n");
		prepareKernelForLoop(F, funcProperties);
		V_PRINT("\nCompleted prepping kernel for early exits phase\n");
		
		// Resolve any special-case functions which were used	
		V_PRINT("\nStart resolving Special-case functions phase...\n");
		retVal = resolveSpecialCaseFunctions(newVectoredFunc, funcProperties);
		V_PRINT("\nCompleted resolving Special-case functions phase\n");
		
		// if there is no early-exit (tid-dependent), need to add a check if local_size is smaller than vector_width
		if (!funcProperties->getFuncProperty(FUNC_HAS_TID_DEPEND_EARLY_EXITS))
		{
			addVectorWidthCheck(F, funcProperties);
		}
	} 
	while (0);
	
	if (retVal)
	{
		// Vectorization succeeded. Update conversion maps.
		V_PRINT("\nVectorization completed successfully!\n");
		V_DUMP(newVectoredFunc); // Print the new vector function
		vectoredKernelsList.push_back(newVectoredFunc);
		vectoredKernelsMaxWidth.push_back(MAX_LOOP_SIZE);
	}
	else
	{
		V_PRINT("\nFailed function: " << newVectoredFunc->getName() << "\n");
		vectoredKernelsList.push_back(NULL);
		vectoredKernelsMaxWidth.push_back(0);
		V_ASSERT(newVectoredFunc->use_empty());
		newVectoredFunc->eraseFromParent();
		eraseSpecialCaseFakeFunctions(funcProperties); // Safely erase all the temporary (fake) special-case functions, after function was erased
	}

	return retVal;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface for Vectorizer API
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::getVectorizedFunctionsPointers(SmallVectorImpl<Function*> &vectorFunctions)
{
	V_ASSERT(singleVisit); // Sanity: Module was vectorized already
	vectorFunctions = vectoredKernelsList;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface for Vectorizer API
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::getVectorizedFunctionsWidths(SmallVectorImpl<int> &vectorMaxWidths)
{
	V_ASSERT(singleVisit); // Sanity: Module was vectorized already
	vectorMaxWidths = vectoredKernelsMaxWidth;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove dead-code from code after scalarization phase
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::removeDeadCode(Function * F, CodeProperties * funcProperties)
{
	std::vector<Instruction *> WorkList;
	for (inst_iterator i = inst_begin(*F), e = inst_end(*F); i != e; ++i)
		WorkList.push_back(&*i);
	
	while (!WorkList.empty())
	{
		Instruction *I = WorkList.back();
		WorkList.pop_back();
		std::string funcName;
		bool deadBuiltIn = false;
		
		if (I->use_empty())
		{
			if (isa<CallInst>(I))
			{
				unsigned dummy;
				funcName = cast<CallInst>(I)->getCalledFunction()->getName();
				if (I->getType() != getVoidTy &&
					(VFH::findVectorFunctionInHash(funcName, &dummy) || 
					 VFH::findScalarFunctionInHash(funcName) ||
					 funcProperties->getAnyProperty(I, PR_ALL_SPECIAL_CASE_FUNCS)))
				{
					deadBuiltIn = true;
				}
			}
			
			if ( deadBuiltIn || ((!I->mayWriteToMemory()) && (!isa<TerminatorInst>(I))))
			{
				// Dead instruction: no uses and has no memory writes and is not a terminator inst, or is a built-in function with no uses
				for (User::op_iterator OI = I->op_begin(), E = I->op_end(); OI != E; ++OI)
					if (Instruction * used = dyn_cast<Instruction>(*OI))
						WorkList.push_back(used);
				
				// Actual erase
				funcProperties->setProperty(I, PR_INST_IS_REMOVED);
				I->eraseFromParent();
				
				for (std::vector<Instruction*>::iterator WI = WorkList.begin(); WI != WorkList.end();)
				{
					if (*WI == I)
						WI = WorkList.erase(WI);
					else
						++WI;
				}
			}
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Prepare for loops: Separate return inst to new block. also clone scalar function into end of vector function
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::prepareKernelForLoop(Function * scalarFunc, CodeProperties * funcProperties)
{
	// Need to make sure that the return instruction has its own basic block
	Instruction * retInst = funcProperties->getRetInst();
	BasicBlock * retBlock = retInst->getParent();
	if (cast<Instruction>(retBlock->begin()) != retInst)
	{
		// Block holds several instructions. Split it to create a ret-only block
		retBlock = retBlock->splitBasicBlock(BasicBlock::iterator(retInst));
	}
	
	// Need to generate a remainder block (a block which comes after the code, but before the exit)
	BasicBlock * remBlock = retBlock;
	retBlock = remBlock->splitBasicBlock(BasicBlock::iterator(retInst));
	funcProperties->setRemainderBlock(remBlock);
	remBlock->setName("Remainder_block");
	
	// save return block in properties
	funcProperties->setExitBlock(retBlock);
	
	// Need to generate a scalar loop, which loops over the scalar code (with incrementing IDs)
	funcProperties->setScalarLoopBlock(loopObject.generateScalarLoop(scalarFunc, funcProperties));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// add check after get_local_size, if size is smaller than vector_width, and branch to scalar code
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::addVectorWidthCheck(Function * scalarFunc, CodeProperties * funcProperties)
{
	BasicBlock * scalarLoopTarget = funcProperties->getScalarLoopBlock();
	Instruction * loopSize = funcProperties->getLoopSizeVal();
	BasicBlock * headBlock = loopSize->getParent();
	
	// Split block right after obtain local_size
	BasicBlock::iterator splitInst = BasicBlock::iterator(loopSize);
	++splitInst;
	BasicBlock * contBlock = headBlock->splitBasicBlock(splitInst);
	
	// Add compare of local_size to vector width
	ICmpInst * compareForVecWidth = new ICmpInst(headBlock->getTerminator(), 
												 CmpInst::ICMP_SLT, 
												 loopSize,
												 ConstantInt::get(loopSize->getType() ,ARCH_VECTOR_WIDTH),
												 "check.width");
	
	// Add conditional branch
	headBlock->getTerminator()->eraseFromParent();
	BranchInst::Create(scalarLoopTarget, contBlock, compareForVecWidth, headBlock);

	// Attach correct PHI node
	PHINode * countNode = dyn_cast<PHINode>(scalarLoopTarget->begin());
	V_ASSERT(countNode); // something fishy...
	countNode->addIncoming(ConstantInt::get(countNode->getType() ,0), headBlock);
	

	
	
	
	// Fix remainder block, to jump to scalar code if remainder > 0
	BasicBlock * remainderBlock = funcProperties->getRemainderBlock();
	Instruction * remainder = BinaryOperator::Create(Instruction::And, 
													 loopSize, 
													 ConstantInt::get(loopSize->getType() ,ARCH_VECTOR_WIDTH-1), 
													 "remainder.calc", 
													 remainderBlock->getTerminator());
	if (remainder->getType() != countNode->getType())
	{
		remainder = CastInst::CreateIntegerCast(remainder, countNode->getType(), false, "cast", remainderBlock->getTerminator());
	}
	ICmpInst * compareForRemainder = new ICmpInst(remainderBlock->getTerminator(), 
												  CmpInst::ICMP_UGT, 
												  remainder,
												  ConstantInt::get(remainder->getType() ,0),
												  "check.remainder");
	// Replace direct branch with conditional branch
	funcProperties->setProperty(remainderBlock->getTerminator(), PR_INST_IS_REMOVED);
	remainderBlock->getTerminator()->eraseFromParent();
	BranchInst::Create(scalarLoopTarget, funcProperties->getExitBlock(), compareForRemainder, remainderBlock);
	countNode->addIncoming(remainder, remainderBlock);
	
}



#if defined(VERIFY_FUNCS)
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug utility, checking that function names which are used in the vectorizer, can actually
// be found in the runtime module
/////////////////////////////////////////////////////////////////////////////////////////////////////
void VectModuleManager::VerifyFunctionsListsAreUpToDate()
{
	V_PRINT("Validating existance of builtin functions....\n");
	bool allFuncsExist = true;
	// Go over entire functions list, and check that each name appears in the builtin funcs module
	unsigned num_functions = VFH::debugGetNumEntries();
	for (unsigned i = 0; i < num_functions; i++)
	{
		VFH::hashEntry * current = VFH::debugGetEntry(i);
		for (unsigned funcIndex = 0; funcIndex < 6; funcIndex++)
		{
			allFuncsExist &= checkFunctionInBuiltin(current->funcs[funcIndex]);
		}
	}
	
	// Go over special-case functions, and check that they appear in the builtin funcs module
	// Select functions:   selectFuncsList[5][SUPPORTED_WIDTHS]	
	for (unsigned i = 0; i < SUPPORTED_WIDTHS; ++i)
	{
		for (unsigned j = 0; j < 5; ++j)
		{
			allFuncsExist &= checkFunctionInBuiltin(selectFuncsList[j][i]);
		}
	}

	// Geometric functions:   typedef const char * geometricListType[GEOMETRIC_WIDTHS][VERTICAL_FUNCS]	
	for (unsigned i = 0; i < GEOMETRIC_WIDTHS; ++i)
	{
		for (unsigned j = 0; j < VERTICAL_FUNCS; ++j)
		{
			allFuncsExist &= checkFunctionInBuiltin(geometric_dot[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_distance[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_fast_distance[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_length[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_fast_length[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_cross[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_normalize[i][j]);
			allFuncsExist &= checkFunctionInBuiltin(geometric_fast_normalize[i][j]);
		}
	}
	
	// Read sampler 2D
	allFuncsExist &= checkFunctionInBuiltin(READ_IMAGEF_2D_NAME);
	allFuncsExist &= checkFunctionInBuiltin(TRANSPOSED_READ_IMAGEF_2D_NAME);
	allFuncsExist &= checkFunctionInBuiltin(STREAM_READ_IMAGEF_2D_NAME);
	// Read sampler 3D
	allFuncsExist &= checkFunctionInBuiltin(READ_IMAGEF_3D_NAME);
	allFuncsExist &= checkFunctionInBuiltin(TRANSPOSED_READ_IMAGEF_3D_NAME);
	// Write sampler	
	allFuncsExist &= checkFunctionInBuiltin(WRITE_IMAGEF_NAME);
	allFuncsExist &= checkFunctionInBuiltin(TRANSPOSED_WRITE_IMAGEF_NAME);
	allFuncsExist &= checkFunctionInBuiltin(STREAM_WRITE_IMAGEF_NAME);
	
	// Fract functions: const char * fractFuncsList[SUPPORTED_WIDTHS];
	for (unsigned i = 0; i < SUPPORTED_WIDTHS; ++i)
	{
		allFuncsExist &= checkFunctionInBuiltin(fractFuncsList[i]);
	}
	
	// Gamma funcs
	allFuncsExist &= checkFunctionInBuiltin(CI_GAMMA_SCALAR_NAME);
	allFuncsExist &= checkFunctionInBuiltin(CI_GAMMA_TRANSPOSED_NAME);
	
	
	
	// If any functions are missing - Assert!
	if (!allFuncsExist)
		V_UNEXPECTED("Some expected builtin functions were not found in builtin module!");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug utility helper, checking that a function name can actually be found in the runtime module
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VectModuleManager::checkFunctionInBuiltin(const char * name)
{
	if (std::strcmp(name, "_") != 0)
	{				
		// Verify that the found function has a declaration in the runtime module
		const Function *foundFunc = runtimeModule->getFunction(name);
		if (!foundFunc)
		{
			V_PRINT("built-in function " << name << " Not found!\n");
			return false;
		}
	}
	return true;
}
#endif



