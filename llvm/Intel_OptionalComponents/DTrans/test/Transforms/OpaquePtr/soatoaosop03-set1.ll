; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=2                                                                     \
; RUN:    -debug-only=dtrans-soatoaosop-arrays,dtrans-soatoaosop                                                \
; RUN:  2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                         \
; RUN:    -passes=soatoaosop-arrays-methods-transform -dtrans-soatoaosop-base-ptr-off=2                         \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                          \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=2                                                                     \
; RUN:    -debug-only=dtrans-soatoaosop-arrays,dtrans-soatoaosop                                                \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                       \
; RUN:    -passes=soatoaosop-arrays-methods-transform -dtrans-soatoaosop-base-ptr-off=2                         \
; RUN:  | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }
; CHECK-OP-MOD-DAG: %__SOA_struct.Arr = type <{ i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-OP-MOD-DAG: %__SOA_EL_struct.Arr = type { ptr, ptr }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>

; Special version of set method setting only the 0th element.

; CHECK:      Checking array's method _ZN3ArrIPiE3setEiS0_
; CHECK-OP:      Checking array's method _ZN3ArrIPiE3setEiS0_
; Transformed instructions are shown.
;   void set(S val) { base[0] = val; }
; CHECK-NEXT: Classification: Set element method
; CHECK-NEXT: ; Dump instructions needing update. Total = 2
; CHECK-OP-NEXT: Classification: Set element method
; CHECK-OP-NEXT: ; Dump instructions needing update. Total = 2
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* "intel_dtrans_func_index"="1" nocapture readonly %arg, i32 %arg1, i32* "intel_dtrans_func_index"="2" %arg2) align 2 !intel.dtrans.func.type !6 {
bb:
  %tmp = getelementptr inbounds %struct.Arr, %struct.Arr* %arg, i64 0, i32 2
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp3 = load i32**, i32*** %tmp, align 8
; CHECK-OP:      ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:   %tmp3 = load ptr, ptr %tmp, align 8
; CHECK-MOD:    %tmp3 = load %__SOA_EL_struct.Arr*,  %__SOA_EL_struct.Arr** %tmp, align 8
; CHECK-OP-MOD: %tmp3 = load ptr, ptr %tmp, align 8
  %tmp3 = load i32**, i32*** %tmp, align 8
; CHECK:      ; MemInst: Element set from arg
; CHECK-NEXT:   store i32* %arg2, i32** %tmp3, align 8
; CHECK-OP:      ; MemInst: Element set from arg
; CHECK-OP-NEXT:   store ptr %arg2, ptr %tmp3, align 8
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %tmp3, i64 0, i32 1
; CHECK-MOD-NEXT:  store i32* %arg2, i32** %elem, align 8
; CHECK-OP-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_struct.Arr, ptr %tmp3, i64 0, i32 1
; CHECK-OP-MOD-NEXT:  store ptr %arg2, ptr %elem, align 8
  store i32* %arg2, i32** %tmp3, align 8
  ret void
}

!intel.dtrans.types = !{!2}

!1 = !{i32 0, i32 0}
!2 = !{!"S", %struct.Arr zeroinitializer, i32 5, !1, !3, !5, !1, !3}
!3 = !{!"A", i32 4, !4}
!4 = !{i8 0, i32 0}
!5 = !{i32 0, i32 2}
!6 = distinct !{!7, !8}
!7 = !{%struct.Arr zeroinitializer, i32 1}
!8 = !{i32 0, i32 1}
