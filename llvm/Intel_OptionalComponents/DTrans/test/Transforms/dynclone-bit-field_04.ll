; UNSUPPORTED: enable-opaque-pointers

; This test verifies that fields 1 and 4 of %struct.test.01 are NOT packed using
; bit-fields because values of field 4 don't fit in 2-bits. It verifies that
; transformations are done correctly for the following
;    1. New layout
;    2. General loads / stores of field 4 (without bitfields)

;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16. 1 and 4 fields are NOT packed using
; bit-fields.

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; New layout for DynClone.
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i16, i16, i16 }>

; This routine is selected as InitRoutine.
define void @init() {

; CHECK-LABEL:   define internal void @init
; CHECK:   [[DALLOC1:%dyn.alloc[0-9]+]] = alloca i64
; CHECK:   [[DALLOC2:%dyn.alloc]] = alloca i8*
; CHECK:   [[DSAFE:%dyn.safe]] = alloca i8
; CHECK:   store i8 0, i8* [[DSAFE]]
; CHECK:   [[DMAX:%d.max]] = alloca i64
; CHECK:   store i64 0, i64* [[DMAX]]
; CHECK:   [[DMIN:%d.min]] = alloca i64
; CHECK:   store i64 65535, i64* [[DMIN]]

  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)

  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8

; CHECK:  store i8 1, i8* @__Shrink__Happened__

  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
  ret i32 0
}

; CHECK-LABEL:   define internal void @proc1()

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L1 = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1

;
; Make sure bitfields are not used for storing field 1.
;
; CHECK: [[PG5:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* %5, i32 0, i32 5
; CHECK: %F1 = bitcast i16* [[PG5]] to i64*
; CHECK: [[PT1:%[0-9]+]] = trunc i64 %L1 to i16
; CHECK: [[PBC1:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK: store i16 [[PT1]], i16* [[PBC1]], align 2

  store i64 %L1, i64* %F1

;
; Make sure bitfields are not used for loading field 1.
;
; CHECK: [[PBC2:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK: [[PLD2:%[0-9]+]] = load i16, i16* [[PBC2]], align 2
; CHECK: %L2 = zext i16 [[PLD2]] to i64

  %L2 = load i64, i64* %F1
  %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4

;
; Make sure bitfields are not used for storing field 4.
;
; CHECK: [[TP2:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: %F4 = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[TP2]], i32 0, i32 6
; CHECK: store i16 0, i16* %F4

  store i16 0, i16* %F4

;
; Make sure bitfields are not used for storing field 4.
;
  %C1 = icmp eq i64 %L2, 2
  %S1 = select i1 %C1, i16 1, i16 4
; CHECK: %S1 = select i1 %C1, i16 1, i16 4
; CHECK: store i16 %S1, i16* %F4
  store i16 %S1, i16* %F4

;
; Make sure bitfields are not used for loading field 4.
;
; CHECK: %L3 = load i16, i16* %F4
  %L3 = load i16, i16* %F4
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
