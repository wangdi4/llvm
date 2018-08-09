// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
