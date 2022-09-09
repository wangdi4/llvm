; This test verifies that runtime checks are generated correctly in "init"
; routine for simple DynClone transformation when more than 8 large
; constants. Verifies that 2147483647 and -2147483648 (32-bit) are used for
; runtime checks.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dynclone 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dynclone' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are qualified as final candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; This routine is selected as InitRoutine. Runtime checks are generated
; in this routine.
define void @init() {

; Creates Local min and max variables and set initial values.
;
; CHECK-LABEL:   define internal void @init
; CHECK:  [[ALLOC_SAFE:%dyn.safe[0-9]*]] = alloca i8
; CHECK:  store i8 0, i8* [[ALLOC_SAFE]]
; CHECK:   [[ALLOC_MAX:%d.max[0-9]*]] = alloca i64
; CHECK-NEXT:  store i64 -2147483648, i64* [[ALLOC_MAX]]
; CHECK-NEXT:  [[ALLOC_MIN:%d.min[0-9]*]] = alloca i64
; CHECK-NEXT:  store i64 2147483647, i64* [[ALLOC_MIN]]

; CHECK: store i8 1, i8* [[ALLOC_SAFE]]
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000

; Update values of min and max variables after checking with %g1, which
; is assigned to field 1.
;
; CHECK-LABEL:  %g1 = select i1 undef, i64 500, i64 1000
; CHECK:  [[MIN_LD_1:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MIN]]
; CHECK-NEXT:  [[MIN_CMP_1:%d.cmp[0-9]*]] = icmp slt i64 [[MIN_LD_1]], %g1
; CHECK-NEXT:  [[MIN_SEL_1:%d.sel[0-9]*]] = select i1 [[MIN_CMP_1]], i64 [[MIN_LD_1]], i64 %g1
; CHECK-NEXT:  store i64 [[MIN_SEL_1]], i64* [[ALLOC_MIN]]
;
; CHECK-NEXT:  [[MAX_LD_1:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MAX]]
; CHECK-NEXT:  [[MAX_CMP_1:%d.cmp[0-9]*]] = icmp sgt i64 [[MAX_LD_1]], %g1
; CHECK-NEXT:  [[MAX_SEL_1:%d.sel[0-9]*]] = select i1 [[MAX_CMP_1]], i64 [[MAX_LD_1]], i64 %g1
; CHECK-NEXT:  store i64 [[MAX_SEL_1]], i64* [[ALLOC_MAX]]
; CHECK-NEXT:  store i64 %g1, i64* %F1, align 8

  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000

; Update values of min and max variables after checking with %g2, which is
; assigned to field 6.
;
; CHECK-LABEL:  %g2 = select i1 undef, i64 -5000, i64 20000
; CHECK-NEXT:  [[MIN_LD_2:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MIN]]
; CHECK-NEXT:  [[MIN_CMP_2:%d.cmp[0-9]*]] = icmp slt i64 [[MIN_LD_2]], %g2
; CHECK-NEXT:  [[MIN_SEL_2:%d.sel[0-9]*]] = select i1 [[MIN_CMP_2]], i64 [[MIN_LD_2]], i64 %g2
; CHECK-NEXT:  store i64 [[MIN_SEL_2]], i64* [[ALLOC_MIN]]
;
; CHECK-NEXT:  [[MAX_LD_2:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MAX]]
; CHECK-NEXT:  [[MAX_CMP_2:%d.cmp[0-9]*]] = icmp sgt i64 [[MAX_LD_2]], %g2
; CHECK-NEXT:  [[MAX_SEL_2:%d.sel[0-9]*]] = select i1 [[MAX_CMP_2]], i64 [[MAX_LD_2]], i64 %g2
; CHECK-NEXT:  store i64 [[MAX_SEL_2]], i64* [[ALLOC_MAX]]

  store i64 %g2, i64* %F6, align 8

  store i64 10000000, i64* %F1
  store i64 -21, i64* %F1
  store i64 -22, i64* %F1
  store i64 -23, i64* %F6
  store i64 -24, i64* %F6
  store i64 -25, i64* %F1
  store i64 -26, i64* %F6
  store i64 -27, i64* %F6
  store i64 -28, i64* %F1

; Generate final checks to determine if max and min values fit in
; shrinked types and set __Shrink__Happened__ if it does.
;
; CHECK-LABEL: store i64 %g2, i64* %F6, align 8
; CHECK:  [[MIN_LD_3:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MIN]]
; CHECK-NEXT:  [[MIN_CMP_3:%d.cmp[0-9]*]] = icmp slt i64 [[MIN_LD_3]], -2147483648
; CHECK-NEXT:  [[MAX_LD_3:%d.ld[0-9]*]] = load i64, i64* [[ALLOC_MAX]]
; CHECK-NEXT:  [[MAX_CMP_3:%d.cmp[0-9]*]] = icmp sgt i64 [[MAX_LD_3]], 2147483647
; CHECK-NEXT:  [[OR_1:%d.or[0-9]*]] = or i1 [[MIN_CMP_3]], [[MAX_CMP_3]]
; CHECK-NEXT:  [[MAX_LD_4:%d.ld[0-9]*]] = load i8, i8* [[ALLOC_SAFE]]
; CHECK-NEXT:  [[MAX_CMP_4:%d.cmp[0-9]*]] = icmp eq i8 [[MAX_LD_4]], 0
; CHECK-NEXT:  [[OR_2:%d.or[0-9]*]] = or i1 [[OR_1]], [[MAX_CMP_4]]
; CHECK-NEXT:  br i1 [[OR_2]],

; CHECK-LABEL:  d.set_happened:
; CHECK:  store i8 1, i8* @__Shrink__Happened__

  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  call void @proc1(%struct.test.01* %j);
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1(%struct.test.01* %tp2) {
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L, i64* %F1
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
