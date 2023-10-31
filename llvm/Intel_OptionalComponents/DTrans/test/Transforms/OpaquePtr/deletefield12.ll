; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the size by which the result of a pointer sub is
; divided is correctly updated after field deletion.

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }


define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate an array of structures.
  %p = call ptr @malloc(i64 96)

  ; Get a pointer to the first struct in the array.
  %p_test1 = getelementptr %struct.test, ptr %p, i64 0

  ; Get a pointer to the third struct in the array.
  %p_test2 = getelementptr %struct.test, ptr %p, i64 2

  ; Calculate its distance from the base as an index.
  %t1 = ptrtoint ptr %p_test1 to i64
  %t2 = ptrtoint ptr %p_test2 to i64
  %sub = sub i64 %t1, %t2
  %offset_idx = sdiv i64 %sub, 24

  ; Get a pointer to the fourth struct in the array.
  %p_test3 = getelementptr %struct.test, ptr %p, i64 3

  ; Calculate its distance from the base as an index of two-struct pairs.
  %t3 = ptrtoint ptr %p_test3 to i64
  %pair_size = mul i64 24, 2
  %sub2 = sub i64 %t1, %t3
  %offset_idx2 = sdiv i64 %sub2, %pair_size

  ; Cause %pair_size to be cloned by using it again.
  %test = icmp eq i64 %pair_size, 24

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the buffer
  call void @free(ptr %p)
  ret i32 %val
}

; CHECK-LABEL: define i32 @main
; CHECK: %p = call ptr @malloc(i64 32)

; CHECK: %sub = sub i64 %t1, %t2
; CHECK: %offset_idx = sdiv i64 %sub, 8

; CHECK: %pair_size.dt = mul i64 8, 2
; CHECK: %pair_size = mul i64 24, 2
; CHECK: %sub2 = sub i64 %t1, %t3
; CHECK: %offset_idx2 = sdiv i64 %sub2, %pair_size.dt
; CHECK: %test = icmp eq i64 %pair_size, 24

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, ptr %p_test_A
  %valA = load i32, ptr %p_test_A
  store i32 2, ptr %p_test_C
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC

  ret i32 %sum
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{i8 0, i32 2}  ; i8**
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!10}
