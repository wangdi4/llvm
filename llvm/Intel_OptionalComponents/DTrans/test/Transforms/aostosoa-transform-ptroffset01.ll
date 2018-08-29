; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This test verifies that the size by which the result of a pointer sub is
; divided by is correctly updated after the AOS-to-SOA transformation converts
; the pointers to an integer index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform. (structure size is 24 bytes)
%struct.test01 = type { i32, i32, i32, i32, i64 }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 960)
  store i8* %mem, i8** bitcast (%struct.test01** @g_test01ptr to i8**)
  %val = call i64 @test01()
  ret i32 0
}

define i64 @test01() {
  %base = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a pointer to the second struct in the array.
  %ar_sub1 = getelementptr %struct.test01, %struct.test01* %base, i64 1

  ; Get a pointer to the fifth struct in the array.
  %ar_sub4 = getelementptr %struct.test01, %struct.test01* %base, i64 4

  ; Calculate the distance between these as an index.
  %ar_sub1_int = ptrtoint %struct.test01* %ar_sub1 to i64
  %ar_sub4_int = ptrtoint %struct.test01* %ar_sub4 to i64
  %sub = sub i64 %ar_sub1_int, %ar_sub4_int

  %offset_idx = sdiv i64 %sub, 24
; CHECK:  %offset_idx = sdiv i64 %sub, 1

  ; Test with multiple of the structure size.
  %offset_idx2 = sdiv i64 %sub, 72
; CHECK:  %offset_idx2 = sdiv i64 %sub, 3

  ret i64 %offset_idx
}

declare i8* @malloc(i64)
