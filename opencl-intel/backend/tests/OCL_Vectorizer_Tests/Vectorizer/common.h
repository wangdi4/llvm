/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__


#include <map>
#include <stack>
#include <vector>
#include <list>
#include <stdio.h>
#include <string>
#include <sstream>
#include "llvm/Pass.h"
#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Target/TargetData.h"
#include "llvm/DerivedTypes.h"
#include "functions.h"
#include "Vectorizer.h"
#include "properties.h"

using namespace llvm;

#if defined(RELEASE)
# define V_ASSERT(x)		while(0);
# define V_PRINT(x)			while(0);
# define V_INIT_PRINT		while(0);
# define V_DESTROY_PRINT	while(0);
# define V_DUMP(ptr)		while(0);
# define V_DUMP_MODULE(ptr)	while(0);
#else
extern FILE * prtFile;
extern FILE * moduleDumpFile;
# include "llvm/Support/raw_ostream.h"
# define V_INIT_PRINT								\
{													\
	prtFile = fopen("/tmp/vectorizer.txt", "a");	\
	moduleDumpFile = fopen("/tmp/module.txt", "a");	\
}
# define V_DESTROY_PRINT							\
{													\
	fclose(prtFile);								\
	fclose(moduleDumpFile);							\
}
# define V_PRINT(x)									\
{													\
	std::string tmpStr;								\
	llvm::raw_string_ostream strstr(tmpStr);		\
	strstr << x ;									\
	fprintf(prtFile, "%s", strstr.str().c_str());	\
	fflush(prtFile);								\
}										
# define V_DUMP(ptr)								\
{													\
	std::string tmpStr;								\
	llvm::raw_string_ostream strstr(tmpStr);		\
	strstr << "**********\n";						\
	ptr->print(strstr, NULL);						\
	strstr << "**********\n";						\
	fprintf(prtFile, "%s", strstr.str().c_str());	\
	fflush(prtFile);								\
}
# define V_DUMP_MODULE(ptr)							\
{													\
	std::string tmpStr;								\
	llvm::raw_string_ostream strstr(tmpStr);		\
	ptr->print(strstr, NULL);						\
	fprintf(moduleDumpFile, "%s", strstr.str().c_str());	\
	fflush(moduleDumpFile);							\
}
# define V_ASSERT(x)										\
{															\
	if (!(x))												\
	{														\
		V_PRINT("Assertion (" << #x << ") in file: " << __FILE__ << "  line " << __LINE__ << "\n");	\
		sleep(1);											\
		exit(-1);											\
	}														\
}
#endif


#if defined(ASSERT_UNEXPECTED)
# define V_UNEXPECTED(x)							\
{													\
	V_PRINT("Unexpected behavior: " << x << "\n");	\
	V_ASSERT(0);									\
}
#else
# define V_UNEXPECTED(x)
#endif


// Macro for pulling out LOG of basic vector widths (1,2,4,8, and 16)
const int logs_vals[] = {-1, 0, 1, -1, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, 4};
#define LOG_(x) logs_vals[x]


// Used for setting the size of a container, which holds pointers to all the functions
#define ESTIMATED_NUM_OF_FUNCTIONS 8

#if !defined(MAX_LOOP_SIZE)
Error! Target max loop size not defined!
#endif

// Maximum supported value for vector width
#define MAX_SUPPORTED_VECTOR_WIDTH 16

// Maximum width supported in OpenCL
#define MAX_OCL_VECTOR_WIDTH 16

// Define amount of VCM/SCM entries in a buffer.
#define ESTIMATED_INST_NUM 128

// Define maximal acceptable size of kernel for vectorization. Anything bigger will be rejected
#define MAX_ACCEPTABLE_KERNEL_SIZE  ESTIMATED_INST_NUM*16


#define getVoidTy	Type::getVoidTy(funcProperties->context())
#define getFloatTy	Type::getFloatTy(funcProperties->context())
#define getInt1Ty	Type::getInt1Ty(funcProperties->context())
#define getInt8Ty	Type::getInt8Ty(funcProperties->context())
#define getInt16Ty	Type::getInt16Ty(funcProperties->context())
#define getInt32Ty	Type::getInt32Ty(funcProperties->context())
#define getInt64Ty	Type::getInt64Ty(funcProperties->context())

#define CURRENT_MODULE		funcProperties->currentModule
#define RUNTIME_MODULE		funcProperties->runtimeModule
#define CURRENT_FUNCTION	funcProperties->currentFunction
#define ARCH_VECTOR_WIDTH   funcProperties->m_archVectorWidth

#define FLOAT_X_WIDTH__ALIGNMENT ARCH_VECTOR_WIDTH * 4

#define GET_GID_NAME  "get_global_id"
#define GET_LID_NAME  "get_local_id"
#define GET_LOCAL_SIZE "get_local_size"


// Definitions for work-group granularity functions
#define BARRIER_FUNC_NAME	  "barrier"
#define WG_FUNCS_NAME_PREFIX  "__async"  


#endif // __COMMON_H__
