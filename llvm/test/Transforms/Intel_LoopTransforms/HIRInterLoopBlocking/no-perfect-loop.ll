; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-inter-loop-blocking -print-before=hir-inter-loop-blocking -print-after=hir-inter-loop-blocking  < %s 2>&1 | FileCheck %s
; opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 < %s | FileCheck %s

; Verify that alignments on memrefs not in innermost loops are correctly done

; Function: sub1_
;
;         BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch.4") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;               |   |   (%"sub1_$C6")[i2] = (%"sub1_$D2")[i2];
;               |   |
;               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;               |   |   |   %add.4 = (%"sub1_$B")[i2 + 1][i3 + 1]  +  1.000000e+00;
;               |   |   |   (%"sub1_$A")[i2 + 1][i3 + 1] = %add.4;
;               |   |   + END LOOP
;               |   + END LOOP
;               |
;               |
;               |   + DO i2 = 0, 1, 1   <DO_LOOP>
;               |   |   %add.11 = (%"sub1_$C6")[i2 + 1]  +  1.000000e+00;
;               |   |   (%"sub1_$D2")[i2] = %add.11;
;               |   |
;               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;               |   |   |   %add.12 = (%"sub1_$A")[i2 + -1][i3]  +  2.000000e+00;
;               |   |   |   (%"sub1_$B")[i2 + -1][i3] = %add.12;
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;         END REGION
;
; Function: sub1_
;
;         BEGIN REGION { modified }
;               + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch.4") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;               |   + DO i2 = -1, 3, 2   <DO_LOOP>
;               |   |   %tile_e_min = (i2 + 1 <= 3) ? i2 + 1 : 3;
;               |   |
;               |   |   + DO i3 = 0, 3, 2   <DO_LOOP>
;               |   |   |   %tile_e_min7 = (i3 + 1 <= 3) ? i3 + 1 : 3;
;               |   |   |   %lb_max8 = (1 <= i2) ? i2 : 1;
;               |   |   |   %ub_min9 = (3 <= %tile_e_min) ? 3 : %tile_e_min;
;               |   |   |
;               |   |   |   + DO i4 = 0, -1 * %lb_max8 + %ub_min9, 1   <DO_LOOP>
;               |   |   |   |   (%"sub1_$C6")[i4 + %lb_max8 + -1] = (%"sub1_$D2")[i4 + %lb_max8 + -1];
;               |   |   |   |   %lb_max = (1 <= i3) ? i3 : 1;
;               |   |   |   |   %ub_min = (3 <= %tile_e_min7) ? 3 : %tile_e_min7;
;               |   |   |   |
;               |   |   |   |   + DO i5 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
;               |   |   |   |   |   %add.4 = (%"sub1_$B")[i4 + %lb_max8][i5 + %lb_max]  +  1.000000e+00;
;               |   |   |   |   |   (%"sub1_$A")[i4 + %lb_max8][i5 + %lb_max] = %add.4;
;               |   |   |   |   + END LOOP
;               |   |   |   + END LOOP
;               |   |   |
;               |   |   |   %lb_max12 = (-1 <= i2) ? i2 : -1;
;               |   |   |   %ub_min13 = (0 <= %tile_e_min) ? 0 : %tile_e_min;
;               |   |   |
;               |   |   |   + DO i4 = 0, -1 * %lb_max12 + %ub_min13, 1   <DO_LOOP>
;               |   |   |   |   %add.11 = (%"sub1_$C6")[i4 + %lb_max12 + 2]  +  1.000000e+00;
;               |   |   |   |   (%"sub1_$D2")[i4 + %lb_max12 + 1] = %add.11;
;               |   |   |   |   %lb_max10 = (0 <= i3) ? i3 : 0;
;               |   |   |   |   %ub_min11 = (2 <= %tile_e_min7) ? 2 : %tile_e_min7;
;               |   |   |   |
;               |   |   |   |   + DO i5 = 0, -1 * %lb_max10 + %ub_min11, 1   <DO_LOOP>
;               |   |   |   |   |   %add.12 = (%"sub1_$A")[i4 + %lb_max12][i5 + %lb_max10]  +  2.000000e+00;
;               |   |   |   |   |   (%"sub1_$B")[i4 + %lb_max12][i5 + %lb_max10] = %add.12;
;               |   |   |   |   + END LOOP
;               |   |   |   + END LOOP
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;         END REGION

;CHECK: Function: sub1_
;
;CHECK:         BEGIN REGION { }
;CHECK:               + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch.4") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;CHECK:               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;CHECK:               |   |   (%"sub1_$C6")[i2] = (%"sub1_$D2")[i2];
;CHECK:               |   |
;CHECK:               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;CHECK:               |   |   |   %add.4 = (%"sub1_$B")[i2 + 1][i3 + 1]  +  1.000000e+00;
;CHECK:               |   |   |   (%"sub1_$A")[i2 + 1][i3 + 1] = %add.4;
;CHECK:               |   |   + END LOOP
;CHECK:               |   + END LOOP
;CHECK:               |
;CHECK:               |
;CHECK:               |   + DO i2 = 0, 1, 1   <DO_LOOP>
;CHECK:               |   |   %add.11 = (%"sub1_$C6")[i2 + 1]  +  1.000000e+00;
;CHECK:               |   |   (%"sub1_$D2")[i2] = %add.11;
;CHECK:               |   |
;CHECK:               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;CHECK:               |   |   |   %add.12 = (%"sub1_$A")[i2 + -1][i3]  +  2.000000e+00;
;CHECK:               |   |   |   (%"sub1_$B")[i2 + -1][i3] = %add.12;
;CHECK:               |   |   + END LOOP
;CHECK:               |   + END LOOP
;CHECK:               + END LOOP
;CHECK:         END REGION
;
;CHECK: Function: sub1_
;
;CHECK:         BEGIN REGION { modified }
;CHECK:               + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch.4") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <nounroll>
;CHECK:               |   + DO i2 = -1, 3, 2   <DO_LOOP>
;CHECK:               |   |   [[TILE_1:%tile_e_min[0-9]*]] = (i2 + 1 <= 3) ? i2 + 1 : 3;
;CHECK:               |   |
;CHECK:               |   |   + DO i3 = 0, 3, 2   <DO_LOOP>
;CHECK:               |   |   |   [[TILE_2:%tile_e_min[0-9]+]] = (i3 + 1 <= 3) ? i3 + 1 : 3;
;CHECK:               |   |   |   [[LBMAX:%lb_max[0-9]+]] = (1 <= i2) ? i2 : 1;
;CHECK:               |   |   |   [[UBMIN:%ub_min[0-9]+]] = (3 <= [[TILE_1]]) ? 3 : [[TILE_1]];
;CHECK:               |   |   |
;CHECK:               |   |   |   + DO i4 = 0, -1 * [[LBMAX]] + [[UBMIN]], 1   <DO_LOOP>
;CHECK:               |   |   |   |   (%"sub1_$C6")[i4 + [[LBMAX]] + -1] = (%"sub1_$D2")[i4 + [[LBMAX]] + -1];
;CHECK:               |   |   |   |   [[LBMAX_2:%lb_max[0-9]*]] = (1 <= i3) ? i3 : 1;
;CHECK:               |   |   |   |   [[UBMIN_2:%ub_min[0-9]*]] = (3 <= [[TILE_2]]) ? 3 : [[TILE_2]];
;CHECK:               |   |   |   |
;CHECK:               |   |   |   |   + DO i5 = 0, -1 * [[LBMAX_2]] + [[UBMIN_2]], 1   <DO_LOOP>
;CHECK:               |   |   |   |   |   %add.4 = (%"sub1_$B")[i4 + [[LBMAX]]][i5 + [[LBMAX_2]]]  +  1.000000e+00;
;CHECK:               |   |   |   |   |   (%"sub1_$A")[i4 + [[LBMAX]]][i5 + [[LBMAX_2]]] = %add.4;
;CHECK:               |   |   |   |   + END LOOP
;CHECK:               |   |   |   + END LOOP
;CHECK:               |   |   |
;CHECK:               |   |   |   [[LBMAX_3:%lb_max[0-9]+]] = (-1 <= i2) ? i2 : -1;
;CHECK:               |   |   |   [[UBMIN_3:%ub_min[0-9]+]] = (0 <= [[TILE_1]]) ? 0 : [[TILE_1]];
;CHECK:               |   |   |
;CHECK:               |   |   |   + DO i4 = 0, -1 * [[LBMAX_3]] + [[UBMIN_3]], 1   <DO_LOOP>
;CHECK:               |   |   |   |   %add.11 = (%"sub1_$C6")[i4 + [[LBMAX_3]] + 2]  +  1.000000e+00;
;CHECK:               |   |   |   |   (%"sub1_$D2")[i4 + [[LBMAX_3]] + 1] = %add.11;
;CHECK:               |   |   |   |   [[LBMAX_4:%lb_max[0-9]+]] = (0 <= i3) ? i3 : 0;
;CHECK:               |   |   |   |   [[UBMIN_4:%ub_min[0-9]+]] = (2 <= [[TILE_2]]) ? 2 : [[TILE_2]];
;CHECK:               |   |   |   |
;CHECK:               |   |   |   |   + DO i5 = 0, -1 * [[LBMAX_4]] + [[UBMIN_4]], 1   <DO_LOOP>
;CHECK:               |   |   |   |   |   %add.12 = (%"sub1_$A")[i4 + [[LBMAX_3]]][i5 + [[LBMAX_4]]]  +  2.000000e+00;
;CHECK:               |   |   |   |   |   (%"sub1_$B")[i4 + [[LBMAX_3]]][i5 + [[LBMAX_4]]] = %add.12;
;CHECK:               |   |   |   |   + END LOOP
;CHECK:               |   |   |   + END LOOP
;CHECK:               |   |   + END LOOP
;CHECK:               |   + END LOOP
;CHECK:               + END LOOP
;CHECK:         END REGION


;Module Before HIR
; ModuleID = 'no-perfect-loop.f90'
source_filename = "no-perfect-loop.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub1_(double* noalias nocapture dereferenceable(8) %"sub1_$A", double* noalias nocapture dereferenceable(8) %"sub1_$B", i32* noalias nocapture readonly dereferenceable(4) %"sub1_$N", i32* noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$N_fetch.3" = load i32, i32* %"sub1_$N", align 1, !tbaa !0
  %int_sext = sext i32 %"sub1_$N_fetch.3" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %"sub1_$D2" = alloca double, i64 %slct.1, align 8
  %"sub1_$C6" = alloca double, i64 %slct.1, align 8
  %mul.3 = shl nsw i64 %int_sext, 3
  %"sub1_$NTIMES_fetch.4" = load i32, i32* %"sub1_$NTIMES", align 1, !tbaa !4
  %rel.3 = icmp slt i32 %"sub1_$NTIMES_fetch.4", 1
  br i1 %rel.3, label %bb2, label %bb1.preheader

bb1.preheader:                                    ; preds = %alloca_0
  %int_sext851 = zext i32 %"sub1_$NTIMES_fetch.4" to i64
  %0 = add nuw nsw i64 %int_sext851, 1
  br label %bb1

bb1:                                              ; preds = %bb1.preheader, %bb16
  %"sub1_$K.0" = phi i64 [ %add.15, %bb16 ], [ 1, %bb1.preheader ]
  br label %bb5

bb5:                                              ; preds = %bb12, %bb1
  %"sub1_$I.0" = phi i64 [ 1, %bb1 ], [ %add.2, %bb12 ]
  %"sub1_$D2[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$D2", i64 %"sub1_$I.0")
  %"sub1_$D2[]_fetch.7" = load double, double* %"sub1_$D2[]", align 1, !tbaa !6
  %"sub1_$C6[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$C6", i64 %"sub1_$I.0")
  store double %"sub1_$D2[]_fetch.7", double* %"sub1_$C6[]", align 1, !tbaa !8
  %add.2 = add nuw nsw i64 %"sub1_$I.0", 1
  %"sub1_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* elementtype(double) nonnull %"sub1_$B", i64 %add.2)
  %"sub1_$A[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* elementtype(double) nonnull %"sub1_$A", i64 %add.2)
  br label %bb9

bb9:                                              ; preds = %bb9, %bb5
  %"sub1_$J.0" = phi i64 [ 1, %bb5 ], [ %add.1, %bb9 ]
  %add.1 = add nuw nsw i64 %"sub1_$J.0", 1
  %"sub1_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$B[]", i64 %add.1)
  %"sub1_$B[][]_fetch.15" = load double, double* %"sub1_$B[][]", align 1, !tbaa !10
  %add.4 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch.15", 1.000000e+00
  %"sub1_$A[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$A[]", i64 %add.1)
  store double %add.4, double* %"sub1_$A[][]", align 1, !tbaa !12
  %exitcond.not = icmp eq i64 %add.1, 4
  br i1 %exitcond.not, label %bb12, label %bb9

bb12:                                             ; preds = %bb9
  %exitcond48.not = icmp eq i64 %add.2, 4
  br i1 %exitcond48.not, label %bb13.preheader, label %bb5

bb13.preheader:                                   ; preds = %bb12
  br label %bb13

bb13:                                             ; preds = %bb13.preheader, %bb20
  %rel.7 = phi i1 [ false, %bb20 ], [ true, %bb13.preheader ]
  %"sub1_$I.1" = phi i64 [ 2, %bb20 ], [ 1, %bb13.preheader ]
  %add.10 = add nuw nsw i64 %"sub1_$I.1", 1
  %"sub1_$C6[]13" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$C6", i64 %add.10)
  %"sub1_$C6[]_fetch.27" = load double, double* %"sub1_$C6[]13", align 1, !tbaa !8
  %add.11 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$C6[]_fetch.27", 1.000000e+00
  %"sub1_$D2[]14" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$D2", i64 %"sub1_$I.1")
  store double %add.11, double* %"sub1_$D2[]14", align 1, !tbaa !6
  %sub.2 = add nsw i64 %"sub1_$I.1", -1
  %"sub1_$A[]15" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* elementtype(double) nonnull %"sub1_$A", i64 %sub.2)
  %"sub1_$B[]17" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* elementtype(double) nonnull %"sub1_$B", i64 %sub.2)
  br label %bb17

bb17:                                             ; preds = %bb17, %bb13
  %"sub1_$J.1" = phi i64 [ 1, %bb13 ], [ %add.13, %bb17 ]
  %"sub1_$A[][]16" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$A[]15", i64 %"sub1_$J.1")
  %"sub1_$A[][]_fetch.33" = load double, double* %"sub1_$A[][]16", align 1, !tbaa !12
  %add.12 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch.33", 2.000000e+00
  %"sub1_$B[][]18" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub1_$B[]17", i64 %"sub1_$J.1")
  store double %add.12, double* %"sub1_$B[][]18", align 1, !tbaa !10
  %add.13 = add nuw nsw i64 %"sub1_$J.1", 1
  %exitcond49.not = icmp eq i64 %add.13, 4
  br i1 %exitcond49.not, label %bb20, label %bb17

bb20:                                             ; preds = %bb17
  br i1 %rel.7, label %bb13, label %bb16

bb16:                                             ; preds = %bb20
  %add.15 = add nuw nsw i64 %"sub1_$K.0", 1
  %exitcond50 = icmp eq i64 %add.15, %0
  br i1 %exitcond50, label %bb2.loopexit, label %bb1, !llvm.loop !14

bb2.loopexit:                                     ; preds = %bb16
  br label %bb2

bb2:                                              ; preds = %bb2.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"Simple Fortran Alias Analysis 1"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$2", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$5", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$6", !2, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$8", !2, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$9", !2, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.unroll.disable"}
