; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a calloc call with a constant size
; parameter equal to the structure size, and a variable number of elements
; being allocated.

; Because the initialization of the field elements is the same as done
; for the malloc test cases, we will omit checking those statements in
; this test.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 3)
  ret i32 0
}

define void @test01(i64 %count) {
  ; Allocate %count elements.
  %mem = call i8* @calloc(i64 %count, i64 12)
; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:   %1 = add i64 %count, 1
; CHECK:   %mem = call i8* @calloc(i64 %1, i64 12)

  %st = bitcast i8* %mem to %struct.test01*
  ret void
}

declare i8* @calloc(i64, i64)
