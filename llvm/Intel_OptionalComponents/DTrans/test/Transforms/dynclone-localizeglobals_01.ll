; UNSUPPORTED: enable-opaque-pointers

; This test checks that the GlobalVariable @opt is split during dynamic cloning
; because it appears in only functions dynamic cloning will clone (@proc1 and
; @proc2), and one extra special function @proc3, which is forced to be cloned.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; CHECK: @opt = dso_local local_unnamed_addr global i64 0, align 8
; CHECK: @opt.[[S1:[0-9]+]] = dso_local local_unnamed_addr global i64 0, align 8

; CHECK: define void @init()
; CHECK: define i32 @main()
; CHECK: define void @proc1()
; CHECK: define void @proc2()
; CHECK: define void @proc3()
; CHECK: define void @proc1.[[S2:[0-9]+]]()
; CHECK: define void @proc2.[[S3:[0-9]+]]()
; CHECK: define void @proc3.[[S4:[0-9]+]]()

; This routine is selected as InitRoutine. Runtime checks are generated
; in this routine.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
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
  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L, i64* %F1
  call void @proc2();
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  call void @proc2();
  ret void
}

define void @proc2() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L, i64* %F1
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  call void @proc3();
  ret void
}

define void @proc3() {
  %t0 = load i64, i64* @opt, align 8
  %add = add nsw i64 %t0, 54
  store i64 %add, i64* @opt, align 8
  ret void
}

@opt = dso_local local_unnamed_addr global i64 0, align 8

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
