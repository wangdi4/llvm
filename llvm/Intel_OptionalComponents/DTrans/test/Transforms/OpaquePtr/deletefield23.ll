; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass is able to handle global
; variables with types that are dependent on a type having fields deleted.

%struct.test = type { i32, i64, i32 }
%struct.dep = type { i32, %struct.test* }
@g_del = private global %struct.test { i32 100, i64 1000, i32 10000 }, align 4
@g_dep = private global %struct.dep zeroinitializer, align 4

; CHECK-NONOPAQUE-DAG: %__DFT_struct.test = type { i32, i32 }
; CHECK-NONOPAQUE-DAG: %__DFDT_struct.dep = type { i32, %__DFT_struct.test* }
; CHECK-NONOPAQUE-DAG: @g_del = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4
; CHECK-NONOPAQUE-DAG: @g_dep = private global %__DFDT_struct.dep zeroinitializer, align 4

; Check order with opaque pointers is different because only one variable is cloned.
; CHECK-OPAQUE-DAG: %struct.dep = type { i32, ptr }
; CHECK-OPAQUE-DAG: %__DFT_struct.test = type { i32, i32 }
; CHECK-OPAQUE-DAG: @g_dep = private global %struct.dep zeroinitializer, align 4
; CHECK-OPAQUE-DAG: @g_del = private global %__DFT_struct.test { i32 100, i32 10000 }, align 4

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !5 {
  ; read dep.A
  ; With opaque pointers, the GEP in this load could be elided, causing the field to be
  ; marked with 'non-GEP access', which would make it non-deletable. This would make the
  ; behavior different between the opaque pointer version of the test and non-opaque pointer.
  ; Therefore, we need to use both fields of the structure to avoid %struct.dep from being
  ; transformed.
  %val = load i32, i32* getelementptr inbounds (%struct.dep,
                                                %struct.dep* @g_dep,
                                                i64 0, i32 0)

  %ptr = load %struct.test*, %struct.test** getelementptr inbounds (%struct.dep,
                                                                    %struct.dep* @g_dep,
                                                                    i64 0, i32 1)

  %same = icmp eq %struct.test* %ptr, @g_del
  br i1 %same, label %t2, label %t1

t2:
  ret i32 0

t1:
  ; Read and write a pointer to del from dep
  store %struct.test* @g_del, %struct.test** getelementptr (%struct.dep,
                                                          %struct.dep* @g_dep,
                                                          i64 0, i32 1)
  %p = load %struct.test*, %struct.test** getelementptr inbounds (%struct.dep,
                                                                %struct.dep*
                                                                    @g_dep,
                                                                i64 0, i32 1)

  ; read A and C
  %valA = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_del,
                                                 i64 0, i32 0)
  %valC = load i32, i32* getelementptr inbounds (%struct.test,
                                                 %struct.test* @g_del,
                                                 i64 0, i32 2)

  ; write B
  store i64 3, i64* getelementptr (%struct.test, %struct.test* @g_del,
                                   i64 0, i32 1)

  %sum = add i32 %valA, %valC
  %result = add i32 %val, %sum
  ret i32 %result
}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = !{i8 0, i32 2}  ; i8**
!5 = distinct !{!4}
!6 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!7 = !{!"S", %struct.dep zeroinitializer, i32 2, !1, !3} ; { i32, %struct.test* }

!intel.dtrans.types = !{!6, !7}
