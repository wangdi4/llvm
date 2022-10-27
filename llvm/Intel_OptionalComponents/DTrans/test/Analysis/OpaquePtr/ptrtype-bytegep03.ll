; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test of byte-flattened GEP analysis for cases that are not
; byte-flattened GEPs.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; None of the elements should be added as byte-flattened GEPs.
; CHECK-NOT: Adding BF-GEP access

; In this case, the value is a ptr-to-ptr, so should not be treated as a
; byte-flattened GEP to %struct.S1
%struct.test01 = type { i32, i32, i32 }
@var_test01 = global %struct.test01** zeroinitializer, !intel_dtrans_type !2
define void @test01() {
  %pp = call noalias i8* @malloc(i64 60)
  %pps1 = bitcast i8* %pp to %struct.test01**
  store %struct.test01** %pps1, %struct.test01*** @var_test01
  %p4s1 = getelementptr i8, i8* %pp, i64 24
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK-NONOPAQUE: %p4s1 = getelementptr i8, i8* %pp, i64 24
; CHECK-OPAQUE: %p4s1 = getelementptr i8, ptr %pp, i64 24
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.test01**{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   No element pointees.

; Access into an array of i8 types. This has an element pointee, but is not
; considered as a byte flattened GEP because the underlying type is i8.
define internal void @test02() {
  %local = alloca [65535 x i8]
  %flat = getelementptr [65535 x i8], [65535 x i8]* %local, i64 0, i64 0
  %faddr = getelementptr i8, i8* %flat, i64 256
  %val = load i8, i8* %faddr
  ret void
}
; CHECK-LABEL: void @test02()
; CHECK-NONOPAQUE: %faddr = getelementptr i8, i8* %flat, i64 256
; CHECK-OPAQUE: %faddr = getelementptr i8, ptr %flat, i64 256
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [65535 x i8] @ 256

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!5}
