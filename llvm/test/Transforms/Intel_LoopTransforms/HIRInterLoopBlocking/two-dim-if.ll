; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -hir-inter-loop-blocking-force-test -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; CHECK: Function: sub1_

; CHECK:          BEGIN REGION { }
; CHECK:                if (%"sub1_$K_fetch" < %"sub1_$NTIMES_fetch")
; CHECK:                {
; CHECK:                   + DO i1 = 0, 2, 1   <DO_LOOP> <nounroll>
; CHECK:                   |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:                   |   |   %add13 = (%"sub1_$B")[i1][i2]  +  1.000000e+00;
; CHECK:                   |   |   (%"sub1_$A")[i1][i2] = %add13;
; CHECK:                   |   + END LOOP
; CHECK:                   + END LOOP


; CHECK:                   + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:                   |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:                   |   |   %add40 = (%"sub1_$A")[i1][i2]  +  2.000000e+00;
; CHECK:                   |   |   (%"sub1_$B")[i1][i2] = %add40;
; CHECK:                   |   + END LOOP
; CHECK:                   + END LOOP
; CHECK:                }
; CHECK:                ret ;
; CHECK:          END REGION

; CHECK: Function: sub1_
; CHECK:        BEGIN REGION { modified }
; CHECK:              if (%"sub1_$K_fetch" < %"sub1_$NTIMES_fetch")
; CHECK:              {
; CHECK:                 + DO i1 = 0, 2, 2   <DO_LOOP>
; CHECK:                 |   [[TILE_1:%tile_e_min[0-9]*]] = (i1 + 1 <= 2) ? i1 + 1 : 2;
; CHECK:                 |
; CHECK:                 |   + DO i2 = 0, 2, 2   <DO_LOOP>
; CHECK:                 |   |   [[TILE_2:%tile_e_min[0-9]+]] = (i2 + 1 <= 2) ? i2 + 1 : 2;
; CHECK:                 |   |   [[LBMAX:%lb_max[0-9]+]] = (0 <= i1) ? i1 : 0;
; CHECK:                 |   |   [[UBMIN:%ub_min[0-9]+]] = (2 <= [[TILE_1]]) ? 2 : [[TILE_1]];
; CHECK:                 |   |
; CHECK:                 |   |   + DO i3 = 0, -1 * [[LBMAX]] + [[UBMIN]], 1   <DO_LOOP> <nounroll>
; CHECK:                 |   |   |   [[LBMAX_1:%lb_max[0-9]*]] = (0 <= i2) ? i2 : 0;
; CHECK:                 |   |   |   [[UBMIN_1:%ub_min[0-9]*]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
; CHECK:                 |   |   |
; CHECK:                 |   |   |   + DO i4 = 0, -1 * [[LBMAX_1]] + [[UBMIN_1]], 1   <DO_LOOP>
; CHECK:                 |   |   |   |   %add13 = (%"sub1_$B")[i3 + [[LBMAX]]][i4 + [[LBMAX_1]]]  +  1.000000e+00;
; CHECK:                 |   |   |   |   (%"sub1_$A")[i3 + [[LBMAX]]][i4 + [[LBMAX_1]]] = %add13;
; CHECK:                 |   |   |   + END LOOP
; CHECK:                 |   |   + END LOOP
; CHECK:                 |   |
; CHECK:                 |   |   [[LBMAX_2:%lb_max[0-9]+]] = (0 <= i1) ? i1 : 0;
; CHECK:                 |   |   [[UBMIN_2:%ub_min[0-9]+]] = (1 <= [[TILE_1]]) ? 1 : [[TILE_1]];
; CHECK:                 |   |
; CHECK:                 |   |   + DO i3 = 0, -1 * [[LBMAX_2]] + [[UBMIN_2]], 1   <DO_LOOP>
; CHECK:                 |   |   |   [[LBMAX_3:%lb_max[0-9]+]] = (0 <= i2) ? i2 : 0;
; CHECK:                 |   |   |   [[UBMIN_3:%ub_min[0-9]+]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
; CHECK:                 |   |   |
; CHECK:                 |   |   |   + DO i4 = 0, -1 * [[LBMAX_3]] + [[UBMIN_3]], 1   <DO_LOOP>
; CHECK:                 |   |   |   |   %add40 = (%"sub1_$A")[i3 + [[LBMAX_2]]][i4 + [[LBMAX_3]]]  +  2.000000e+00;
; CHECK:                 |   |   |   |   (%"sub1_$B")[i3 + [[LBMAX_2]]][i4 + [[LBMAX_3]]] = %add40;
; CHECK:                 |   |   |   + END LOOP
; CHECK:                 |   |   + END LOOP
; CHECK:                 |   + END LOOP
; CHECK:                 + END LOOP
; CHECK:              }
; CHECK:              ret ;
; CHECK:        END REGION



;Module Before HIR
; ModuleID = 'two-dim-if.f90'
source_filename = "two-dim-if.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub1_(ptr noalias nocapture dereferenceable(8) %"sub1_$A", ptr noalias nocapture dereferenceable(8) %"sub1_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$K", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$N_fetch" = load i32, ptr %"sub1_$N", align 1
  %int_sext = sext i32 %"sub1_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %"sub1_$K_fetch" = load i32, ptr %"sub1_$K", align 1
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %rel65 = icmp slt i32 %"sub1_$K_fetch", %"sub1_$NTIMES_fetch"
  br i1 %rel65, label %bb4.preheader, label %bb28_endif

bb4.preheader:                                    ; preds = %alloca_0
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb12
  %indvars.iv91 = phi i64 [ %indvars.iv.next92, %bb12 ], [ 1, %bb4.preheader ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %indvars.iv91)
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %indvars.iv91)
  br label %bb9

bb9:                                              ; preds = %bb9, %bb4
  %indvars.iv88 = phi i64 [ %indvars.iv.next89, %bb9 ], [ 1, %bb4 ]
  %"sub1_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]", i64 %indvars.iv88)
  %"sub1_$B[][]_fetch" = load double, ptr %"sub1_$B[][]", align 1
  %add13 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch", 1.000000e+00
  %"sub1_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]", i64 %indvars.iv88)
  store double %add13, ptr %"sub1_$A[][]", align 1
  %indvars.iv.next89 = add nuw nsw i64 %indvars.iv88, 1
  %exitcond90.not = icmp eq i64 %indvars.iv.next89, 4
  br i1 %exitcond90.not, label %bb12, label %bb9

bb12:                                             ; preds = %bb9
  %indvars.iv.next92 = add nuw nsw i64 %indvars.iv91, 1
  %exitcond93.not = icmp eq i64 %indvars.iv.next92, 4
  br i1 %exitcond93.not, label %bb18.preheader, label %bb4, !llvm.loop !0

bb18.preheader:                                   ; preds = %bb12
  br label %bb18

bb18:                                             ; preds = %bb18.preheader, %bb25
  %rel64 = phi i1 [ false, %bb25 ], [ true, %bb18.preheader ]
  %"sub1_$I.1" = phi i64 [ 2, %bb25 ], [ 1, %bb18.preheader ]
  %"sub1_$A[]38" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$B[]47" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  br label %bb22

bb22:                                             ; preds = %bb22, %bb18
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb22 ], [ 1, %bb18 ]
  %"sub1_$A[][]39" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]38", i64 %indvars.iv)
  %"sub1_$A[][]_fetch" = load double, ptr %"sub1_$A[][]39", align 1
  %add40 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch", 2.000000e+00
  %"sub1_$B[][]48" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]47", i64 %indvars.iv)
  store double %add40, ptr %"sub1_$B[][]48", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %bb25, label %bb22

bb25:                                             ; preds = %bb22
  br i1 %rel64, label %bb18, label %bb28_endif.loopexit

bb28_endif.loopexit:                              ; preds = %bb25
  br label %bb28_endif

bb28_endif:                                       ; preds = %bb28_endif.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.disable"}
