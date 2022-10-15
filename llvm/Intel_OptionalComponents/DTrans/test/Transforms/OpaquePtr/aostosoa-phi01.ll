; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion for PHINodes

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

; In this case, the phi instruction should be changed to be the indexed type
@var01 = internal global %struct.test01dep zeroinitializer
@var02 = internal global %struct.test01dep zeroinitializer
@var03 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  br i1 undef, label %block1, label %block2

block1:
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0
  br label %merge
block2:
  %q0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %q1 = load %struct.test01*, %struct.test01** %q0
  br label %merge

merge:
  ; This phi should be updated to use integers
  %choice = phi %struct.test01* [%p1, %block1], [%q1, %block2]

  ; This phi should NOT change to using integers
  %var = phi %struct.test01dep* [@var01, %block1], [@var02, %block2]

  %t0 = getelementptr %struct.test01, %struct.test01* %choice, i64 0, i32 2
  %val = load i32, i32* %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %choice = phi i64 [ %p1, %block1 ], [ %q1, %block2 ]
; CHECK-NONOPAQUE: %var = phi %__SOADT_struct.test01dep* [ @var01, %block1 ], [ @var02, %block2 ]

; CHECK-OPAQUE: %choice = phi i64 [ %p1, %block1 ], [ %q1, %block2 ]
; CHECK-OPAQUE: %var = phi ptr [ @var01, %block1 ], [ @var02, %block2 ]

; Test with multiple 'null' values on the instruction to be changed to the
; integer 0.
define i32 @test02() {
  br i1 undef, label %block1, label %block2

block1:
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0
  br label %merge
block2:
  br i1 undef, label %block3, label %block4
block4:
  br label %merge
block3:
  br label %merge

merge:
  ; This phi should be updated to use integers
  %choice = phi %struct.test01* [%p1, %block1], [null, %block3], [null, %block4]

  ; This phi should NOT change to using integers
  %var = phi %struct.test01dep* [@var01, %block1], [@var02, %block3], [@var03, %block4]

  %t0 = getelementptr %struct.test01, %struct.test01* %choice, i64 0, i32 2
  %val = load i32, i32* %t0
  ret i32 %val
}
; CHECK-LABEL: define i32 @test02
; CHECK-NONOPAQUE: %choice = phi i64 [ %p1, %block1 ], [ 0, %block3 ], [ 0, %block4 ]
; CHECK-NONOPAQUE: %var = phi %__SOADT_struct.test01dep* [ @var01, %block1 ], [ @var02, %block3 ], [ @var03, %block4 ]

; CHECK-OPAQUE: %choice = phi i64 [ %p1, %block1 ], [ 0, %block3 ], [ 0, %block4 ]
; CHECK-OPAQUE: %var = phi ptr [ @var01, %block1 ], [ @var02, %block3 ], [ @var03, %block4 ]

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
