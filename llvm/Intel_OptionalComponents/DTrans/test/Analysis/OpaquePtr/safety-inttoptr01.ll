; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test safety analyzer handling of a store instruction that uses an
; inttoptr operator as the pointer operand.

define internal void @test01() {
  store i32 1, i32* inttoptr (i64 120 to i32*)
  ret void
}

%struct.test02 = type { i64, i64, i64 }
define internal void @test02() {
  %l = alloca %struct.test02
  %pti = ptrtoint %struct.test02* %l to i64
  store i64 %pti, i64* inttoptr (i64 1024 to i64*)
  ret void
}

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Unsafe pointer store | Address taken | Local instance{{ *}}
; CHECK: End LLVMType: %struct.test02
