; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                 \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                 \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                         \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                     \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                 \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                              \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                    \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                 \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                     \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                 \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

; The following method should be classified as realloc-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem>
;  void ValueVectorOf<TElem>::ensureExtraCapacity(const unsigned int length) {
;    unsigned int newMax = fCurCount + length;
;    if (newMax <= fMaxCount)
;      return;
;    // Avoid too many reallocations by expanding by a percentage
;    unsigned int minNewMax = (unsigned int)((double)fCurCount * 1.25);
;    if (newMax < minNewMax)
;      newMax = minNewMax;
;    TElem *newList = (TElem *)fMemoryManager->allocate(
;        newMax * sizeof(TElem)); // new TElem[newMax];
;    for (unsigned int index = 0; index < fCurCount; index++)
;      newList[index] = fElemList[index];
;    fMemoryManager->deallocate(fElemList); // delete [] fElemList;
;    fElemList = newList;
;    fMaxCount = newMax;
;  }
; CHECK: ; Classification: Realloc method
; CHECK: ; Dump instructions needing update. Total = 8

define void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(ptr "intel_dtrans_func_index"="1" %this, i32 %length) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 1
  %tmp = load i32, ptr %fCurCount, align 4
  %add = add i32 %tmp, 1
; CHECK-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 2
  %tmp1 = load i32, ptr %fMaxCount, align 4
  %cmp = icmp ugt i32 %add, %tmp1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 4
  %tmp2 = load ptr, ptr %fMemoryManager, align 8
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp2, i64 %mul8)
; CHECK-MOD:       %nsz = mul nuw i64 %mul8, 2
; CHECK-MOD-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp2, i64 %nsz)
  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp2, i64 %mul8)
; CHECK-MOD:    %tmp5 = bitcast ptr %call to ptr
  %tmp5 = bitcast ptr %call to ptr
  %tmp6 = load i32, ptr %fCurCount, align 4
  %cmp1031 = icmp eq i32 %tmp6, 0
; CHECK-MOD:   %.pre = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %.pre = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 3
  br i1 %cmp1031, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %tmp7 = zext i32 %tmp6 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %tmp8 = load ptr, ptr %fMemoryManager, align 8
; CHECK-MOD:   %tmp9 = bitcast ptr %.pre to ptr
  %tmp9 = bitcast ptr %.pre to ptr
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp10 = load ptr, ptr %tmp9
; CHECK-MOD:   %tmp10 = load ptr, ptr %tmp9
  %tmp10 = load ptr, ptr %tmp9, align 8
  tail call void @_ZN10MemManager10deallocateEPv(ptr %tmp8, ptr %tmp10)
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store ptr %call, ptr %tmp9
; CHECK-MOD:   store ptr %call, ptr %tmp9
  store ptr %call, ptr %tmp9, align 8
  store i32 %spec.select, ptr %fMaxCount, align 4
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp13 = load ptr, ptr %.pre
; CHECK-MOD:   %tmp13 = load ptr, ptr %.pre
  %tmp13 = load ptr, ptr %.pre, align 8
; CHECK:     ; MemInstGEP: Element load
; CHECK-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp13, i64 %indvars.iv
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp13, i64 %indvars.iv
  %arrayidx = getelementptr inbounds ptr, ptr %tmp13, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem2 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %0 = bitcast ptr %elem to ptr
; CHECK-MOD-NEXT:   %tmp14 = bitcast ptr %elem2 to ptr
  %tmp14 = bitcast ptr %arrayidx to ptr
; CHECK:     ; MemInst: Element load
; CHECK-NEXT:  %tmp15 = load i64, ptr %tmp14
; CHECK-MOD:        %copy = load i64, ptr %0
; CHECK-MOD-NEXT:   %tmp15 = load i64, ptr %tmp14
  %tmp15 = load i64, ptr %tmp14, align 8
; CHECK:     ; MemInstGEP: Element store to new mem
; CHECK-NEXT:  %arrayidx12 = getelementptr inbounds ptr, ptr %tmp5, i64 %indvars.iv
; CHECK-MOD:   %arrayidx12 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp5, i64 %indvars.iv
  %arrayidx12 = getelementptr inbounds ptr, ptr %tmp5, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx12, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx12, i64 0, i32 0
; CHECK-MOD:        %1 = bitcast ptr %elem1 to ptr
; CHECK-MOD-NEXT:   %tmp16 = bitcast ptr %elem3 to ptr
  %tmp16 = bitcast ptr %arrayidx12 to ptr
; CHECK:     ; MemInst: Element store to new mem
; CHECK-NEXT:  store i64 %tmp15, ptr %tmp16
; CHECK-MOD:        store i64 %copy, ptr %1
; CHECK-MOD-NEXT:   store i64 %tmp15, ptr %tmp16
  store i64 %tmp15, ptr %tmp16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp10 = icmp ult i64 %indvars.iv.next, %tmp7
  br i1 %cmp10, label %for.body, label %for.cond.cleanup

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(ptr nocapture "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !11 {
entry:
  tail call void @free(ptr %p)
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !13 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !15 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!0, !1, !5}

!0 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!1 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !2}
!2 = !{!3, i32 2}
!3 = !{!"F", i1 true, i32 0, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %class.ValueVectorOf zeroinitializer, i32 5, !6, !4, !4, !7, !8}
!6 = !{i8 0, i32 0}
!7 = !{%class.IC_Field zeroinitializer, i32 2}
!8 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!9 = distinct !{!10}
!10 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!11 = distinct !{!8, !12}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12, !8}
!14 = distinct !{!10}
!15 = distinct !{!10}
