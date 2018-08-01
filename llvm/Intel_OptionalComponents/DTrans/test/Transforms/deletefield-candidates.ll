; REQUIRES: asserts
; RUN: sed -e s/.T1:// %s | \
; RUN:   opt -whole-program-assume -dtrans-identify-unused-values=false  -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK1 %s
; RUN: sed -e s/.T2:// %s | \
; RUN:   opt -whole-program-assume -dtrans-identify-unused-values=false  -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK2 %s
; RUN: sed -e s/.T3:// %s | \
; RUN:   opt -whole-program-assume -dtrans-identify-unused-values=false  -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK3 %s
; RUN: sed -e s/.T4:// %s | \
; RUN:   opt -whole-program-assume -dtrans-identify-unused-values=false  -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK4 %s
; RUN: sed -e s/.T5:// %s | \
; RUN:   opt -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -debug-only=dtrans-deletefield \
; RUN:       -disable-output 2>&1 | FileCheck --check-prefix=CHECK5 %s

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

; T5 -- check that we don't optimize a struct when all fields are unread.

%struct.test = type { i32, i64, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  ; Common to all tests, allocate a struct and get pointers to its fields.
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test*
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; Common to all tests, read A and C
  store i32 %argc, i32* %p_test_A
  store i32 %argc, i32* %p_test_C

  ; For T1-T4, write A and C
;T1:  %valA = load i32, i32* %p_test_A
;T1:  %valC = load i32, i32* %p_test_C
;T2:  %valA = load i32, i32* %p_test_A
;T2:  %valC = load i32, i32* %p_test_C
;T3:  %valA = load i32, i32* %p_test_A
;T3:  %valC = load i32, i32* %p_test_C
;T4:  %valA = load i32, i32* %p_test_A
;T4:  %valC = load i32, i32* %p_test_C

;T2: %val = sext i32 %argc to i64
;T2: store i64 %val, i64* %p_test_B

;T3: %val = sext i32 %argc to i64
;T3: store i64 %val, i64* %p_test_B
;T3: %valB = load i64, i64* %p_test_B

;T4: %valB = load i64, i64* %p_test_B

;T5: ; No substitutions required.

  ; Common to all tests, free the structure
  call void @free(i8* %p)
  ret i32 0
}

; CHECK1: Selected for deletion: %struct.test
; CHECK2: Selected for deletion: %struct.test
; CHECK3-NOT: Selected for deletion: %struct.test
; CHECK3: No candidates found.
; CHECK4-NOT: Selected for deletion: %struct.test
; CHECK4: No candidates found.
; CHECK5-NOT: Selected for deletion: %struct.test
; CHECK5: No candidates found.

declare i8* @malloc(i64)
declare void @free(i8*)
