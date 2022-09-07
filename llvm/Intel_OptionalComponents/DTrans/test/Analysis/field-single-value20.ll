; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check the case of an array of structs within a struct.

%struct.MYOUTERSTRUCT = type { i32, float, [2 x %struct.MYINNERSTRUCT] }
%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT* null, align 8

declare noalias i8* @malloc(i64)

define dso_local i32 @main() {
  %1 = tail call noalias i8* @malloc(i64 24) #2
  store i8* %1, i8** bitcast (%struct.MYOUTERSTRUCT** @coxglobalstruct to i8**), align 8
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: No Value
; CHECK: Field LLVM Type: float
; CHECK: No Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, [2 x %struct.MYINNERSTRUCT] }
; CHECK: Field LLVM Type: i32
; CHECK: No Value
; CHECK: Field LLVM Type: float
; CHECK: No Value
; CHECK: Field LLVM Type: [2 x %struct.MYINNERSTRUCT]
; CHECK: Multiple Value

