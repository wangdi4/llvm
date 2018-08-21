; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                    \
; RUN:        -passes=soatoaos-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2                                                 \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3                                                   \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
; CHECK-MOD: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }

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
define void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%class.ValueVectorOf* %this, i32 %length) {
entry:
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  %add = add i32 %tmp, 1
; CHECK-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
  %tmp1 = load i32, i32* %fMaxCount
  %cmp = icmp ugt i32 %add, %tmp1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %cmp4 = icmp ult i32 %add, %conv3
  %spec.select = select i1 %cmp4, i32 %conv3, i32 %add
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp2 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
  %tmp3 = bitcast %class.XMLMsgLoader* %tmp2 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp3
  %vfn = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable, i64 2
  %tmp4 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* %tmp4(%class.XMLMsgLoader* %tmp2, i64 %mul8)
; CHECK-MOD:       %nsz = mul nuw i64 %mul8, 2
; CHECK-MOD-NEXT:  %call = tail call i8* %tmp4(%class.XMLMsgLoader* %tmp2, i64 %nsz)
  %call = tail call i8* %tmp4(%class.XMLMsgLoader* %tmp2, i64 %mul8)
; CHECK-MOD:       %tmp5 = bitcast i8* %call to %__SOA_EL_class.ValueVectorOf*
  %tmp5 = bitcast i8* %call to %class.IC_Field**
  %tmp6 = load i32, i32* %fCurCount
  %cmp1031 = icmp eq i32 %tmp6, 0
; CHECK-MOD:   %.pre = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
  %.pre = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
  br i1 %cmp1031, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %tmp7 = zext i32 %tmp6 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %tmp8 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-MOD:   %tmp9 = bitcast %__SOA_EL_class.ValueVectorOf** %.pre to i8**
  %tmp9 = bitcast %class.IC_Field*** %.pre to i8**
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp10 = load i8*, i8** %tmp9
; CHECK-MOD:   %tmp10 = load i8*, i8** %tmp9
  %tmp10 = load i8*, i8** %tmp9
  %tmp11 = bitcast %class.XMLMsgLoader* %tmp8 to void (%class.XMLMsgLoader*, i8*)***
  %vtable15 = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp11
  %vfn16 = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vtable15, i64 3
  %tmp12 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vfn16
  tail call void %tmp12(%class.XMLMsgLoader* %tmp8, i8* %tmp10)
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %tmp9
; CHECK-MOD:   store i8* %call, i8** %tmp9
  store i8* %call, i8** %tmp9
  store i32 %spec.select, i32* %fMaxCount
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %.pre
; CHECK-MOD:   %tmp13 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %.pre
  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %.pre
; CHECK:     ; MemInstGEP: Element load
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp13, i64 %indvars.iv
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem4 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %copy1 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp14 = bitcast %class.IC_Field** %elem4 to i64*
  %tmp14 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element load
; CHECK-NEXT:  %tmp15 = load i64, i64* %tmp14
; CHECK-MOD:        %copy = load i64, i64* %copy1
; CHECK-MOD-NEXT:   %tmp15 = load i64, i64* %tmp14
  %tmp15 = load i64, i64* %tmp14
; CHECK:     ; MemInstGEP: Element store to new mem
; CHECK-NEXT:  %arrayidx12 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp5, i64 %indvars.iv
; CHECK-MOD:   %arrayidx12 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp5, i64 %indvars.iv
  %arrayidx12 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp5, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem5 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx12, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx12, i64 0, i32 0
; CHECK-MOD:        %copy2 = bitcast float** %elem3 to i64*
; CHECK-MOD-NEXT:   %tmp16 = bitcast %class.IC_Field** %elem5 to i64*
  %tmp16 = bitcast %class.IC_Field** %arrayidx12 to i64*
; CHECK:     ; MemInst: Element store to new mem
; CHECK-NEXT:  store i64 %tmp15, i64* %tmp16
; CHECK-MOD:        store i64 %copy, i64* %copy2
; CHECK-MOD-NEXT:   store i64 %tmp15, i64* %tmp16
  store i64 %tmp15, i64* %tmp16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp10 = icmp ult i64 %indvars.iv.next, %tmp7
  br i1 %cmp10, label %for.body, label %for.cond.cleanup

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}
