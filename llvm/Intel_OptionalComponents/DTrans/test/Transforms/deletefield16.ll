; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies the case of a byte-flattened GEPs accessing a deleted
; field which is written after a bitcast.

%struct.test = type { i32, i64, i32 }

define i32 @doSomething(%struct.test* %p_test) {
  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test* %p_test to i8*

  ; Get pointers to each field
  %p8_B = getelementptr i8, i8* %p, i64 4
  %p8_C = getelementptr i8, i8* %p, i64 12
  %p_test_A = bitcast i8* %p to i32*
  %p_test_B = bitcast i8* %p8_B to i64*
  %p_test_C = bitcast i8* %p8_C to i32*

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; write B
  store i64 3, i64* %p_test_B

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

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @malloc(i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p = bitcast %__DFT_struct.test* %p_test to i8*
; CHECK-NOT: %p8_B = getelementptr i8, i8* %p, i64 4
; CHECK: %p8_C = getelementptr i8, i8* %p, i64 4
; CHECK: %p_test_A = bitcast i8* %p to i32*
; CHECK-NOT: %p_test_B = bitcast i8* %p8_B to i32*
; CHECK:  %p_test_C = bitcast i8* %p8_C to i32*
; CHECK-NOT: store i64 3, i64* %p_test_B


declare i8* @malloc(i64)
declare void @free(i8*)
