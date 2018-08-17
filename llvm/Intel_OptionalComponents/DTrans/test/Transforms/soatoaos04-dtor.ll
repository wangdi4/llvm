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

; The following method should be classified as dtor.
; Instructions to transform are shown.
;  template <class TElem> ValueVectorOf<TElem>::~ValueVectorOf() {
;    if (fCallDestructor) {
;      for (int index = fMaxCount - 1; index >= 0; index--)
;        fElemList[index].~TElem();
;    }
;    fMemoryManager->deallocate(fElemList);
;  }
; CHECK: ; Classification: Dtor method
; CHECK: ; Dump instructions needing update. Total = 1
define void @"ValueVectorOf<IC_Field*>::~ValueVectorOf()"(%class.ValueVectorOf* %this) {
entry:
  %fMemoryManager = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 4
  %tmp = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager
  %fElemList2 = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 3
  %tmp1 = bitcast %class.IC_Field*** %fElemList2 to i8**
; CHECK:     ; BasePtrInst: Load of base pointer
; CHECK-NEXT:  %tmp2 = load i8*, i8** %tmp1
  %tmp2 = load i8*, i8** %tmp1
  %tmp3 = bitcast %class.XMLMsgLoader* %tmp to void (%class.XMLMsgLoader*, i8*)***
  %vtable = load void (%class.XMLMsgLoader*, i8*)**, void (%class.XMLMsgLoader*, i8*)*** %tmp3
  %vfn = getelementptr inbounds void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vtable, i64 3
  %tmp4 = load void (%class.XMLMsgLoader*, i8*)*, void (%class.XMLMsgLoader*, i8*)** %vfn
  tail call void %tmp4(%class.XMLMsgLoader* %tmp, i8* %tmp2)
  ret void
}
