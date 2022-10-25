; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete field pass correctly transforms
; a global variable that is a pointer to types being changed. Also, checks that
; metadata attached gets updated.

%struct.test = type { i32, i64, i32 }

@g_test = private global %struct.test zeroinitializer
@g_ptr = private global %struct.test* @g_test, !intel_dtrans_type !3
; CHECK-NONOPAQUE-DAG: @g_test = private global %__DFT_struct.test zeroinitializer
; CHECK-NONOPAQUE-DAG: @g_ptr = private global %__DFT_struct.test* @g_test, !intel_dtrans_type ![[MD_PTR1:[0-9]+]]

; Variable print order differs because with opaque pointers only one variable
; is changed during the transformation.
; CHECK-OPAQUE-DAG: @g_ptr = private global ptr @g_test, !intel_dtrans_type ![[MD_PTR1:[0-9]+]]
; CHECK-OPAQUE-DAG: @g_test = private global %__DFT_struct.test zeroinitializer

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !5 {
  %p_test_A = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* @g_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C
  %sum = add i32 %valA, %valC

  ; write B
  store i64 3, i64* %p_test_B

  ret i32 %sum
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = !{i8 0, i32 2}  ; i8**
!5 = distinct !{!4}
!6 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!6}

; CHECK-NONOPAQUE: ![[MD_PTR1]] = !{%__DFT_struct.test zeroinitializer, i32 1}

; CHECK-OPAQUE: ![[MD_PTR1]] = !{%__DFT_struct.test zeroinitializer, i32 1}
