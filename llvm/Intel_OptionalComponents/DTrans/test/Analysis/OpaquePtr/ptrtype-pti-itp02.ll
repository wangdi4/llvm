; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for "ptrtoint" and "inttoptr" instructions
; where result type of "inttoptr" differs from source type of "ptrtoint"

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i32 }
define internal void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  %i = ptrtoint %struct.test01a* %in to i64
  %p = inttoptr i64 %i to %struct.test01b*
  %half0 = getelementptr %struct.test01b, %struct.test01b* %p, i64 0, i32 0
  store i16 1, i16* %half0
  %half1 = getelementptr %struct.test01b, %struct.test01b* %p, i64 0, i32 1
  store i16 1, i16* %half1
  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK-NONOPAQUE: %p = inttoptr i64 %i to %struct.test01b*
; CHECK-OPAQUE: %p = inttoptr i64 %i to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test01a*
; CHECK-NEXT:     %struct.test01b*
; CHECK-NEXT:   No element pointees.

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !1} ; { i16, i16, i32 }

!intel.dtrans.types = !{!5, !6}
