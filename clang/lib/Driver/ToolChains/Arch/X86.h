//===--- X86.h - X86-specific Tool Helpers ----------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_X86_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_X86_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/Option.h"
#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace x86 {

std::string getX86TargetCPU(const Driver &D, const llvm::opt::ArgList &Args,
                            const llvm::Triple &Triple);

void getX86TargetFeatures(const Driver &D, const llvm::Triple &Triple,
                          const llvm::opt::ArgList &Args,
                          std::vector<llvm::StringRef> &Features);

#if INTEL_CUSTOMIZATION
bool isValidIntelCPU(StringRef CPU, const llvm::Triple &Triple);
std::string getCPUForIntel(StringRef Arch, const llvm::Triple &Triple,
                           bool IsArchOpt = false);
std::string getCPUForIntelOnly(const Driver &D, StringRef Arch,
                               const llvm::Triple &Triple,
                               const llvm::opt::Arg *A);
#endif // INTEL_CUSTOMIZATION

} // end namespace x86
} // end namespace target
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_X86_H
