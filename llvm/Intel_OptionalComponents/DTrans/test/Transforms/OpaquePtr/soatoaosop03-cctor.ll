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
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                                       \
; RUN:    -passes=soatoaosop-arrays-methods-transform                                                           \
; RUN:    -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=0                                        \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }
; CHECK-MOD-DAG: %__SOA_struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr.0 = type { ptr, ptr }

; The following method should be classified as copy-ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   Arr(const Arr &A) {
;     mem = A.mem;
;     capacilty = A.capacilty;
;     size = A.size;
;     base = (S *)mem->allocate(size + capacilty * sizeof(S));
;     for (int i = 0; i < size; ++i)
;       base[size + i] = A.base[i];
;   }
; CHECK:      Checking array's method _ZN3ArrIPvEC2ERKS1_
; CHECK-NEXT: Classification: CCtor method
; CHECK-TRANS:; Dump instructions needing update. Total = 8

define void @_ZN3ArrIPvEC2ERKS1_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %A) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:  %mem = getelementptr inbounds %__SOA_struct.Arr.0, ptr %A, i32 0, i32 0
  %mem = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 0
  %tmp = load ptr, ptr %mem, align 8
; CHECK-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 0
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 0
  store ptr %tmp, ptr %mem2, align 8
; CHECK-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr.0, ptr %A, i32 0, i32 1
  %capacilty = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 1
  %tmp1 = load i32, ptr %capacilty, align 8
; CHECK-MOD:  %capacilty3 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 1
  %capacilty3 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  store i32 %tmp1, ptr %capacilty3, align 8
; CHECK-MOD:  %size = getelementptr inbounds %__SOA_struct.Arr.0, ptr %A, i32 0, i32 4
  %size = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 4
  %tmp2 = load i32, ptr %size, align 8
; CHECK-MOD:  %size4 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 4
  %size4 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  store i32 %tmp2, ptr %size4, align 8
; CHECK-MOD:  %mem5 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 0
  %mem5 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 0
  %tmp3 = load ptr, ptr %mem5, align 8
; CHECK-MOD:  %size6 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 4
  %size6 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp4 = load i32, ptr %size6, align 8
  %conv = sext i32 %tmp4 to i64
; CHECK-MOD:  %capacilty7 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 1
  %capacilty7 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  %tmp5 = load i32, ptr %capacilty7, align 8
  %conv8 = sext i32 %tmp5 to i64
  %mul = mul i64 %conv8, 8
  %add = add i64 %conv, %mul
; CHECK-TRANS:      ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:   %call = call ptr @malloc(i64 %add)
; CHECK-MOD:  %nsz = mul nuw i64 %add, 2
; CHECK-MOD-NEXT:  %call = call ptr @malloc(i64 %nsz)
  %call = call ptr @malloc(i64 %add)
; CHECK-MOD:  %tmp8 = bitcast ptr %call to ptr
  %tmp8 = bitcast ptr %call to ptr
; CHECK-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:   store ptr %tmp8, ptr %base, align 8
; CHECK-MOD:  store ptr %tmp8, ptr %base, align 8
  store ptr %tmp8, ptr %base, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
; CHECK-MOD:  %size10 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 4
  %size10 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp9 = load i32, ptr %size10, align 8
  %cmp = icmp slt i32 %i.0, %tmp9
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
; CHECK-MOD:  %base11 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %A, i32 0, i32 3
  %base11 = getelementptr inbounds %struct.Arr.0, ptr %A, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp10 = load ptr, ptr %base11, align 8
; CHECK-MOD:  %tmp10 = load ptr, ptr %base11, align 8
  %tmp10 = load ptr, ptr %base11, align 8
  %idxprom = sext i32 %i.0 to i64
; CHECK-TRANS:      ; MemInstGEP: Element load
; CHECK-TRANS-NEXT:   %arrayidx = getelementptr inbounds ptr, ptr %tmp10, i64 %idxprom
; CHECK-MOD:          %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp10, i64 %idxprom
  %arrayidx = getelementptr inbounds ptr, ptr %tmp10, i64 %idxprom
; CHECK-MOD-NEXT:     %elem2 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:     %elem = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element load
; CHECK-TRANS-NEXT:   %tmp11 = load ptr, ptr %arrayidx, align 8
; CHECK-MOD-NEXT:     %copy = load ptr, ptr %elem, align 8
; CHECK-MOD-NEXT:     %tmp11 = load ptr, ptr %elem2, align 8
  %tmp11 = load ptr, ptr %arrayidx, align 8
; CHECK-MOD:  %base12 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 3
  %base12 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp12 = load ptr, ptr %base12, align 8
; CHECK-MOD:  %tmp12 = load ptr, ptr %base12, align 8
  %tmp12 = load ptr, ptr %base12, align 8
; CHECK-MOD:  %size13 = getelementptr inbounds %__SOA_struct.Arr.0, ptr %this, i32 0, i32 4
  %size13 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  %tmp13 = load i32, ptr %size13, align 8
  %add14 = add nsw i32 %tmp13, %i.0
  %idxprom15 = sext i32 %add14 to i64
; CHECK-TRANS:      ; MemInstGEP: Element copy
; CHECK-TRANS-NEXT:   %arrayidx16 = getelementptr inbounds ptr, ptr %tmp12, i64 %idxprom15
; CHECK-MOD:       %arrayidx16 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %tmp12, i64 %idxprom15
  %arrayidx16 = getelementptr inbounds ptr, ptr %tmp12, i64 %idxprom15
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx16, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_struct.Arr.0, ptr %arrayidx16, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element copy
; CHECK-TRANS-NEXT:   store ptr %tmp11, ptr %arrayidx16, align 8
; CHECK-MOD-NEXT:   store ptr %copy, ptr %elem1, align 8
; CHECK-MOD-NEXT:   store ptr %tmp11, ptr %elem3, align 8
  store ptr %tmp11, ptr %arrayidx16, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

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
!9 = distinct !{!10, !10}
!10 = !{%struct.Arr.0 zeroinitializer, i32 1}
!11 = distinct !{!12}
!12 = !{i8 0, i32 1}
