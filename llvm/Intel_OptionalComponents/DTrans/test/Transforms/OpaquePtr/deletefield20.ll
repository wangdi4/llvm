; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass correctly transforms
; structures with global instances and operator accesses.

%struct.test = type { i32, i64, i32 }
@g_test = private global %struct.test { i32 100, i64 1000, i32 10000 }, align 4

; CHECK: %__DFT_struct.test = type { i32, i32 }
; CHECK: @g_test = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  ; read and write A and C
  store i32 1, i32* getelementptr inbounds (%struct.test, %struct.test* @g_test,
                                            i64 0, i32 0)
  %valA = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 0)
  store i32 2, i32* getelementptr inbounds (%struct.test, %struct.test* @g_test,
                                            i64 0, i32 2)
  %valC = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 2)

  ; read B, but let it be an unused value.
  %valB = load i64, i64* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_test,
                                                 i64 0, i32 1)

  ; write B
  store i64 3, i64* getelementptr (%struct.test, %struct.test* @g_test,
                                   i64 0, i32 1)

  %sum = add i32 %valA, %valC
  ret i32 %sum
}

; CHECK-LABEL: define i32 @main
; CHECK: store i32 1, {{.*}} getelementptr inbounds (%__DFT_struct.test, {{.*}} @g_test, i64 0, i32 0)
; CHECK: %valA = load i32, {{.*}} getelementptr inbounds (%__DFT_struct.test, {{.*}} @g_test, i64 0, i32 0)
; CHECK: store i32 2, {{.*}} getelementptr inbounds (%__DFT_struct.test, {{.*}} @g_test, i64 0, i32 1)
; CHECK: %valC = load i32, {{.*}} getelementptr inbounds (%__DFT_struct.test, {{.*}} @g_test, i64 0, i32 1)
; CHECK-NOT: %valB = load i64
; CHECK-NOT: store i64 3

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!5}
