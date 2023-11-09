; RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the DTrans delete pass correctly updates the offsets
; used in byte-flattened GEPs after field deletion.

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; Allocate a structure.
  %p = call ptr @malloc(i64 24)

  ; Get pointers to each field
  %p8_B = getelementptr i8, ptr %p, i64 8
  %p8_C = getelementptr i8, ptr %p, i64 16

  ; Write A and C
  store i32 1, ptr %p
  store i64 2, ptr %p8_B
  store i32 3, ptr %p8_C
  %res = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret i32 0
}

; CHECK-LABEL: define i32 @main

; CHECK-NOT: %p8_B = getelementptr i8, ptr %p, i64 8
; CHECK: %p8_C = getelementptr i8, ptr %p, i64 4
; CHECK: store i32 1, ptr %p
; CHECK-NOT: store i64 2, ptr %p8_B
; CHECK: store i32 3, ptr %p8_C

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !6 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2

  ; Read A and C
  %valA = load i32, ptr %p_test_A
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC
  ret i32 %sum
}

declare !intel.dtrans.func.type !8 ptr @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!10}
