; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                                          \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=2                                                                     \
; RUN:    -debug-only=dtrans-soatoaosop-arrays,dtrans-soatoaosop                                                \
; RUN:  2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                                       \
; RUN:    -passes=soatoaosop-arrays-methods-transform -dtrans-soatoaosop-base-ptr-off=2                         \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { ptr, ptr }
%struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>

; Special version of set method setting only the 0th element.

; CHECK:      Checking array's method _ZN3ArrIPiE3setEiS0_
; Transformed instructions are shown.
;   void set(S val) { base[0] = val; }
; CHECK-NEXT: Classification: Set element method
; CHECK-NEXT: ; Dump instructions needing update. Total = 2

define void @_ZN3ArrIPiE3setEiS0_(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg, i32 %arg1, ptr "intel_dtrans_func_index"="2" %arg2) align 2 !intel.dtrans.func.type !5 {
bb:
  %tmp = getelementptr inbounds %struct.Arr, ptr %arg, i64 0, i32 2
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp3 = load ptr, ptr %tmp, align 8
; CHECK-MOD: %tmp3 = load ptr, ptr %tmp, align 8
  %tmp3 = load ptr, ptr %tmp, align 8
; CHECK:      ; MemInst: Element set from arg
; CHECK-NEXT:   store ptr %arg2, ptr %tmp3, align 8
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_struct.Arr, ptr %tmp3, i64 0, i32 1
; CHECK-MOD-NEXT:  store ptr %arg2, ptr %elem, align 8
  store ptr %arg2, ptr %tmp3, align 8
  ret void
}

!intel.dtrans.types = !{!0}

!0 = !{!"S", %struct.Arr zeroinitializer, i32 5, !1, !2, !4, !1, !2}
!1 = !{i32 0, i32 0}
!2 = !{!"A", i32 4, !3}
!3 = !{i8 0, i32 0}
!4 = !{i32 0, i32 2}
!5 = distinct !{!6, !7}
!6 = !{%struct.Arr zeroinitializer, i32 1}
!7 = !{i32 0, i32 1}
