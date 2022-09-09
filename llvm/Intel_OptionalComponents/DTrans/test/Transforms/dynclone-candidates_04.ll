; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that possible candidate fields are pruned with
; compile-time analysis for DynClone transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK-LABEL:  Final Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK-NOT:    struct: struct.test.01 Index: 7

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning,
; only 1 and 6 fields are qualified as final candidates for DynClone.
; Field 7 is rejected as final candidate because a large constant is
; stored into this field.
; Fields 1 and 6: Unknown values are assigned to these fields only in
; one routine "init". In other places, fields are written as
;  1. small values are assigned or
;  2. assigned with one of the final candidate fields or
;  3. reset using memset
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; In this routine, fields 1 and 6 are assigned with unknown values.
; Field 7 is assigned with large value.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %g1, i64* %F6, align 8
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 7
  store i64 -1, i64* %F7, align 8
  ret void
}

; field_6 = 100;
; field_6 = field_1;
define void @proc1(%struct.test.01* %tp2) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 100, i64* %F6, align 8
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %ld1 = load i64, i64* %F1
  store i64 %ld1, i64* %F6, align 8
  ret void
}

; field_1 = 99999;
; field_1 = field_6;
; field_1 = field_1;
define void @proc2(%struct.test.01* %tp3) {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 1
  %ld3 = load i64, i64* %F1
  store i64 99999, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 6
  %ld2 = load i64, i64* %F6
  store i64 %ld2, i64* %F1, align 8
  store i64 %ld3, i64* %F1, align 8
  ret void
}

define i32 @main() {
entry:
  call void @init();
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  %i = bitcast i8* %call1 to i32*
  call void @proc1(%struct.test.01* %j);
  call void @proc2(%struct.test.01* %j);
  call void @llvm.memset.p0i8.i64(i8* %call1, i8 0, i64 56, i1 false)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
