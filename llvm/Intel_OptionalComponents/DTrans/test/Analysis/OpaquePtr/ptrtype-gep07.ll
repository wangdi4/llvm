; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on getelementptr instructions involving the
; address of an array element for an array contained within structure.


%struct.test01a = type { i64, float, %struct.test01b }
%struct.test01b = type { i64, [10 x i8] }
@var01a = internal global %struct.test01a zeroinitializer

define i8 @test01()  {
  %array_elem_addr = getelementptr %struct.test01a, ptr @var01a, i64 0, i32 2, i32 1, i64 3
  %val = load i8, ptr %array_elem_addr
  ret i8 %val
}

; CHECK-LABEL: i8 @test01
; CHECK:  %array_elem_addr = getelementptr %struct.test01a, ptr @var01a, i64 0, i32 2, i32 1, i64 3
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [10 x i8] @ 3 ElementOf: %struct.test01b@1{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{!"A", i32 10, !5}  ; [10 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i64, float, %struct.test01b }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }

!intel.dtrans.types = !{!6, !7}
