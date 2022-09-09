; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the index arguments of GEP instructions are correctly
; updated when they are accessing a field within a fixed array of a structure
; with a deleted field, and that the size of the allocation of the array is
; updated.

%struct.test = type { i32, i64, i32 }

define i32 @doSomething([4 x %struct.test]* %p_test) {
  ; Get pointers to each field
  %p_test_A = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 0
  %p_test_B = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 1
  %p_test_C = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  %sum = add i32 %valA, %valC
  ret i32 %sum
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate a structure.
  %p = call i8* @malloc(i64 64)
  %p_test = bitcast i8* %p to [4 x %struct.test]*

  ; Call a function to do something.
  %val = call i32 @doSomething([4 x %struct.test]* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %val
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @malloc(i64 32)
; CHECK: %p_test = bitcast i8* %p to [4 x %__DFT_struct.test]*
; CHECK: %val = call i32 @doSomething.1([4 x %__DFT_struct.test]* %p_test)


; CHECK-LABEL: define internal i32 @doSomething.1([4 x %__DFT_struct.test]* %p_test)
; CHECK: %p_test_A = getelementptr [4 x %__DFT_struct.test],
; CHECK-SAME:                      [4 x %__DFT_struct.test]* %p_test,
; CHECK-SAME:                      i64 0, i32 1, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr [4 x %__DFT_struct.test],
; CHECK-SAME:                      [4 x %__DFT_struct.test]* %p_test,
; CHECK-SAME:                      i64 0, i32 1, i32 1

declare i8* @malloc(i64)
declare void @free(i8*)
