; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the size argument of a malloc call is correctly
; updated when it is a variable multiple of the structure size using a left
; shift where the intermediate values have other uses.

%struct.test = type { i32, i64, i32, i64 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

@someMemoryLoc = internal global i32 zeroinitializer
define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Calculate a size.
  %sz = shl i32 %argc, 5
  %sz64 = sext i32 %sz to i64
  %other = add i32 %sz, 32
  store i32 %other, ptr @someMemoryLoc

  ; Allocate an array of structures.
  %p = call ptr @malloc(i64 %sz64)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret i32 %val
}

; CHECK-LABEL: define i32 @main
; CHECK: %sz.dt = mul i32 %argc, 8
; CHECK: %sz = shl i32 %argc, 5
; CHECK: %sz64 = sext i32 %sz.dt to i64
; CHECK: %p = call ptr @malloc(i64 %sz64)

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2
  %p_test_D = getelementptr %struct.test, ptr %p_test, i64 0, i32 3

  ; read and write A and C
  store i32 1, ptr %p_test_A
  %valA = load i32, ptr %p_test_A
  store i32 2, ptr %p_test_C
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC
  ret i32 %sum
}
; CHECK-LABEL: define i32 @doSomething
; CHECK: %p_test_A = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 1
; CHECK-NOT: %p_test_D = getelementptr


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
!10 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !2, !1, !2} ; { i32, i64, i32, i64 }

!intel.dtrans.types = !{!10}
