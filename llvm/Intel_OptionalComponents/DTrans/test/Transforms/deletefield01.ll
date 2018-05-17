; RUN: opt -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass correctly transforms
; structures that have unused fields and meet the necessary safety conditions.

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
  ; Allocate a structure.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %val
}

; FIXME: These checks currently assume the type is only renamed.
;        Several checks will need to be updated when the optimization
;        is fully implemented.

; CHECK: %__DFT_struct.test = type { i32, i64, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @malloc(i64 16)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)


; CHECK: define i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; FIXME: When the optimization is working, we should check that the B GEP
;        is gone, and the index for C should become 1.
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 2


declare i8* @malloc(i64)
declare void @free(i8*)
