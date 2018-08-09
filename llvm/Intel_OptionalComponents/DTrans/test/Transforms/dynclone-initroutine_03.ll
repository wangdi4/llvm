; REQUIRES: asserts
; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation due to indirect call.

;  RUN: opt < %s -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK:    Indirect Call ... Skip DynClone
; CHECK-NOT:    Init Routine: init

; Indirect call %t0() and direct call "proc2()" are two legality issues
; here to qualify  "init" routine as InitRoutine. These two calls are called
; before "init()". It will report "Indirect call" is the reason for not
; qualified because analysis starts from entry block.
define i32 @main() {
entry:
  %i = alloca i64, align 8
  %t0 = bitcast i64* %i to i32 ()*
  %call = call i32 %t0()
  br label %D

D:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  call void @proc2()
  %j = bitcast i8* %call1 to %struct.test.01*
  call void @init(%struct.test.01* %j)
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

define void @proc2() {
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
