; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -dtrans-deletefield-max-struct=1 -debug-only=dtrans-deletefield -disable-output %s 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -dtrans-deletefield-max-struct=1 -debug-only=dtrans-deletefield -disable-output %s 2>&1 | FileCheck %s

; This test verifies that the -dtrans-deletefield-max-structure triaging
; flag can be used to control the number of structures that have fields
; deleted from them to help with triaging problems.

%struct.test = type { i32, i64, i32, %struct.other* }
%struct.other = type { i32, %struct.test* }

define i1 @doSomething(%struct.test* %p_test, %struct.other* %p_other_in) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; load a pointer to Other
  %pp_other = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  %p_other = load %struct.other*, %struct.other** %pp_other

  %cmp = icmp eq %struct.other* %p_other_in, %p_other

  ret i1 %cmp
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate two structures.
  %p1 = call i8* @malloc(i64 24)
  %p_test = bitcast i8* %p1 to %struct.test*
  %p2 = call i8* @malloc(i64 16)
  %p_other = bitcast i8* %p2 to %struct.other*

  ; Store a pointer to each structures in the other
  %pp_test = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 1
  store %struct.test* %p_test, %struct.test** %pp_test
  %pp_other = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  store %struct.other* %p_other, %struct.other** %pp_other

  ; Re-load p_test from p_other and call doSomething
  %pp_test2 = getelementptr %struct.other, %struct.other* %p_other, i64 0, i32 1
  %p_test2 = load %struct.test*, %struct.test** %pp_test2
  %ret = call i1 @doSomething(%struct.test* %p_test2, %struct.other* %p_other)

  ; Free the structures
  call void @free(i8* %p1)
  call void @free(i8* %p2)
  ret i32 0
}

; CHECK-DAG: LLVM Type: %struct.other
; CHECK-DAG:  Can delete field: %struct.other @ 0
; CHECK-DAG:LLVM Type: %struct.test
; CHECK-DAG: Can delete field: %struct.test @ 1
; CHECK: Triaging: Reducing 2 candidates down to 1 structures
; CHECK:0:  Triaging will process: %struct.other
; CHECK:1:  Triaging will NOT process: %struct.test
; CHECK:  Selected for deletion: %struct.other

declare i8* @malloc(i64)
declare void @free(i8*)
