; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size argument of a malloc call is correctly
; updated when it is a variable multiple of the structure size where the
; intermediate values have other uses.

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
  ; Calculate a size.
  %sz = mul i32 %argc, 16
  %sz64 = sext i32 %sz to i64
  ; This is an arbitrary use. Imagine it having some purpose.
  %other = add i32 %sz, 32
  ; Allocate an array of structures.
  %p = call i8* @malloc(i64 %sz64)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %val
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %sz.dt = mul i32 %argc, 8
; CHECK: %sz = mul i32 %argc, 16
; CHECK: %sz64 = sext i32 %sz.dt to i64
; CHECK: %p = call i8* @malloc(i64 %sz64)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1


declare i8* @malloc(i64)
declare void @free(i8*)
