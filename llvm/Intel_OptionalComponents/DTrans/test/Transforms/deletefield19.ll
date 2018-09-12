; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass correctly transforms
; structures with global instances and a struct initializer.

%struct.test = type { i32, i64, i32 }
@g_test = private global %struct.test { i32 100, i64 1000, i32 10000 }, align 4

define i32 @main(i32 %argc, i8** %argv) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; write B
  store i64 3, i64* %p_test_B

  ret i32 %valA
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK: @g_test = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* @g_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* @g_test, i64 0, i32 1
; CHECK-NOT: store i64 3. i64* p_test_B
