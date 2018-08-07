; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This test verifies special cases of the AOS to SOA transformation
; where PtrToInt instructions need to be updated/removed from the IR.

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
  ; transformed to be an int.
  %cast1 = ptrtoint %struct.test01* %base to i64
  %cmp1 = icmp eq i64 %cast1, 0
; CHECK-NOT: %cast1 = ptrtoint %struct.test01* %base to i64
; CHECK-NOT: %cast1 = ptrtoint i64 %base to i64
; CHECK: %cmp1 = icmp eq i64 %base, 0

  ; This cast should not be removed, but should be converted
  ; to a cast from a i64*
  %cast2 = ptrtoint %struct.test01** @g_test01ptr to i64
  %cmp2 = icmp eq i64 %cast2, 0
; CHECK:  %cast2 = ptrtoint i64* @g_test01ptr to i64
; CHECK:  %cmp2 = icmp eq i64 %cast2, 0

  ret void
}

declare i8* @calloc(i64, i64)
