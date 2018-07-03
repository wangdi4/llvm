; RUN: opt -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete field pass correctly transforms
; structures with global arrays of instances with non-default initializers.

%struct.test = type { i32, i64, i32 }
@g_test = private global [4 x %struct.test] [
  %struct.test { i32 100, i64 1000, i32 10000 },
  %struct.test { i32 200, i64 2000, i32 20000 },
  %struct.test { i32 300, i64 3000, i32 30000 },
  %struct.test { i32 400, i64 4000, i32 40000 } ]

define void @f() {
  %p = getelementptr [4 x %struct.test], [4 x %struct.test]* @g_test,
                     i64 0, i32 2
  ; read A and C
  %pA = getelementptr %struct.test, %struct.test* %p, i64 0, i32 0
  %valA = load i32, i32* %pA
  %pC = getelementptr %struct.test, %struct.test* %p, i64 0, i32 2
  %valC = load i32, i32* %pC

  ; write B
  %pB = getelementptr %struct.test, %struct.test* %p, i64 0, i32 1
  store i64 3, i64* %pB

  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @f()
  ret i32 0
}

; Note: Delete fields is currently disabled for structures used in global
;       array variables because the transformation was broken. When it is
;       re-enabled, we should be able to just remove this first check and
;       remove the "FUTURE-" prefix from the remaining checks.
; CHECK-NOT: %__DFT_struct.test = type { i32, i32 }
; FUTURE-CHECK: %__DFT_struct.test = type { i32, i32 }

; FUTURE-CHECK-LABEL: @g_test = private global [4 x %__DFT_struct.test] [
; FUTURE-CHECK-SAME: %__DFT_struct.test { i32 100, i32 10000 },
; FUTURE-CHECK-SAME: %__DFT_struct.test { i32 200, i32 20000 },
; FUTURE-CHECK-SAME: %__DFT_struct.test { i32 300, i32 30000 },
; FUTURE-CHECK-SAME: %__DFT_struct.test { i32 400, i32 40000 }]

; FUTURE-CHECK-LABEL: define void @f() {
; FUTURE-CHECK: %p = getelementptr [4 x %__DFT_struct.test],
; FUTURE-CHECK-SAME:           [4 x %__DFT_struct.test]* @g_test, i64 0, i32 2
; FUTURE-CHECK: %pA = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p,
; FUTURE-CHECK-SAME:                i64 0, i32 0
; FUTURE-CHECK: %valA = load i32, i32* %pA
; FUTURE-CHECK: %pC = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p
; FUTURE-CHECK-SAME:                i64 0, i32 1
; FUTURE-CHECK: %valC = load i32, i32* %pC
; FUTURE-CHECK-NOT: %pB =
; FUTURE-CHECK-NOT: store

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: call void @f()
