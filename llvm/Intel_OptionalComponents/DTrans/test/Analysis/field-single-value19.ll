; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that an unsafe pointer store sets structs to Multiple Value.

%struct.Geometry = type { i32, i32, i32 }

@raw_data = global [6 x i32] [i32 0, i32 0, i32 0, i32 1, i32 1, i32 1], align 16
@geo_data = common global %struct.Geometry* null, align 8

define i32 @main() {
  store %struct.Geometry* bitcast ([6 x i32]* @raw_data to %struct.Geometry*), %struct.Geometry** @geo_data, align 8
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.Geometry = type { i32, i32, i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value

