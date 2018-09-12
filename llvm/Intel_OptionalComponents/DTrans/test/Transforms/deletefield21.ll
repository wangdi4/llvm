; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass correctly transforms
; structures with global instances and operator accesses.

%struct.test = type { i32, i64, i32 }
@g_test = private global %struct.test { i32 100, i64 1000, i32 10000 }, align 4

define i32 @main(i32 %argc, i8** %argv) {
  ; read and write A and C
  %t1 = bitcast i32* getelementptr inbounds (%struct.test,
                                             %struct.test* @g_test,
                                             i64 0, i32 0) to i8*
  %p_test_A = bitcast i8* %t1 to i32*
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 0)
  %t2 = bitcast i32* getelementptr inbounds (%struct.test,
                                             %struct.test* @g_test,
                                             i64 0, i32 2) to i8*
  %p_test_C = bitcast i8* %t2 to i32*
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 2)

  ; write B
  %t3 = bitcast i64* getelementptr inbounds (%struct.test,
                                             %struct.test* @g_test,
                                             i64 0, i32 1) to i8*
  %p_test_B = bitcast i8* %t3 to i64*
  store i64 3, i64* %p_test_B

  ret i32 %valA
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK: @g_test = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %t1 = bitcast i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                                       %__DFT_struct.test* @g_test,
; CHECK-SAME:                                       i64 0, i32 0) to i8*
; CHECK: %p_test_A = bitcast i8* %t1 to i32*
; CHECK: store i32 1, i32* %p_test_A
; CHECK: %t2 = bitcast i32* getelementptr inbounds (%__DFT_struct.test,
; CHECK-SAME:                                       %__DFT_struct.test* @g_test,
; CHECK-SAME:                                       i64 0, i32 1) to i8*
; CHECK: %p_test_C = bitcast i8* %t2 to i32*
; CHECK: store i32 2, i32* %p_test_C
; CHECK-NOT: %t3 = bitcast i64* getelementptr
; CHECK-NOT: %p_test_B = bitcast
; CHECK-NOT: store i64 3, i64*
