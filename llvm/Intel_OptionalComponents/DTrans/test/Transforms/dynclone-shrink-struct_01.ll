; This test verifies that shrinked type is created correctly for DynClone
; transformation.

;  RUN: opt < %s -S -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are qualified as final candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; CHECK: %struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }
; Shrunken type is marked as packed.
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i32, i16 }>

; This routine is selected as InitRoutine. Runtime checks are generated
; in this routine.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
