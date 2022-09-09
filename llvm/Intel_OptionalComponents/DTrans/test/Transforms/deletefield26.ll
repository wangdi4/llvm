; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the DTrans delete field pass correctly transforms
; structures with global arrays of instances with non-default initializers.

%struct.test = type { i32, i64, i32 }
@g_test = private global [4 x %struct.test] [
  %struct.test { i32 100, i64 1000, i32 10000 },
  %struct.test { i32 200, i64 2000, i32 20000 },
  %struct.test { i32 300, i64 3000, i32 30000 },
  %struct.test { i32 400, i64 4000, i32 40000 } ]

define internal i32 @f() {
  %p = getelementptr [4 x %struct.test], [4 x %struct.test]* @g_test,
                     i64 0, i32 2
  ; read A and C
  %pA = getelementptr %struct.test, %struct.test* %p, i64 0, i32 0
  %valA = load i32, i32* %pA
  %pC = getelementptr %struct.test, %struct.test* %p, i64 0, i32 2
  %valC = load i32, i32* %pC
  %sum = add i32 %valA, %valC

  ; write B
  %pB = getelementptr %struct.test, %struct.test* %p, i64 0, i32 1
  store i64 3, i64* %pB

  ret i32 %sum
}

define i32 @main(i32 %argc, i8** %argv) {
  %res = call i32 @f()
  ret i32 %res
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: @g_test = private global [4 x %__DFT_struct.test] [
; CHECK-SAME: %__DFT_struct.test { i32 100, i32 10000 },
; CHECK-SAME: %__DFT_struct.test { i32 200, i32 20000 },
; CHECK-SAME: %__DFT_struct.test { i32 300, i32 30000 },
; CHECK-SAME: %__DFT_struct.test { i32 400, i32 40000 }]

; CHECK-LABEL: define internal i32 @f() {
; CHECK: %p = getelementptr [4 x %__DFT_struct.test],
; CHECK-SAME:           [4 x %__DFT_struct.test]* @g_test, i64 0, i32 2
; CHECK: %pA = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p,
; CHECK-SAME:                i64 0, i32 0
; CHECK: %valA = load i32, i32* %pA
; CHECK: %pC = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p
; CHECK-SAME:                i64 0, i32 1
; CHECK: %valC = load i32, i32* %pC
; CHECK-NOT: %pB =
; CHECK-NOT: store

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %res = call i32 @f()
