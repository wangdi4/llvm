; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass correctly transforms
; structures that have unused fields and meet the necessary safety conditions
; and that other structures that point to the optimized structure are correctly
; updated.

%struct.test = type { i32, i64, i32 }
%struct.other = type { %struct.test* }

define i32 @doSomething(%struct.test* %p_test) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ret i32 %valA
}

define void @connect(%struct.test* %p_test, %struct.other* %p_other) {
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 0
  store %struct.test* %p_test, %struct.test** %pp_test
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate two structures.
  %p1 = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p1 to %struct.test*
  %p2 = call i8* @malloc(i64 16)
  %p_other = bitcast i8* %p2 to %struct.other*

  ; Call a helper function to store p_test in the other struct.
  call void @connect(%struct.test* %p_test, %struct.other* %p_other)

  ; Re-load p_test from p_other and call doSomething
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 0
  %p_test2 = load %struct.test*, %struct.test** %pp_test
  %val = call i32 @doSomething(%struct.test* %p_test2)

  ; Free the structures
  call void @free(i8* %p1)
  call void @free(i8* %p2)
  ret i32 %val
}

; CHECK-DAG: %__DFDT_struct.other = type { %__DFT_struct.test* }
; CHECK-DAG: %__DFT_struct.test = type { i32, i32 }

; CHECK: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p1 = call i8* @malloc(i64 8)
; CHECK: %p_test = bitcast i8* %p1 to %__DFT_struct.test*
; CHECK: %p2 = call i8* @malloc(i64 16)
; CHECK: %p_other = bitcast i8* %p2 to %__DFDT_struct.other*
; CHECK: call void @connect.{{[0-9]+}}(%__DFT_struct.test* %p_test,
; CHECK-SAME:                          %__DFDT_struct.other* %p_other)
; CHECK: %pp_test = getelementptr %__DFDT_struct.other,
; CHECK-SAME:                     %__DFDT_struct.other* %p_other, i64 0, i32 0
; CHECK: %p_test2 = load %__DFT_struct.test*, %__DFT_struct.test** %pp_test
; CHECK: %val = call i32 @doSomething.{{[0-9]+}}(%__DFT_struct.test* %p_test2)

; The instruction updating is verified in other tests, so here it is
; sufficient to check the function signatures. This allows us to tolerate
; any ordering of the functions.

; CHECK-DAG: define internal i32 @doSomething.{{[0-9]+}}(%__DFT_struct.test* %p_test)
; CHECK-DAG: define internal void @connect.{{[0-9]+}}(%__DFT_struct.test* %p_test, %__DFDT_struct.other* %p_other)

declare i8* @malloc(i64)
declare void @free(i8*)
