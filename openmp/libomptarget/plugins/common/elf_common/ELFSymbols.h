#if INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===-- ELFSymbols.h - ELF Symbol look-up functionality ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ELF routines for obtaining a symbol from an Elf file without loading it.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_SYMBOLS_H
#define LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_SYMBOLS_H

#include "llvm/Object/ELF.h"
#include "llvm/Object/ELFObjectFile.h"

/// Returns the symbol associated with the \p Name in the \p ELFObj. It will
/// first search for the hash sections to identify symbols from the hash table.
/// If that fails it will fall back to a linear search in the case of an
/// executable file without a hash table.
llvm::Expected<const typename llvm::object::ELF64LE::Sym *>
getELFSymbol(const llvm::object::ELFObjectFile<llvm::object::ELF64LE> &ELFObj,
             llvm::StringRef Name);

#endif
#endif // INTEL_CUSTOMIZATION
