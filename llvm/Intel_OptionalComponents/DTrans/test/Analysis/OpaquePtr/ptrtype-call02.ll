; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK

; Test that pointer type recovery handles "call" instructions to vararg
; functions. By handled, it means that the value info for the argument
; should not get tagged as "UNHANDLED"

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64, i64 }
define internal void @test01() {
  %type1 = alloca %struct.test01a
  %type2 = alloca %struct.test01b
  call void (i32, ...) @va_test01(i32 0, %struct.test01a* %type1);
  call void (i32, ...) @va_test01(i32 1, %struct.test01b* %type2);
  ret void
}
; CHECK-LABEL: internal void @test01
; CHECK:  %type1 = alloca %struct.test01a
; CHECK:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        %struct.test01a*
; CHECK:      No element pointees.

; CHECK:  %type2 = alloca %struct.test01b
; CHECK:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        %struct.test01b*
; CHECK:      No element pointees.


define internal void @va_test01(i32 %in1, ...) {
  ret void
}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!4 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i64, i64 }

!dtrans_types = !{!3, !4}
