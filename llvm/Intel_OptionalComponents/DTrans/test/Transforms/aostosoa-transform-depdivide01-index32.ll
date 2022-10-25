; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -intel-libirc-allowed 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"


; This test verifies that the size by which the result of a pointer sub
; is divided is correctly updated on dependent types when the size of
; the structure is modified.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64 }
%struct.test01dep = type { i32, %struct.test01*, i32 }

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01();
  call void @test02();
  ret i32 0
}

; Verify the sdiv instruction gets updated.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  ; Allocate an array of structures.
  %p = call i8* @malloc(i64 96)
  %p_test = bitcast i8* %p to %struct.test01dep*

  ; Get a pointer to the first struct in the array.
  %p_test1 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 0

  ; Get a pointer to the third struct in the array.
  %p_test2 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 2

  %t1 = ptrtoint %struct.test01dep* %p_test1 to i64
  %t2 = ptrtoint %struct.test01dep* %p_test2 to i64
  %sub = sub i64 %t1, %t2
  %offset_idx = sdiv i64 %sub, 24
; CHECK:  %offset_idx = sdiv i64 %sub, 12

  ret void
}

; Verify the udiv instruction gets updated.
define internal void @test02() {
; CHECK-LABEL: define internal void @test02

  ; Allocate an array of structures.
  %p = call i8* @malloc(i64 96)
  %p_test = bitcast i8* %p to %struct.test01dep*

  ; Get a pointer to the first struct in the array.
  %p_test1 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 0

  ; Get a pointer to the third struct in the array.
  %p_test2 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 2

  %t1 = ptrtoint %struct.test01dep* %p_test1 to i64
  %t2 = ptrtoint %struct.test01dep* %p_test2 to i64
  %sub = sub i64 %t1, %t2
  %offset_idx = udiv i64 %sub, 24
; CHECK:  %offset_idx = udiv i64 %sub, 12

  ret void
}

declare i8* @calloc(i64, i64) #0
declare i8* @malloc(i64) #1

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
