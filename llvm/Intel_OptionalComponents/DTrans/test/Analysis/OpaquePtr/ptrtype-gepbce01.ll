; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test access to first element of the structure, which happens to also serve
; as the address of the structure. This is for the BitCastEquivalent clause
; of the getelementptr analyzer.


; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test bitcast equivalent for a GEP
define internal void @test01() {
  %local = alloca [80 x i8]
  %bce = getelementptr inbounds [80 x i8], [80 x i8]* %local, i64 0, i64 0
  store i8 32, i8* %bce
  ret void
}
; CHECK-LABEL: void @test01
; CHECK:   %local = alloca [80 x i8]
; CHECK:      Aliased types:
; CHECK-NEXT:   [80 x i8]*
; CHECK:      No element pointees.

; CHECK-CUR: %bce = getelementptr inbounds [80 x i8], [80 x i8]* %local, i64 0, i64 0
; CHECK-FUT: %bce = getelementptr inbounds [80 x i8], p0 %local, i64 0, i64 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   [80 x i8]*
; CHECK-NEXT:   i8*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT    [80 x i8] @ 0


; Test bitcast equivalent for GEP that addresses a structure element that starts
; with an i8 element.
%struct.test02 = type { i8, i32, i64 }
define internal void @test02(%struct.test02* %in) !dtrans_type !4 {
  %ptr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 0
  store i8 64, i8* %ptr
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-CUR: %ptr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02*
; CHECK-NEXT:   i8*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test02 @ 0


; Test bitcast equivalent for GEP that addresses a structure element that starts
; with an array of i8 elements.
%struct.test03a = type { [256 x i8], i32 }
%struct.test03b = type { %struct.test03a }
define internal void @test03(%struct.test03b* %in) !dtrans_type !10 {
  %ptr = getelementptr %struct.test03b, %struct.test03b* %in, i64 0, i32 0, i32 0, i32 0
  store i8 64, i8* %ptr
  ret void
}
; CHECK-LABEL: void @test03
; CHECK-CUR: %ptr = getelementptr %struct.test03b, %struct.test03b* %in, i64 0, i32 0, i32 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test03b*
; CHECK-NEXT:   i8*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:  [256 x i8] @ 0


!1 = !{i8 0, i32 0}  ; i8
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test02*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"A", i32 256, !1}  ; [256 x i8]
!9 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!10 = !{!"F", i1 false, i32 1, !5, !11}  ; void (%struct.test03b*)
!11 = !{!12, i32 1}  ; %struct.test03b*
!12 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!13 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i8, i32, i64 }
!14 = !{!"S", %struct.test03a zeroinitializer, i32 2, !8, !2} ; { [256 x i8], i32 }
!15 = !{!"S", %struct.test03b zeroinitializer, i32 1, !9} ; { %struct.test03a }

!dtrans_types = !{!13, !14, !15}
