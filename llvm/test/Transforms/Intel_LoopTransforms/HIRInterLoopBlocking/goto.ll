; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -hir-inter-loop-blocking-force-test -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Make sure gotos and their targets are outside of bystrip loops.

; CHECK: Function: sub1_

; CHECK:          BEGIN REGION { }
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          if
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          else
; CHECK:          if
; CHECK:          goto
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          END REGION

; CHECK: Function: sub1_

; CHECK:          BEGIN REGION { modified }
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          DO i3
; CHECK:          DO i4
; CHECK:          DO i3
; CHECK:          DO i4
; CHECK:          if
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          else
; CHECK:          if
; CHECK:          goto
; CHECK:          DO i1
; CHECK:          DO i2
; CHECK:          END REGION

; *** IR Dump Before HIR Spatial blocking over multiple loopnests (hir-inter-loop-blocking) ***
;
; <0>          BEGIN REGION { }
; <2>                if (%"sub1_$K_fetch.2" < %"sub1_$NTIMES_fetch.3")
; <2>                {
; <159>                 + DO i1 = 0, 2, 1   <DO_LOOP> <nounroll>
; <160>                 |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <16>                  |   |   %add.2 = (%"sub1_$B")[i1][i2]  +  1.000000e+00;
; <18>                  |   |   (%"sub1_$A")[i1][i2] = %add.2;
; <160>                 |   + END LOOP
; <159>                 + END LOOP
; <159>
; <161>
; <161>                 + DO i1 = 0, 1, 1   <DO_LOOP>
; <162>                 |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <44>                  |   |   %add.6 = (%"sub1_$A")[i1][i2]  +  2.000000e+00;
; <46>                  |   |   (%"sub1_$B")[i1][i2] = %add.6;
; <162>                 |   + END LOOP
; <161>                 + END LOOP
; <161>
; <60>                  %"sub1_$X_fetch.34" = (%"sub1_$X")[0];
; <64>                  if (%"sub1_$X_fetch.34" == 0 && %"sub1_$NTIMES_fetch.3" > 100)
; <64>                  {
; <163>                    + DO i1 = 0, 1, 1   <DO_LOOP>
; <164>                    |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <78>                     |   |   %add.9 = (%"sub1_$B")[i1][i2]  +  2.000000e+00;
; <79>                     |   |   (%"sub1_$B")[i1][i2] = %add.9;
; <164>                    |   + END LOOP
; <163>                    + END LOOP
; <64>                  }
; <64>                  else
; <64>                  {
; <96>                     if (%"sub1_$X_fetch.34" == 10)
; <96>                     {
; <97>                        goto bb1;
; <96>                     }
; <165>
; <165>                    + DO i1 = 0, 1, 1   <DO_LOOP>
; <166>                    |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <113>                    |   |   %add.12 = (%"sub1_$A")[i1][i2]  +  (%"sub1_$B")[i1][i2];
; <114>                    |   |   (%"sub1_$A")[i1][i2] = %add.12;
; <166>                    |   + END LOOP
; <165>                    + END LOOP
; <64>                  }
; <135>                 %mul.5 = (%"sub1_$B")[sext.i32.i64(%"sub1_$N_fetch.1") + -2][sext.i32.i64(%"sub1_$N_fetch.1") + -2]  *  5.000000e-01;
; <138>                 (%"sub1_$B")[14][14] = %mul.5;
; <140>                 bb1:
; <142>                 (%"(&)val$")[0][0] = 56;
; <144>                 (%"(&)val$")[0][1] = 4;
; <146>                 (%"(&)val$")[0][2] = 1;
; <148>                 (%"(&)val$")[0][3] = 0;
; <150>                 (%argblock)[0].0 = 25;
; <152>                 (%argblock)[0].1 = &((@strlit)[0][0]);
; <155>                 %func_result = @for_write_seq_lis(&((i8*)(%"$io_ctx")[0]),  -1,  1239157112576,  &((%"(&)val$")[0][0]),  &((i8*)(%argblock)[0]));
; <2>                }
; <158>              ret ;
; <0>          END REGION

; *** IR Dump After HIR Spatial blocking over multiple loopnests (hir-inter-loop-blocking) ***
; Function: sub1_
;
; <0>          BEGIN REGION { modified }
; <2>                if (%"sub1_$K_fetch.2" < %"sub1_$NTIMES_fetch.3")
; <2>                {
; <167>                 + DO i1 = 0, 2, 2   <DO_LOOP>
; <168>                 |   %tile_e_min = (i1 + 1 <= 2) ? i1 + 1 : 2;
; <169>                 |
; <169>                 |   + DO i2 = 0, 2, 2   <DO_LOOP>
; <170>                 |   |   %tile_e_min17 = (i2 + 1 <= 2) ? i2 + 1 : 2;
; <174>                 |   |   %lb_max18 = (0 <= i1) ? i1 : 0;
; <175>                 |   |   %ub_min19 = (2 <= %tile_e_min) ? 2 : %tile_e_min;
; <159>                 |   |
; <159>                 |   |   + DO i3 = 0, -1 * %lb_max18 + %ub_min19, 1   <DO_LOOP> <nounroll>
; <171>                 |   |   |   %lb_max = (0 <= i2) ? i2 : 0;
; <172>                 |   |   |   %ub_min = (2 <= %tile_e_min17) ? 2 : %tile_e_min17;
; <160>                 |   |   |
; <160>                 |   |   |   + DO i4 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; <16>                  |   |   |   |   %add.2 = (%"sub1_$B")[i3 + %lb_max18][i4 + %lb_max]  +  1.000000e+00;
; <18>                  |   |   |   |   (%"sub1_$A")[i3 + %lb_max18][i4 + %lb_max] = %add.2;
; <160>                 |   |   |   + END LOOP
; <159>                 |   |   + END LOOP
; <159>                 |   |
; <180>                 |   |   %lb_max22 = (0 <= i1) ? i1 : 0;
; <181>                 |   |   %ub_min23 = (1 <= %tile_e_min) ? 1 : %tile_e_min;
; <161>                 |   |
; <161>                 |   |   + DO i3 = 0, -1 * %lb_max22 + %ub_min23, 1   <DO_LOOP>
; <177>                 |   |   |   %lb_max20 = (0 <= i2) ? i2 : 0;
; <178>                 |   |   |   %ub_min21 = (2 <= %tile_e_min17) ? 2 : %tile_e_min17;
; <162>                 |   |   |
; <162>                 |   |   |   + DO i4 = 0, -1 * %lb_max20 + %ub_min21, 1   <DO_LOOP>
; <44>                  |   |   |   |   %add.6 = (%"sub1_$A")[i3 + %lb_max22][i4 + %lb_max20]  +  2.000000e+00;
; <46>                  |   |   |   |   (%"sub1_$B")[i3 + %lb_max22][i4 + %lb_max20] = %add.6;
; <162>                 |   |   |   + END LOOP
; <161>                 |   |   + END LOOP
; <169>                 |   + END LOOP
; <167>                 + END LOOP
; <167>
; <60>                  %"sub1_$X_fetch.34" = (%"sub1_$X")[0];
; <64>                  if (%"sub1_$X_fetch.34" == 0 && %"sub1_$NTIMES_fetch.3" > 100)
; <64>                  {
; <163>                    + DO i1 = 0, 1, 1   <DO_LOOP>
; <164>                    |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <78>                     |   |   %add.9 = (%"sub1_$B")[i1][i2]  +  2.000000e+00;
; <79>                     |   |   (%"sub1_$B")[i1][i2] = %add.9;
; <164>                    |   + END LOOP
; <163>                    + END LOOP
; <64>                  }
; <64>                  else
; <64>                  {
; <96>                     if (%"sub1_$X_fetch.34" == 10)
; <96>                     {
; <97>                        goto bb1;
; <96>                     }
; <165>
; <165>                    + DO i1 = 0, 1, 1   <DO_LOOP>
; <166>                    |   + DO i2 = 0, 2, 1   <DO_LOOP>
; <113>                    |   |   %add.12 = (%"sub1_$A")[i1][i2]  +  (%"sub1_$B")[i1][i2];
; <114>                    |   |   (%"sub1_$A")[i1][i2] = %add.12;
; <166>                    |   + END LOOP
; <165>                    + END LOOP
; <64>                  }
; <135>                 %mul.5 = (%"sub1_$B")[sext.i32.i64(%"sub1_$N_fetch.1") + -2][sext.i32.i64(%"sub1_$N_fetch.1") + -2]  *  5.000000e-01;
; <138>                 (%"sub1_$B")[14][14] = %mul.5;
; <140>                 bb1:
; <142>                 (%"(&)val$")[0][0] = 56;
; <144>                 (%"(&)val$")[0][1] = 4;
; <146>                 (%"(&)val$")[0][2] = 1;
; <148>                 (%"(&)val$")[0][3] = 0;
; <150>                 (%argblock)[0].0 = 25;
; <152>                 (%argblock)[0].1 = &((@strlit)[0][0]);
; <155>                 %func_result = @for_write_seq_lis(&((i8*)(%"$io_ctx")[0]),  -1,  1239157112576,  &((%"(&)val$")[0][0]),  &((i8*)(%argblock)[0]));
; <2>                }
; <158>              ret ;
; <0>          END REGION


;Module Before HIR
; ModuleID = 'two-dim-if-goto.f90'
source_filename = "two-dim-if-goto.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit = internal unnamed_addr constant [25 x i8] c"Loop exited at data point"

; Function Attrs: nofree nounwind uwtable
define void @sub1_(ptr noalias nocapture dereferenceable(8) %"sub1_$A", ptr noalias nocapture dereferenceable(8) %"sub1_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$K", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$X") local_unnamed_addr #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { i64, ptr }, align 8
  %"sub1_$N_fetch.1" = load i32, ptr %"sub1_$N", align 1, !tbaa !0
  %int_sext = sext i32 %"sub1_$N_fetch.1" to i64
  %mul.1 = shl nsw i64 %int_sext, 3
  %"sub1_$K_fetch.2" = load i32, ptr %"sub1_$K", align 1, !tbaa !4
  %"sub1_$NTIMES_fetch.3" = load i32, ptr %"sub1_$NTIMES", align 1, !tbaa !6
  %rel.1 = icmp slt i32 %"sub1_$K_fetch.2", %"sub1_$NTIMES_fetch.3"
  br i1 %rel.1, label %bb3.preheader, label %bb39_endif

bb3.preheader:                                    ; preds = %alloca_0
  br label %bb3

bb3:                                              ; preds = %bb3.preheader, %bb10
  %indvars.iv120 = phi i64 [ %indvars.iv.next121, %bb10 ], [ 1, %bb3.preheader ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 %indvars.iv120)
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$A", i64 %indvars.iv120)
  br label %bb7

bb7:                                              ; preds = %bb7, %bb3
  %indvars.iv117 = phi i64 [ %indvars.iv.next118, %bb7 ], [ 1, %bb3 ]
  %"sub1_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]", i64 %indvars.iv117)
  %"sub1_$B[][]_fetch.10" = load double, ptr %"sub1_$B[][]", align 1, !tbaa !8
  %add.2 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch.10", 1.000000e+00
  %"sub1_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]", i64 %indvars.iv117)
  store double %add.2, ptr %"sub1_$A[][]", align 1, !tbaa !10
  %indvars.iv.next118 = add nuw nsw i64 %indvars.iv117, 1
  %exitcond119.not = icmp eq i64 %indvars.iv.next118, 4
  br i1 %exitcond119.not, label %bb10, label %bb7

bb10:                                             ; preds = %bb7
  %indvars.iv.next121 = add nuw nsw i64 %indvars.iv120, 1
  %exitcond122.not = icmp eq i64 %indvars.iv.next121, 4
  br i1 %exitcond122.not, label %bb11.preheader, label %bb3, !llvm.loop !12

bb11.preheader:                                   ; preds = %bb10
  br label %bb11

bb11:                                             ; preds = %bb11.preheader, %bb18
  %rel.5 = phi i1 [ false, %bb18 ], [ true, %bb11.preheader ]
  %"sub1_$I.1" = phi i64 [ 2, %bb18 ], [ 1, %bb11.preheader ]
  %"sub1_$A[]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$B[]14" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  br label %bb15

bb15:                                             ; preds = %bb15, %bb11
  %indvars.iv114 = phi i64 [ %indvars.iv.next115, %bb15 ], [ 1, %bb11 ]
  %"sub1_$A[][]11" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]10", i64 %indvars.iv114)
  %"sub1_$A[][]_fetch.25" = load double, ptr %"sub1_$A[][]11", align 1, !tbaa !10
  %add.6 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch.25", 2.000000e+00
  %"sub1_$B[][]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]14", i64 %indvars.iv114)
  store double %add.6, ptr %"sub1_$B[][]15", align 1, !tbaa !8
  %indvars.iv.next115 = add nuw nsw i64 %indvars.iv114, 1
  %exitcond116.not = icmp eq i64 %indvars.iv.next115, 4
  br i1 %exitcond116.not, label %bb18, label %bb15

bb18:                                             ; preds = %bb15
  br i1 %rel.5, label %bb11, label %bb14

bb14:                                             ; preds = %bb18
  %"sub1_$X_fetch.34" = load i32, ptr %"sub1_$X", align 1, !tbaa !14
  %rel.6 = icmp eq i32 %"sub1_$X_fetch.34", 0
  %rel.7 = icmp sgt i32 %"sub1_$NTIMES_fetch.3", 100
  %and.1 = and i1 %rel.6, %rel.7
  br i1 %and.1, label %bb19.preheader, label %bb_new15_else

bb19.preheader:                                   ; preds = %bb14
  br label %bb19

bb19:                                             ; preds = %bb19.preheader, %bb26
  %rel.9 = phi i1 [ false, %bb26 ], [ true, %bb19.preheader ]
  %"sub1_$I.2" = phi i64 [ 2, %bb26 ], [ 1, %bb19.preheader ]
  %"sub1_$B[]24" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.2")
  br label %bb23

bb23:                                             ; preds = %bb23, %bb19
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb23 ], [ 1, %bb19 ]
  %"sub1_$B[][]25" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]24", i64 %indvars.iv)
  %"sub1_$B[][]_fetch.40" = load double, ptr %"sub1_$B[][]25", align 1, !tbaa !8
  %add.9 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch.40", 2.000000e+00
  store double %add.9, ptr %"sub1_$B[][]25", align 1, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %bb26, label %bb23

bb26:                                             ; preds = %bb23
  br i1 %rel.9, label %bb19, label %bb36_endif.loopexit

bb27:                                             ; preds = %bb27.preheader, %bb34
  %rel.12 = phi i1 [ false, %bb34 ], [ true, %bb27.preheader ]
  %"sub1_$I.3" = phi i64 [ 2, %bb34 ], [ 1, %bb27.preheader ]
  %"sub1_$A[]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.3")
  %"sub1_$B[]36" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.3")
  br label %bb31

bb31:                                             ; preds = %bb31, %bb27
  %indvars.iv111 = phi i64 [ %indvars.iv.next112, %bb31 ], [ 1, %bb27 ]
  %"sub1_$A[][]33" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]32", i64 %indvars.iv111)
  %"sub1_$A[][]_fetch.54" = load double, ptr %"sub1_$A[][]33", align 1, !tbaa !10
  %"sub1_$B[][]37" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]36", i64 %indvars.iv111)
  %"sub1_$B[][]_fetch.59" = load double, ptr %"sub1_$B[][]37", align 1, !tbaa !8
  %add.12 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[][]_fetch.54", %"sub1_$B[][]_fetch.59"
  store double %add.12, ptr %"sub1_$A[][]33", align 1, !tbaa !10
  %indvars.iv.next112 = add nuw nsw i64 %indvars.iv111, 1
  %exitcond113.not = icmp eq i64 %indvars.iv.next112, 4
  br i1 %exitcond113.not, label %bb34, label %bb31

bb34:                                             ; preds = %bb31
  br i1 %rel.12, label %bb27, label %bb36_endif.loopexit124

bb_new15_else:                                    ; preds = %bb14
  %rel.10 = icmp eq i32 %"sub1_$X_fetch.34", 10
  br i1 %rel.10, label %bb1, label %bb27.preheader

bb27.preheader:                                   ; preds = %bb_new15_else
  br label %bb27

bb36_endif.loopexit:                              ; preds = %bb26
  br label %bb36_endif

bb36_endif.loopexit124:                           ; preds = %bb34
  br label %bb36_endif

bb36_endif:                                       ; preds = %bb36_endif.loopexit124, %bb36_endif.loopexit
  %sub.4 = add nsw i32 %"sub1_$N_fetch.1", -1
  %int_sext47 = sext i32 %sub.4 to i64
  %"sub1_$B[]49" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 %int_sext47)
  %"sub1_$B[][]50" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]49", i64 %int_sext47)
  %"sub1_$B[][]_fetch.72" = load double, ptr %"sub1_$B[][]50", align 1, !tbaa !8
  %mul.5 = fmul reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch.72", 5.000000e-01
  %"sub1_$B[]51" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub1_$B", i64 15)
  %"sub1_$B[][]52" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]51", i64 15)
  store double %mul.5, ptr %"sub1_$B[][]52", align 1, !tbaa !8
  br label %bb1

bb1:                                              ; preds = %bb_new15_else, %bb36_endif
  %.fca.0.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 0
  store i8 56, ptr %.fca.0.gep, align 1, !tbaa !16
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 1
  store i8 4, ptr %.fca.1.gep, align 1, !tbaa !16
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 2
  store i8 1, ptr %.fca.2.gep, align 1, !tbaa !16
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 3
  store i8 0, ptr %.fca.3.gep, align 1, !tbaa !16
  %"argblock.field_0$" = getelementptr inbounds { i64, ptr }, ptr %argblock, i64 0, i32 0
  store i64 25, ptr %"argblock.field_0$", align 8, !tbaa !17
  %"argblock.field_1$" = getelementptr inbounds { i64, ptr }, ptr %argblock, i64 0, i32 1
  store ptr @strlit, ptr %"argblock.field_1$", align 8, !tbaa !19
  %"(ptr)$io_ctx$" = bitcast ptr %"$io_ctx" to ptr
  %"(ptr)argblock$" = bitcast ptr %argblock to ptr
  %func_result = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"(ptr)$io_ctx$", i32 -1, i64 1239157112576, ptr nonnull %.fca.0.gep, ptr nonnull %"(ptr)argblock$") #3
  br label %bb39_endif

bb39_endif:                                       ; preds = %alloca_0, %bb1
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree
declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"Simple Fortran Alias Analysis 1"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$2", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$3", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$6", !2, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$7", !2, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.unroll.disable"}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$8", !2, i64 0}
!16 = !{!2, !2, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"ifx$unique_sym$10", !2, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"ifx$unique_sym$11", !2, i64 0}
