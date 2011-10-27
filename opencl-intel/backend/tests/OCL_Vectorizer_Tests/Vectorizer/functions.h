/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __FuNCTIONs_H_
#define __FuNCTIONs_H_

#include <string>


#define MAX_INPUT_ARGS_IN_FUNCTION_HASH 3

class VFH // VectorizerFuncsHash
{
public:
	// Enumeration of possible argument (and ret value) types
	enum data_type
	{
		T_NONE,		// No type (meaning no value)
		T_I8,		// Type i8 (or vector of that type)
		T_I16,		// Type i16 (or vector of that type)
		T_I32,		// Type i32 (or vector of that type)
		T_I64,		// Type i64 (or vector of that type)
		T_FLOAT,	// Type float (or vector of that type)
		T_DOUBLE,	// Type double (or vector of that type)
		T_I32p,		// Type pointer to i32 (or vector of that type)
		T_I32gp,	// Type pointer to i32 [global mem] (or vector of that type)
		T_I32lp,	// Type pointer to i32 [local mem] (or vector of that type)
		T_FLOATp,	// Type pointer to float (or vector of that type)
		T_FLOATgp,	// Type pointer to float [global mem] (or vector of that type)
		T_FLOATlp,	// Type pointer to float [local mem] (or vector of that type)
		T_DOUBLEp,	// Type pointer to double (or vector of that type)
		T_DOUBLEgp,	// Type pointer to double [global mem] (or vector of that type)
		T_DOUBLElp,	// Type pointer to double [local mem] (or vector of that type)
		T_STATIC	// Any type - but not vectorized
	};

	typedef struct hashEntry
	{
		char funcArgs[1 + MAX_INPUT_ARGS_IN_FUNCTION_HASH];	// The return value type, followed by input arguments
		const char * funcs[6]; // Name of functions, sorted as (1,2,4,8,16,3)
	} hashEntry;
	
	
	/*************************************************************************
	 * Search the hash for a vector function (used by scalarizer)
	 *
	 * Search the given name in the functions hash (may be of any vector size,
	 * but not scalar).
	 * Returns a structure which holds all the vector width "versions" of the 
	 * function (and types of required arguments), or NULL if not found
	 * also returns the vector width of found functions (2, 4, 8, or 16) 
	 *************************************************************************/
	static hashEntry * findVectorFunctionInHash(std::string &inp_name, unsigned * vecWidth);

	/*************************************************************************
	 * Search the hash for a scalar function (used by vectorizer)
	 *
	 * Search the given name in the functions hash (may only be of scalar type)
	 * Returns a structure which holds all the vector width "versions" of the 
	 * function (and tpye of required arguments), or NULL if not found
	 *************************************************************************/
	static hashEntry * findScalarFunctionInHash(std::string &inp_name);
	
	/*************************************************************************
	 * Debug-related functions:
	 *
	 * 1) get the amount of hashEntries (all the different builtin functions)
	 * 2) get a specific hashEntry, according to its "serial" ordering  
	 *************************************************************************/
	static unsigned debugGetNumEntries();
	static hashEntry * debugGetEntry(unsigned num);
	
};

#endif // __FuNCTIONs_H_
