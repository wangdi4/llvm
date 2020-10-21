; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on getelementptr instructions involving the
; address of an array element for an array contained within structure. In
; this case, the address does not also match the address of the
; structure element, so no extra information needs to be stored with the
; "Element pointee".

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01a = type { i64, float, %struct.test01b }
%struct.test01b = type { i64, [10 x i8] }
@var01a = internal global %struct.test01a zeroinitializer

define i8 @test01()  {
  %array_elem_addr = getelementptr %struct.test01a, %struct.test01a* @var01a, i64 0, i32 2, i32 1, i64 3
  %val = load i8, i8* %array_elem_addr
  ret i8 %val
}

; Cases where the indexed element is not an unknown offset or the same
; address as the parent container, do not require extra information to
; be stores in the "Element pointee".

; CHECK-LABEL: i8 @test01
; CHECK-CUR:  %array_elem_addr = getelementptr %struct.test01a, %struct.test01a* @var01a, i64 0, i32 2, i32 1, i64 3
; CHECK-FUT:  %array_elem_addr = getelementptr %struct.test01a, p0 @var01a, i64 0, i32 2, i32 1, i64 3
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [10 x i8] @ 3{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{!"A", i32 10, !5}  ; [10 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i64, float, %struct.test01b }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }

!dtrans_types = !{!6, !7}
