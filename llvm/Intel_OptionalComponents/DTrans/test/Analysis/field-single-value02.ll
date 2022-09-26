; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that assignments to fields inconsistent with an initializer yields
; Multiple Value.  Check that struct within a struct is Multiple Value.

%struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT zeroinitializer, align 4

define dso_local i32 @main() {
  store i32 6, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i64 0, i32 0), align 4
  store float 8.000000e+00, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i64 0, i32 1), align 4
  store i32 6, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i64 0, i32 2, i32 0), align 4
  store float 8.000000e+00, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i64 0, i32 2, i32 1), align 4
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Multiple Value

