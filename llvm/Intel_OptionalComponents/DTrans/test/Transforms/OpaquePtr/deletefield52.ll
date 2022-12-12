; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the size by which the result of a pointer sub is
; divided is correctly updated after field deletion for a structure that
; contains a nested structure which has fields deleted.

%struct.test = type { i64, %struct.inner, i64 }
%struct.inner = type { i32, i32, i64 }
; CHECK-DAG: %__DFDT_struct.test = type { i64, %__DFT_struct.inner, i64 }
; CHECK-DAG: %__DFT_struct.inner = type { i64 }

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !5 {
  ; Allocate an array of structures.
  %p = call ptr @malloc(i64 128)

  ; Get a pointer to the first struct in the array.
  %p_test1 = getelementptr %struct.test, ptr %p, i64 0

  ; Get a pointer to the third struct in the array.
  %p_test2 = getelementptr %struct.test, ptr %p, i64 2

  ; Calculate the distance from the base of the outer structure.
  %t1 = ptrtoint ptr %p_test1 to i64
  %t2 = ptrtoint ptr %p_test2 to i64
  %sub = sub i64 %t1, %t2
  %offset_idx = sdiv i64 %sub, 32

  ; Call a function to use the structure fields.
  %val = call i64 @doSomething(ptr %p)
  %tmp = trunc i64 %val to i32

  ; Free the buffer
  call void @free(ptr %p)
  ret i32 %tmp
}
; CHECK-LABEL: define i32 @main
; CHECK: %p = call ptr @malloc(i64 96)

; CHECK: %sub = sub i64 %t1, %t2
; CHECK: %offset_idx = sdiv i64 %sub, 24


define i64 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !7 {
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2

  ; read and write A and C to prevent these fields from being deleted.
  store i64 1, ptr %p_test_A
  %valA = load i64, ptr %p_test_A
  store i64 2, ptr %p_test_C
  %valC = load i64, ptr %p_test_C
  %tmp = add i64 %valA, %valC

  ; read and write one field of the inner structure, so the other two will be
  ; deleted.
  %p_test_B2 = getelementptr %struct.inner, ptr %p_test_B, i64 0, i32 2
  store i64 4, ptr %p_test_B2
  %inner_C = load i64, ptr %p_test_B2
  %sum = add i64 %tmp, %inner_C

  ret i64 %sum
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !10 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.inner zeroinitializer, i32 0}  ; %struct.inner
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i8 0, i32 2}  ; i8**
!5 = distinct !{!4}
!6 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i64, %struct.inner, i64 }
!12 = !{!"S", %struct.inner zeroinitializer, i32 3, !3, !3, !1} ; { i32, i32, i64 }

!intel.dtrans.types = !{!11, !12}
