; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation because unexpected library call (i.e free)
; is called in init routine.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are candidates to shrunk.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; CHECK:  Calls in InitRoutine Failed

; This routine is not qualified as InitRoutine because unexpected library
; call "free" is called.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  tail call void @free(i8* null)
  ret void
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 0, i64* %F6, align 8
  ret void
}

define i32 @main() {
  call void @init()
  call void @proc1()
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local void @free(i8*)
