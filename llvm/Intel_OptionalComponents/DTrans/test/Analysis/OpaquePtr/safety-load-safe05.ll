; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that safety analyzer handling of load instructions with pointer operands
; of 'undef' or 'null' do not cause an assertion failure.

%struct.test01 = type { i64, i64, i64 }

define internal void @test01() {
  %l = alloca %struct.test01
  %u = load i64, ptr undef
  %n = load i64, ptr null

  %f0 = getelementptr %struct.test01, ptr %l, i64 0, i32 0
  %f1 = getelementptr %struct.test01, ptr %l, i64 0, i32 1
  store i64 %u, ptr %f0
  store i64 %n, ptr %f1
  ret void
}

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local instance{{ *}}
; CHECK: End LLVMType: %struct.test01
