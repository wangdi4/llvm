; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                     \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>'  \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3                                                                      \
; RUN:    -debug-only=dtrans-soatoaosop                                                                          \
; RUN:  2>&1 | FileCheck %s
;
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                     \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>'  \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3                                                                      \
; RUN:    -debug-only=dtrans-soatoaosop-arrays                                                                   \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
;
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                  \
; RUN:    -passes=soatoaosop-arrays-methods-transform                                                            \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                         \
; RUN:    -dtransop-optbase-process-function-declaration                                                         \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { ptr, ptr }

; The following method should be classified as append-like.
; Instructions to transform are shown.
;   void add(const S &e) {
;     realloc(1);
;
;     base[size] = e;
;     ++size;
;   }
; CHECK:      Checking array's method _ZN3ArrIPiE3addERKS0_
; CHECK-NEXT: Classification: Append element method

; CHECK-TRANS: ; Dump instructions needing update. Total = 4
; Argument's name reused.
; CHECK-MOD: @_ZN3ArrIPiE3addERKS0_{{.*}}

define void @_ZN3ArrIPiE3addERKS0_(ptr "intel_dtrans_func_index"="1" %this, ptr nocapture readonly dereferenceable(8) "intel_dtrans_func_index"="2" %e) !intel.dtrans.func.type !9 {
entry:
  call void @_ZN3ArrIPiE7reallocEi(ptr %this, i32 1)
; CHECK-TRANS:      ; Arg: Load from arg
; CHECK-TRANS-NEXT: %tmp = load ptr, ptr %e, align 8
; CHECK-MOD:        %copy = load ptr, ptr %0, align 8
; CHECK-MOD-NEXT:   %tmp = load ptr, ptr %e, align 8
  %tmp = load ptr, ptr %e, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT: %tmp1 = load ptr, ptr %base, align 8
; CHECK-MOD:        %tmp1 = load ptr, ptr %base, align 8
  %tmp1 = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp2 = load i32, ptr %size, align 8
  %idxprom = sext i32 %tmp2 to i64
; CHECK-TRANS:      ; MemInstGEP: Element set from arg
; CHECK-TRANS-NEXT: %arrayidx = getelementptr inbounds ptr, ptr %tmp1, i64 %idxprom
; CHECK-MOD:        %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr, ptr %tmp1, i64 %idxprom
  %arrayidx = getelementptr inbounds ptr, ptr %tmp1, i64 %idxprom
; CHECK-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_struct.Arr, ptr %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_struct.Arr, ptr %arrayidx, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element set from arg
; CHECK-TRANS-NEXT: store ptr %tmp, ptr %arrayidx, align 8
; CHECK-MOD-NEXT:   store ptr %copy, ptr %elem, align 8
; CHECK-MOD-NEXT:   store ptr %tmp, ptr %elem1, align 8
  store ptr %tmp, ptr %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp3 = load i32, ptr %size2, align 8
  %inc = add nsw i32 %tmp3, 1
  store i32 %inc, ptr %size2, align 8
  ret void
}

declare !intel.dtrans.func.type !11 void @_ZN3ArrIPiE7reallocEi(ptr "intel_dtrans_func_index"="1", i32)

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i32 0, i32 2}
!9 = distinct !{!10, !8}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = !{!10}
