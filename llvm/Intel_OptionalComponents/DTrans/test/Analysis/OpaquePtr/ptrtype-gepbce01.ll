; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test access to first element of the structure, which happens to also serve
; as the address of the structure. This is for the BitCastEquivalent clause
; of the getelementptr analyzer.


; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
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
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [80 x i8]*{{ *$}}
; CHECK:      No element pointees.

; CHECK-NONOPAQUE: %bce = getelementptr inbounds [80 x i8], [80 x i8]* %local, i64 0, i64 0
; CHECK-OPAQUE: %bce = getelementptr inbounds [80 x i8], ptr %local, i64 0, i64 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   [80 x i8]*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    [80 x i8] @ 0


; Test bitcast equivalent for GEP that addresses a structure element that starts
; with an i8 element.
%struct.test02 = type { i8, i32, i64 }
define internal void @test02(%struct.test02* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %ptr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 0
  store i8 64, i8* %ptr
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-NONOPAQUE: %ptr = getelementptr %struct.test02, %struct.test02* %in, i64 0, i32 0
; CHECK-OPAQUE: %ptr = getelementptr %struct.test02, ptr %in, i64 0, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test02 @ 0


; Test bitcast equivalent for GEP that addresses a structure element that starts
; with an array of i8 elements.
%struct.test03a = type { [256 x i8], i32 }
%struct.test03b = type { %struct.test03a }
define internal void @test03(%struct.test03b* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  %ptr = getelementptr %struct.test03b, %struct.test03b* %in, i64 0, i32 0, i32 0, i32 0
  store i8 64, i8* %ptr
  ret void
}
; CHECK-LABEL: void @test03
; CHECK-NONOPAQUE: %ptr = getelementptr %struct.test03b, %struct.test03b* %in, i64 0, i32 0, i32 0, i32 0
; CHECK-OPAQUE: %ptr = getelementptr %struct.test03b, ptr %in, i64 0, i32 0, i32 0, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test03b*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:  [256 x i8] @ 0


!1 = !{i8 0, i32 0}  ; i8
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{!"A", i32 256, !1}  ; [256 x i8]
!7 = !{%struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!8 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i8, i32, i64 }
!11 = !{!"S", %struct.test03a zeroinitializer, i32 2, !6, !2} ; { [256 x i8], i32 }
!12 = !{!"S", %struct.test03b zeroinitializer, i32 1, !7} ; { %struct.test03a }

!intel.dtrans.types = !{!10, !11, !12}
