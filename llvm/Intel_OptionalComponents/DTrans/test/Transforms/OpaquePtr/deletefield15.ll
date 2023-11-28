; RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the DTrans delete pass correctly updates the offsets
; used in byte-flattened GEPs after field deletion within a cloned function.
; (The function cloning only occurs with the non-opaque pointer form because
; the function signature changes. Once opaque pointers are in use, this case
; may no longer be relevant because the signature will not change.)

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate a structure.
  %p = call ptr @malloc(i64 24)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret i32 %val
}

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {

  ; Get pointers to each field
  %p8_A = getelementptr i8, ptr %p_test, i64 0
  %p8_B = getelementptr i8, ptr %p_test, i64 8
  %p8_C = getelementptr i8, ptr %p_test, i64 16

  ; read and write A and C
  store i32 1, ptr %p_test
  %valA = load i32, ptr %p_test
  store i32 2, ptr %p8_C
  %valC = load i32, ptr %p8_C
  %sum = add i32 %valA, %valC

  ; write B
  store i64 3, ptr %p8_B

  ret i32 %sum
}
; CHECK-LABEL: define i32 @doSomething

; CHECK: %p8_A = getelementptr i8, ptr %p_test, i64 0
; CHECK-NOT: %p8_B = getelementptr i8, ptr %p_test, i64 8
; CHECK: %p8_C = getelementptr i8, ptr %p_test, i64 4

; CHECK-NOT: store i64 3, ptr %p8_B


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
