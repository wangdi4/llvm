#ifndef __CLANG_UTILS_H__
#define __CLANG_UTILS_H__

#include <string>
#include "OclBuiltinEmitter.h"

#define XSTR(A) STR(A)
#define STR(A) #A

///////////////////////////////////////////////////////////////////////////////
//Purpose: generates a dummy body for the given ocl (primitive) type, with the
//given vector length.
///////////////////////////////////////////////////////////////////////////////
std::string generateDummyBody(const std::string& type, size_t veclen);

///////////////////////////////////////////////////////////////////////////////
//Purpose: compiles the given ocl code (using clang), and writes the result to
//the given file name.
///////////////////////////////////////////////////////////////////////////////
void build(const std::string& code, std::string fileName);

#endif//__CLANG_UTILS_H__
