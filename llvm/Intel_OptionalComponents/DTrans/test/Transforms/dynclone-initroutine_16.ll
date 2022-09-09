; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation because init routine has two unsafe calls
; (unsafecall1 and unsafecall2). Pointer of %struct.test.01 is saved in
; (in 2nd field of @glob) in init routine. The same location (2nd field
; of @glob) is accessed in unsafecall1 and unsafecall2.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK:    Calls in InitRoutine Failed

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields candidate fields for DynClone.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; This is used to save pointers of %struct.test.01.
%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is not qualified as InitRoutine because it has two unsafe
; calls (unsafecall1 and unsafecall2). At most only one unsafe call is
; allowed right now.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  tail call void @unsafecall1()
  tail call void @unsafecall2()
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; Accesses 2nd field of @glob
define void @unsafecall1() {
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; Accesses 2nd field of @glob
define void @unsafecall2() {
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; This routine just accesses candidate field. This also helps
; make %struct.test.01 as most frequently used struct.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 0, i64* %F6, align 8
  store i64 1, i64* %F6, align 8
  store i64 2, i64* %F6, align 8
  ret void
}

define i32 @main() {
  call void @init()
  call void @proc1()
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
