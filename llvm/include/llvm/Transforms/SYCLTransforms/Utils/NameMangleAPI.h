//===- NameMangleAPI.h - Name mangle APIs -----------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_NAME_MANGLE_API_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_NAME_MANGLE_API_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"

namespace llvm {
namespace NameMangleAPI {

/// Indicates whether the given string is the named of a mangled function.
/// \return true if rawString is the name of a mangled function, false
/// otherwise.
bool isMangledName(StringRef RawString);

/// Converts the given string to function descriptor, that represents the
/// function's prototype.
/// In case of failures, an exception is thrown.
reflection::FunctionDescriptor demangle(StringRef Rawstring,
                                        bool IsSpir12Name = false);

/// Return the stripped function name, of the function mangled by the given
/// string. If the given string is not the mangled name of a builtin, an
/// exception is thrown.
StringRef stripName(StringRef Rawstring);

/// Converts the given function descriptor to string that represents the
/// function's prototype.
/// The mangling algorithm is based on Itanium mangling algorithm
/// (http://sourcery.mentor.com/public/cxx-abi/abi.html#mangling), with SPIR
/// extensions.
std::string mangle(const reflection::FunctionDescriptor &FD);

} // namespace NameMangleAPI
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_NAME_MANGLE_API_H
