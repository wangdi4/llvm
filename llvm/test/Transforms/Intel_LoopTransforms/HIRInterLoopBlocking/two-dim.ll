; REQUIRES: asserts
;
; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking-profit -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s -disable-output | FileCheck %s
;
; Verify that the input is a profitable candidate of spatial inter loop blocking.
; Array B is used in the first i2-i3 loopnest, while is written in the second loop nest.
; Array A is written in the first i2-i3 loopnest, while is read in the second loop nest.
; Spatial localities of A[i][j] and B[i][j] across the two loopnest can be utilized by blocking
; i2-i3 levels and adding by-strip loops enclosing both loops.

; CHECK:Function: sub1_

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   |   %add6 = (%"sub1_$B")[i2][i3]  +  1.000000e+00;
; CHECK:           |   |   |   (%"sub1_$A")[i2][i3] = %add6;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   |   %add34 = (%"sub1_$A")[i2][i3]  +  2.000000e+00;
; CHECK:           |   |   |   (%"sub1_$B")[i2][i3] = %add34;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK: Profitable
; CHECK: Legal

; i3 - loop LB = min(0,0), UB = max (2, 2)
; CHECK: ByStripLoop LB at DimNum 1 : 0
; CHECK: ByStripLoop UB at DimNum 1 : 2

; i2 - loop LB = min(0,0), UB = max (2, 1)
; CHECK: ByStripLoop LB at DimNum 2 : 0
; CHECK: ByStripLoop UB at DimNum 2 : 2

; CHECK:Function: sub1_
; CHECK:      BEGIN REGION { modified }
; CHECK:            + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:            |   + DO i2 = 0, 2, 2   <DO_LOOP>
; CHECK:            |   |   [[TILE_1:%tile_e_min[0-9]*]] = (i2 + 1 <= 2) ? i2 + 1 : 2;
; CHECK:            |   |
; CHECK:            |   |   + DO i3 = 0, 2, 2   <DO_LOOP>
; CHECK:            |   |   |   [[TILE_2:%tile_e_min[0-9]+]] = (i3 + 1 <= 2) ? i3 + 1 : 2;
; CHECK:            |   |   |   [[LBMAX:%lb_max[0-9]+]] = (0 <= i2) ? i2 : 0;
; CHECK:            |   |   |   [[UBMIN:%ub_min[0-9]+]] = (2 <= [[TILE_1]]) ? 2 : [[TILE_1]];
; CHECK:            |   |   |
; CHECK:            |   |   |   + DO i4 = 0, -1 * [[LBMAX]] + [[UBMIN]], 1   <DO_LOOP>
; CHECK:            |   |   |   |   [[LBMAX_1:%lb_max[0-9]*]] = (0 <= i3) ? i3 : 0;
; CHECK:            |   |   |   |   [[UBMIN_1:%ub_min[0-9]*]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
; CHECK:            |   |   |   |
; CHECK:            |   |   |   |   + DO i5 = 0, -1 * [[LBMAX_1]] + [[UBMIN_1]], 1   <DO_LOOP>
; CHECK:            |   |   |   |   |   %add6 = (%"sub1_$B")[i4 + [[LBMAX]]][i5 + [[LBMAX_1]]]  +  1.000000e+00;
; CHECK:            |   |   |   |   |   (%"sub1_$A")[i4 + [[LBMAX]]][i5 + [[LBMAX_1]]] = %add6;
; CHECK:            |   |   |   |   + END LOOP
; CHECK:            |   |   |   + END LOOP
; CHECK:            |   |   |
; CHECK:            |   |   |   [[LBMAX_2:%lb_max[0-9]+]] = (0 <= i2) ? i2 : 0;
; CHECK:            |   |   |   [[UBMIN_2:%ub_min[0-9]+]] = (1 <= [[TILE_1]]) ? 1 : [[TILE_1]];
; CHECK:            |   |   |
; CHECK:            |   |   |   + DO i4 = 0, -1 * [[LBMAX_2]] + [[UBMIN_2]], 1   <DO_LOOP>
; CHECK:            |   |   |   |   [[LBMAX_3:%lb_max[0-9]+]] = (0 <= i3) ? i3 : 0;
; CHECK:            |   |   |   |   [[UBMIN_3:%ub_min[0-9]+]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
; CHECK:            |   |   |   |
; CHECK:            |   |   |   |   + DO i5 = 0, -1 * [[LBMAX_3]] + [[UBMIN_3]], 1   <DO_LOOP>
; CHECK:            |   |   |   |   |   %add34 = (%"sub1_$A")[i4 + [[LBMAX_2]]][i5 + [[LBMAX_3]]]  +  2.000000e+0
; CHECK:            |   |   |   |   |   (%"sub1_$B")[i4 + [[LBMAX_2]]][i5 + [[LBMAX_3]]] = %add34;
; CHECK:            |   |   |   |   + END LOOP
; CHECK:            |   |   |   + END LOOP
; CHECK:            |   |   + END LOOP
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR
; ModuleID = 'two-dim.f90'
source_filename = "two-dim.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind
define void @sub1_(ptr noalias nocapture %"sub1_$A", ptr noalias nocapture %"sub1_$B", ptr noalias nocapture readonly %"sub1_$N", ptr noalias nocapture readonly %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$N_fetch" = load i32, ptr %"sub1_$N", align 1
  %int_sext = sext i32 %"sub1_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %rel = icmp slt i32 %"sub1_$NTIMES_fetch", 1
  br i1 %rel, label %bb1, label %bb4.preheader

bb4.preheader:                                    ; preds = %alloca_0
  %int_sext199 = zext i32 %"sub1_$NTIMES_fetch" to i64
  %0 = add nuw nsw i64 %int_sext199, 1
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb30
  %"sub1_$K.0" = phi i64 [ %add65, %bb30 ], [ 1, %bb4.preheader ]
  br label %bb8

bb8:                                              ; preds = %bb13, %bb4
  %"sub1_$I.0" = phi i64 [ 1, %bb4 ], [ %add27, %bb13 ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) %"sub1_$B", i64 %"sub1_$I.0")
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) %"sub1_$A", i64 %"sub1_$I.0")
  br label %bb12

bb12:                                             ; preds = %bb12, %bb8
  %"sub1_$J.0" = phi i64 [ 1, %bb8 ], [ %add19, %bb12 ]
  %"sub1_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$B[]", i64 %"sub1_$J.0")
  %"sub1_$B[][]_fetch" = load double, ptr %"sub1_$B[][]", align 1
  %add6 = fadd double %"sub1_$B[][]_fetch", 1.000000e+00
  %"sub1_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$A[]", i64 %"sub1_$J.0")
  store double %add6, ptr %"sub1_$A[][]", align 1
  %add19 = add nuw nsw i64 %"sub1_$J.0", 1
  %exitcond = icmp eq i64 %add19, 4
  br i1 %exitcond, label %bb13, label %bb12

bb13:                                             ; preds = %bb12
  %add27 = add nuw nsw i64 %"sub1_$I.0", 1
  %exitcond95 = icmp eq i64 %add27, 4
  br i1 %exitcond95, label %bb29.preheader, label %bb8

bb29.preheader:                                   ; preds = %bb13
  br label %bb29

bb29:                                             ; preds = %bb29.preheader, %bb34
  %"sub1_$I.1" = phi i64 [ %add59, %bb34 ], [ 1, %bb29.preheader ]
  %"sub1_$A[]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$B[]43" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  br label %bb33

bb33:                                             ; preds = %bb33, %bb29
  %"sub1_$J.1" = phi i64 [ 1, %bb29 ], [ %add51, %bb33 ]
  %"sub1_$A[]32[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$A[]32", i64 %"sub1_$J.1")
  %"sub1_$A[]32[]_fetch" = load double, ptr %"sub1_$A[]32[]", align 1
  %add34 = fadd double %"sub1_$A[]32[]_fetch", 2.000000e+00
  %"sub1_$B[]43[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$B[]43", i64 %"sub1_$J.1")
  store double %add34, ptr %"sub1_$B[]43[]", align 1
  %add51 = add nuw nsw i64 %"sub1_$J.1", 1
  %exitcond96 = icmp eq i64 %add51, 4
  br i1 %exitcond96, label %bb34, label %bb33

bb34:                                             ; preds = %bb33
  %add59 = add nuw nsw i64 %"sub1_$I.1", 1
  %exitcond97 = icmp eq i64 %add59, 3
  br i1 %exitcond97, label %bb30, label %bb29

bb30:                                             ; preds = %bb34
  %add65 = add nuw nsw i64 %"sub1_$K.0", 1
  %exitcond98 = icmp eq i64 %add65, %0
  br i1 %exitcond98, label %bb1.loopexit, label %bb4

bb1.loopexit:                                     ; preds = %bb30
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
