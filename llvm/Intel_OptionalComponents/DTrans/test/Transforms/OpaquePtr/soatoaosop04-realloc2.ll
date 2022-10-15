; CMPLRLLVM-36359: This test is same as soatoaosop04-realloc.ll except
; llvm.umax.i32 is used instead of SelectInst.
;
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                   \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager10deallocateEPv                                   \
; RUN:        | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }
; CHECK-OP-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

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
; CHECK-OP: ; Classification: Realloc method
; CHECK-OP: ; Dump instructions needing update. Total = 8
define void @"ValueVectorOf<IC_Field*>::ensureExtraCapacity(unsigned int)"(%class.ValueVectorOf* "intel_dtrans_func_index"="1" %this, i32 %length) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
; CHECK-OP-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  %add = add i32 %tmp, 1
; CHECK-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
; CHECK-OP-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
  %tmp1 = load i32, i32* %fMaxCount
  %cmp = icmp ugt i32 %add, %tmp1
  br i1 %cmp, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %conv = uitofp i32 %tmp to double
  %mul = fmul fast double %conv, 1.250000e+00
  %conv3 = fptoui double %mul to i32
  %spec.select = tail call i32 @llvm.umax.i32(i32 %add, i32 %conv3)
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-OP-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp2 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %conv7 = zext i32 %spec.select to i64
  %mul8 = shl nuw nsw i64 %conv7, 3
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp2, i64 %mul8)
; CHECK-MOD:       %nsz = mul nuw i64 %mul8, 2
; CHECK-MOD-NEXT:  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp2, i64 %nsz)
; CHECK-OP:     ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp2, i64 %mul8)
; CHECK-OP-MOD:       %nsz = mul nuw i64 %mul8, 2
; CHECK-OP-MOD-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp2, i64 %nsz)
  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp2, i64 %mul8)
; CHECK-MOD:       %tmp5 = bitcast i8* %call to %__SOA_EL_class.ValueVectorOf*
; CHECK-OP-MOD:    %tmp5 = bitcast ptr %call to ptr
  %tmp5 = bitcast i8* %call to %class.IC_Field**
  %tmp6 = load i32, i32* %fCurCount
  %cmp1031 = icmp eq i32 %tmp6, 0
; CHECK-MOD:   %.pre = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-OP-MOD:   %.pre = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %.pre = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
  br i1 %cmp1031, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %if.end
  %tmp7 = zext i32 %tmp6 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end
  %tmp8 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
; CHECK-MOD:   %tmp9 = bitcast %__SOA_EL_class.ValueVectorOf** %.pre to i8**
; CHECK-OP-MOD:   %tmp9 = bitcast ptr %.pre to ptr
  %tmp9 = bitcast %class.IC_Field*** %.pre to i8**
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp10 = load i8*, i8** %tmp9
; CHECK-MOD:   %tmp10 = load i8*, i8** %tmp9
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp10 = load ptr, ptr %tmp9
; CHECK-OP-MOD:   %tmp10 = load ptr, ptr %tmp9
  %tmp10 = load i8*, i8** %tmp9
  tail call void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* %tmp8, i8* %tmp10)
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %tmp9
; CHECK-MOD:   store i8* %call, i8** %tmp9
; CHECK-OP:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-NEXT:  store ptr %call, ptr %tmp9
; CHECK-OP-MOD:   store ptr %call, ptr %tmp9
  store i8* %call, i8** %tmp9
  store i32 %spec.select, i32* %fMaxCount
  br label %cleanup

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %.pre
; CHECK-MOD:   %tmp13 = load %__SOA_EL_class.ValueVectorOf*, %__SOA_EL_class.ValueVectorOf** %.pre
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp13 = load ptr, ptr %.pre
; CHECK-OP-MOD:   %tmp13 = load ptr, ptr %.pre
  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %.pre
; CHECK:     ; MemInstGEP: Element load
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-OP:     ; MemInstGEP: Element load
; CHECK-OP-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp13, i64 %indvars.iv
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp13, i64 %indvars.iv
; CHECK-OP-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp13, i64 %indvars.iv
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem2 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %0 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp14 = bitcast %class.IC_Field** %elem2 to i64*
; CHECK-OP-MOD-NEXT:   %elem2 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 0
; CHECK-OP-MOD-NEXT:   %0 = bitcast ptr %elem to ptr
; CHECK-OP-MOD-NEXT:   %tmp14 = bitcast ptr %elem2 to ptr
  %tmp14 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element load
; CHECK-NEXT:  %tmp15 = load i64, i64* %tmp14
; CHECK-MOD:        %copy = load i64, i64* %0
; CHECK-MOD-NEXT:   %tmp15 = load i64, i64* %tmp14
; CHECK-OP:     ; MemInst: Element load
; CHECK-OP-NEXT:  %tmp15 = load i64, ptr %tmp14
; CHECK-OP-MOD:        %copy = load i64, ptr %0
; CHECK-OP-MOD-NEXT:   %tmp15 = load i64, ptr %tmp14
  %tmp15 = load i64, i64* %tmp14
; CHECK:     ; MemInstGEP: Element store to new mem
; CHECK-NEXT:  %arrayidx12 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp5, i64 %indvars.iv
; CHECK-OP:     ; MemInstGEP: Element store to new mem
; CHECK-OP-NEXT:  %arrayidx12 = getelementptr inbounds ptr, ptr %tmp5, i64 %indvars.iv
; CHECK-MOD:   %arrayidx12 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp5, i64 %indvars.iv
; CHECK-OP-MOD:   %arrayidx12 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp5, i64 %indvars.iv
  %arrayidx12 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp5, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx12, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx12, i64 0, i32 0
; CHECK-MOD:        %1 = bitcast float** %elem1 to i64*
; CHECK-MOD-NEXT:   %tmp16 = bitcast %class.IC_Field** %elem3 to i64*
; CHECK-OP-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx12, i64 0, i32 1
; CHECK-OP-MOD-NEXT:   %elem1 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx12, i64 0, i32 0
; CHECK-OP-MOD:        %1 = bitcast ptr %elem1 to ptr
; CHECK-OP-MOD-NEXT:   %tmp16 = bitcast ptr %elem3 to ptr
  %tmp16 = bitcast %class.IC_Field** %arrayidx12 to i64*
; CHECK:     ; MemInst: Element store to new mem
; CHECK-NEXT:  store i64 %tmp15, i64* %tmp16
; CHECK-MOD:        store i64 %copy, i64* %1
; CHECK-MOD-NEXT:   store i64 %tmp15, i64* %tmp16
; CHECK-OP:     ; MemInst: Element store to new mem
; CHECK-OP-NEXT:  store i64 %tmp15, ptr %tmp16
; CHECK-OP-MOD:        store i64 %copy, ptr %1
; CHECK-OP-MOD-NEXT:   store i64 %tmp15, ptr %tmp16
  store i64 %tmp15, i64* %tmp16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp10 = icmp ult i64 %indvars.iv.next, %tmp7
  br i1 %cmp10, label %for.body, label %for.cond.cleanup

cleanup:                                          ; preds = %for.cond.cleanup, %entry
  ret void
}

define dso_local void @_ZN10MemManager10deallocateEPv(%class.XMLMsgLoader* nocapture "intel_dtrans_func_index"="1" %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !11 {
entry:
  tail call void @free(i8* %p)
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !13 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !15 void @free(i8* "intel_dtrans_func_index"="1") #1
declare i32 @llvm.umax.i32(i32, i32)

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
