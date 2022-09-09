; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is qualified as InitRoutine
; for DynClone transformation even though user defined printf routine
; is called before "init" is called. printf routine calls another user
; defined __local_stdio_printf_options routine, which calls putchar
; library routine.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-LABEL:    Init Routine: init
; CHECK:   Verified InitRoutine ...
; CHECK-NOT:    InitRoutine failed...User routine called:   call void @printf()

; User defined routine without any side effects except calling library call.
define weak_odr i32 @__local_stdio_printf_options() {
  %k = tail call i32 @putchar(i32 10)
  ret i32 10
}

; User defined printf routine
define internal void @printf() {
  %o = call i32 @__local_stdio_printf_options()
  %i = add i32 %o, 5
  ret void
}

; "init" routine is qualified as InitRoutine even though user defined
; "printf" is called.
define i32 @main() {
entry:
  %i = alloca [80 x i8], align 16
  br i1 undef, label %B, label %C

B:
  %k = getelementptr inbounds [80 x i8], [80 x i8]* %i, i64 0, i64 0
  br label %D

C:
  %m = tail call i32 @putchar(i32 10)
  call void @printf()
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

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare i32 @putchar(i32)
