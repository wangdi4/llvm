; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -intel-libirc-allowed 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume -intel-libirc-allowed 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies special cases of the AOS to SOA transformation
; where PtrToInt instructions need to be updated/removed from the IR
; when using a 32-bit index for the peeling index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to target transforming.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }

; This structure will be used for verifying accesses within dependent data
; structures get handled.
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 400)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01()
  ret i32 0
}

; This test checks the AOS-to-SOA handling when there are already
; ptrtoint instructions.
define void @test01() {
; CHECK: void @test01()
  %base = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Test where there is an existing ptrtoint case on the type being
  ; transformed to be an int. Because %base will now be an i32, the
  ; conversion should be done to extend it into an i64.
  %cast1 = ptrtoint %struct.test01* %base to i64
  %cmp1 = icmp eq i64 %cast1, 0
; CHECK-NOT: %cast1 = ptrtoint %struct.test01* %base to i64
; CHECK: %cast1 = zext i32 %base to i64
; CHECK: %cmp1 = icmp eq i64 %cast1, 0

  ; This cast should not be removed, but should be converted
  ; to a cast from an i32*
  %cast2 = ptrtoint %struct.test01** @g_test01ptr to i64
  %cmp2 = icmp eq i64 %cast2, 0
; CHECK:  %cast2 = ptrtoint i32* @g_test01ptr to i64
; CHECK:  %cmp2 = icmp eq i64 %cast2, 0

  ret void
}

declare i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
