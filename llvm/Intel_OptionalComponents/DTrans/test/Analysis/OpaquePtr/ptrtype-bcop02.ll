; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on bitcast operator when used on
; variables.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are tests for the future opaque pointer form of IR.
; Lines marked with CHECK should remain the same when changing to use opaque pointers.

%struct.test01 = type { i64, i32*, %struct.test01* }
@test_var01 = internal global %struct.test01* zeroinitializer, !dtrans_type !3
; Test bitcast operator processing when used in a load instruction.
define internal void @test01() {
  %v0 = load i8*, i8** bitcast (%struct.test01** @test_var01 to i8**)
  call void @foo(i8* %v0)
  ret void
}

; NOTE: In the future, when pointers are opaque types, this case will no
; longer involve a constant expression bitcast operator.

; CHECK-LABEL: void @test01() {
; CHECK-CUR:  %v0 = load i8*, i8** bitcast (%struct.test01** @test_var01 to i8**)
; CHECK-FUT:  %v0 = load p0, p0 @test_var01
; CHECK-CUR:     CE: i8** bitcast (%struct.test01** @test_var01 to i8**)
; CHECK-CUR:          LocalPointerInfo:
; CHECK-CUR:      Aliased types:
; CHECK-CUR-NEXT:   %struct.test01**{{ *$}}
; CHECK-CUR-NEXT:   i8**{{ *$}}
; CHECK-CUR-NEXT: No element pointees.

; This corresponds to the type recovered for %v0.
; CHECK:    LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

; Helper function. Nothing of interest to check.
define internal void @foo(i8* %in) !dtrans_type !5 {
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"F", i1 false, i32 1, !6, !7}  ; void (i8*)
!6 = !{!"void", i32 0}  ; void
!7 = !{i8 0, i32 1}  ; i8*
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64, i32*, %struct.test01* }

!dtrans_types = !{!8}
