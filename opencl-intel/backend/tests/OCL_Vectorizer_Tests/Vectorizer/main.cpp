/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "main.h"

#ifndef RELEASE
FILE * prtFile;
FILE * moduleDumpFile;
#endif

char Vectorizer::ID = 0;


/////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the main method of the pass. It is responsible for the following
// 0.5) Prepare some useful constant values
// 1) Create a copy of all functions in the module
// 2) Iterate over the "new" functions, and try to convert (vectorize) them
// 2.1) If vectorization succeeded - add the vectorized function to the module
// 2.2) Else - erase the new function
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool Vectorizer::runOnModule(Module &M)
{	
	// Sanity check - we rely on our "flexible" vector width to be inside a limit. if not - must extend the limit!
	if (MAX_SUPPORTED_VECTOR_WIDTH < m_archVectorWidth)
	{
		V_UNEXPECTED("Error! requested vector size is bigger than expected limit!\n");
		return false;
	}	
		
	// set isVectorized and proper number of kernels to zero, just in case vectorization will fail
	numOfKernels = 0;
	isModuleVectorized = false;
	
	// check for some common module errors, before actually diving in
	GlobalVariable *annotation = M.getGlobalVariable("llvm.global.annotations");
	if (!annotation || !annotation->hasInitializer())
	{
		V_PRINT("Error! Module does not contain list of kernels. Not vectorizing!\n");
		return false;
	}	
	ConstantArray *init = dyn_cast<ConstantArray>(annotation->getInitializer());
	numOfKernels = init->getType()->getNumElements();
	if (numOfKernels == 0)
	{
		V_PRINT("Error! Module does not contain any kernels. Nothing to vectorize!\n");
		return false;
	}
	if (!runtimeModule)
	{
		V_PRINT("Error! No runtime module provided. Not vectorizing!\n");
		return false;
	}
	
	// Engulf entire vectorizer operation with try-catch. If an exception happens, we gracefully fail vectorization, but don't collapse
	try {
		
		// List all kernels in module
		V_PRINT("Found annotations in module. Listing:\n");
		for (unsigned i = 0, e = numOfKernels; i != e; ++i) 
		{
			ConstantStruct *elt = cast<ConstantStruct>(init->getOperand(i));
			Value *field0 = elt->getOperand(0)->stripPointerCasts();
			if (Function *F = dyn_cast<Function>(field0)) 
			{
				V_PRINT("\tKernel: " << F->getName() << "\n");
				scalarFuncsList.push_back(F);
			}
		}
		
		// Generate moduleManager, and pass module as input
		VectModuleManager modManager(&M, runtimeModule, m_archVectorWidth);
		
		// Perform actual vectorization of module
		isModuleVectorized = modManager.runOnModule(scalarFuncsList);
		
		// fill lists with vectorized kernels data
		if (isModuleVectorized)
		{
			modManager.getVectorizedFunctionsPointers(targetFunctionsList);
			modManager.getVectorizedFunctionsWidths(targetFunctionsMaxWidth);
		}		
	}
	catch (...) {
		// An exception happened. Just mark isModuleVectorized to false, so later queries will show no vectored functions
		V_PRINT("************** Exception caught!  Something went wrong during vectorization. Reporting no vectorization...\n");
		isModuleVectorized = false;
		return false;
	}	
	
	return isModuleVectorized;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface for Intel's OCL and Apple's OCL
// Class factory: Create a new Pass object  
/////////////////////////////////////////////////////////////////////////////////////////////////////
Pass *createVectorizerPass(const Module *runtimeModule, int basicVectorWidth)
{	
	return new Vectorizer(runtimeModule, basicVectorWidth);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface for Apple's OCL
// Populate list with names of vectored functions, or NULLs if vectorization failed  
/////////////////////////////////////////////////////////////////////////////////////////////////////
int Vectorizer::getVectorizerFunctions(SmallVectorImpl<Function*> &Functions)
{
	V_PRINT("\n\nArrived to getVectorizerFunctions.\n");
	if (isModuleVectorized)
	{
		Functions = targetFunctionsList; // Copy list of vectorized kernels
	}
	else
	{
		for (unsigned i = 0; i < numOfKernels; i++)
		{
			Functions.push_back(NULL); // report all kernels as failed vectorization
		}
	}
	return 0;
}
int getVectorizerFunctions(Vectorizer *V, SmallVectorImpl<Function*> &Functions)
{
	return V->getVectorizerFunctions(Functions);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface for Apple's OCL
// Populate lists with max&min widths of vectored functions, or 0s if vectorization failed  
/////////////////////////////////////////////////////////////////////////////////////////////////////
int Vectorizer::getVectorizerWidths(SmallVectorImpl<int> &MaxWidths)
{
	V_PRINT("\n\nArrived to getVectorizerWidths\n");
	if (isModuleVectorized)
	{
		MaxWidths = targetFunctionsMaxWidth; 
	}
	else
	{
		for (unsigned i = 0; i < numOfKernels; i++)
		{
			MaxWidths.push_back(0); // report all kernels as failed vectorization
		}
	}
#ifndef RELEASE	
	for (unsigned i = 0; i < numOfKernels; i++)
	{
		V_PRINT("\tConvert " << scalarFuncsList[i]->getName() << "   to max-Width " << MaxWidths[i] << "\n");		
	}
#endif	
	return 0;
}
int getVectorizerWidths(Vectorizer *V, SmallVectorImpl<int> &MaxWidths)
{
	return V->getVectorizerWidths(MaxWidths);
}




