; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion for SelectInst

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

; In this case, the select instruction should be changed to be the indexed type
@var01 = internal global %struct.test01dep zeroinitializer
@var02 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0

  %q0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %q1 = load %struct.test01*, %struct.test01** %q0

  ; This select should be updated to use integers
  %choice = select i1 undef, %struct.test01* %p1, %struct.test01* %q1

  ; This select should NOT change to using integers
  %var = select i1 undef, %struct.test01dep* @var01, %struct.test01dep* @var02

  %t0 = getelementptr %struct.test01, %struct.test01* %choice, i64 0, i32 2
  %val = load i32, i32* %t0
  ret i32 %val
}

; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %choice = select i1 undef, i64 %p1, i64 %q1
; CHECK-NONOPAQUE: %var = select i1 undef, %__SOADT_struct.test01dep* @var01, %__SOADT_struct.test01dep* @var02

; CHECK-OPAQUE: %choice = select i1 undef, i64 %p1, i64 %q1
; CHECK-OPAQUE: %var = select i1 undef, ptr @var01, ptr @var02

; In this case, the select instruction should be changed to be the indexed type,
; which also requires changing the 'null' pointer to the integer 0.
define i32 @test02() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0

  %q0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %q1 = load %struct.test01*, %struct.test01** %q0

  ; The 'null' in this case should change to an integer 0
  %choice = select i1 undef, %struct.test01* %p1, %struct.test01* null

  ; The 'null' in this case should NOT change to an integer 0
  %var = select i1 undef, %struct.test01dep* @var01, %struct.test01dep* null

  %t0 = getelementptr %struct.test01, %struct.test01* %choice, i64 0, i32 2
  %val = load i32, i32* %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test02
; CHECK-NONOPAQUE: %choice = select i1 undef, i64 %p1, i64 0
; CHECK-NONOPAQUE: %var = select i1 undef, %__SOADT_struct.test01dep* @var01, %__SOADT_struct.test01dep* null

; CHECK-OPAQUE: %choice = select i1 undef, i64 %p1, i64 0
; CHECK-OPAQUE: %var = select i1 undef, ptr @var01, ptr null

define i32 @test03() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0

  %q0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %q1 = load %struct.test01*, %struct.test01** %q0

  ; The 'null' in this case should change to an integer 0
  %choice = select i1 undef, %struct.test01* null, %struct.test01* null

   ; The 'null' in this case should NOT change to an integer 0
  %var = select i1 undef, %struct.test01dep* null, %struct.test01dep* @var01

  %t0 = getelementptr %struct.test01, %struct.test01* %choice, i64 0, i32 2
  %val = load i32, i32* %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test03
; CHECK: %choice = select i1 undef, i64 0, i64 0
; CHECK-NONOPAQUE: %var = select i1 undef, %__SOADT_struct.test01dep* null, %__SOADT_struct.test01dep* @var01
; CHECK-OPAQUE: %var = select i1 undef, ptr null, ptr @var01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
