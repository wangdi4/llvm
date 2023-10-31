; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the size argument of realloc calls are correctly
; updated in a variety of cases.

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }


define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate a structure.
  %p = call ptr @realloc(ptr null, i64 24)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Grow the buffer
  %ra1 = call ptr @realloc(ptr %p, i64 48)
  ; FIXME: This use of %ra after the realloc should NOT be necessary, but we
  ;        don't transfer the type alias information from the realloc operand.
  %p_test2_A = getelementptr %struct.test, ptr %ra1, i64 0, i32 0

  ; Calculate a new size
  %mul = mul i32 96, %val
  %sz = zext i32 %mul to i64

  ; Change the size of the buffer
  %ra2 = call ptr @realloc(ptr %ra1, i64 %sz)
  ; FIXME: This use of %ra2 after the realloc should NOT be necessary, but we
  ;        don't transfer the type alias information from the realloc operand.
  %p_test3_A = getelementptr %struct.test, ptr %ra2, i64 0, i32 0

  ; Use the value where we had the size constant.
  icmp eq i32 128, %mul

  ; Free the buffer
  call void @free(ptr %ra2)
  ret i32 %val
}

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
; CHECK-LABEL: define i32 @main
; CHECK: %p = call ptr @realloc(ptr null, i64 8)
; CHECK: %ra1 = call ptr @realloc(ptr %p, i64 16)

; CHECK: %mul.dt = mul i32 32, %val
; CHECK: %mul = mul i32 96, %val
; CHECK: %sz = zext i32 %mul.dt to i64
; CHECK: %ra2 = call ptr @realloc(ptr %ra1, i64 %sz)
; CHECK: icmp eq i32 128, %mul

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @realloc(ptr "intel_dtrans_func_index"="2", i64) #0
declare !intel.dtrans.func.type !9 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("realloc") allocsize(1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{i8 0, i32 2}  ; i8**
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7, !7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!10}
