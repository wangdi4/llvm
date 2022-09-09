; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -internalize -internalize-public-api-list main -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -passes='internalize,dtrans-deletefield' -internalize-public-api-list main -S -o - %s | FileCheck %s

; This test case checks that delete fields pass runs correctly when there is
; a base-named type but there is no use across the whole module. From the
; test case, %class.simple is the base name for %class.simple.1, but there
; is no use for %class.simple in the input module.

%struct.simple = type { %struct.test }
%struct.simple.1 = type { %struct.test }

%struct.test = type { i32, i64, i32 }

define void @foo(%struct.simple.1* %0) {
  ret void
}

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

; CHECK: %__DFT_struct.test = type { i32 }
; CHECK: %__DFDT_struct.simple.1 = type { %__DFT_struct.test }
; CHECK-NOT: %__DFDT_struct.simple = type { %__DFT_struct.test }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @malloc(i64 4)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.2(%__DFT_struct.test* %p_test)

; CHECK: define internal void @foo.1(%__DFDT_struct.simple.1* %0)

; CHECK: define internal i32 @doSomething.2(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK-NOT: %p_test_C = getelementptr

declare i8* @malloc(i64)
declare void @free(i8*)
