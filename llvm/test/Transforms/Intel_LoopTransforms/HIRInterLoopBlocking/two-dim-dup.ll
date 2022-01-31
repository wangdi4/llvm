; RUN: opt -hir-inter-loop-blocking-force-test -hir-ssa-deconstruction -hir-temp-cleanup -disable-hir-inter-loop-blocking=false -hir-inter-loop-blocking -print-before=hir-inter-loop-blocking -hir-inter-loop-blocking-stripmine-size=2 -print-after=hir-inter-loop-blocking < %s 2>&1 | FileCheck %s
; RUN: opt -hir-inter-loop-blocking-force-test -disable-hir-inter-loop-blocking=false -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s | FileCheck %s

; Notice that in input, i2-loop is a common ancestor of two i3-loops. Also, i2 & i3 loops are spatial loops. Verify that i3-loop is processed once and two-level by-strip loopnests are introduced.
; TODO: This test corrently testing the capability of the transformation only.

; Input
; Function: sub1_
;
;          BEGIN REGION { }
;                + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;                |   + DO i2 = 0, 2, 1   <DO_LOOP> // common outer-spatial loop to two i3-loops
;                |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;                |   |   |   %add.2 = (%"sub1_$B")[i2][i3]  +  1.000000e+00;
;                |   |   |   (%"sub1_$A")[i2][i3] = %add.2;
;                |   |   + END LOOP
;                |   |
;                |   |
;                |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;                |   |   |   %add.5 = (%"sub1_$A")[i2][i3]  +  2.000000e+00;
;                |   |   |   (%"sub1_$B")[i2][i3] = %add.5;
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP
;          END REGION

; *** IR Dump After HIR Spatial blocking over multiple loopnests ***
; Function: sub1_

;          BEGIN REGION { modified }
;                + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;                |   + DO i2 = 0, 2, 2   <DO_LOOP>   // by-strip loop
;                |   |   %tile_e_min = (i2 + 1 <= 2) ? i2 + 1 : 2;
;                |   |
;                |   |   + DO i3 = 0, 2, 2   <DO_LOOP> // by-strip loop
;                |   |   |   %tile_e_min4 = (i3 + 1 <= 2) ? i3 + 1 : 2;
;                |   |   |   %lb_max5 = (0 <= i2) ? i2 : 0;
;                |   |   |   %ub_min6 = (2 <= %tile_e_min) ? 2 : %tile_e_min;
;                |   |   |
;                |   |   |   + DO i4 = 0, -1 * %lb_max5 + %ub_min6, 1   <DO_LOOP>
;                |   |   |   |   %lb_max = (0 <= i3) ? i3 : 0;
;                |   |   |   |   %ub_min = (2 <= %tile_e_min4) ? 2 : %tile_e_min4;
;                |   |   |   |
;                |   |   |   |   + DO i5 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
;                |   |   |   |   |   %add.2 = (%"sub1_$B")[i4 + %lb_max5][i5 + %lb_max]  +  1.000000e+00;
;                |   |   |   |   |   (%"sub1_$A")[i4 + %lb_max5][i5 + %lb_max] = %add.2;
;                |   |   |   |   + END LOOP
;                |   |   |   |
;                |   |   |   |   %lb_max7 = (0 <= i3) ? i3 : 0;
;                |   |   |   |   %ub_min8 = (2 <= %tile_e_min4) ? 2 : %tile_e_min4;
;                |   |   |   |
;                |   |   |   |   + DO i5 = 0, -1 * %lb_max7 + %ub_min8, 1   <DO_LOOP>
;                |   |   |   |   |   %add.5 = (%"sub1_$A")[i4 + %lb_max5][i5 + %lb_max7]  +  2.000000e+00;
;                |   |   |   |   |   (%"sub1_$B")[i4 + %lb_max5][i5 + %lb_max7] = %add.5;
;                |   |   |   |   + END LOOP
;                |   |   |   + END LOOP
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP
;          END REGION


; CHECK:               DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
; CHECK:               DO i2 = 0, 2, 2
; CHECK:               %tile_e_min
; CHECK-NOT:           %tile_e_min
; CHECK:               DO i3 = 0, 2, 2
; CHECK:               DO i4 = 0, -1 * %lb_max{{[0-9]+}} + %ub_min6, 1   <DO_LOOP>
; CHECK:               DO i5 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; CHECK:               DO i5 = 0, -1 * %lb_max7 + %ub_min8, 1   <DO_LOOP>



;Module Before HIR
; ModuleID = 'two-dim-dup.f90'
source_filename = "two-dim-dup.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub1_(double* noalias nocapture dereferenceable(8) %"sub1_$A", double* noalias nocapture dereferenceable(8) %"sub1_$B", i32* noalias nocapture readonly dereferenceable(4) %"sub1_$N", i32* noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$N_fetch" = load i32, i32* %"sub1_$N", align 1
  %int_sext = sext i32 %"sub1_$N_fetch" to i64
  %mul.1 = shl nsw i64 %int_sext, 3
  %"sub1_$NTIMES_fetch" = load i32, i32* %"sub1_$NTIMES", align 1
  %rel.1 = icmp slt i32 %"sub1_$NTIMES_fetch", 1
  br i1 %rel.1, label %bb4, label %bb3.preheader

bb3.preheader:                                    ; preds = %alloca_0
  %int_sext162 = zext i32 %"sub1_$NTIMES_fetch" to i64
  %0 = add nuw nsw i64 %int_sext162, 1
  br label %bb3

bb3:                                              ; preds = %bb3.preheader, %bb10
  %"sub1_$K.0" = phi i64 [ %add.8, %bb10 ], [ 1, %bb3.preheader ]
  br label %bb7

bb7:                                              ; preds = %bb23, %bb3
  %"sub1_$I.0" = phi i64 [ 1, %bb3 ], [ %add.7, %bb23 ]
  %"sub1_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.1, double* elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.0")
  %"sub1_$A[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.1, double* elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.0")
  br label %bb11

bb11:                                             ; preds = %bb11, %bb7
  %"sub1_$J.0" = phi i64 [ 1, %bb7 ], [ %add.4, %bb11 ]
  %"sub1_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$B[]", i64 %"sub1_$J.0")
  %"sub1_$B[][]_fetch" = load double, double* %"sub1_$B[][]", align 1
  %add.2 = fadd fast double %"sub1_$B[][]_fetch", 1.000000e+00
  %"sub1_$A[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$A[]", i64 %"sub1_$J.0")
  store double %add.2, double* %"sub1_$A[][]", align 1
  %add.4 = add nuw nsw i64 %"sub1_$J.0", 1
  %exitcond.not = icmp eq i64 %add.4, 4
  br i1 %exitcond.not, label %bb20.preheader, label %bb11

bb20.preheader:                                   ; preds = %bb11
  br label %bb20

bb20:                                             ; preds = %bb20.preheader, %bb20
  %"sub1_$J.1" = phi i64 [ %add.6, %bb20 ], [ 1, %bb20.preheader ]
  %"sub1_$A[][]19" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$A[]", i64 %"sub1_$J.1")
  %"sub1_$A[][]_fetch" = load double, double* %"sub1_$A[][]19", align 1
  %add.5 = fadd fast double %"sub1_$A[][]_fetch", 2.000000e+00
  %"sub1_$B[][]25" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$B[]", i64 %"sub1_$J.1")
  store double %add.5, double* %"sub1_$B[][]25", align 1
  %add.6 = add nuw nsw i64 %"sub1_$J.1", 1
  %exitcond59.not = icmp eq i64 %add.6, 4
  br i1 %exitcond59.not, label %bb23, label %bb20

bb23:                                             ; preds = %bb20
  %add.7 = add nuw nsw i64 %"sub1_$I.0", 1
  %exitcond60.not = icmp eq i64 %add.7, 4
  br i1 %exitcond60.not, label %bb10, label %bb7

bb10:                                             ; preds = %bb23
  %add.8 = add nuw nsw i64 %"sub1_$K.0", 1
  %exitcond61 = icmp eq i64 %add.8, %0
  br i1 %exitcond61, label %bb4.loopexit, label %bb3, !llvm.loop !0

bb4.loopexit:                                     ; preds = %bb10
  br label %bb4

bb4:                                              ; preds = %bb4.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
