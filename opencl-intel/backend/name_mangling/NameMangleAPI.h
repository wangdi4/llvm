/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FunctionDescriptor.h"
#include "llvm/ADT/StringRef.h"
#include <string>

/// Indicates whether the given string is the named of a mangled function
/// \return true if rawString is the name of a mangled function, false
/// otherwise.
bool isMangledName(const char *rawString);

/// Converts the given string to function descriptor, that represents the
/// function's prototype.
/// In case of failures, an exception is thrown.
reflection::FunctionDescriptor demangle(const char *rawstring,
                                        bool isSpir12Name = false);

/// \returns the stripped function name, of the function mangled by the given
/// string. If the given string is not the mangled name of a builtin, an
/// exception is thrown.
llvm::StringRef stripName(const char *rawstring);

/// Converts the given function descriptor to string that represents the
/// function's prototype.
/// The mangling algorithm is based on Itanium mangling algorithm
/// (http://sourcery.mentor.com/public/cxx-abi/abi.html#mangling), with SPIR
/// extensions.
std::string mangle(const reflection::FunctionDescriptor &);
