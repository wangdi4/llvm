#if INTEL_COLLAB
//===-- elf_constants.h - ELF constants -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Here we duplicate some ELF constants that may not be yet available
// in elf.h or llvm/BinaryFormat/ELF.h (e.g. in an out-of-tree build).
// Note that we use the same constant names as in llvm/BinaryFormat/ELF.h.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_CONSTANTS_H
#define LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_CONSTANTS_H

// ELF notes with "LLVMOMPOFFLOAD" name may be of either of these types.
enum : unsigned {
  NT_LLVM_OPENMP_OFFLOAD_VERSION = 1,
  NT_LLVM_OPENMP_OFFLOAD_PRODUCER = 2,
  NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION = 3,
  NT_LLVM_OPENMP_OFFLOAD_LAST = NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION
};

constexpr const char *OMPNoteTypeNames[NT_LLVM_OPENMP_OFFLOAD_LAST + 1] = {
    "NT_LLVM_OPENMP_OFFLOAD_UNKNOWN",
    "NT_LLVM_OPENMP_OFFLOAD_VERSION",
    "NT_LLVM_OPENMP_OFFLOAD_PRODUCER",
    "NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION",
};

#endif // LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_CONSTANTS_H
#endif // INTEL_COLLAB
