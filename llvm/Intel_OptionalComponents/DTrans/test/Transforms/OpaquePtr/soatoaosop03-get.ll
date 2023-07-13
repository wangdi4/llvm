; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop                                                                         \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN:    -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:    -debug-only=dtrans-soatoaosop-arrays                                                                  \
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
;   S* get(int i) {
;     if (size > 7)
;       return base + i * i + 1;
;     return base + i;
;   }
; CHECK:      Checking array's method _ZN3ArrIPvE3getEi
; CHECK-NEXT: Classification: Get pointer to element method
; CHECK-TRANS:; Dump instructions needing update. Total = 7

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPvE3getEi(ptr "intel_dtrans_func_index"="2" %this, i32 %i) !intel.dtrans.func.type !9 {
entry:
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp = load i32, ptr %size, align 8
  %cmp = icmp sgt i32 %tmp, 7
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT: %tmp1 = load ptr, ptr %base, align 8
; CHECK-MOD:      %tmp1 = load ptr, ptr %base, align 8
  %tmp1 = load ptr, ptr %base, align 8
  %mul = mul nsw i32 %i, %i
  %idx.ext = sext i32 %mul to i64
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %add.ptr = getelementptr inbounds ptr, ptr %tmp1, i64 %idx.ext
; CHECK-MOD:      %add.ptr = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp1, i64 %idx.ext
  %add.ptr = getelementptr inbounds ptr, ptr %tmp1, i64 %idx.ext
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT: %add.ptr2 = getelementptr inbounds ptr, ptr %add.ptr, i64 1
; CHECK-MOD:      %add.ptr2 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %add.ptr, i64 1
  %add.ptr2 = getelementptr inbounds ptr, ptr %add.ptr, i64 1
  br label %return

if.end:                                           ; preds = %entry
  %base3 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp2 = load ptr, ptr %base3, align 8
; CHECK-MOD:      %tmp2 = load ptr, ptr %base3, align 8
  %tmp2 = load ptr, ptr %base3, align 8
  %idx.ext4 = sext i32 %i to i64
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %add.ptr5 = getelementptr inbounds ptr, ptr %tmp2, i64 %idx.ext4
; CHECK-MOD:      %add.ptr5 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp2, i64 %idx.ext4
  %add.ptr5 = getelementptr inbounds ptr, ptr %tmp2, i64 %idx.ext4
  br label %return

return:                                           ; preds = %if.end, %if.then
; CHECK-TRANS:     ; MemInstGEP: Address in ret
; CHECK-TRANS-NEXT:  %retval.0 = phi ptr [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
; CHECK-MOD:      %retval.0 = phi ptr [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
; CHECK-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %retval.0, i64 0, i32 1
  %retval.0 = phi ptr [ %add.ptr2, %if.then ], [ %add.ptr5, %if.end ]
; CHECK-TRANS:     ; MemInst: Address in ret
; CHECK-TRANS-NEXT:  ret ptr %retval.0
  ret ptr %retval.0
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
