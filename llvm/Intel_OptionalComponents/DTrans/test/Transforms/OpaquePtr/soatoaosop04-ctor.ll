; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output                    \
; RUN: -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-array-methods>'    \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-arrays                                            \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed                                 \
; RUN:        -passes=soatoaosop-arrays-methods-transform                                                       \
; RUN:        -dtrans-soatoaosop-base-ptr-off=3 -dtrans-soatoaosop-mem-off=4                                    \
; RUN:        -dtrans-soatoaosop-ignore-funcs=_ZN10MemManager8allocateEl                                        \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, ptr, ptr }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { ptr, ptr }

; The following method should be classified as ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem>
;  ValueVectorOf<TElem>::ValueVectorOf(const unsigned int maxElems,
;                                      MemoryManager *const manager,
;                                      const bool toCallDestructor)
;      : fCallDestructor(toCallDestructor), fCurCount(0), fMaxCount(maxElems),
;        fElemList(0), fMemoryManager(manager) {
;    fElemList = (TElem *)fMemoryManager->allocate(
;        fMaxCount * sizeof(TElem));
;    memset(fElemList, 0, fMaxCount * sizeof(TElem));
;  }
; CHECK: ; Classification: Ctor method
; CHECK: ; Dump instructions needing update. Total = 4

define void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(ptr "intel_dtrans_func_index"="1" %this, i32 %maxElems, ptr "intel_dtrans_func_index"="2" %manager, i1 zeroext %toCallDestructor) !intel.dtrans.func.type !9 {
entry:
; CHECK-MOD:   %fCallDestructor = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 0
  %fCallDestructor = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 0
  store i8 0, ptr %fCallDestructor, align 1
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 1
  store i32 0, ptr %fCurCount, align 4
; CHECK-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 2
  store i32 4, ptr %fMaxCount, align 4
; CHECK-MOD:   %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:  store ptr null, ptr %fElemList
; CHECK-MOD:   store ptr null, ptr %fElemList
  store ptr null, ptr %fElemList, align 8
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, ptr %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, ptr %this, i64 0, i32 4
  store ptr %manager, ptr %fMemoryManager, align 8
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %manager, i64 32)
; CHECK-MOD:   %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %manager, i64 64)
  %call = tail call ptr @_ZN10MemManager8allocateEl(ptr %manager, i64 32)
; CHECK-MOD:   %bc = bitcast ptr %fElemList to ptr
  %bc = bitcast ptr %fElemList to ptr
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store ptr %call, ptr %bc
; CHECK-MOD:   store ptr %call, ptr %bc
  store ptr %call, ptr %bc, align 8
  %MaxCnt = load i32, ptr %fMaxCount, align 4
  %conv8 = zext i32 %MaxCnt to i64
  %mul9 = shl nuw nsw i64 %conv8, 3
; CHECK:     ; MemInst: Memset of elements
; CHECK-NEXT:  tail call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %mul9, i1 false)
; CHECK-MOD:        %conv8 = zext i32 %MaxCnt to i64
; CHECK-MOD-NEXT:   %mul9 = shl nuw nsw i64 %conv8, 3
; CHECK-MOD-NEXT:   %nsz = mul nuw i64 %mul9, 2
; CHECK-MOD-NEXT:   tail call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %nsz, i1 false)
  tail call void @llvm.memset.p0.i64(ptr %call, i8 0, i64 %mul9, i1 false)
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !11 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }

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
!9 = distinct !{!10, !8}
!10 = !{%class.ValueVectorOf zeroinitializer, i32 1}
!11 = distinct !{!12, !8}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12}
