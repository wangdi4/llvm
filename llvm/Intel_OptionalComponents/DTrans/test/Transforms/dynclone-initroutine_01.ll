; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is qualified as InitRoutine
; for DynClone transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:    Init Routine: init
; CHECK:   Verified InitRoutine ...

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are qualified as final candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; This struct is not selected as candidate due to safety issues.
%struct.test.02 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; Call to "init" routine is qualified as InitRoutine for DynClone. It proves
; that all instructions till "init()" don't cause any legality issues to
; enable DynClone.
define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 50)
  %i = bitcast i8* %call to %struct.test.02*
  %F1_2 = getelementptr %struct.test.02, %struct.test.02* %i, i32 0, i32 1
  br i1 undef, label %A, label %B

A:
  %k = tail call i32 @putchar(i32 10)
  br i1 undef, label %C, label %D

C:
  %F2_2 = getelementptr %struct.test.02, %struct.test.02* %i, i32 0, i32 2
  %ld2 = load i32, i32* %F2_2, align 8
  br label %F

D:
  %F3_2 = getelementptr %struct.test.02, %struct.test.02* %i, i32 0, i32 3
  %ld3 = load i32, i32* %F3_2, align 8
  br label %F

F:
  call void @init();
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  call void @proc1(%struct.test.01* %j);
  br label %E

B:
  ret i32 1

E:
  ret i32 0
}

; This routine is selected as InitRoutine.
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

; This routine just accesses candidate field.
define void @proc1(%struct.test.01* %tp2) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare i32 @putchar(i32)
