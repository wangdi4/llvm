; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies only i64 type fields are selected as candidates for
; DynClone transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK: DynCloning Transformation
; CHECK:    Possible Candidate fields:
; CHECK:    struct: struct.test.01    Index: 1
; CHECK-NOT:    struct: struct.test.01    Index: 5
; CHECK:    struct: struct.test.01    Index: 6

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Only 1 and 6 fields are selected for candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

define i32 @main() {
entry:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  %i = bitcast i8* %call1 to i32*
  store i32 10, i32* %i, align 8
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
