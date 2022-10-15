; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                                                       \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>' \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt -S < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed                                                                    \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
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
; CHECK-OP: ; Classification: CCtor method
; CHECK-OP: ; Dump instructions needing update. Total = 10
define void @"ValueVectorOf<IC_Field*>::ValueVectorOf(ValueVectorOf<IC_Field*> const&)"(%class.ValueVectorOf* "intel_dtrans_func_index"="1" %this, %class.ValueVectorOf* "intel_dtrans_func_index"="2" %toCopy) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %fCallDestructor = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 0
; CHECK-OP-MOD:   %fCallDestructor = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 0
  %fCallDestructor = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 0
; CHECK-MOD:   %fCallDestructor2 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 0
; CHECK-OP-MOD:   %fCallDestructor2 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %toCopy, i64 0, i32 0
  %fCallDestructor2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 0
  %tmp = load i8, i8* %fCallDestructor2
  store i8 %tmp, i8* %fCallDestructor
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
; CHECK-OP-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
; CHECK-MOD:   %fCurCount3 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 1
; CHECK-OP-MOD:   %fCurCount3 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %toCopy, i64 0, i32 1
  %fCurCount3 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 1
  %tmp1 = load i32, i32* %fCurCount3
  store i32 %tmp1, i32* %fCurCount
; CHECK-MOD:  %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
; CHECK-OP-MOD:  %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
; CHECK-MOD:  %fMaxCount4 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 2
; CHECK-OP-MOD:  %fMaxCount4 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %toCopy, i64 0, i32 2
  %fMaxCount4 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 2
  %tmp2 = load i32, i32* %fMaxCount4
  store i32 %tmp2, i32* %fMaxCount
; CHECK-MOD:   %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
; CHECK-OP-MOD:   %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   store %__SOA_EL_class.ValueVectorOf* null, %__SOA_EL_class.ValueVectorOf** %fElemList
; CHECK-OP:     ; BasePtrInst: Nullify base pointer
; CHECK-OP-NEXT:  store ptr null, ptr %fElemList
; CHECK-OP-MOD:   store ptr null, ptr %fElemList
  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-OP-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
; CHECK-MOD:   %fMemoryManager5 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 4
; CHECK-OP-MOD:   %fMemoryManager5 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %toCopy, i64 0, i32 4
  %fMemoryManager5 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %toCopy, i64 0, i32 4
  %tmp3 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager5
  store %class.XMLMsgLoader* %tmp3, %class.XMLMsgLoader** %fMemoryManager
  %conv = zext i32 %tmp2 to i64
  %mul = shl nuw nsw i64 %conv, 3
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp3, i64 %mul)
; CHECK-OP:     ; BasePtrInst: Allocation call
; CHECK-OP-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp3, i64 %mul)
; CHECK-MOD:       %nsz = mul nuw i64 %mul, 2
; CHECK-MOD-NEXT:  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp3, i64 %nsz)
; CHECK-OP-MOD:       %nsz = mul nuw i64 %mul, 2
; CHECK-OP-MOD-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %tmp3, i64 %nsz)
  %call = tail call i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* %tmp3, i64 %mul)
; CHECK-MOD:   %tmp6 = bitcast %__SOA_EL_class.ValueVectorOf** %fElemList to i8**
; CHECK-OP-MOD:   %tmp6 = bitcast ptr %fElemList to ptr
  %tmp6 = bitcast %class.IC_Field*** %fElemList to i8**
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %tmp6
; CHECK-MOD:   store i8* %call, i8** %tmp6
; CHECK-OP:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-OP-NEXT:  store ptr %call, ptr %tmp6
; CHECK-OP-MOD:   store ptr %call, ptr %tmp6
  store i8* %call, i8** %tmp6
  %tmp7 = load i32, i32* %fMaxCount
  %conv11 = zext i32 %tmp7 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
; CHECK:     ; MemInst: Memset of elements
; CHECK-NEXT:  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul12, i1 false)
; CHECK-OP:     ; MemInst: Memset of elements
; CHECK-OP-NEXT:  tail call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %mul12, i1 false)
; CHECK-MOD:        %nsz1 = mul nuw i64 %mul12, 2
; CHECK-MOD-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %nsz1, i1 false)
; CHECK-OP-MOD:        %nsz1 = mul nuw i64 %mul12, 2
; CHECK-OP-MOD-NEXT:   tail call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %nsz1, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul12, i1 false)
  %tmp8 = load i32, i32* %fCurCount
  %cmp27 = icmp eq i32 %tmp8, 0
  br i1 %cmp27, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
; CHECK-MOD:  %fElemList14 = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %toCopy, i64 0, i32 3
; CHECK-OP-MOD:  %fElemList14 = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %toCopy, i64 0, i32 3
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
; CHECK-OP:     ; BasePtrInst: Load of base pointer
; CHECK-OP-NEXT:  %tmp10 = load ptr, ptr %fElemList14
; CHECK-OP-MOD:   %tmp10 = load ptr, ptr %fElemList14
  %tmp10 = load %class.IC_Field**, %class.IC_Field*** %fElemList14
; CHECK:     ; MemInstGEP: Element load
; CHECK-NEXT:  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp10, i64 %indvars.iv
; CHECK-OP:     ; MemInstGEP: Element load
; CHECK-OP-NEXT:  %arrayidx = getelementptr inbounds ptr, ptr %tmp10, i64 %indvars.iv
; CHECK-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp10, i64 %indvars.iv
  %arrayidx = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp10, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx, i64 0, i32 0
; CHECK-MOD-NEXT:   %0 = bitcast float** %elem to i64*
; CHECK-MOD-NEXT:   %tmp11 = bitcast %class.IC_Field** %elem3 to i64*
; CHECK-OP-MOD:   %arrayidx = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp10, i64 %indvars.iv
; CHECK-OP-MOD-NEXT:   %elem3 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 1
; CHECK-OP-MOD-NEXT:   %elem = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx, i64 0, i32 0
; CHECK-OP-MOD-NEXT:   %0 = bitcast ptr %elem to ptr
; CHECK-OP-MOD-NEXT:   %tmp11 = bitcast ptr %elem3 to ptr
  %tmp11 = bitcast %class.IC_Field** %arrayidx to i64*
; CHECK:     ; MemInst: Element load
; CHECK-NEXT:  %tmp12 = load i64, i64* %tmp11
; CHECK-MOD:        %copy = load i64, i64* %0
; CHECK-MOD-NEXT:   %tmp12 = load i64, i64* %tmp11
; CHECK-OP:     ; MemInst: Element load
; CHECK-OP-NEXT:  %tmp12 = load i64, ptr %tmp11
; CHECK-OP-MOD:        %copy = load i64, ptr %0
; CHECK-OP-MOD-NEXT:   %tmp12 = load i64, ptr %tmp11
  %tmp12 = load i64, i64* %tmp11
  %tmp13 = load %class.IC_Field**, %class.IC_Field*** %fElemList
; CHECK:     ; MemInstGEP: Element copy
; CHECK-NEXT:  %arrayidx17 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD:   %arrayidx17 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %tmp13, i64 %indvars.iv
; CHECK-OP:     ; MemInstGEP: Element copy
; CHECK-OP-NEXT:  %arrayidx17 = getelementptr inbounds ptr, ptr %tmp13, i64 %indvars.iv
; CHECK-OP-MOD:   %arrayidx17 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %tmp13, i64 %indvars.iv
  %arrayidx17 = getelementptr inbounds %class.IC_Field*, %class.IC_Field** %tmp13, i64 %indvars.iv
; CHECK-MOD-NEXT:   %elem4 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx17, i64 0, i32 1
; CHECK-MOD-NEXT:   %elem2 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, %__SOA_EL_class.ValueVectorOf* %arrayidx17, i64 0, i32 0
; CHECK-MOD-NEXT:   %1 = bitcast float** %elem2 to i64*
; CHECK-MOD-NEXT:   %tmp14 = bitcast %class.IC_Field** %elem4 to i64*
; CHECK-OP-MOD-NEXT:   %elem4 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx17, i64 0, i32 1
; CHECK-OP-MOD-NEXT:   %elem2 = getelementptr inbounds %__SOA_EL_class.ValueVectorOf, ptr %arrayidx17, i64 0, i32 0
; CHECK-OP-MOD-NEXT:   %1 = bitcast ptr %elem2 to ptr
; CHECK-OP-MOD-NEXT:   %tmp14 = bitcast ptr %elem4 to ptr
  %tmp14 = bitcast %class.IC_Field** %arrayidx17 to i64*
; CHECK:     ; MemInst: Element copy
; CHECK-NEXT:  store i64 %tmp12, i64* %tmp14
; CHECK-MOD:       store i64 %copy, i64* %1
; CHECK-MOD-NEXT:  store i64 %tmp12, i64* %tmp14
; CHECK-OP:     ; MemInst: Element copy
; CHECK-OP-NEXT:  store i64 %tmp12, ptr %tmp14
; CHECK-OP-MOD:       store i64 %copy, ptr %1
; CHECK-OP-MOD-NEXT:  store i64 %tmp12, ptr %tmp14
  store i64 %tmp12, i64* %tmp14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, %tmp9
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1"  i8* @_ZN10MemManager8allocateEl(%class.XMLMsgLoader* "intel_dtrans_func_index"="2" nocapture readnone %this, i64 %size) align 2 !intel.dtrans.func.type !11 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" i8* @malloc(i64) #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare !intel.dtrans.func.type !14 void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { argmemonly nofree nounwind willreturn writeonly }
attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

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
!9 = distinct !{!10, !10}
!10 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!11 = distinct !{!12, !8}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12}
!14 = distinct !{!12}
