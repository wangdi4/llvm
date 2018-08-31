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

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

; The following method should be classified as copy-ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem>
;  ValueVectorOf<TElem>::ValueVectorOf(const ValueVectorOf<TElem> &toCopy)
;      : XMemory(toCopy), fCallDestructor(toCopy.fCallDestructor),
;        fCurCount(toCopy.fCurCount), fMaxCount(toCopy.fMaxCount), fElemList(0),
;        fMemoryManager(toCopy.fMemoryManager) {
;    fElemList = (TElem *)fMemoryManager->allocate(
;        fMaxCount * sizeof(TElem)); // new TElem[fMaxCount];
;    memset(fElemList, 0, fMaxCount * sizeof(TElem));
;    for (unsigned int index = 0; index < fCurCount; index++)
;      fElemList[index] = toCopy.fElemList[index];
;  }
; CHECK: ; Classification: CCtor method
; CHECK: ; Dump instructions needing update. Total = 10
define void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf* %this, %class.ValueVectorOf* %toCopy) {
entry:
; CHECK-MOD:   %fCallDestructor = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 0
  %fCallDestructor = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 0
; CHECK-MOD:   %fCallDestructor2 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 0
  %fCallDestructor2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 0
  %tmp = load i8, i8* %fCallDestructor2
  store i8 %tmp, i8* %fCallDestructor
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
; CHECK-MOD:   %fCurCount3 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 1
  %fCurCount3 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 1
  %tmp1 = load i32, i32* %fCurCount3
  store i32 %tmp1, i32* %fCurCount
; CHECK-MOD:  %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
; CHECK-MOD:  %fMaxCount4 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 2
  %fMaxCount4 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 2
  %tmp2 = load i32, i32* %fMaxCount4
  store i32 %tmp2, i32* %fMaxCount
; CHECK-MOD:   %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   store %__SOA_EL_class.ValueVectorOf* null, %__SOA_EL_class.ValueVectorOf** %fElemList
  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-MOD:   %fMemoryManager5 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 4
  %fMemoryManager5 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 4
  %tmp3 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager5
  store %class.XMLMsgLoader* %tmp3, %class.XMLMsgLoader** %fMemoryManager
  %conv = zext i32 %tmp2 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %tmp4 = bitcast %class.XMLMsgLoader* %tmp3 to i8* (%class.XMLMsgLoader*, i64)***
  %vtable = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %tmp4
  %vfn = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable, i64 2
  %tmp5 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* %tmp5(%class.XMLMsgLoader* %tmp3, i64 %mul)
; CHECK-MOD:       %nsz = mul nuw i64 %mul, 2
; CHECK-MOD-NEXT:  %call = tail call i8* %tmp5(%class.XMLMsgLoader* %tmp3, i64 %nsz)
  %call = tail call i8* %tmp5(%class.XMLMsgLoader* %tmp3, i64 %mul)
; CHECK-MOD:   %tmp6 = bitcast %__SOA_EL_class.ValueVectorOf** %fElemList to i8**
  %tmp6 = bitcast %class.IC_Field*** %fElemList to i8**
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %tmp6
; CHECK-MOD:   store i8* %call, i8** %tmp6
  store i8* %call, i8** %tmp6
  %tmp7 = load i32, i32* %fMaxCount
  %conv11 = zext i32 %tmp7 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
; CHECK:     ; MemInst: Memset of elements
; CHECK-NEXT:  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul12, i1 false)
; CHECK-MOD:        %nsz1 = mul nuw i64 %mul12, 2
; CHECK-MOD-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %nsz1, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul12, i1 false)
  %tmp8 = load i32, i32* %fCurCount
  %cmp27 = icmp eq i32 %tmp8, 0
  br i1 %cmp27, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
; CHECK-MOD:  %fElemList14 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 3
  %fElemList14 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 3
  %tmp9 = zext i32 %tmp8 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp10 = load %class.IC_Field**, %class.IC_Field*** %fElemList14
; CHECK-MOD:   %tmp10 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %fElemList14
  %tmp10 = load %class.IC_Field**, %class.IC_Field*** %fElemList14
; CHECK:     ; MemInstGEP: Element load
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp10, i64 %indvars.iv
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp10, i64 %indvars.iv
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp10, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem5 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %copy2 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp11 = bitcast %class.IC_Field** %elem5 to i64*
  %tmp11 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element load
; CHECK-NEXT:  %tmp12 = load i64, i64* %tmp11
; CHECK-MOD:        %copy = load i64, i64* %copy2
; CHECK-MOD-NEXT:   %tmp12 = load i64, i64* %tmp11
  %tmp12 = load i64, i64* %tmp11
  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %fElemList
; CHECK:     ; MemInstGEP: Element copy
; CHECK-NEXT:  %arrayidx17 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD:   %arrayidx17 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp13, i64 %indvars.iv
  %arrayidx17 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem6 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx17, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem4 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx17, i64 0, i32 0
; CHECK-MOD-NEXT:   %copy3 = bitcast float** %elem4 to i64*
; CHECK-MOD-NEXT:   %tmp14 = bitcast %class.IC_Field** %elem6 to i64*
  %tmp14 = bitcast %class.IC_Field** %arrayidx17 to i64*
; CHECK:     ; MemInst: Element copy
; CHECK-NEXT:  store i64 %tmp12, i64* %tmp14
; CHECK-MOD:       store i64 %copy, i64* %copy3
; CHECK-MOD-NEXT:  store i64 %tmp12, i64* %tmp14
  store i64 %tmp12, i64* %tmp14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, %tmp9
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}
