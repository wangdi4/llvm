; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Bitcast initializer will be unsafe pointer store. Check that the
; corresponding fields are Multiple Value.

%struct.MYSTRUCT = type { i32, i32, i32 }

@raw_data = internal dso_local global [6 x i32][i32 0, i32 0, i32 0, i32 1, i32 1, i32 1], align 16

@geo_data = internal dso_local global %struct.MYSTRUCT* bitcast([6 x i32]* @raw_data to %struct.MYSTRUCT*), align 8

define dso_local i32 @main() {
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32, i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value

