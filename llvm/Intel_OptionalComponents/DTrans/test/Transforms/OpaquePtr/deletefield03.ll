; RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the DTrans delete pass correctly transforms
; two structures that have unused fields and meet the necessary safety
; conditions where each of the structures being updated contains a pointer to
; the other.

%struct.test = type { i32, i64, i32, ptr }
%struct.other = type { i32, ptr }


; CHECK-DAG: %__DFT_struct.other = type { ptr }
; CHECK-DAG: %__DFT_struct.test = type { i32, i32, ptr }

@result = global i32 zeroinitializer
define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !7 {
  ; Allocate two structures.
  %p1 = call ptr @malloc(i64 32)
  %p2 = call ptr @malloc(i64 16)

  ; Store a pointer to each structures in the other
  %pp_test = getelementptr %struct.other, ptr %p2, i64 0, i32 1
  store ptr %p1, ptr %pp_test
  %pp_other = getelementptr %struct.test, ptr %p1, i64 0, i32 3
  store ptr %p2, ptr %pp_other

  ; Re-load p_test from p_other and call doSomething
  %pp_test2 = getelementptr %struct.other, ptr %p2, i64 0, i32 1
  %p_test2 = load ptr, ptr %pp_test2
  %ret = call i1 @doSomething(ptr %p_test2, ptr %p2)

  ; Free the structures
  call void @free(ptr %p1)
  call void @free(ptr %p2)
  ret i32 0
}

; CHECK-LABEL: define i32 @main

; CHECK: %p1 = call ptr @malloc(i64 16)
; CHECK: %p2 = call ptr @malloc(i64 8)
; CHECK: %pp_test = getelementptr %__DFT_struct.other, ptr %p2, i64 0, i32 0
; CHECK: store ptr %p1, ptr %pp_test
; CHECK: %pp_other = getelementptr %__DFT_struct.test, ptr %p1, i64 0, i32 2
; CHECK: store ptr %p2, ptr %pp_other
; CHECK: %pp_test2 = getelementptr %__DFT_struct.other, ptr %p2, i64 0, i32 0
; CHECK: %p_test2 = load ptr, ptr %pp_test2
; CHECK: %ret = call i1 @doSomething(ptr %p_test2, ptr %p2)

define i1 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test, ptr "intel_dtrans_func_index"="2" %p_other_in) !intel.dtrans.func.type !5 {
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
  store i32 %sum, ptr @result

  ; load a pointer to Other
  %pp_other = getelementptr %struct.test, ptr %p_test, i64 0, i32 3
  %p_other = load ptr, ptr %pp_other

  %cmp = icmp eq ptr %p_other_in, %p_other

  ret i1 %cmp
}


; CHECK: define i1 @doSomething(
; CHECK: %p_test_A = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr %__DFT_struct.test,
; CHECK: %p_test_C = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 1
; CHECK: %pp_other = getelementptr %__DFT_struct.test, ptr %p_test, i64 0, i32 2



declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !10 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!11, !12}

; Verify that the metadata representation was updated.
; CHECK: !intel.dtrans.types = !{![[S1MD:[0-9]+]], ![[S2MD:[0-9]+]]}
; CHECK: ![[S1MD]] = !{!"S", %__DFT_struct.test zeroinitializer, i32 3, ![[I32MD:[0-9]+]], ![[I32MD]], ![[SOTHERREF:[0-9]+]]}
; CHECK: ![[I32MD]] = !{i32 0, i32 0}
; CHECK: ![[SOTHERREF]] = !{%__DFT_struct.other zeroinitializer, i32 1}
; CHECK: ![[S2MD]] = !{!"S", %__DFT_struct.other zeroinitializer, i32 1, ![[STESTREF:[0-9]+]]}
; CHECK: ![[STESTREF]] = !{%__DFT_struct.test zeroinitializer, i32 1}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.other zeroinitializer, i32 1}  ; %struct.other*
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4, !3}
!6 = !{i8 0, i32 2}  ; i8**
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !2, !1, !3} ; { i32, i64, i32, %struct.other* }
!12 = !{!"S", %struct.other zeroinitializer, i32 2, !1, !4} ; { i32, %struct.test* }
