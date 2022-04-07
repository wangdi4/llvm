//===- Intel_InferAddressSpacesUtils.h - Utilities for addrspace-*- C++ -*-===//
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
//===----------------------------------------------------------------------===//
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
#ifndef LLVM_TRANSFORMS_UTILS_INTEL_INFERADDRESSSPACESUTILS_H
#define LLVM_TRANSFORMS_UTILS_INTEL_INFERADDRESSSPACESUTILS_H

namespace llvm {

class Module;

// The utility modifies types of variables, so that the member/element
// pointers are transformed from flat address space pointers to
// pointers to specific address space.
//
// This utility may transform LLVM Module, so it may only be called
// from a Module pass.
bool InferAddrSpacesForGlobals(unsigned FlatAddrSpace, Module &M);
} // namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_INTEL_INFERADDRESSSPACESUTILS_H
