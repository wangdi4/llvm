; RUN: opt < %s -whole-program-assume \
; RUN:                -disable-output \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4  \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays          \
; RUN:        -dtrans-malloc-functions=class.XMLMsgLoader,2  \
; RUN:        -dtrans-free-functions=class.XMLMsgLoader,3    \
; RUN:        2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

; The following method should be classified as ctor.
; Instructions to transform are shown.
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
  %fCallDestructor = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 0
  store i8 0, i8* %fCallDestructor
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  store i32 0, i32* %fCurCount
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
  store i32 4, i32* %fMaxCount
  %fElemList = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
; CHECK:     ; BasePtrInst: Nullify base pointer
; CHECK-NEXT:  store %class.IC_Field** null, %class.IC_Field*** %fElemList
  store %class.IC_Field** null, %class.IC_Field*** %fElemList
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  store %class.XMLMsgLoader* %manager, %class.XMLMsgLoader** %fMemoryManager
  %0 = bitcast %class.XMLMsgLoader* %manager to i8* (%class.XMLMsgLoader*, i64)***
  %vtable = load i8* (%class.XMLMsgLoader*, i64)**, i8* (%class.XMLMsgLoader*, i64)*** %0
  %vfn = getelementptr inbounds i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vtable, i64 2
  %1 = load i8* (%class.XMLMsgLoader*, i64)*, i8* (%class.XMLMsgLoader*, i64)** %vfn
; CHECK:     ; BasePtrInst: Allocation call
; CHECK-NEXT:  %call = tail call i8* %1(%class.XMLMsgLoader* %manager, i64 32)
  %call = tail call i8* %1(%class.XMLMsgLoader* %manager, i64 32)
  %2 = bitcast %class.IC_Field*** %fElemList to i8**
; CHECK:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-NEXT:  store i8* %call, i8** %2
  store i8* %call, i8** %2
  %3 = load i32, i32* %fMaxCount
  %conv8 = zext i32 %3 to i64
  %mul9 = shl nuw nsw i64 %conv8, 3
; CHECK:     ; MemInst: Memset of elements
; CHECK-NEXT:  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul9, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* %call, i8 0, i64 %mul9, i1 false)
  ret void
}
