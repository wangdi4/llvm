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
define void @"ValueVectorOf<IC_Field*>::ValueVectorOf(unsigned int, MemoryManager*, bool)"(%class.ValueVectorOf* %this, i32 %maxElems, %class.XMLMsgLoader* %manager, i1 zeroext %toCallDestructor) {
entry:
; CHECK-MOD:   %fCallDestructor = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 0
  %fCallDestructor = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 0
  store i8 0, i8* %fCallDestructor
; CHECK-MOD:   %fCurCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 1
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  store i32 0, i32* %fCurCount
; CHECK-MOD:   %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
  store i32 4, i32* %fMaxCount
; CHECK-MOD:   %fElemList = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 3
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   store %__SOA_EL_class.ValueVectorOf* null, %__SOA_EL_class.ValueVectorOf** %fElemList
  store %class.IC_Field** null, %class.IC_Field*** %fElemList
; CHECK-MOD:   %fMemoryManager = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 4
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  store %class.XMLMsgLoader* %manager, %class.XMLMsgLoader** %fMemoryManager
  %0 = bitcast %class.XMLMsgLoader* %manager to i8* (%class.XMLMsgLoader*, i64)***
  %vtable = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %0
  %vfn = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable, i64 2
  %1 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* %1(%class.XMLMsgLoader* %manager, i64 32)
; CHECK-MOD:   %call = tail call i8* %1(%class.XMLMsgLoader* %manager, i64 64)
  %call = tail call i8* %1(%class.XMLMsgLoader* %manager, i64 32)
; CHECK-MOD:   %bc = bitcast %__SOA_EL_class.ValueVectorOf** %fElemList to i8**
  %bc = bitcast %class.IC_Field*** %fElemList to i8**
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %bc
; CHECK-MOD:   store i8* %call, i8** %bc
  store i8* %call, i8** %bc
  %MaxCnt = load i32, i32* %fMaxCount
  %conv8 = zext i32 %MaxCnt to i64
  %mul9 = shl nuw nsw i64 %conv8, 3
; CHECK:     ; MemInst: Memset of elements
; CHECK-NEXT:  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul9, i1 false)
; CHECK-MOD:        %conv8 = zext i32 %MaxCnt to i64
; CHECK-MOD-NEXT:   %mul9 = shl nuw nsw i64 %conv8, 3
; CHECK-MOD-NEXT:   %nsz = mul nuw i64 %mul9, 2
; CHECK-MOD-NEXT:   tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %nsz, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul9, i1 false)
  ret void
}
