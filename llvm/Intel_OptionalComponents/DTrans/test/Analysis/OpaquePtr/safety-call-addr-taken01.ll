; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

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
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Address taken | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!3 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2, !3}
