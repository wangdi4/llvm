; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on getelementptr instructions involving 0 or
; 1 indices

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test with the special case of a GEP with 0 indices.
%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %in) !dtrans_type !2 {
  %tmp = getelementptr %struct.test01, %struct.test01* %in
  %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR: %tmp = getelementptr %struct.test01, %struct.test01* %in
; CHECK-FUT: %tmp = getelementptr %struct.test01, p0 %in
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01*
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
; CHECK-FUT: %f0 = getelementptr %struct.test01, p0 %tmp, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0


; Test a GEP that use used for a pointer-to-pointer
%struct.test02 = type { i64, i64 }
define internal void @test02(%struct.test02** %in) !dtrans_type !7 {
  %p2p = getelementptr %struct.test02*, %struct.test02** %in, i64 5
  %ptr = load %struct.test02*, %struct.test02** %p2p
  %field_addr = getelementptr %struct.test02, %struct.test02* %ptr, i64 0, i32 1
  store i64 0, i64* %field_addr
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-CUR: %p2p = getelementptr %struct.test02*, %struct.test02** %in, i64 5
; CHECK-FUT: %p2p = getelementptr p0, p0 %in, i64 5
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02**
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %ptr = load %struct.test02*, %struct.test02** %p2p
; CHECK-FUT: %ptr = load p0, p0 %p2p
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02*
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %field_addr = getelementptr %struct.test02, %struct.test02* %ptr, i64 0, i32 1
; CHECK-FUT: %field_addr = getelementptr %struct.test02, p0 %ptr, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %struct.test02 @ 1


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i64 0, i32 0}  ; i64
!7 = !{!"F", i1 false, i32 1, !3, !8}  ; void (%struct.test02**)
!8 = !{!9, i32 2}  ; %struct.test02**
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 2, !6, !6} ; { i64, i64 }

!dtrans_types = !{!10, !11}
