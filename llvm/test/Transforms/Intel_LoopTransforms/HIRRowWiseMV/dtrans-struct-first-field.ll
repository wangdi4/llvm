; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -dtransanalysis -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; RUN: opt -whole-program-assume -passes='require<dtransanalysis>,function(hir-ssa-deconstruction,hir-temp-cleanup,hir-rowwise-mv)' -print-before=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

; This test checks that HIRRowWiseMV doesn't crash if it encounters an opaque
; pointer struct GEP where the base pointer is the first struct field and the
; GEP doesn't use an explicit 0 struct field index for it.

; Print before:

; CHECK: BEGIN REGION { }
; CHECK:       %9 = (ptr)(@upml_mod_mp_bxe_)[0];

; CHECK:       + DO i1 = 0, -1 * %0, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, -1 * %0, 1   <DO_LOOP>
; CHECK:       |   |   %18 = 0.000000e+00  *  (%9)[i2 + %0];
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank1$.71.314.470.507.512.531.556.562.564.572.575.580.581.585.589.594.613.633.639.644.663.683.689" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@upml_mod_mp_bxe_ = external global %"QNCA_a0$double*$rank1$.71.314.470.507.512.531.556.562.564.572.575.580.581.585.589.594.613.633.639.644.663.683.689"

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define internal fastcc void @leapfrog_(i64 %0) #1 {
  br label %2

2:                                                ; preds = %1
  br label %3

3:                                                ; preds = %5, %2
  %4 = phi i64 [ %0, %2 ], [ %6, %5 ]
  br label %5

5:                                                ; preds = %3
  %6 = add nsw i64 %4, 1
  %7 = icmp eq i64 %4, 0
  br i1 %7, label %8, label %3

8:                                                ; preds = %5
  %9 = load ptr, ptr @upml_mod_mp_bxe_, align 16
  br label %10

10:                                               ; preds = %21, %8
  %11 = phi i64 [ %0, %8 ], [ %22, %21 ]
  br i1 true, label %21, label %12

12:                                               ; preds = %10
  br label %13

13:                                               ; preds = %13, %12
  %14 = phi i64 [ %0, %12 ], [ %15, %13 ]
  %15 = add nsw i64 %14, 1
  %16 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) %9, i64 %14)
  %17 = load double, ptr %16, align 8
  %18 = fmul fast double 0.000000e+00, %17
  %19 = icmp eq i64 %14, 0
  br i1 %19, label %20, label %13

20:                                               ; preds = %13
  br label %21

21:                                               ; preds = %20, %10
  %22 = add nsw i64 %11, 1
  %23 = icmp eq i64 %11, 0
  br i1 %23, label %24, label %10

24:                                               ; preds = %21
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { "unsafe-fp-math"="true" }

; end INTEL_FEATURE_SW_DTRANS
