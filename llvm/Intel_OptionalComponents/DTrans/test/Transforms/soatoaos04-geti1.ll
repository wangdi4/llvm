; RUN: opt < %s -whole-program-assume -disable-output                                                       \
; RUN: -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)' \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        -debug-only=dtrans-soatoaos,dtrans-soatoaos-arrays                                            \
; RUN:        2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                    \
; RUN:        -passes=soatoaos-arrays-methods-transform                                                     \
; RUN:        -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=4                                    \
; RUN:        | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.ValueVectorOf = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_class.ValueVectorOf = type { i8, i32, i32, %__SOA_EL_class.ValueVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.ValueVectorOf = type { float*, %class.IC_Field* }

; The following method should be classified as get-integer-field-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;  template <class TElem> unsigned int ValueVectorOf<TElem>::curCapacity() const {
;    return fMaxCount;
;  }
; CHECK:; Classification: Get integer field method
; CHECK:; Dump instructions needing update. Total = 0
define i32 @"ValueVectorOf<IC_Field*>::curCapacity() const"(%class.ValueVectorOf* %this) {
entry:
; CHECK-MOD:  %fMaxCount = getelementptr inbounds %__SOA_class.ValueVectorOf, %__SOA_class.ValueVectorOf* %this, i64 0, i32 2
  %fMaxCount = getelementptr inbounds %class.ValueVectorOf, %class.ValueVectorOf* %this, i64 0, i32 2
  %tmp = load i32, i32* %fMaxCount
  ret i32 %tmp
}
