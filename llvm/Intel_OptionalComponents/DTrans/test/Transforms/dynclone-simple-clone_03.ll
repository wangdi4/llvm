; This test verifies that fields 1 and 4 of %struct.test.01 are NOT packed using
; bit-fields because simple dynclone is triggered due to many special constants.
; It also verifies that transformations are done correctly for
; the following
;    1. New layout
;    2. General loads / stores of field 2 in @proc1

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
; RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; New layout for DynClone.
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i32, i32, i16 }>

; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  store i64 20000000, i64* %F6, align 8
  store i64 20000001, i64* %F6, align 8
  store i64 20000003, i64* %F6, align 8
  store i64 20000004, i64* %F6, align 8
  store i64 20000005, i64* %F6, align 8
  store i64 20000006, i64* %F6, align 8
  store i64 20000007, i64* %F6, align 8
  store i64 20000008, i64* %F6, align 8
  store i64 20000009, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L1 = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L1, i64* %F1
  %L2 = load i64, i64* %F1
  %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4
  store i16 0, i16* %F4
  %C1 = icmp eq i64 %L2, 2
  %S1 = select i1 %C1, i16 1, i16 2
  store i16 %S1, i16* %F4

; Checking there is no packing
; CHECK:  %F4 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* %11, i32 0, i32 7
; CHECK:   %S1 = select i1 %C1, i16 1, i16 2
; CHECK:  store i16 %S1, i16* %F4, align 2

  %L3 = load i16, i16* %F4

; Checking there is no packing
; CHECK:  %L3 = load i16, i16* %F4, align 2

  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
