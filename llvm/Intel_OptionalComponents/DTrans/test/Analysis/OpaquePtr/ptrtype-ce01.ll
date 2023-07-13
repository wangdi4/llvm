; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on constant operator expressions


%struct.test01 = type { i64, double, [25 x i8] }
@test_var01 = internal global %struct.test01 zeroinitializer

; Test GEPOperator processing
define internal void @test01() {
  %v0 = load i64, ptr getelementptr (%struct.test01, ptr @test_var01, i64 0, i32 0)
  %v1 = load double, ptr getelementptr (%struct.test01, ptr @test_var01, i64 0, i32 1)
  %v2 = load i8, ptr getelementptr (%struct.test01, ptr @test_var01, i64 0, i32 2, i32 0)
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK:  %v1 = load double, ptr getelementptr inbounds (%struct.test01, ptr @test_var01, i64 0, i32 1)
; CHECK:         CE: ptr getelementptr inbounds (%struct.test01, ptr @test_var01, i64 0, i32 1)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        double*{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; CHECK:  %v2 = load i8, ptr getelementptr inbounds (%struct.test01, ptr @test_var01, i64 0, i32 2, i32 0)
; CHECK:         CE: ptr getelementptr inbounds (%struct.test01, ptr @test_var01, i64 0, i32 2, i32 0)
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*{{ *$}}
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:        [25 x i8] @ 0

; Test PtrToIntOperator processing
%struct.test02 = type { ptr, ptr }
@test_var02 = internal global %struct.test02 zeroinitializer
define internal void @test02() {
  %local = alloca i64
  store i64 ptrtoint (ptr getelementptr (%struct.test02, ptr @test_var02, i64 0, i32 1) to i64), ptr %local
  ret void
}
; CHECK-LABEL: void @test02()
; CHECK:  store i64 ptrtoint (ptr getelementptr inbounds (%struct.test02, ptr @test_var02, i64 0, i32 1) to i64), ptr %local
; CHECK:     CE: i64 ptrtoint (ptr getelementptr inbounds (%struct.test02, ptr @test_var02, i64 0, i32 1) to i64)
; CHECK-NEXT:       LocalPointerInfo:
; CHECK-NEXT:            Aliased types:
; CHECK-NEXT:              %struct.test02**{{ *$}}
; CHECK-NEXT:            Element pointees:
; CHECK-NEXT:              %struct.test02 @ 1

; CHECK:     CE: ptr getelementptr inbounds (%struct.test02, ptr @test_var02, i64 0, i32 1)
; CHECK-NEXT:       LocalPointerInfo:
; CHECK-NEXT:          Aliased types:
; CHECK-NEXT:            %struct.test02**{{ *$}}
; CHECK-NEXT:          Element pointees:
; CHECK-NEXT:            %struct.test02 @ 1


; Test that unhandled constant expression operators get marked as UNHANDLED.
; AddOperator and IntToPtrOperator are not handled.
%struct.test03 = type { i64, i64 }
@test_var03 = internal global %struct.test03 zeroinitializer
define internal void @test03() {
  store ptr @test_var03, ptr inttoptr (i64 add (i64 4, i64 ptrtoint (ptr @test_var03 to i64)) to ptr)
  ret void
}
; CHECK-LABEL: void @test03()
; CHECK: store ptr @test_var03, ptr inttoptr (i64 add (i64 ptrtoint (ptr @test_var03 to i64), i64 4) to ptr)
; CHECK:   CE: ptr inttoptr (i64 add (i64 ptrtoint (ptr @test_var03 to i64), i64 4) to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>

; CHECK:   CE: i64 add (i64 ptrtoint (ptr @test_var03 to i64), i64 4)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>

!1 = !{i64 0, i32 0}  ; i64
!2 = !{double 0.0e+00, i32 0}  ; double
!3 = !{!"A", i32 25, !4}  ; [25 x i8]
!4 = !{i8 0, i32 0}  ; i8
!5 = !{i64 0, i32 1}  ; i64*
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64, double, [25 x i8] }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !6} ; { i64*, %struct.test02* }
!9 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!7, !8, !9}
