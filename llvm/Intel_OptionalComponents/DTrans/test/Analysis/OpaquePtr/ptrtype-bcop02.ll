; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on bitcast operator when used on
; variables.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.

%struct.test01 = type { i64, i32*, %struct.test01* }
@test_var01 = internal global %struct.test01* zeroinitializer, !intel_dtrans_type !3
; Test bitcast operator processing when used in a load instruction.
define internal void @test01() {
  %v0 = load i8*, i8** bitcast (%struct.test01** @test_var01 to i8**)
  call void @foo(i8* %v0)
  ret void
}

; NOTE: In the future, when pointers are opaque types, this case will no
; longer involve a constant expression bitcast operator.

; CHECK-LABEL: void @test01() {
; CHECK-NONOPAQUE:  %v0 = load i8*, i8** bitcast (%struct.test01** @test_var01 to i8**)
; CHECK-OPAQUE:  %v0 = load ptr, ptr @test_var01
; CHECK-NONOPAQUE:     CE: i8** bitcast (%struct.test01** @test_var01 to i8**)
; CHECK-NONOPAQUE-NEXT: LocalPointerInfo:
; CHECK-NONOPAQUE-NEXT: Aliased types:
; CHECK-NONOPAQUE-NEXT:   %struct.test01**{{ *$}}
; CHECK-NONOPAQUE-NEXT:   i8**{{ *$}}
; CHECK-NONOPAQUE-NEXT: No element pointees.

; This corresponds to the type recovered for %v0.
; CHECK: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; Helper function. Nothing of interest to check.
define internal void @foo(i8* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ; Need to use the value to establish the callsite parameter as getting used
  ; as the type.
  %val = load i8, i8* %in
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64, i32*, %struct.test01* }

!intel.dtrans.types = !{!6}
