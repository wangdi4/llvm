; UNSUPPORTED: enable-opaque-pointers

; This test verifies that DynClone+Reencoding transformation is not
; triggered because there are too many large constants to be reencoded
; when -dtrans-simple-dynclone-enable=false.

; REQUIRES: asserts
;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -dtrans-simple-dynclone-enable=false -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone-reencoding 2>&1 | FileCheck %s
;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -dtrans-simple-dynclone-enable=false -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone-reencoding 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Too many large constants for encoding...Skip DynClone
; CHECK-NOT: store i8 1, i8* @__Shrink__Happened__

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }
%struct.nw = type { i32, i64 }
; Variable for candidate fields initialization.
@nw = internal global %struct.nw zeroinitializer, align 8


; This routine has basic instructions to transform.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %LF1 = load i64, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 %LF1, i64* %F6, align 8
  ret void
}

; This routine has basic instructions to transform.
define void @proc2() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %LF1 = load i64, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 300000, i64* %F6, align 8
  store i64 300001, i64* %F6, align 8
  store i64 300002, i64* %F6, align 8
  store i64 300003, i64* %F6, align 8
  store i64 300004, i64* %F6, align 8
  store i64 300005, i64* %F6, align 8
  ret void
}

; This routine is selected as InitRoutine.
define void @init() {
  store i64 2000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  %L = load i64, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1)
  %S1 = mul i64 2, %L
  store i64 %S1, i64* %F1, align 8
  %S2 = add i64 15, %S1
  store i64 %S2, i64* %F6, align 8

  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  store i64 1000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  store i64 1000001, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  call void @init();
;  call void @proc1();
;  call void @proc2();
  ret i32 0
}
; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
