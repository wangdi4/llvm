; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -dtrans-annotator-cleaner 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-annotator-cleaner 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify that !dtrans-soatoaos metadata tags get removed from
; function definitions.

%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

; This is an example of the output produced by the SOA-to-AOS transformation to
; mark a function as having been transformed.
define internal void @"Arr<int*>::Arr(int).12"(%__SOADT_AR_struct.Arr* %this, i32 %c) !dtrans-soatoaos !0 {
; CHECK: define internal void @"Arr<int*>::Arr(int).12"(%__SOADT_AR_struct.Arr* %this, i32 %c) {
  ret void
}

!0 = !{%__SOADT_EL_class.F* null}
