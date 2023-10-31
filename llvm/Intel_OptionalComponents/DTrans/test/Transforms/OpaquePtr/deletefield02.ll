; RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the DTrans delete pass correctly transforms
; structures that have unused fields and meet the necessary safety conditions
; and that other structures that point to the optimized structure are correctly
; updated.

%struct.test = type { i32, i64, i32 }
%struct.other = type { ptr }

; CHECK-DAG: %struct.other = type { ptr }
; CHECK-DAG: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !8 {
  ; Allocate two structures.
  %p1 = call ptr @malloc(i64 24)
  %p2 = call ptr @malloc(i64 16)

  ; Call a helper function to store p_test in the other struct.
  call void @connect(ptr %p1, ptr %p2)

  ; Re-load p_test from p_other and call doSomething
  %pp_test = getelementptr %struct.other, ptr %p2, i64 0, i32 0
  %p_test2 = load ptr, ptr %pp_test
  %val = call i32 @doSomething(ptr %p_test2)

  ; Free the structures
  call void @free(ptr %p1)
  call void @free(ptr %p2)
  ret i32 %val
}

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, ptr %p_test_A
  store i32 2, ptr %p_test_C
  %valA = load i32, ptr %p_test_A
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC

  ret i32 %sum
}

; CHECK-LABEL: define i32 @main

; CHECK: %p1 = call ptr @malloc(i64 8)
; CHECK: %p2 = call ptr @malloc(i64 16)
; CHECK: call void @connect(ptr %p1, ptr %p2)
; CHECK: %pp_test = getelementptr %struct.other, ptr %p2, i64 0, i32 0
; CHECK: %p_test2 = load ptr, ptr %pp_test
; CHECK: %val = call i32 @doSomething(ptr %p_test2)

define void @connect(ptr "intel_dtrans_func_index"="1" %p_test, ptr "intel_dtrans_func_index"="2" %p_other) !intel.dtrans.func.type !6 {
  %pp_test = getelementptr %struct.other, ptr %p_other, i64 0, i32 0
  store ptr %p_test, ptr %pp_test
  ret void
}

; The instruction updating is verified in other tests, so here it is
; sufficient to check the function signatures for the cloned version
; when opaque pointres are not in use. When opaque pointers are used
; there is no change to signatures, so they are not checked.


declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !11 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

; Verify that the metadata representation was updated.

; CHECK: !intel.dtrans.types = !{![[S1MD:[0-9]+]], ![[S2MD:[0-9]+]]}
; CHECK: ![[S1MD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 2, ![[I32MD:[0-9]+]], ![[I32MD]]}
; CHECK: ![[I32MD]] = !{i32 0, i32 0}
; CHECK: ![[S2MD]] = !{!"S", %struct.other zeroinitializer, i32 1, ![[SREF:[0-9]+]]}
; CHECK: ![[SREF]] = !{%__DFT_struct.test zeroinitializer, i32 1}

!intel.dtrans.types = !{!12, !13}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{%struct.other zeroinitializer, i32 1}  ; %struct.other*
!6 = distinct !{!3, !5}
!7 = !{i8 0, i32 2}  ; i8**
!8 = distinct !{!7}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = distinct !{!9}
!12 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!13 = !{!"S", %struct.other zeroinitializer, i32 1, !3} ; { %struct.test* }

