#if INTEL_COLLAB //  -*- C++ -*-
//===---- InferAddressSpacesUtils.h - Utilities for addrspace-*- C++ -*----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
#if INTEL_CUSTOMIZATION
class Module;
#endif  // INTEL_CUSTOMIZATION
class TargetTransformInfo;

// The utility performs the propagation of specific address space from
// type-quailifed variable declarations to its users.
bool InferAddrSpaces(const TargetTransformInfo &TTI, unsigned addrSpace,
                     Function &F);

#if INTEL_CUSTOMIZATION
// The utility modifies types of variables, so that the member/element
// pointers are transformed from flat address space pointers to
// pointers to specific address space.
//
// This utility may transform LLVM Module, so it may only be called
// from a Module pass.
bool InferAddrSpacesForGlobals(unsigned FlatAddrSpace, Module &M);
#endif  // INTEL_CUSTOMIZATION
} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_INFERADDRESSSPACESUTILS_H
#endif // INTEL_COLLAB
