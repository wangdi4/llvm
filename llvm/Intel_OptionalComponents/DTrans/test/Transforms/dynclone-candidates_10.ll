; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test is same as dynclone-candidates_09.ll except one additional call
; "call void @proc3(i64 %bc1, %struct.test.01* %tp3)", which passes value of
; field_0 as first argument, in "proc2" routine. Since it is not safe to
; assign value of field_0 (non-candidate field) to field_1 of the struct,
; field_1 is not qualified as candidate field.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning and
; dependency analysis, none of the fields is qualified as final candidate
; for DynClone.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK: Pruning in routine: proc3
; CHECK: Checking i64 %cost argument assignment to struct: struct.test.01 Index: 1
; CHECK: Checking callsite:   call void @proc3(i64 %bc1, %struct.test.01* %tp3)
; CHECK: Argument:   %bc1 = zext i32 %ld5 to i64
; CHECK: Invalid...More than one init routine: struct: struct.test.01 Index: 1

; CHECK: No Candidate is qualified after Dep...Skip DynClone

; In this routine, fields 1, 6 and 7 are assigned with unknown values.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %g1, i64* %F6, align 8
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 7
  store i64 %g1, i64* %F7, align 8
  ret void
}

; field_1 = cost;
define void @proc3(i64 %cost, %struct.test.01* %tp4) {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 1
  store i64 %cost, i64* %F1, align 8
  ret void
}

; field_6 = 100;
; field_6 = field_1;
; call proc3(field_1, tp2);
; call proc3(30, tp2);
define void @proc1(%struct.test.01* %tp2) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 100, i64* %F6, align 8
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %ld1 = load i64, i64* %F1
  store i64 %ld1, i64* %F6, align 8
  call void @proc3(i64 %ld1, %struct.test.01* %tp2);
  call void @proc3(i64 30, %struct.test.01* %tp2);
  ret void
}

; field_1 = 99999;
; field_1 = field_6;
; call proc3(field_6, tp3);
; field_1 = field_7;
; call proc3(field_7, tp3);
; call proc3(field_0, tp3);
define void @proc2(%struct.test.01* %tp3) {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 1
  %ld3 = load i64, i64* %F1
  store i64 99999, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 6
  %ld2 = load i64, i64* %F6
  store i64 %ld2, i64* %F1, align 8
  call void @proc3(i64 %ld2, %struct.test.01* %tp3);
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 7
  %ld4 = load i64, i64* %F7
  store i64 %ld4, i64* %F1, align 8
  %F0 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 0
  %ld5 = load i32, i32* %F0
  %bc1 = zext i32 %ld5 to i64
  call void @proc3(i64 %bc1, %struct.test.01* %tp3);
  ret void
}

; Reset struct.test.01 using memset in this routine.
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
