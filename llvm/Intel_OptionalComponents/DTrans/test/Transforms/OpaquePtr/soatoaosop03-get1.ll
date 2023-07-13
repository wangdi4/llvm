; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop  \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop-arrays \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                 \
; RUN:    -passes=soatoaosop-arrays-methods-transform -dtrans-soatoaosop-base-ptr-off=3                         \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-MOD-DAG: %__SOA_struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { ptr, ptr }
%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

; The following method should be classified as get-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   S* get() {
;     return base;
;   }
; CHECK:      Checking array's method _ZN3ArrIPvE3getEi
; CHECK-NEXT: Classification: Get pointer to element method
; CHECK-TRANS:; Dump instructions needing update. Total = 2

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPvE3getEi(ptr "intel_dtrans_func_index"="2" %this) !intel.dtrans.func.type !9 {
entry:
  %base3 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp2 = load ptr, ptr %base3, align 8
; CHECK-MOD:      %tmp2 = load ptr, ptr %base3, align 8
  %tmp2 = load ptr, ptr %base3, align 8
; CHECK-TRANS:       ; MemInst: Address in ret
; CHECK-TRANS-NEXT:  ret ptr %tmp2
; CHECK-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp2, i64 0, i32 1
; CHECK-MOD-NEXT:    ret ptr %elem
  ret ptr %tmp2
}

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i8 0, i32 2}
!9 = distinct !{!8, !10}
!10 = !{%struct.Arr.0 zeroinitializer, i32 1}
