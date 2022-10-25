; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; structures with global instances and a zero initializer.

%struct.test = type { i32, i64, i32 }
@g_test = private global %struct.test zeroinitializer, align 4

; CHECK: %__DFT_struct.test = type { i32, i32 }
; CHECK: @g_test = private global %__DFT_struct.test zeroinitializer, align 4


define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; write B
  store i64 3, i64* %p_test_B

  %sum = add i32 %valA, %valC
  ret i32 %sum
}

; CHECK-LABEL: define i32 @main
; CHECK: %p_test_A = getelementptr %__DFT_struct.test, {{.*}} @g_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test, {{.*}} @g_test, i64 0, i32 1
; CHECK-NOT: store i64 3

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!5}
