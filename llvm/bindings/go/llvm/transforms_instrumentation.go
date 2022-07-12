//===- transforms_instrumentation.go - Bindings for instrumentation -------===//
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
// This file defines bindings for the instrumentation component.
//
//===----------------------------------------------------------------------===//

package llvm

/*
#include "InstrumentationBindings.h"
#include <stdlib.h>
*/
import "C"
import "unsafe"

#if INTEL_CUSTOMIZATION
func (pm PassManager) AddThreadSanitizerPass() {
	C.LLVMAddThreadSanitizerPass(pm.C)
}

func (pm PassManager) AddMemorySanitizerLegacyPassPass() {
	C.LLVMAddMemorySanitizerLegacyPassPass(pm.C)
}

#endif // INTEL_CUSTOMIZATION

func (pm PassManager) AddDataFlowSanitizerPass(abilist []string) {
	abiliststrs := make([]*C.char, len(abilist))
	for i, arg := range abilist {
		abiliststrs[i] = C.CString(arg)
		defer C.free(unsafe.Pointer(abiliststrs[i]))
	}
	C.LLVMAddDataFlowSanitizerPass(pm.C, C.int(len(abilist)), &abiliststrs[0])
}
