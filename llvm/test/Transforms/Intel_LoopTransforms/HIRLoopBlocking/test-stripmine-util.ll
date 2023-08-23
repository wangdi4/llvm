; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking" -print-after=hir-loop-blocking -print-before=hir-loop-blocking -aa-pipeline="basic-aa" -hir-loop-blocking-skip-anti-pattern-check -disable-output 2>&1 < %s | FileCheck %s

; Verify that the transformations end successfully

; CHECK: Function: xlm_apply_y_rotation_
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %"xlm_apply_y_rotation_$L_fetch.1784", 1   <DO_LOOP>  <MAX_TC_EST = 31>
; CHECK:               |   + DO i2 = 0, %"xlm_apply_y_rotation_$L_fetch.1784", 1   <DO_LOOP>  <MAX_TC_EST = 961>
; CHECK:               |   |   %"xlm_apply_y_rotation_$QQ[]_fetch.1823" = (@"xlm_apply_y_rotation_$QQ")[0][i1 + 30];
; CHECK:               |   |   %mul.295 = (@"xlm_apply_y_rotation_$DP")[0][(1 + %"xlm_apply_y_rotation_$L_fetch.1784") * i1 + i2]  *  %"xlm_apply_y_rotation_$QQ[]_fetch.1823";
; CHECK:               |   |   %add.278 = (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][i2]  +  %mul.295;
; CHECK:               |   |   (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][i2] = %add.278;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION
;
; CHECK: Function: xlm_apply_y_rotation_
;
; CHECK:         BEGIN REGION { modified }
; CHECK:               + DO i1 = 0, (%"xlm_apply_y_rotation_$L_fetch.1784")/u64, 1   <DO_LOOP>  <MAX_TC_EST = 31>
; CHECK:               |   %min = (-64 * i1 + %"xlm_apply_y_rotation_$L_fetch.1784" <= 63) ? -64 * i1 + %"xlm_apply_y_rotation_$L_fetch.1784" : 63;
; CHECK:               |
; CHECK:               |   + DO i2 = 0, (%"xlm_apply_y_rotation_$L_fetch.1784")/u64, 1   <DO_LOOP>  <MAX_TC_EST = 961>
; CHECK:               |   |   %min3 = (-64 * i2 + %"xlm_apply_y_rotation_$L_fetch.1784" <= 63) ? -64 * i2 + %"xlm_apply_y_rotation_$L_fetch.1784" : 63;
; CHECK:               |   |
; CHECK:               |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 31>
; CHECK:               |   |   |   + DO i4 = 0, %min3, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:               |   |   |   |   %"xlm_apply_y_rotation_$QQ[]_fetch.1823" = (@"xlm_apply_y_rotation_$QQ")[0][64 * i1 + i3 + 30];
; CHECK:               |   |   |   |   %mul.295 = (@"xlm_apply_y_rotation_$DP")[0][64 * (1 + %"xlm_apply_y_rotation_$L_fetch.1784") * i1 + 64 * i2 + (1 + %"xlm_apply_y_rotation_$L_fetch.1784") * i3 + i4]  *  %"xlm_apply_y_rotation_$QQ[]_fetch.1823";
; CHECK:               |   |   |   |   %add.278 = (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][64 * i2 + i4]  +  %mul.295;
; CHECK:               |   |   |   |   (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][64 * i2 + i4] = %add.278;
; CHECK:               |   |   |   + END LOOP
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


source_filename = "/tmp/ifxRlySTi.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ncomxlm_ = external unnamed_addr global [458560 x i8], align 32
@"xlm_apply_y_rotation_$QQ" = external hidden unnamed_addr global [61 x double], align 16
@"xlm_apply_y_rotation_$DM" = external hidden global [900 x double], align 16
@"xlm_apply_y_rotation_$DP" = external hidden global [961 x double], align 16

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind uwtable
define void @xlm_apply_y_rotation_(ptr noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$LMAX", ptr noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$BETA", ptr noalias nocapture dereferenceable(8) %"xlm_apply_y_rotation_$Q", ptr noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$LQ") local_unnamed_addr #1 {
alloca_13:
  %"xlm_apply_y_rotation_$L" = alloca i64, align 8
  %"xlm_apply_y_rotation_$LQ_fetch.1780" = load i64, ptr %"xlm_apply_y_rotation_$LQ", align 1
  %neg.214 = sub nsw i64 0, %"xlm_apply_y_rotation_$LQ_fetch.1780"
  %sub.197 = shl i64 %"xlm_apply_y_rotation_$LQ_fetch.1780", 4
  %add.274 = or i64 %sub.197, 8
  %"xlm_apply_y_rotation_$LMAX_fetch.1782" = load i64, ptr %"xlm_apply_y_rotation_$LMAX", align 1
  store i64 0, ptr %"xlm_apply_y_rotation_$L", align 8
  %rel.141 = icmp slt i64 %"xlm_apply_y_rotation_$LMAX_fetch.1782", 0
  br i1 %rel.141, label %bb396, label %bb395.preheader

bb395.preheader:                                  ; preds = %alloca_13
  br label %bb395

bb395:                                            ; preds = %bb426, %bb395.preheader
  call void @xlm_y_rotation_matrix_(ptr nonnull %"xlm_apply_y_rotation_$L", ptr nonnull %"xlm_apply_y_rotation_$BETA", ptr @"xlm_apply_y_rotation_$DP", ptr @"xlm_apply_y_rotation_$DM") #2
  %"xlm_apply_y_rotation_$L_fetch.1784" = load i64, ptr %"xlm_apply_y_rotation_$L", align 8
  %neg.213 = sub nsw i64 0, %"xlm_apply_y_rotation_$L_fetch.1784"
  %rel.142 = icmp slt i64 %"xlm_apply_y_rotation_$L_fetch.1784", 0
  br i1 %rel.142, label %bb395.bb426_crit_edge, label %bb403.preheader

bb395.bb426_crit_edge:                            ; preds = %bb395
  %.pre = add nsw i64 %"xlm_apply_y_rotation_$L_fetch.1784", 1
  br label %bb426

bb403.preheader:                                  ; preds = %bb395
  %"xlm_apply_y_rotation_$Q[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %add.274, ptr elementtype(double) nonnull %"xlm_apply_y_rotation_$Q", i64 %"xlm_apply_y_rotation_$L_fetch.1784")
  %"val$[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 488, ptr elementtype(double) getelementptr inbounds ([458560 x i8], ptr @ncomxlm_, i64 0, i64 163480), i64 %"xlm_apply_y_rotation_$L_fetch.1784")
  %0 = add i64 %"xlm_apply_y_rotation_$L_fetch.1784", 1
  br label %bb407.preheader

bb407.preheader:                                  ; preds = %bb403.preheader
  br label %bb411.preheader

bb411.preheader:                                  ; preds = %bb412, %bb407.preheader
  %"xlm_apply_y_rotation_$MP.0" = phi i64 [ %add.281, %bb412 ], [ 0, %bb407.preheader ]
  %"xlm_apply_y_rotation_$IND.0" = phi i64 [ %1, %bb412 ], [ 1, %bb407.preheader ]
  %"xlm_apply_y_rotation_$QQ[]5" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 -30, i64 8, ptr elementtype(double) @"xlm_apply_y_rotation_$QQ", i64 %"xlm_apply_y_rotation_$MP.0")
  %"xlm_apply_y_rotation_$QQ[]_fetch.1823" = load double, ptr %"xlm_apply_y_rotation_$QQ[]5", align 1
  br label %bb411

bb411:                                            ; preds = %bb411, %bb411.preheader
  %"xlm_apply_y_rotation_$M.1" = phi i64 [ %add.280, %bb411 ], [ 0, %bb411.preheader ]
  %"xlm_apply_y_rotation_$IND.1" = phi i64 [ %add.279, %bb411 ], [ %"xlm_apply_y_rotation_$IND.0", %bb411.preheader ]
  %"xlm_apply_y_rotation_$Q[][]4" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %neg.214, i64 8, ptr elementtype(double) nonnull %"xlm_apply_y_rotation_$Q[]", i64 %"xlm_apply_y_rotation_$M.1")
  %"xlm_apply_y_rotation_$Q[][]_fetch.1819" = load double, ptr %"xlm_apply_y_rotation_$Q[][]4", align 1
  %"xlm_apply_y_rotation_$DP[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"xlm_apply_y_rotation_$DP", i64 %"xlm_apply_y_rotation_$IND.1")
  %"xlm_apply_y_rotation_$DP[]_fetch.1821" = load double, ptr %"xlm_apply_y_rotation_$DP[]", align 1
  %mul.295 = fmul reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$DP[]_fetch.1821", %"xlm_apply_y_rotation_$QQ[]_fetch.1823"
  %add.278 = fadd reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$Q[][]_fetch.1819", %mul.295
  store double %add.278, ptr %"xlm_apply_y_rotation_$Q[][]4", align 1
  %add.279 = add nsw i64 %"xlm_apply_y_rotation_$IND.1", 1
  %add.280 = add nuw nsw i64 %"xlm_apply_y_rotation_$M.1", 1
  %exitcond84 = icmp eq i64 %add.280, %0
  br i1 %exitcond84, label %bb412, label %bb411

bb412:                                            ; preds = %bb411
  %1 = add i64 %0, %"xlm_apply_y_rotation_$IND.0"
  %add.281 = add nuw nsw i64 %"xlm_apply_y_rotation_$MP.0", 1
  %exitcond85 = icmp eq i64 %add.281, %0
  br i1 %exitcond85, label %bb408, label %bb411.preheader

bb408:                                            ; preds = %bb412
  br label %bb426.loopexit

bb426.loopexit:                                   ; preds = %bb425
  br label %bb426

bb426:                                            ; preds = %bb426.loopexit, %bb395.bb426_crit_edge
  %add.287.pre-phi = phi i64 [ %.pre, %bb395.bb426_crit_edge ], [ %0, %bb426.loopexit ]
  store i64 %add.287.pre-phi, ptr %"xlm_apply_y_rotation_$L", align 8
  %rel.155.not.not = icmp slt i64 %"xlm_apply_y_rotation_$L_fetch.1784", %"xlm_apply_y_rotation_$LMAX_fetch.1782"
  br i1 %rel.155.not.not, label %bb395, label %bb396.loopexit

bb396.loopexit:                                   ; preds = %bb426
  br label %bb396

bb396:                                            ; preds = %bb396.loopexit, %alloca_13
  ret void
}

; Function Attrs: nounwind uwtable
declare void @xlm_y_rotation_matrix_(ptr noalias dereferenceable(8), ptr noalias nocapture readonly dereferenceable(8), ptr noalias dereferenceable(8), ptr noalias dereferenceable(8)) local_unnamed_addr #1

