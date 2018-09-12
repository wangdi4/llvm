; REQUIRES: asserts
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield < %s -debug-only=dtrans-deletefield \
; RUN:     -disable-output 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -disable-output -passes=dtrans-deletefield \
; RUN:     -debug-only=dtrans-deletefield %s 2>&1 | FileCheck %s

; This test verifies that the dtrans delete pass does not try to transform
; structures that do not meet the necessary safety conditions.
;
; The safety check is straightforward, so it is not necessary to check every
; safety condition individually.

%struct.test = type { i32, i64, i32 }
%struct.other = type { i32, i32, i32, i32 }


define i32 @main(i32 %argc, i8** %argv) {
  ; Common to all tests, allocate a struct and get pointers to its fields.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 %argc, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 %argc, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; Introduce a bad bitcast.
  %p_other = bitcast i32* %p_test_A to %struct.other*

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %valA
}

; CHECK: Delete field: looking for candidate structures.
; CHECK-DAG: Can delete field: %struct.test @ 1
; CHECK-DAG: Rejecting %struct.test based on safety data.
; CHECK-DAG: Can delete field: %struct.other @ 0
; CHECK-DAG: Can delete field: %struct.other @ 1
; CHECK-DAG: Can delete field: %struct.other @ 2
; CHECK-DAG: Can delete field: %struct.other @ 3
; CHECK-DAG: Rejecting %struct.other based on safety data.
; CHECK-NOT: Selected for deletion:
; CHECK: No candidates found.

declare i8* @malloc(i64)
declare void @free(i8*)
