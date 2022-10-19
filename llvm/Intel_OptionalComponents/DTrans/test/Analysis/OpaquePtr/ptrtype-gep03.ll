; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on getelementptr instructions involving 0 or
; 1 indices

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test with the special case of a GEP with 0 indices.
%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  %tmp = getelementptr %struct.test01, %struct.test01* %in
  %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE: %tmp = getelementptr %struct.test01, %struct.test01* %in
; CHECK-OPAQUE: %tmp = getelementptr %struct.test01, ptr %in
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
; CHECK-OPAQUE: %f0 = getelementptr %struct.test01, ptr %tmp, i64 0, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0


; Test a GEP that use used for a pointer-to-pointer
%struct.test02 = type { i64, i64 }
define internal void @test02(%struct.test02** "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  %p2p = getelementptr %struct.test02*, %struct.test02** %in, i64 5
  %ptr = load %struct.test02*, %struct.test02** %p2p
  %field_addr = getelementptr %struct.test02, %struct.test02* %ptr, i64 0, i32 1
  store i64 0, i64* %field_addr
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-NONOPAQUE: %p2p = getelementptr %struct.test02*, %struct.test02** %in, i64 5
; CHECK-OPAQUE: %p2p = getelementptr ptr, ptr %in, i64 5
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02**{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %ptr = load %struct.test02*, %struct.test02** %p2p
; CHECK-OPAQUE: %ptr = load ptr, ptr %p2p
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE: %field_addr = getelementptr %struct.test02, %struct.test02* %ptr, i64 0, i32 1
; CHECK-OPAQUE: %field_addr = getelementptr %struct.test02, ptr %ptr, i64 0, i32 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %struct.test02 @ 1


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }

!intel.dtrans.types = !{!7, !8}
