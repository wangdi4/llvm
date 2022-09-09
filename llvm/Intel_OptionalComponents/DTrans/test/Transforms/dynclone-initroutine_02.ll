; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation since user-defined routine is called before
; "init" is called.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-LABEL:   Init Routine: init
; CHECK:    InitRoutine failed...User routine called:   call void @proc2()

; "init" routine is not qualified as InitRoutine due to "proc2", which
; is user defined routine, call. "proc2" is called before "init".
define i32 @main() {
entry:
  %i = alloca [80 x i8], align 16
  br i1 undef, label %B, label %C

B:
  %k = getelementptr inbounds [80 x i8], [80 x i8]* %i, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %k)
  br label %D

C:
  %m = tail call i32 @putchar(i32 10)
  call void @proc2()
  br label %D

D:
  call void @init()
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

; This routine is used to make verification failed for InitRoutine.
define void @proc2() {
  ret void
}
; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare i32 @putchar(i32)
