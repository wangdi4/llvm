; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -dtrans-identify-unused-values=false -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes=dtrans-deletefield -dtrans-identify-unused-values=false -S -o - %s | FileCheck %s

; This test verifies that the DTrans delete field pass correctly transforms
; structures with global arrays of instances with non-default initializers
; and operator getelementptr accesses.

%struct.test = type { i32, i64, i32 }
@g_test = private global [4 x %struct.test] [
  %struct.test { i32 100, i64 1000, i32 10000 },
  %struct.test { i32 200, i64 2000, i32 20000 },
  %struct.test { i32 300, i64 3000, i32 30000 },
  %struct.test { i32 400, i64 4000, i32 40000 } ]

define void @f() {
  ; read A and C
  %valA = load i32, i32 * getelementptr ([4 x %struct.test],
                                         [4 x %struct.test]* @g_test,
                                         i64 0, i32 2, i32 0)
  %valC = load i32, i32 * getelementptr ([4 x %struct.test],
                                         [4 x %struct.test]* @g_test,
                                         i64 0, i32 2, i32 2)

  ; write B
  store i64 3, i64 * getelementptr ([4 x %struct.test],
                                    [4 x %struct.test]* @g_test,
                                    i64 0, i32 2, i32 1)

  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @f()
  ret i32 0
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: @g_test = private global [4 x %__DFT_struct.test] [
; CHECK-SAME: %__DFT_struct.test { i32 100, i32 10000 },
; CHECK-SAME: %__DFT_struct.test { i32 200, i32 20000 },
; CHECK-SAME: %__DFT_struct.test { i32 300, i32 30000 },
; CHECK-SAME: %__DFT_struct.test { i32 400, i32 40000 }]

; CHECK-LABEL: define void @f() {
; CHECK: %valA = load i32, i32* getelementptr inbounds
; CHECK-SAME:                           ([4 x %__DFT_struct.test],
; CHECK-SAME:                            [4 x %__DFT_struct.test]* @g_test,
; CHECK-SAME:                            i64 0, i32 2, i32 0)
; CHECK: %valC = load i32, i32* getelementptr inbounds
; CHECK-SAME:                           ([4 x %__DFT_struct.test],
; CHECK-SAME:                            [4 x %__DFT_struct.test]* @g_test,
; CHECK-SAME:                            i64 0, i32 2, i32 1)
; CHECK-NOT: store i64 3

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: call void @f()
