; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test access of a pointer to struct.test.c via a bitcast and load of
; a pointer to pointer to struct.test.b.

; This should not generate bad bitcast or mismatched element access warnings.

%struct.test.a = type { %struct.test.b*, i32, i32 }
%struct.test.b = type { %struct.test.c, i32, i32 }
%struct.test.c = type { i32, i32, i32, i32 }

define void @test(%struct.test.a* %p) {
  %tmp1 = bitcast %struct.test.a* %p to %struct.test.c**
  %tmp2 = load %struct.test.c*, %struct.test.c** %tmp1
  %tmp3 = getelementptr %struct.test.c, %struct.test.c* %tmp2, i64 0, i32 2
  %tmp4 = load i32, i32* %tmp3
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.a = type { %struct.test.b*, i32, i32 }
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.b = type { %struct.test.c, i32, i32 }
; CHECK: Safety data: Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.c = type { i32, i32, i32, i32 }
; CHECK: Safety data: Nested structure
