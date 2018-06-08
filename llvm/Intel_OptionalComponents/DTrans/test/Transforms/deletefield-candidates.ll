; REQUIRES: asserts
; RUN: sed -e s/.T1:// %s | \
; RUN:   opt -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK1 %s
; RUN: sed -e s/.T2:// %s | \
; RUN:   opt -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK2 %s
; RUN: sed -e s/.T3:// %s | \
; RUN:   opt -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK3 %s
; RUN: sed -e s/.T4:// %s | \
; RUN:   opt -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK4 %s

; This test verifies only the candidate structure selection of the
; dtrans delete fields pass. Safety condition checks are verified elsewhere.

; The use of 'sed' above allows IR for multiple tests to be combined into a
; single test file while avoiding problems with ordering of the output.

; T1 -- check for the identification of struct with an unused field.

; T2 -- check for the identification of struct with a field that is
;       written but not read.

; T3 -- check that we don't select struct where all fields are read and written.

; T4 -- check that we don't select a struct with a field that are read but not
;       written.

%struct.test = type { i32, i64, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  ; Common to all tests, allocate a struct and get pointers to its fields.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; Common to all tests, read and write A and C
  store i32 %argc, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 %argc, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

;T1: ; No extra IR needed.

;T2: %val = sext i32 %argc to i64
;T2: store i64 %val, i64* %p_test_B

;T3: %val = sext i32 %argc to i64
;T3: store i64 %val, i64* %p_test_B
;T3: %valB = load i64, i64* %p_test_B

;T4: %valB = load i64, i64* %p_test_B

  ; Common to all tests, free the structure
  call void @free(i8* %p)
  ret i32 %valA
}

; CHECK1: Selected for deletion: %struct.test
; CHECK2: Selected for deletion: %struct.test
; CHECK3-NOT: Selected for deletion: %struct.test
; CHECK3: No candidates found.
; CHECK4-NOT: Selected for deletion: %struct.test
; CHECK4: No candidates found.

declare i8* @malloc(i64)
declare void @free(i8*)
