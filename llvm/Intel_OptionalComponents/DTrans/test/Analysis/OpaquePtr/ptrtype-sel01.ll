; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for "select" instructions.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test of "select" instruction with non-pointer types.
%struct.test01 = type { i64, i64, i64 }
define internal void @test01() {
  %struct = alloca %struct.test01
  %f0 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 1
  %v1 = load i64, i64* %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %struct, i64 0, i32 2
  %v2 = load i64, i64* %f2

 ; Select instruction between two values
  %c0 = select i1 undef, i64 %v0, i64 %v1

 ; Select instruction involving compiler constant value
 %c1 = select i1 undef, i64 0, i64 %v2

  ret void
}
; In this case, the results of the select are not pointer types, so no
; pointer type alias information should be reported.
; CHECK-LABEL: void @test01
; CHECK: %c0 = select i1 undef, i64 %v0, i64 %v1
; CHECK-NOT: Aliased types:
; CHECK: %c1 = select i1 undef, i64 0, i64 %v2
; CHECK-NOT: Aliased types:


; Test of "select" instruction with pointer types.
%struct.test02 = type { %struct.test02*, %struct.test02* }
define internal void @test02() {
  %struct = alloca %struct.test02
  %f0 = getelementptr %struct.test02, %struct.test02* %struct, i64 0, i32 0
  %v0 = load %struct.test02*, %struct.test02** %f0

  %f1 = getelementptr %struct.test02, %struct.test02* %struct, i64 0, i32 1
  %v1 = load %struct.test02*, %struct.test02** %f1

 ; Select instruction between two values
  %c0 = select i1 undef, %struct.test02* %v0, %struct.test02* %v1

 ; Select instruction involving compiler constant value
 %c1 = select i1 undef, %struct.test02* %c0, %struct.test02* null

  ret void
}
; CHECK-LABEL: void @test02
; CHECK-NONOPAQUE:  %c0 = select i1 undef, %struct.test02* %v0, %struct.test02* %v1
; CHECK-OPAQUE:  %c0 = select i1 undef, ptr %v0, ptr %v1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.

; CHECK-NONOPAQUE:  %c1 = select i1 undef, %struct.test02* %c0, %struct.test02* null
; CHECK-OPAQUE:  %c1 = select i1 undef, ptr %c0, ptr null
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test02*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }
!4 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { %struct.test02*, %struct.test02* }

!intel.dtrans.types = !{!3, !4}
