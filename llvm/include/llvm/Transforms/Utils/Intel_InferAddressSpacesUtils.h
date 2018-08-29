#if INTEL_COLLAB //  -*- C++ -*-
//===- Intel_InferAddressSpacesUtils.h - Utilities for addrspace-*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides utilities to infer the address space.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_UTILS_INFERADDRESSSPACESUTILS_H
#define LLVM_TRANSFORMS_UTILS_INFERADDRESSSPACESUTILS_H

namespace llvm {

class Function;
class TargetTransformInfo;

// The utility performs the propagation of specific address space from
// type-quailifed variable declarations to its users.
bool InferAddrSpaces(const TargetTransformInfo &TTI, unsigned addrSpace,
                     Function &F);

} // namespace llvm

#endif
#endif // INTEL_COLLAB
