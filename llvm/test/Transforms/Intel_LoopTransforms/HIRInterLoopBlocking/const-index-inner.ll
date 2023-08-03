; REQUIRES: asserts
; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking-profit -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s | FileCheck %s

; Verify that the input is a profitable candidate of spatial inter loop blocking.
; Array B is used in the first i2-i3 loopnest, while is written in the second loop nest.
; Array A is written in the first i2-i3 loopnest, while is read in the second loop nest.
; Spatial localities of A[i][j] and B[i][j] across the two loopnest can be utilized by blocking
; i2-i3 levels and adding by-strip loops enclosing both loops.
; Note that small TCs are picked intentionally for demonstrating purposes. Also for the second loop
; i3-loop is missing because the innermost dimension has a constant index with value 0.

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
; CHECK:           |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   %add34 = (%"sub1_$A")[i2][0]  +  2.000000e+00;
; CHECK:           |   |   (%"sub1_$B")[i2][0] = %add34;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK: Profitable
; CHECK: Legal
; CHECK: ByStripLoop LB at DimNum 1 : 0
; CHECK: ByStripLoop UB at DimNum 1 : 2
; CHECK: ByStripLoop LB at DimNum 2 : 0
; CHECK: ByStripLoop UB at DimNum 2 : 2

; Blocking i2 and i3 loops, by stripmine size 2.
; Notice that i2-loop at line 16 above is actually, i2 and i3 loop, where i3-loop's LB, UB, and inc are 0, 0, and 1.
;
; After transformation - after normalization
;           + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;           |   + DO i2 = 0, 2, 2   <DO_LOOP>       // By-strip loop of the original i2-loop. Stripmine size is 2, and loop normalization was not applied.
;           |   |   %tile_e_min = (i2 + 1 <= 2) ? i2 + 1 : 2;  // Loop tile begin is i2, and end will be (i2 + 1). The guard is for taking care of the last tile,
;                                                              // in case iteration range is not divisible by the tile size. Typical for any loop blocking.
;           |   |
;           |   |   + DO i3 = 0, 2, 2   <DO_LOOP>   // By-strip loop of the original i3-loop. Stripmine size is 2, and loop normalization was not applied.
;           |   |   |   %tile_e_min92 = (i3 + 1 <= 2) ? i3 + 1 : 2; // Loop tile begin is i3, and end will be (i3 + 1). The guard is for taking care of the last tile,
;                                                                   // in case iteration range is not divisible by the tile size. Typical for any loop blocking.
;
;                       // Intersection of [tile_begin, tile_end] and global [LB, UB]
;                       // Global [LB, UB] = [minimum of all spatial LB of this level, maximum of all spatial UB of this level].
;           |   |   |   %lb_max93 = (0 <= i2) ? i2 : 0;  // unit-strided loop's LB = max(0, i2), where 0 = min of LB of original two i2 loops (0 = min(0,0))
;           |   |   |   %ub_min94 = (2 <= %tile_e_min) ? 2 : %tile_e_min; // unit-strided loops' UB = min(2, %tile_e_min), where 2 = max(UB of original i2-loop = 2, UB of original i2 loop = 2)
;           |   |   |
;           |   |   |   + DO i4 = 0, -1 * %lb_max93 + %ub_min94, 1   <DO_LOOP> // unit-strided loop [lb_max93, ub_min94] after loop normalization.
;           |   |   |   |   %lb_max = (0 <= i3) ? i3 : 0;                      // Similar to the above changes, now for original loop level at i3 (Loop at line 9)
;           |   |   |   |   %ub_min = (2 <= %tile_e_min92) ? 2 : %tile_e_min92;
;           |   |   |   |
;           |   |   |   |   + DO i5 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
;           |   |   |   |   |   %add6 = (%"sub1_$B")[i4 + %lb_max93][i5 + %lb_max]  +  1.000000e+00;
;           |   |   |   |   |   (%"sub1_$A")[i4 + %lb_max93][i5 + %lb_max] = %add6;
;           |   |   |   |   + END LOOP
;           |   |   |   + END LOOP
;           |   |   |
;           |   |   |   %lb_max95 = (0 <= i2) ? i2 : 0;                      // Similar to the above changes, now for the original loop at line 16.
;           |   |   |   %ub_min96 = (2 <= %tile_e_min) ? 2 : %tile_e_min;
;           |   |   |   if (i3 <= 0 & 0 <= %tile_e_min92)                   // From, i3 = 0, in the original loop at line 17. See A[i2][0] and B[i2][0].
;                                                                            // First dimensions are for i3-loop, thus i3 = 0;
;                                                                            // After the transformation i3 becomes i5.
;                                                                            // guard is ( tile_begin <= i5 <= tile_end) --> (i3 <= 0 <= tile_end)
;           |   |   |   {
;           |   |   |      + DO i4 = 0, -1 * %lb_max95 + %ub_min96, 1   <DO_LOOP>
;           |   |   |      |   %add34 = (%"sub1_$A")[i4 + %lb_max95][0]  +  2.000000e+00;
;           |   |   |      |   (%"sub1_$B")[i4 + %lb_max95][0] = %add34;
;           |   |   |      + END LOOP
;           |   |   |   }
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP


; CHECK:Function: sub1_

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   + DO i2 = 0, 2, 2   <DO_LOOP>
; CHECK:           |   |   [[TILE_1:%tile_e_min[0-9]*]] = (i2 + 1 <= 2) ? i2 + 1 : 2;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, 2, 2   <DO_LOOP>
; CHECK:           |   |   |   [[TILE_2:%tile_e_min[0-9]+]] = (i3 + 1 <= 2) ? i3 + 1 : 2;
; CHECK:           |   |   |   [[LBMAX_2:%lb_max[0-9]+]] = (0 <= i2) ? i2 : 0;
; CHECK:           |   |   |   [[UBMIN_2:%ub_min[0-9]+]] = (2 <= [[TILE_1]]) ? 2 : [[TILE_1]];
; CHECK:           |   |   |
; CHECK:           |   |   |   + DO i4 = 0, -1 * [[LBMAX_2]] + [[UBMIN_2]], 1   <DO_LOOP>
; CHECK:           |   |   |   |   [[LBMAX:%lb_max[0-9]*]] = (0 <= i3) ? i3 : 0;
; CHECK:           |   |   |   |   [[UBMIN:%ub_min[0-9]*]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
; CHECK:           |   |   |   |
; CHECK:           |   |   |   |   + DO i5 = 0, -1 * [[LBMAX]] + [[UBMIN]], 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   %add6 = (%"sub1_$B")[i4 + [[LBMAX_2]]][i5 + [[LBMAX]]]  +  1.000000e+00;
; CHECK:           |   |   |   |   |   (%"sub1_$A")[i4 + [[LBMAX_2]]][i5 + [[LBMAX]]] = %add6;
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   |
; CHECK:           |   |   |   [[LBMAX_3:%lb_max[0-9]+]] = (0 <= i2) ? i2 : 0;
; CHECK:           |   |   |   [[UBMIN_3:%ub_min[0-9]+]] = (2 <= [[TILE_1]]) ? 2 : [[TILE_1]];
; CHECK:           |   |   |   if (i3 <= 0 & 0 <= [[TILE_2]])
; CHECK:           |   |   |   {
; CHECK:           |   |   |      + DO i4 = 0, -1 * [[LBMAX_3]] + [[UBMIN_3]], 1   <DO_LOOP>
; CHECK:           |   |   |      |   %add34 = (%"sub1_$A")[i4 + [[LBMAX_3]]][0]  +  2.000000e+00;
; CHECK:           |   |   |      |   (%"sub1_$B")[i4 + [[LBMAX_3]]][0] = %add34;
; CHECK:           |   |   |      + END LOOP
; CHECK:           |   |   |   }
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 'const-index-inner.f90'
source_filename = "const-index-inner.f90"
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
  %int_sext188 = zext i32 %"sub1_$NTIMES_fetch" to i64
  %0 = add nuw nsw i64 %int_sext188, 1
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb31
  %"sub1_$K.0" = phi i64 [ %add57, %bb31 ], [ 1, %bb4.preheader ]
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
  %exitcond85 = icmp eq i64 %add27, 4
  br i1 %exitcond85, label %bb30.preheader, label %bb8

bb30.preheader:                                   ; preds = %bb13
  br label %bb30

bb30:                                             ; preds = %bb30.preheader, %bb30
  %"sub1_$I.1" = phi i64 [ %add51, %bb30 ], [ 1, %bb30.preheader ]
  %"sub1_$A[]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$A[]32[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$A[]32", i64 1)
  %"sub1_$A[]32[]_fetch" = load double, ptr %"sub1_$A[]32[]", align 1
  %add34 = fadd double %"sub1_$A[]32[]_fetch", 2.000000e+00
  %"sub1_$B[]43" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  %"sub1_$B[]43[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sub1_$B[]43", i64 1)
  store double %add34, ptr %"sub1_$B[]43[]", align 1
  %add51 = add nuw nsw i64 %"sub1_$I.1", 1
  %exitcond86 = icmp eq i64 %add51, 4
  br i1 %exitcond86, label %bb31, label %bb30

bb31:                                             ; preds = %bb30
  %add57 = add nuw nsw i64 %"sub1_$K.0", 1
  %exitcond87 = icmp eq i64 %add57, %0
  br i1 %exitcond87, label %bb1.loopexit, label %bb4

bb1.loopexit:                                     ; preds = %bb31
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
