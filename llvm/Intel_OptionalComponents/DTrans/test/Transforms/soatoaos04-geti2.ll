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

; The following method should be classified as get-integer-field-like method.
; Instructions to transform are shown.
;  template <class TElem> unsigned int ValueVectorOf<TElem>::size() const {
;    return fCurCount;
;  }
; CHECK:; Classification: Get integer field method
; CHECK:; Dump instructions needing update. Total = 0
define i32 @"ValueVectorOf<IC_Field*>::size() const"(%class.ValueVectorOf* %this) {
entry:
  %fCurCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 1
  %tmp = load i32, i32* %fCurCount
  ret i32 %tmp
}
