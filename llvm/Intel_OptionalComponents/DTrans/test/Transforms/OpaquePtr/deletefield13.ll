; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the size by which the result of a pointer sub is
; divided is correctly updated after field deletion when the sub takes place in
; a cloned function.

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate an array of structures.
  %p = call i8* @malloc(i64 64)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the buffer
  call void @free(i8* %p)
  ret i32 %val
}

define i32 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; Get a pointer to the second struct in the array.
  %p_test2 = getelementptr %struct.test, %struct.test* %p_test, i64 1

  ; Get a pointer to the third struct in the array.
  %p_test3 = getelementptr %struct.test, %struct.test* %p_test, i64 2

  ; Calculate the distance between these as an index.
  %t2 = ptrtoint %struct.test* %p_test2 to i64
  %t3 = ptrtoint %struct.test* %p_test3 to i64
  %sub = sub i64 %t2, %t3
  %offset_idx = sdiv i64 %sub, 16

  %sum = add i32 %valA, %valC
  ret i32 %sum
}

; CHECK-LABEL: define internal i32 @doSomething
; CHECK: %t2 = ptrtoint {{.*}} %p_test2 to i64
; CHECK: %t3 = ptrtoint {{.*}} %p_test3 to i64
; CHECK: %sub = sub i64 %t2, %t3
; CHECK: %offset_idx = sdiv i64 %sub, 8

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @free(i8* "intel_dtrans_func_index"="1") #1

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
