; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on pointer arithmetic idioms involving subtraction
; with a constant integer. When a constant is subtracted from a pointer,
; the subtraction result should track the pointer type associated with the
; original operand.

%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01** "intel_dtrans_func_index"="1" %p1, %struct.test01** "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !3 {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test01** %p1 to i64
  %t2 = ptrtoint %struct.test01** %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: void @test01
; CHECK:  %tmp = sub i64 %t1, 8
; CHECK-NEXT: LocalPointerInfo
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT: No element pointees.

%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %p1, %struct.test02* "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !5 {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test02* %p1 to i64
  %t2 = ptrtoint %struct.test02* %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: void @test02
; CHECK:  %tmp = sub i64 %t1, 8
; CHECK-NEXT: LocalPointerInfo
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6, !7}
