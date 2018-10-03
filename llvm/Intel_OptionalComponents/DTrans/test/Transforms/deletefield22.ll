; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass correctly transforms
; structures with global instances and operator accesses with operator bitcasts.

%struct.test = type { i32, i64, i32 }
@g_test = private global %struct.test { i32 100, i64 1000, i32 10000 }, align 4

define i32 @main(i32 %argc, i8** %argv) {
  ; read and write A and C
  store i32 1, i32* bitcast (i8* bitcast (i32* getelementptr inbounds
                                           (%struct.test, %struct.test* @g_test,
                                            i64 0, i32 0) to i8*) to i32*)
  %valA = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 0)
  store i32 2, i32* getelementptr inbounds (%struct.test, %struct.test* @g_test,
                                            i64 0, i32 2)
  %valC = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 2)

  ; write B
  store i64 3, i64* getelementptr (%struct.test, %struct.test* @g_test,
                                   i64 0, i32 1)

  ret i32 %valA
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK: @g_test = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; Note: The redundant casts are eliminated automatically by the constant folder
;       and for some reason the constant folder turns the first index argument
;       into an i32.
; CHECK: store i32 1, i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                             %__DFT_struct.test* @g_test,
; CHECK-SAME:                             i32 0, i32 0)
; CHECK: %valA = load i32, i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                             %__DFT_struct.test* @g_test,
; CHECK-SAME:                             i64 0, i32 0)
; CHECK: store i32 2, i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                             %__DFT_struct.test* @g_test,
; CHECK-SAME:                             i64 0, i32 1)
; CHECK: %valC = load i32, i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                             %__DFT_struct.test* @g_test,
; CHECK-SAME:                             i64 0, i32 1)
; CHECK-NOT: store i64 3, i64*
