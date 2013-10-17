/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __LOGGER__H__
#define __LOGGER__H__

#include "llvm/Support/raw_ostream.h"

#ifdef WIN_DEBUG

#include <stdlib.h>
extern FILE * prtFile;
extern FILE * moduleDmp;

#define V_INIT_PRINT                                    \
{                                                       \
    prtFile = fopen("vectorizer.txt", "a");             \
    moduleDmp = fopen("module.txt", "a");               \
}
#define V_DESTROY_PRINT                                 \
{                                                       \
    fclose(prtFile);                                    \
    fclose(moduleDmp);                                  \
}
#define V_PRINT(type,x)                                 \
{                                                       \
    std::string tmpStr;                                 \
    llvm::raw_string_ostream strstr(tmpStr);            \
    strstr << x ;                                       \
    fprintf(prtFile, "%s", strstr.str().c_str());       \
    fflush(prtFile);                                    \
}    
#define V_DUMP(ptr)                                     \
{                                                       \
    std::string tmpStr;                                 \
    llvm::raw_string_ostream strstr(tmpStr);            \
    ptr->print(strstr, NULL);                           \
    fprintf(prtFile, "%s", strstr.str().c_str());       \
    fflush(prtFile);                                    \
}
#define V_DUMP_MODULE(ptr)                              \
{                                                       \
    std::string tmpStr;                                 \
    llvm::raw_string_ostream strstr(tmpStr);            \
    ptr->print(strstr, NULL);                           \
    fprintf(moduleDmp, "%s", strstr.str().c_str());     \
    fflush(moduleDmp);                                  \
}
#define V_ASSERT(x)                                     \
{                                                       \
    if (!(x))                                           \
    {                                                   \
        V_PRINT(assert, "Assertion in: " <<    __FILE__ \
        << ":" << __LINE__ << ": " << #x << "\n");      \
        assert(0);                                      \
    }                                                   \
}


#else 

#include "llvm/Support/Debug.h"
#define V_INIT_PRINT
#define V_DESTROY_PRINT
#ifndef NDEBUG
    #define V_PRINT(type, x)    DEBUG_WITH_TYPE( #type , errs() << x )
#else
    #define V_PRINT(type, x)    do { } while (0)
#endif
#define V_DUMP(ptr)
#define V_DUMP_MODULE(ptr)
#define V_ASSERT(x)         assert( x )

#endif 

#ifndef NDEBUG
#define V_STAT(x) x
#else
#define V_STAT(x)
#endif

#endif // __LOGGER__H__
