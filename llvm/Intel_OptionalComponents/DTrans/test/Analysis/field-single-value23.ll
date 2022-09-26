; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that consistent writes to fields do not destroy single values.
; Here, ints and floats in different structures have different values.

%struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct1 = internal dso_local global %struct.MYOUTERSTRUCT { i32 7, float 3.000000e+00, %struct.MYINNERSTRUCT { i32 6, float 7.000000e+00 } }, align 4

@coxglobalstruct2 = internal dso_local global %struct.MYOUTERSTRUCT { i32 7, float 9.000000e+00, %struct.MYINNERSTRUCT { i32 6, float 8.000000e+00 } }, align 4

define dso_local i32 @main() {
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 6
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 7
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Multiple Value


