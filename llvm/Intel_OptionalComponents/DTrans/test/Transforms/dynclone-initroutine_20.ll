; UNSUPPORTED: enable-opaque-pointers

; It verifies that code is generated correctly to initialize/set/
; reset/test allocation of unsafe flag correctly for DynClone
; transformation even though init routine has two user calls
; (@safecall and @unsafecall). Only unsafecall is considered as
; unsafe because 2nd field of @glob, where pointer of %struct.test.01
; is saved in init routine, is accessed.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; CHECK-LABEL: define internal void @init()

; Initialize safe flag
; CHECK: [[ALLOC_SAFE:%dyn.safe[0-9]*]] = alloca i8
; CHECK: store i8 0, i8* [[ALLOC_SAFE]]

; Set safe flag
; CHECK: store i8 1, i8* %dyn.safe
; CHECK: %call1 = tail call noalias i8* @calloc(i64 10, i64 48)

; Reset safe flag
; CHECK: store i8 0, i8* %dyn.safe
; CHECK: tail call void @unsafecall()

; Test safe flag
; CHECK: [[OR_1:%d.or[0-9]*]] = or i1
; CHECK: [[LD:%d.ld[0-9]*]] = load i8, i8* [[ALLOC_SAFE]]
; CHECK: [[CMP:%d.cmp[0-9]*]] = icmp eq i8 [[LD]], 0
; CHECK:  [[OR_2:%d.or[0-9]*]] = or i1 [[OR_1]], [[CMP]]
; CHECK: br i1 [[OR_2]],

; CHECK: store i8 1, i8* @__Shrink__Happened__

; This is selected as InitRoutine. Pointer of %struct.test.01 is saved
; in 2nd field of @glob.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  tail call void @safecall()
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  tail call void @unsafecall()
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; 1st field of @glob is accessed.
define void @safecall() {
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 0)
  ret void
}

; 2nd field of @glob is accessed.
define void @unsafecall() {
  store %struct.test.01* null, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  ret void
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 0, i64* %F6, align 8
  store i64 1, i64* %F6, align 8
  ret void
}

define i32 @main() {
  call void @init()
  call void @proc1()
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
