; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test the PtrTypeAnalyzer handling of a store instruction that uses an
; inttoptr operator as the pointer operand that is not from a variable.


define internal void @test01() {
  store i32 1, ptr inttoptr (i64 120 to ptr)
  ret void
}

; CHECK-LABEL: define internal void @test01
; CHECK: store i32 1, ptr inttoptr (i64 120 to ptr)
; CHECK-NEXT: ptr inttoptr (i64 120 to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHEKC-NEXT: No Element pointees

%struct.test02 = type { i64, i64, i64 }
define internal void @test02() {
  %l = alloca %struct.test02
  %pti = ptrtoint ptr %l to i64
  store i64 %pti, ptr inttoptr (i64 1024 to ptr)
  ret void
}

; CHECK-LABEL: define internal void @test02
; CHECK: store i64 %pti, ptr inttoptr (i64 1024 to ptr)
; CHECK-NEXT: ptr inttoptr (i64 1024 to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHEKC-NEXT: No Element pointees

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

