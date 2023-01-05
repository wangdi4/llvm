; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check the case of an array of structs within a struct.

%struct.MYOUTERSTRUCT = type { i32, float, [2 x %struct.MYINNERSTRUCT] }
%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT* null, align 8

declare noalias i8* @calloc(i64, i64) #0

define dso_local i32 @main() {
  %1 = tail call noalias i8* @calloc(i64 1, i64 24) #2
  store i8* %1, i8** bitcast (%struct.MYOUTERSTRUCT** @coxglobalstruct to i8**), align 8
  ret i32 0
}

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 0
; CHECK: Field LLVM Type: float
; CHECK: float 0.000000e+00

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, [2 x %struct.MYINNERSTRUCT] }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 0
; CHECK: Field LLVM Type: float
; CHECK: Single Value: float 0.000000e+00
; CHECK: Field LLVM Type: [2 x %struct.MYINNERSTRUCT]
; CHECK: Multiple Value

