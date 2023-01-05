; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation since it is called in loop.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-LABEL:    Init Routine: init
; CHECK:   InitRoutine failed...call in Loop

; "init" routine is not qualified as InitRoutine since it is called
; in loop.
define i32 @main() {
entry:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  call void @init(%struct.test.01* %j)
  %cmp = icmp eq i32 %iv, 20
  br i1 %cmp, label %exit, label %loop
exit:
  ret i32 0
}

; This routine is selected as InitRoutine.
define void @init(%struct.test.01* %tp1) {
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
declare dso_local noalias i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
