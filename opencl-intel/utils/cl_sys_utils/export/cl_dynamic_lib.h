/////////////////////////////////////////////////////////////////////////
// cl_dynamic_lib.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once

#include "cl_utils.h"

#include <map>
#include <string>

namespace Intel { namespace OpenCL { namespace Utils
{
    /************************************************************************
     * OclDynamicLib is a class that handles load/release and function pointer
     * retrival from the dynmaically loaded libraries
    /************************************************************************/ 
    class OclDynamicLib
    {
    public:
	    OclDynamicLib();
	    virtual ~OclDynamicLib();

		// Checks for existance of a file with specified name
		// Input
		//		pLibName	- A pointer to null tirminated string that describes library file name
		// Returns
		//		true - file exists
		//		false - file doesn't exist
		static	bool	IsExists(const char* pLibName);

		// Loads a dynamically link library into process address space
		// Input
		//		pLibName	- A pointer to null tirminated string that describes library file name
		// Returns
		//		true - if succesully loaded
		//		false - if file doesn't exists or other error has occured
				bool	Load(const char* pLibName);

		// Release all allocated resourses and unloads the library
				void	Close();

		// Informative functions

		// Returns a number of named functions found in the library
				unsigned int	GetNumberOfFunctions() const;

		// Returns a pointer to function name
		// Input
		//		uiFuncId	-	An ordinal number of the function
		// Return
		//		A pointer to valid function name
		//		NULL - if ordinal number is out of bounds
				const char*		GetFunctionName(unsigned int uiFuncId) const;

		// Returns a function pointer
		// Input
		//		uiFuncId	-	An ordinal number of the function
		// Return
		//		A pointer to valid function
		//		NULL - if ordinal number is out of bounds
				const void*		GetFunctionPtr(unsigned int uiFuncId) const;

		// Returns a function pointer by name
		// Input
		//		szFuncName	-	Function name, null terminated string
		// Return
		//		A pointer to valid function
		//		NULL - if ordinal number is out of bounds
				const void*		GetFunctionPtrByName(const char* szFuncName) const;

	protected:
		void*			m_hLibrary;		// A handle to loaded library

		unsigned int	m_uiFuncCount;
		unsigned int*	m_pOffsetNames;	// A pointer to offsets of function names
		unsigned int*	m_pOffsetFunc;	// A pointer to offsets of functions
	};

}}}    // Intel::OpenCL::Utils
