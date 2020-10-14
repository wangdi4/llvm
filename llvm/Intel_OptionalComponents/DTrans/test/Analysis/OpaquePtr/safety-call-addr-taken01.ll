; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure as a pointer-sized int type. This should
; result in the "Address taken" safety bit on the type passed because it is unknown
; what type the callee will use the value as. On the callee side, this test will
; produce the "Unsafe pointer store" safety flag because a pointer-sized int is
; used as a pointer to an aggregate type.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01a
  %pStruct.as.i64 = ptrtoint %struct.test01a* %pStruct to i64
  call void @test01h(i64 %pStruct.as.i64)
  ret void
}
define void @test01h(i64 %in) {
  %pStruct = alloca %struct.test01b
  %pStruct.as.p64 = bitcast %struct.test01b* %pStruct to i64*
  store i64 %in, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Address taken | Local instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!3 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!2, !3}
