; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that no field is qualified for DynClone transformation
; due to memset.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK:  Invalid...written by memset
; CHECK:  No Candidate is qualified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning,
; none of the fields is selected as final candidates due to memset.
; Fields are written using memset with non-zero value.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

define i32 @main() {
entry:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  %i = bitcast i8* %call1 to i32*
  call void @llvm.memset.p0i8.i64(i8* %call1, i8 1, i64 56, i1 false)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
