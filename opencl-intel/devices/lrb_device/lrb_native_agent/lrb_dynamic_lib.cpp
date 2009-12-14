// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_dynamic_lib.cpp
///////////////////////////////////////////////////////////
#include "lrb_dynamic_lib.h"
#include <stdio.h>
#ifdef NETSIM_ENABLED
#include <windows.h>
#endif

/************************************************************************
 * Return the lib name with extension depends on the platform this call
 * is running on.
 ************************************************************************/
static char* get_lib_full_name(const char* pLibName)
{
    char* exten = NULL;
    char* pFullFileName = NULL;
#ifdef NETSIM_ENABLED
    exten = ".dll";
#else
    exten = ".so";
#endif

    size_t szFileNameLen = strlen(pLibName) + strlen(exten) + 1;
    pFullFileName = (char*)malloc(szFileNameLen);
    sprintf_s(pFullFileName, szFileNameLen, "%s%s", pLibName, exten);
    
    return pFullFileName;
}

/************************************************************************
 * 
 ************************************************************************/
void* lrb_dynmic_load_library(const char* pLibName)
{
    if( pLibName == NULL )
    {
        return NULL;
    }
    void* hLib = NULL;
    char* pLibFullName = get_lib_full_name(pLibName);

#ifdef NETSIM_ENABLED
    hLib = LoadLibraryA(pLibFullName);
#endif

    free(pLibFullName);
    return hLib;
}

/************************************************************************
 *
 ************************************************************************/
void lrb_dynmic_close_library( void* hLib)
{
#ifdef NETSIM_ENABLED
    FreeLibrary((HMODULE)hLib);
#endif
    return;
}

/************************************************************************
 * 
 ************************************************************************/
void* lrb_dynmic_get_function( void* hLib, char* pFuncName)
{
    void* pFunc = NULL;
#ifdef NETSIM_ENABLED    
    pFunc = GetProcAddress((HMODULE)hLib, pFuncName);
#endif
    return pFunc;
}

/************************************************************************
 *
 ************************************************************************/
void* lrb_dynmic_get_variable( void* hLib, char* pVarName)
{
    void* varAddr = NULL;
#ifdef NETSIM_ENABLED    
    varAddr = GetProcAddress((HMODULE)hLib, pVarName);
#endif
    return varAddr;
}

/************************************************************************
 *
 ************************************************************************/
const char* lrb_dynmic_trim_file_path( const char* pFileName )
{
    char delimiter = '\0';
#ifdef NETSIM_ENABLED    
    delimiter = '\\';
#endif
    size_t len = strlen(pFileName); 
    // Load the program/library into the context
    
    while ( pFileName[--len] != delimiter ) {};
    return &(pFileName[len+1]);
}
