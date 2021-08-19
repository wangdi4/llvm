; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -force-opaque-pointers -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -force-opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test the PtrTypeAnalyzer handling of a store instruction that uses an
; inttoptr operator as the pointer operand that is not from a variable.

; These cases currently result in the pointer operand being marked as
; UNHANDLED which will disqualify types associated with the value
; operand by the safety analyzer.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

define internal void @test01() {
  store i32 1, i32* inttoptr (i64 120 to i32*)
  ret void
}

; CHECK-LABEL: define internal void @test01
; CHECK-NONOPAQUE: store i32 1, i32* inttoptr (i64 120 to i32*)
; CHECK-NONOPAQUE-NEXT: i32* inttoptr (i64 120 to i32*)
; CHECK-OPAQUE: store i32 1, ptr inttoptr (i64 120 to ptr)
; CHECK-OPAQUE-NEXT: ptr inttoptr (i64 120 to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>

%struct.test02 = type { i64, i64, i64 }
define internal void @test02() {
  %l = alloca %struct.test02
  %pti = ptrtoint %struct.test02* %l to i64
  store i64 %pti, i64* inttoptr (i64 1024 to i64*)
  ret void
}

; CHECK-LABEL: define internal void @test02
; CHECK-NONOPAQUE: store i64 %pti, i64* inttoptr (i64 1024 to i64*)
; CHECK-NONOPAQUE-NEXT: i64* inttoptr (i64 1024 to i64*)
; CHECK-OPAQUE: store i64 %pti, ptr inttoptr (i64 1024 to ptr), align 4
; CHECK-OPAQUE-NEXT: ptr inttoptr (i64 1024 to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>

!intel.dtrans.types = !{!2}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

