; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion for SelectInst

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, ptr }

; In this case, the select instruction should be changed to be the indexed type
@var01 = internal global %struct.test01dep zeroinitializer
@var02 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0

  %q0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %q1 = load ptr, ptr %q0

  ; This select should be updated to use integers
  %choice = select i1 undef, ptr %p1, ptr %q1

  ; This select should NOT change to using integers
  %var = select i1 undef, ptr @var01, ptr @var02

  %t0 = getelementptr %struct.test01, ptr %choice, i64 0, i32 2
  %val = load i32, ptr %t0
  ret i32 %val
}

; CHECK-LABEL: define i32 @test01

; CHECK: %choice = select i1 undef, i64 %p1, i64 %q1
; CHECK: %var = select i1 undef, ptr @var01, ptr @var02

; In this case, the select instruction should be changed to be the indexed type,
; which also requires changing the 'null' pointer to the integer 0.
define i32 @test02() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0

  %q0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %q1 = load ptr, ptr %q0

  ; The 'null' in this case should change to an integer 0
  %choice = select i1 undef, ptr %p1, ptr null

  ; The 'null' in this case should NOT change to an integer 0
  %var = select i1 undef, ptr @var01, ptr null

  %t0 = getelementptr %struct.test01, ptr %choice, i64 0, i32 2
  %val = load i32, ptr %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test02

; CHECK: %choice = select i1 undef, i64 %p1, i64 0
; CHECK: %var = select i1 undef, ptr @var01, ptr null

define i32 @test03() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0

  %q0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %q1 = load ptr, ptr %q0

  ; The 'null' in this case should change to an integer 0
  %choice = select i1 undef, ptr null, ptr null

   ; The 'null' in this case should NOT change to an integer 0
  %var = select i1 undef, ptr null, ptr @var01

  %t0 = getelementptr %struct.test01, ptr %choice, i64 0, i32 2
  %val = load i32, ptr %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test03
; CHECK: %choice = select i1 undef, i64 0, i64 0
; CHECK: %var = select i1 undef, ptr null, ptr @var01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
