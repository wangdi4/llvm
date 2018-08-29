; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for a calloc with a constant number of elements
; but a multiple of the struct size for the size parameter.

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

define void @test01(i64 %some_val) {
  ; Allocate 5 * %some_val * sizeof(struct) elements.
  %mul = mul i64 %some_val, 12
  %mem = call i8* @calloc(i64 5, i64 %mul)

; Verify the calculation for the current allocation size and number of
; elements allocated; followed by an increment of 1 more structure element.
; CHECK:  %1 = mul i64 5, %mul
; CHECK:  %2 = sdiv i64 %1, 12
; CHECK:  %3 = add i64 %2, 1
; CHECK:  %mem = call i8* @calloc(i64 %3, i64 12)

  %st = bitcast i8* %mem to %struct.test01*
  ret void
}

declare i8* @calloc(i64, i64)
