; This test verifies that DynClone is not triggered because it is unable to
; track all uses of memory allocation pointer in init routine because %tp4
; is escaped from init routine.

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

; CHECK: Track uses of AllocCalls Failed
; CHECK-NOT: store i8 1, i8* @__Shrink__Happened__

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Memory allocation pointer, which is returned by calloc in "init" routine,
; is saved in @glob.
%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is selected as InitRoutine.
; Memory allocation pointer (i.e return value of calloc) is escaped
; through ret instruction. DynClone will be disabled since it is
; unable to track all uses of calloc.
define %struct.test.01* @init() {
blk0:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  br i1 undef, label %blk1, label %blk2

blk1:
  %tp2 = load %struct.test.01*, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  br label %blk2

blk2:
  %tp3 = phi %struct.test.01* [%tp1, %blk0], [%tp2, %blk1]
  %tp4 = getelementptr %struct.test.01, %struct.test.01* %tp3, i64 2
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret %struct.test.01* %tp4
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call %struct.test.01* @init();
  call void @proc1();
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
declare dso_local noalias i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
