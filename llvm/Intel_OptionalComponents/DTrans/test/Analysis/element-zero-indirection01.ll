; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test access of a pointer to struct.test.c via a bitcast and load of
; a pointer to pointer to struct.test.b.

; This should not generate bad bitcast or mismatched element access warnings.

%struct.test.a = type { i32, i32, %struct.test.b* }
%struct.test.b = type { %struct.test.c, i32, i32 }
%struct.test.c = type { i32, i32, i32, i32 }

define void @test(%struct.test.a* %p) {
  %tmp1 = getelementptr %struct.test.a, %struct.test.a* %p, i64 0, i32 2
  %tmp2 = bitcast %struct.test.b** %tmp1 to %struct.test.c**
  %tmp3 = load %struct.test.c*, %struct.test.c** %tmp2
  %tmp4 = getelementptr %struct.test.c, %struct.test.c* %tmp3, i64 0, i32 2
  %tmp5 = load i32, i32* %tmp4
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.a = type { i32, i32, %struct.test.b* }
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.b = type { %struct.test.c, i32, i32 }
; CHECK: Safety data: Contains nested structure 

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test.c = type { i32, i32, i32, i32 }
; CHECK: Safety data: Nested structure
