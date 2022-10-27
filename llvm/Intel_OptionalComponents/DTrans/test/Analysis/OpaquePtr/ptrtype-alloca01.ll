; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for alloca instruction

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test = type { i32, i32, i64 }

; allocate a simple type
define internal void @test01() {
  %local = alloca i32
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK:  %local = alloca i32
; CHECK:      Aliased types:
; CHECK:        i32*
; CHECK:      No element pointees.


; allocate an array of simple types
define internal void @test02() {
  %local = alloca [16 x float]
  ret void
}
; CHECK-LABEL: void @test02()
; CHECK:  %local = alloca [16 x float]
; CHECK:      Aliased types:
; CHECK:        [16 x float]*
; CHECK:      No element pointees.


; allocate a simple vector type
define internal void @test03() {
  %local = alloca <4 x i64>
  ret void
}
; CHECK-LABEL: void @test03()
; CHECK:  %local = alloca <4 x i64>
; CHECK:      Aliased types:
; CHECK:        <4 x i64>*
; CHECK:      No element pointees.


; allocate a structure type
define internal void @test04() {
  %local = alloca %struct.test
  ret void
}
; CHECK-LABEL: void @test04()
; CHECK:  %local = alloca %struct.test
; CHECK:      Aliased types:
; CHECK:        %struct.test*
; CHECK:      No element pointees.


; allocate an array of structures
define internal void @test05() {
  %local = alloca [10 x %struct.test]
  ret void
}
; CHECK-LABEL: void @test05()
; CHECK:  %local = alloca [10 x %struct.test]
; CHECK:      Aliased types:
; CHECK:        [10 x %struct.test]*
; CHECK:      No element pointees.


; allocate a simple literal structure
define internal void @test06() {
  %local = alloca { i8, i32 }
  ret void
}
; CHECK-LABEL: void @test06()
; CHECK:  %local = alloca { i8, i32 }
; CHECK:      Aliased types:
; CHECK:        { i8, i32 }*
; CHECK:      No element pointees.


;
; Cases that require metadata
;

; allocate a pointer to a structure
define internal void @test07() {
  %local = alloca %struct.test*, !intel_dtrans_type !3
  ret void
}
; CHECK-LABEL: void @test07()
; CHECK-NONOPAQUE:  %local = alloca %struct.test*
; CHECK-OPAQUE:  %local = alloca ptr
; CHECK:      Aliased types:
; CHECK:        %struct.test**
; CHECK:      No element pointees.


; allocate a pointer to an array of structure pointers
define internal void @test08() {
  %local = alloca [9 x %struct.test*]*, !intel_dtrans_type !4
  ret void
}
; CHECK-LABEL: void @test08()
; CHECK-NONOPAQUE:  %local = alloca [9 x %struct.test*]*
; CHECK-OPAQUE:  %local = alloca ptr
; CHECK:      Aliased types:
; CHECK:        [9 x %struct.test*]**
; CHECK:      No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = !{!5, i32 1}  ; [9 x %struct.test*]*
!5 = !{!"A", i32 9, !3}  ; [9 x %struct.test*]
!6 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !1, !2} ; { i32, i32, i64 }

!intel.dtrans.types = !{!6}
