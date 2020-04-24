; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test of byte-flattened GEP analysis for cases that are not
; byte-flattened GEPs.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; None of the elements should be added as byte-flattened GEPs.
; CHECK-NOT: Adding BF-GEP access

; In this case, the value is a ptr-to-ptr, so should not be treated as a
; byte-flattened GEP to %struct.S1
%struct.test01 = type { i32, i32, i32 }
@var_test01 = global %struct.test01** zeroinitializer, !dtrans_type !2
define void @test01() {
  %pp = call noalias i8* @malloc(i64 60)
  %pps1 = bitcast i8* %pp to %struct.test01**
  store %struct.test01** %pps1, %struct.test01*** @var_test01
  %p4s1 = getelementptr i8, i8* %pp, i64 24
  ret void
}
; CHECK-LABEL: void @test01()
; CHECK-CUR: %p4s1 = getelementptr i8, i8* %pp, i64 24
; CHECK-FUT: %p4s1 = getelementptr i8, p0 %pp, i64 24
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
; CHECK-CUR: %faddr = getelementptr i8, i8* %flat, i64 256
; CHECK-FUT: %faddr = getelementptr i8, p0 %flat, i64 256
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [65535 x i8] @ 256

declare i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 2}  ; %struct.test01**
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!dtrans_types = !{!4}
