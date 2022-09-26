// INTEL_CUSTOMIZATION
//===- transforms_coroutines.go - Bindings for coroutines -----------------===//
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
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
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines bindings for the coroutines component.
//
//===----------------------------------------------------------------------===//

package llvm

/*
#include "llvm-c/Transforms/Coroutines.h"
*/
import "C"

func (pm PassManager) AddCoroEarlyPass()   { C.LLVMAddCoroEarlyPass(pm.C) }
func (pm PassManager) AddCoroSplitPass()   { C.LLVMAddCoroSplitPass(pm.C) }
func (pm PassManager) AddCoroElidePass()   { C.LLVMAddCoroElidePass(pm.C) }
func (pm PassManager) AddCoroCleanupPass() { C.LLVMAddCoroCleanupPass(pm.C) }
// end INTEL_CUSTOMIZATION
