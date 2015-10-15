// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#pragma once

#include "crt_types.h"


#include <map>
#include <string>


namespace OCLCRT
{
namespace Utils
{

/************************************************************************
Class:          OclDynamicLib
Description:    Handles load/release and function pointer
                retrieval from the dynamically loaded libraries
************************************************************************/
class OclDynamicLib
{
public:
    OclDynamicLib();
    virtual ~OclDynamicLib();

    // Checks for existance of a file with specified name
    // Input
    //      pLibName    - A pointer to null tirminated string that describes library file name
    // Returns
    //      true - file exists
    //      false - file doesn't exist
    static  crt_err_code    IsExists( const char* pLibName );

    // Loads a dynamically link library into process address space
    // Input
    //      pLibName    - A pointer to null tirminated string that describes library file name
    // Returns
    //      true - if succesully loaded
    //      false - if file doesn't exists or other error has occured
    crt_err_code    Load( const char* pLibName );

    // Release all allocated resourses and unloads the library
    void Close();


    // Returns a number of named functions found in the library
    unsigned int    GetNumberOfFunctions() const;

    // Returns a pointer to function name
    // Input
    //      uiFuncId    -   An ordinal number of the function
    // Return
    //      A pointer to valid function name
    //      NULL - if ordinal number is out of bounds
    const char*     GetFunctionName( unsigned int uiFuncId ) const;

    // Returns a function pointer
    // Input
    //      uiFuncId    -   An ordinal number of the function
    // Return
    //      A pointer to valid function
    //      NULL - if ordinal number is out of bounds
    const void*     GetFunctionPtr( unsigned int uiFuncId ) const;

    // Returns a function pointer by name
    // Input
    //      szFuncName  -   Function name, null terminated string
    // Return
    //      A pointer to valid function
    //      NULL - if ordinal number is out of bounds
    typedef void (*func_t)( void );
    func_t    GetFunctionPtrByName( const char* szFuncName ) const;

protected:
    void*           m_hLibrary;     // A handle to loaded library

    unsigned int    m_uiFuncCount;
    unsigned int*   m_pOffsetNames; // A pointer to offsets of function names
    unsigned int*   m_pOffsetFunc;  // A pointer to offsets of functions
};

} // namespace Utils
} // namespace OCLCRT
