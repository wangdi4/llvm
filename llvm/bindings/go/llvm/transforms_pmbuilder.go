//===- transforms_pmbuilder.go - Bindings for PassManagerBuilder ----------===//
// INTEL_CUSTOMIZATION
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
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines bindings for the PassManagerBuilder class.
//
//===----------------------------------------------------------------------===//

package llvm

/*
#include "llvm-c/Transforms/PassManagerBuilder.h"
#include "llvm-c/Transforms/Coroutines.h" // INTEL
*/
import "C"

type PassManagerBuilder struct {
	C C.LLVMPassManagerBuilderRef
}

func NewPassManagerBuilder() (pmb PassManagerBuilder) {
	pmb.C = C.LLVMPassManagerBuilderCreate()
	return
}

func (pmb PassManagerBuilder) SetOptLevel(level int) {
	C.LLVMPassManagerBuilderSetOptLevel(pmb.C, C.uint(level))
}

func (pmb PassManagerBuilder) SetSizeLevel(level int) {
	C.LLVMPassManagerBuilderSetSizeLevel(pmb.C, C.uint(level))
}

func (pmb PassManagerBuilder) Populate(pm PassManager) {
	C.LLVMPassManagerBuilderPopulateModulePassManager(pmb.C, pm.C)
}

func (pmb PassManagerBuilder) PopulateFunc(pm PassManager) {
	C.LLVMPassManagerBuilderPopulateFunctionPassManager(pmb.C, pm.C)
}

func (pmb PassManagerBuilder) Dispose() {
	C.LLVMPassManagerBuilderDispose(pmb.C)
}

func (pmb PassManagerBuilder) SetDisableUnitAtATime(val bool) {
	C.LLVMPassManagerBuilderSetDisableUnitAtATime(pmb.C, boolToLLVMBool(val))
}

func (pmb PassManagerBuilder) SetDisableUnrollLoops(val bool) {
	C.LLVMPassManagerBuilderSetDisableUnrollLoops(pmb.C, boolToLLVMBool(val))
}

func (pmb PassManagerBuilder) SetDisableSimplifyLibCalls(val bool) {
	C.LLVMPassManagerBuilderSetDisableSimplifyLibCalls(pmb.C, boolToLLVMBool(val))
}

func (pmb PassManagerBuilder) UseInlinerWithThreshold(threshold uint) {
	C.LLVMPassManagerBuilderUseInlinerWithThreshold(pmb.C, C.uint(threshold))
}
// INTEL_CUSTOMIZATION
func (pmb PassManagerBuilder) AddCoroutinePassesToExtensionPoints() {
	C.LLVMPassManagerBuilderAddCoroutinePassesToExtensionPoints(pmb.C);
}
// end INTEL_CUSTOMIZATION
