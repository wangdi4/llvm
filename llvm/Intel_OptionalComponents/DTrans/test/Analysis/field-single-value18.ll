; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check the fields of an array of structures

%struct.MYSTRUCT = type { i32, i32, i32 }

@geo_data = internal dso_local global [2 x %struct.MYSTRUCT] [%struct.MYSTRUCT {i32 0, i32 0, i32 1}, %struct.MYSTRUCT {i32 0, i32 1, i32 1}], align 8

define dso_local i32 @main() {
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32, i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 0
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 1

