; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test call to memset with pointer that can alias multiple types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01a = type { i64 }
%struct.test01b = type { i32, i32 }
define void @test01() {
  %pStructA = alloca %struct.test01a
  %pB = bitcast %struct.test01a* %pStructA to %struct.test01b*

  ; Use the pointer as %strut.test01b to infer the bitcast type.
  %field = getelementptr %struct.test01b, %struct.test01b* %pB, i64 0, i32 1
  store i32 0, i32* %field

  %pStart = bitcast %struct.test01b* %pB to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target | Local instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Ambiguous GEP | Ambiguous pointer target{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i64 }
!4 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!dtrans_types = !{!3, !4}
