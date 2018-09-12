; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size by which the result of a pointer sub is
; divided is correctly updated after field deletion.

%struct.test = type { i32, i64, i32 }

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

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate an array of structures.
  %p = call i8* @malloc(i64 64)
  %p_test = bitcast i8* %p to %struct.test*

  ; Get a pointer to the first struct in the array.
  %p_test1 = getelementptr %struct.test, %struct.test* %p_test, i64 0

  ; Get a pointer to the third struct in the array.
  %p_test2 = getelementptr %struct.test, %struct.test* %p_test, i64 2

  ; Calculate its distance from the base as an index.
  %t1 = ptrtoint %struct.test* %p_test1 to i64
  %t2 = ptrtoint %struct.test* %p_test2 to i64
  %sub = sub i64 %t1, %t2
  %offset_idx = sdiv i64 %sub, 16

  ; Get a pointer to the fourth struct in the array.
  %p_test3 = getelementptr %struct.test, %struct.test* %p_test, i64 3

  ; Calculate its distance from the base as an index of two-struct pairs.
  %t3 = ptrtoint %struct.test* %p_test3 to i64
  %pair_size = mul i64 16, 2
  %sub2 = sub i64 %t1, %t3
  %offset_idx2 = sdiv i64 %sub2, %pair_size

  ; Cause %pair_size to be cloned by using it again.
  %test = icmp eq i64 %pair_size, 16

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the buffer
  call void @free(i8* %p)
  ret i32 %val
}


; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @malloc(i64 32)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %p_test1 = getelementptr %__DFT_struct.test,
; CHECK-SAME: %__DFT_struct.test* %p_test, i64 0
; CHECK: %p_test2 = getelementptr %__DFT_struct.test,
; CHECK-SAME: %__DFT_struct.test* %p_test, i64 2
; CHECK: %t1 = ptrtoint %__DFT_struct.test* %p_test1 to i64
; CHECK: %t2 = ptrtoint %__DFT_struct.test* %p_test2 to i64
; CHECK: %sub = sub i64 %t1, %t2
; CHECK: %offset_idx = sdiv i64 %sub, 8
; CHECK: %p_test3 = getelementptr %__DFT_struct.test,
; CHECK-SAME: %__DFT_struct.test* %p_test, i64 3
; CHECK: %t3 = ptrtoint %__DFT_struct.test* %p_test3 to i64
; CHECK: %pair_size.dt = mul i64 8, 2
; CHECK: %pair_size = mul i64 16, 2
; CHECK: %sub2 = sub i64 %t1, %t3
; CHECK: %offset_idx2 = sdiv i64 %sub2, %pair_size.dt
; CHECK: %test = icmp eq i64 %pair_size, 16
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1

declare i8* @malloc(i64)
declare void @free(i8*)
