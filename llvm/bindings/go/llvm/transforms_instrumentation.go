//===- transforms_instrumentation.go - Bindings for instrumentation -------===//
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

<<<<<<< HEAD
func (pm PassManager) AddThreadSanitizerPass() {
	C.LLVMAddThreadSanitizerPass(pm.C)
}

#if INTEL_CUSTOMIZATION
func (pm PassManager) AddMemorySanitizerLegacyPassPass() {
	C.LLVMAddMemorySanitizerLegacyPassPass(pm.C)
}

func (pm PassManager) AddAddressSanitizerFunctionPass() {
	C.LLVMAddAddressSanitizerFunctionPass(pm.C)
}

func (pm PassManager) AddAddressSanitizerModulePass() {
	C.LLVMAddAddressSanitizerModulePass(pm.C)
}
#endif // INTEL_CUSTOMIZATION

=======
>>>>>>> c74a706893f0667d6aae2d7704d21af97c92dc07
func (pm PassManager) AddDataFlowSanitizerPass(abilist []string) {
	abiliststrs := make([]*C.char, len(abilist))
	for i, arg := range abilist {
		abiliststrs[i] = C.CString(arg)
		defer C.free(unsafe.Pointer(abiliststrs[i]))
	}
	C.LLVMAddDataFlowSanitizerPass(pm.C, C.int(len(abilist)), &abiliststrs[0])
}
