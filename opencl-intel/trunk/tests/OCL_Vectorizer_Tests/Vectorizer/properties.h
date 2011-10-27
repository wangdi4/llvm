/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "common.h"


// Instruction properties: Each property occupies a bit in the properties bit-field
typedef long long instProperty;

#define	PR_INST_IS_REMOVED			0x0000000000000001ll // Instruction was removed
#define	PR_FUNC_PREP_TO_REMOVE		0x0000000000000002ll // Instruction to be removed (as it is a temp used for function preparation)
#define	PR_OBTAIN_CL_INDEX			0x0000000000000004ll // Instruction is an index creator (get_global/local_id(0))
#define	PR_MOVE_TO_LOOP_HEAD		0x0000000000000008ll // Mark instruction which was moved to the head of the mega-loop
#define	PR_MOVE_TO_LOOP_TAIL		0x0000000000000010ll // Mark instruction which was moved to the tail of the mega-loop
#define	PR_LOOP_PIVOT				0x0000000000000020ll // Mark instruction which became a loop pivot
#define	PR_TID_DEPEND				0x0000000000000040ll // Instruction's value is dependent on input (get_global_id, etc..)
#define	PR_TID_VALS_CONSECUTIVE		0x0000000000000080ll // In TID-dependent instruction, values are consecutive
#define	PR_TID_VALS_EQUAL_DIST		0x0000000000000100ll // In TID-dependent instruction, values are within equal distances of each other
#define	PR_PTRS_CONSECUTIVE			0x0000000000000200ll // Pointers (TID dependent) are consecutive (in quantum of pointer type!)
#define	PR_APPEARS_IN_LISTS			0x0000000000000400ll // Mark instructions that appear in any of the lists (TID setters, etc...)
#define	PR_SCALARIZABLE_BUILT_IN	0x0000000000000800ll // Mark function-call to scalarizable/vectorizable built-in function (will also appear in scalarizableFuncs list)
#define	PR_SPECIAL_CASE_BUILT_IN	0x0000000000001000ll // Mark function-call to special case built-in function (will also appear in SClist)
#define	PR_NO_SIDE_EFFECT			0x0000000000002000ll // Mark instruction as having no side-effect. Used especially for fake unctions
	
	// Special case functions
#define	PR_SC_SELECT_INST			0x0000000100000000ll // Select function call
#define	PR_SC_READ_SAMPLER_F_2D		0x0000000200000000ll // Read sampler 2D
#define	PR_SC_READ_SAMPLER_F_3D		0x0000000400000000ll // Read sampler 3D
#define	PR_SC_WRITEF_SAMPLER		0x0000000800000000ll // Write sampler 
#define	PR_SC_DOTPROD				0x0000001000000000ll // Dot-product
#define	PR_SC_GEO_DISTANCE			0x0000002000000000ll // Geometric distance
#define	PR_SC_GEO_FAST_DISTANCE		0x0000004000000000ll // Geometric fast_distance
#define	PR_SC_GEO_LENGTH			0x0000008000000000ll // Geometric length
#define	PR_SC_GEO_FAST_LENGTH		0x0000010000000000ll // Geometric fast_length
#define PR_SC_CROSS					0x0000020000000000ll // Geometric cross-product
#define PR_SC_GEO_NORMALIZE			0x0000040000000000ll // Geometric normalize
#define PR_SC_GEO_FAST_NORMALIZE	0x0000080000000000ll // Geometric fast_normalize
#define	PR_SC_STREAM_READ_SAMPLER	0x0000100000000000ll // Stream Read sampler
#define	PR_SC_STREAM_WRITE_SAMPLER	0x0000200000000000ll // Stream Write sampler 
#define	PR_SC_FRACT					0x0000400000000000ll // Fract function
#define	PR_SC_CI_GAMMA				0x0000800000000000ll // ci_gamma function
#define	PR_SC_BOUNDARY_CHECK		0x0001000000000000ll // Function representing a kernel boundary check (early exit)
	
#define	PR_ALL_SPECIAL_CASE_FUNCS	0x0001FFFF00000000ll // Mask of all special-case function markings
	
#define	PR_MAX_PROPERTY				0x8000000000000000ll


// Function properties
enum funcProperty {
	ERROR__FUNC_HAS_UNSUPPORTED_SEQ	 = 0x1,	// Function contains unsupported or unknown code sequence. Serious error - Cannot be vectorized!
	FUNC_CONTAINS_BARRIER			 = 0x2,	// Function contains a barrier (one or more)
	FUNC_CONTAINS_WG_SYNC_OP		 = 0x4,	// Function contains "async" function calls (which operate on work-group granularity)
	FUNC_HAS_MULTIPLE_EXITS			 = 0x8,	// Function has more than one exit point
	FUNC_HAS_INDEPENDENT_EARLY_EXITS = 0x10, // Function contains early-exits which are TID-independent
	FUNC_HAS_TID_DEPEND_EARLY_EXITS	 = 0x20, // Function contains early-exits which depend on TID value
};


// For performance reasons, objects of this class must be created per-function, and not per-module!

class CodeProperties
{
	typedef SmallVectorImpl<Value *> ValueList;

public:			

	CodeProperties(Module *targetModule, const Module *runtimeMod, Function * currFunc, unsigned archVectorWidth);
	~CodeProperties();
	
	// Manage instructions properties
	bool getProperty(Value * inst, instProperty prop);
	bool getAnyProperty(Value * inst, instProperty properties);
	instProperty getPropertyGroup(Value * inst, instProperty propertyMask);
	void setProperty(Value * inst, instProperty prop);
	void clearProperty(Value * inst, instProperty prop);
	void duplicateProperties(Value * dst, Value * src);

	// Manage function properties
	bool getFuncProperty(funcProperty prop);
	void setFuncProperty(funcProperty prop);
	
	// Manage TID-setters list
	void addToTIDSettersList(Instruction * inst);
	Instruction * firstTIDSetterFromList();
	Instruction * nextTIDSetterFromList();
	
	// Manage pseudo-dependent inst list
	void addToPseudoDependentList(Instruction * inst);
	Instruction * firstPseudoDependentFromList();
	Instruction * nextPseudoDependentFromList();
	
	// Manage per-instruction special-case lists (for special case function calls)
	bool setSCList(Instruction * inst, ValueList *list);
	ValueList * getSCList(Instruction * inst);
	
	// Manage scalarizable Functions list
	void setScalarizableFunc(Instruction * callInst, ValueList *list);
	ValueList * getScalarizableList(Instruction * inst);
	
	// Manage saving the ret instruction
	void setRetInst(Instruction * inst);
	Instruction * getRetInst();

	// Manage saving the Loop iteration count instruction
	void setIterCountInst(Instruction * inst);
	Instruction * getIterCountInst();

	// Manage saving the Loop size value
	void setLoopSizeVal(Instruction * inst);
	Instruction * getLoopSizeVal();	
	
	// Manage saving exit-block (for early exits)
	void setExitBlock(BasicBlock * block);
	BasicBlock * getExitBlock();

	// Manage saving remainder block (for early exits)
	void setRemainderBlock(BasicBlock * block);
	BasicBlock * getRemainderBlock();
	
	// Manage saving scalar-loop code section (for early exits)
	void setScalarLoopBlock(BasicBlock * block);
	BasicBlock * getScalarLoopBlock();
	
	// Manage list of fake functions which were used as temporaries in the vectorization process
	void addFakeFunctionToList(Function * func);
	Function * firstFakeFunctionFromList();
	Function * nextFakeFunctionFromList();
	void clearFakeFunctionFromList();

	void replaceAllInstProperties(Instruction * src, Instruction * dst);

	LLVMContext& context() {return *moduleContext;}
	
	// Pointers for usage of all service classes (scalarize, vectorize, etc..)
	Module * currentModule; // Pointer to current module
	const Module * runtimeModule; // Pointer to runtime module (which holds the OCL builtin functions)
	Function * currentFunction; // Pointer to current function
	
	// Used vector width 
	unsigned m_archVectorWidth;
	
private:
	CodeProperties(); // Do not implement
	LLVMContext * moduleContext; // Pointer to LLVM Context of module
	
	DenseMap<Value*,instProperty> propertiesMap;
	uint32_t functionProperties;
	SmallVector<Instruction *, 2> TIDSetters;
	unsigned TIDSetter_iterator;
	SmallVector<Instruction *, 4> PseudoDependentInsts;
	unsigned PseudoDependentInsts_iterator;
	Instruction * retInst;
	Instruction * iterCount;
	Instruction * loopSize;
	BasicBlock * exitBlock;
	BasicBlock * remainderBlock;
	BasicBlock * scalarLoopBlock;
	SmallVector<Function *, 8> FakeFunctions;
	unsigned FakeFunctions_iterator;
	
	// A map between special-case func calls and their inputs/outputs list (structure of list is specific per-callType)
	DenseMap<Instruction*,ValueList*> specialCaseListsMap;

	// a map between Call instructions, and their output/input arguments (their root values, without casts)
	DenseMap<Instruction*,ValueList*> scalarizableFunctionsMap;
};


#endif // __PROPERTIES_H__

