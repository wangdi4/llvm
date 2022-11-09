; RUN: opt -intel-libirc-allowed -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-loop-blocking -print-after=hir-loop-blocking -print-before=hir-loop-blocking -hir-loop-blocking-skip-anti-pattern-check < %s 2>&1 | FileCheck %s
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking" -print-after=hir-loop-blocking -print-before=hir-loop-blocking -aa-pipeline="basic-aa" -hir-loop-blocking-skip-anti-pattern-check 2>&1 < %s | FileCheck %s

; Verify that the transformations end successfully

; CHECK: Dump Before

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %"xlm_apply_y_rotation_$L_fetch.1784", 1   <DO_LOOP>  <MAX_TC_EST = 31>
; CHECK: |   + DO i2 = 0, %"xlm_apply_y_rotation_$L_fetch.1784", 1   <DO_LOOP>  <MAX_TC_EST = 961>
; CHECK: |   |   %"xlm_apply_y_rotation_$QQ[]_fetch.1823" = (@"xlm_apply_y_rotation_$QQ")[0][i1 + 30];
; CHECK: |   |   %mul.295 = (@"xlm_apply_y_rotation_$DP")[0][(1 + %"xlm_apply_y_rotation_$L_fetch.1784") * i1 + i2]  *  %"xlm_apply_y_rotation_$QQ[]_fetch.1823";
; CHECK: |   |   %add.278 = (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][i2]  +  %mul.295;
; CHECK: |   |   (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][i2] = %add.278;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK:  Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, (%"xlm_apply_y_rotation_$L_fetch.1784" + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 61>
; CHECK: |   %min6 = (-64 * i1 + %"xlm_apply_y_rotation_$L_fetch.1784" + -1 <= 63) ? -64 * i1 + %"xlm_apply_y_rotation_$L_fetch.1784" + -1 : 63;
; CHECK: |
; CHECK: |   + DO i2 = 0, (%"xlm_apply_y_rotation_$L_fetch.1784" + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 900>
; CHECK: |   |   %min7 = (-64 * i2 + %"xlm_apply_y_rotation_$L_fetch.1784" + -1 <= 63) ? -64 * i2 + %"xlm_apply_y_rotation_$L_fetch.1784" + -1 : 63;
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, %min6, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   |   + DO i4 = 0, %min7, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   |   |   %"xlm_apply_y_rotation_$QQ[]_fetch.1848" = (@"xlm_apply_y_rotation_$QQ")[0][64 * i1 + i3 + -1 * %"xlm_apply_y_rotation_$L_fetch.1784" + 30];
; CHECK: |   |   |   |   %mul.296 = (@"xlm_apply_y_rotation_$DM")[0][64 * %"xlm_apply_y_rotation_$L_fetch.1784" * i1 + 64 * i2 + %"xlm_apply_y_rotation_$L_fetch.1784" * i3 + i4]  *  %"xlm_apply_y_rotation_$QQ[]_fetch.1848";
; CHECK: |   |   |   |   %add.282 = (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][64 * i2 + i4 + -1 * %"xlm_apply_y_rotation_$L_fetch.1784"]  +  %mul.296;
; CHECK: |   |   |   |   (%"xlm_apply_y_rotation_$Q")[%"xlm_apply_y_rotation_$L_fetch.1784"][64 * i2 + i4 + -1 * %"xlm_apply_y_rotation_$L_fetch.1784"] = %add.282;
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

source_filename = "/tmp/ifxRlySTi.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ncomxlm_ = external unnamed_addr global [458560 x i8], align 32
@"xlm_apply_y_rotation_$QQ" = external hidden unnamed_addr global [61 x double], align 16
@"xlm_apply_y_rotation_$DM" = external hidden global [900 x double], align 16
@"xlm_apply_y_rotation_$DP" = external hidden global [961 x double], align 16

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nounwind uwtable
define void @xlm_apply_y_rotation_(i64* noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$LMAX", double* noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$BETA", double* noalias nocapture dereferenceable(8) %"xlm_apply_y_rotation_$Q", i64* noalias nocapture readonly dereferenceable(8) %"xlm_apply_y_rotation_$LQ") local_unnamed_addr #1 {
alloca_13:
  %"xlm_apply_y_rotation_$L" = alloca i64, align 8
  %"xlm_apply_y_rotation_$LQ_fetch.1780" = load i64, i64* %"xlm_apply_y_rotation_$LQ", align 1
  %neg.214 = sub nsw i64 0, %"xlm_apply_y_rotation_$LQ_fetch.1780"
  %sub.197 = shl i64 %"xlm_apply_y_rotation_$LQ_fetch.1780", 4
  %add.274 = or i64 %sub.197, 8
  %"xlm_apply_y_rotation_$LMAX_fetch.1782" = load i64, i64* %"xlm_apply_y_rotation_$LMAX", align 1
  store i64 0, i64* %"xlm_apply_y_rotation_$L", align 8
  %rel.141 = icmp slt i64 %"xlm_apply_y_rotation_$LMAX_fetch.1782", 0
  br i1 %rel.141, label %bb396, label %bb395.preheader

bb395.preheader:                                  ; preds = %alloca_13
  br label %bb395

bb395:                                            ; preds = %bb426, %bb395.preheader
  call void @xlm_y_rotation_matrix_(i64* nonnull %"xlm_apply_y_rotation_$L", double* nonnull %"xlm_apply_y_rotation_$BETA", double* getelementptr inbounds ([961 x double], [961 x double]* @"xlm_apply_y_rotation_$DP", i64 0, i64 0), double* getelementptr inbounds ([900 x double], [900 x double]* @"xlm_apply_y_rotation_$DM", i64 0, i64 0)) #2
  %"xlm_apply_y_rotation_$L_fetch.1784" = load i64, i64* %"xlm_apply_y_rotation_$L", align 8
  %neg.213 = sub nsw i64 0, %"xlm_apply_y_rotation_$L_fetch.1784"
  %rel.142 = icmp slt i64 %"xlm_apply_y_rotation_$L_fetch.1784", 0
  br i1 %rel.142, label %bb395.bb426_crit_edge, label %bb403.preheader

bb395.bb426_crit_edge:                            ; preds = %bb395
  %.pre = add nsw i64 %"xlm_apply_y_rotation_$L_fetch.1784", 1
  br label %bb426

bb403.preheader:                                  ; preds = %bb395
  %"xlm_apply_y_rotation_$Q[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 %add.274, double* elementtype(double) nonnull %"xlm_apply_y_rotation_$Q", i64 %"xlm_apply_y_rotation_$L_fetch.1784")
  %"val$[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 488, double* elementtype(double) bitcast (i8* getelementptr inbounds ([458560 x i8], [458560 x i8]* @ncomxlm_, i64 0, i64 163480) to double*), i64 %"xlm_apply_y_rotation_$L_fetch.1784")
  %0 = add i64 %"xlm_apply_y_rotation_$L_fetch.1784", 1
  br label %bb403

bb403:                                            ; preds = %bb403, %bb403.preheader
  %"xlm_apply_y_rotation_$M.0" = phi i64 [ %add.277, %bb403 ], [ %neg.213, %bb403.preheader ]
  %"xlm_apply_y_rotation_$Q[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %neg.214, i64 8, double* elementtype(double) nonnull %"xlm_apply_y_rotation_$Q[]", i64 %"xlm_apply_y_rotation_$M.0")
  %"xlm_apply_y_rotation_$Q[][]_fetch.1797" = load double, double* %"xlm_apply_y_rotation_$Q[][]", align 1
  %"val$[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 -30, i64 8, double* elementtype(double) %"val$[]", i64 %"xlm_apply_y_rotation_$M.0")
  %"val$[][]_fetch.1800" = load double, double* %"val$[][]", align 1
  %mul.294 = fmul reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$Q[][]_fetch.1797", %"val$[][]_fetch.1800"
  %"xlm_apply_y_rotation_$QQ[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 -30, i64 8, double* elementtype(double) getelementptr inbounds ([61 x double], [61 x double]* @"xlm_apply_y_rotation_$QQ", i64 0, i64 0), i64 %"xlm_apply_y_rotation_$M.0")
  store double %mul.294, double* %"xlm_apply_y_rotation_$QQ[]", align 1
  store double 0.000000e+00, double* %"xlm_apply_y_rotation_$Q[][]", align 1
  %add.277 = add nsw i64 %"xlm_apply_y_rotation_$M.0", 1
  %exitcond = icmp eq i64 %add.277, %0
  br i1 %exitcond, label %bb407.preheader, label %bb403

bb407.preheader:                                  ; preds = %bb403
  br label %bb411.preheader

bb411.preheader:                                  ; preds = %bb412, %bb407.preheader
  %"xlm_apply_y_rotation_$MP.0" = phi i64 [ %add.281, %bb412 ], [ 0, %bb407.preheader ]
  %"xlm_apply_y_rotation_$IND.0" = phi i64 [ %1, %bb412 ], [ 1, %bb407.preheader ]
  %"xlm_apply_y_rotation_$QQ[]5" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 -30, i64 8, double* elementtype(double) getelementptr inbounds ([61 x double], [61 x double]* @"xlm_apply_y_rotation_$QQ", i64 0, i64 0), i64 %"xlm_apply_y_rotation_$MP.0")
  %"xlm_apply_y_rotation_$QQ[]_fetch.1823" = load double, double* %"xlm_apply_y_rotation_$QQ[]5", align 1
  br label %bb411

bb411:                                            ; preds = %bb411, %bb411.preheader
  %"xlm_apply_y_rotation_$M.1" = phi i64 [ %add.280, %bb411 ], [ 0, %bb411.preheader ]
  %"xlm_apply_y_rotation_$IND.1" = phi i64 [ %add.279, %bb411 ], [ %"xlm_apply_y_rotation_$IND.0", %bb411.preheader ]
  %"xlm_apply_y_rotation_$Q[][]4" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %neg.214, i64 8, double* elementtype(double) nonnull %"xlm_apply_y_rotation_$Q[]", i64 %"xlm_apply_y_rotation_$M.1")
  %"xlm_apply_y_rotation_$Q[][]_fetch.1819" = load double, double* %"xlm_apply_y_rotation_$Q[][]4", align 1
  %"xlm_apply_y_rotation_$DP[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([961 x double], [961 x double]* @"xlm_apply_y_rotation_$DP", i64 0, i64 0), i64 %"xlm_apply_y_rotation_$IND.1")
  %"xlm_apply_y_rotation_$DP[]_fetch.1821" = load double, double* %"xlm_apply_y_rotation_$DP[]", align 1
  %mul.295 = fmul reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$DP[]_fetch.1821", %"xlm_apply_y_rotation_$QQ[]_fetch.1823"
  %add.278 = fadd reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$Q[][]_fetch.1819", %mul.295
  store double %add.278, double* %"xlm_apply_y_rotation_$Q[][]4", align 1
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
  %rel.148 = icmp sgt i64 %"xlm_apply_y_rotation_$L_fetch.1784", 0
  br i1 %rel.148, label %bb419.preheader.preheader, label %bb425.preheader

bb419.preheader.preheader:                        ; preds = %bb408
  br label %bb419.preheader

bb419.preheader:                                  ; preds = %bb420, %bb419.preheader.preheader
  %"xlm_apply_y_rotation_$MP.1" = phi i64 [ %add.285, %bb420 ], [ %neg.213, %bb419.preheader.preheader ]
  %"xlm_apply_y_rotation_$IND.3" = phi i64 [ %2, %bb420 ], [ 1, %bb419.preheader.preheader ]
  %"xlm_apply_y_rotation_$QQ[]10" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 -30, i64 8, double* elementtype(double) getelementptr inbounds ([61 x double], [61 x double]* @"xlm_apply_y_rotation_$QQ", i64 0, i64 0), i64 %"xlm_apply_y_rotation_$MP.1")
  %"xlm_apply_y_rotation_$QQ[]_fetch.1848" = load double, double* %"xlm_apply_y_rotation_$QQ[]10", align 1
  br label %bb419

bb419:                                            ; preds = %bb419, %bb419.preheader
  %"xlm_apply_y_rotation_$M.2" = phi i64 [ %add.284, %bb419 ], [ %neg.213, %bb419.preheader ]
  %"xlm_apply_y_rotation_$IND.4" = phi i64 [ %add.283, %bb419 ], [ %"xlm_apply_y_rotation_$IND.3", %bb419.preheader ]
  %"xlm_apply_y_rotation_$Q[][]9" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %neg.214, i64 8, double* elementtype(double) nonnull %"xlm_apply_y_rotation_$Q[]", i64 %"xlm_apply_y_rotation_$M.2")
  %"xlm_apply_y_rotation_$Q[][]_fetch.1844" = load double, double* %"xlm_apply_y_rotation_$Q[][]9", align 1
  %"xlm_apply_y_rotation_$DM[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([900 x double], [900 x double]* @"xlm_apply_y_rotation_$DM", i64 0, i64 0), i64 %"xlm_apply_y_rotation_$IND.4")
  %"xlm_apply_y_rotation_$DM[]_fetch.1846" = load double, double* %"xlm_apply_y_rotation_$DM[]", align 1
  %mul.296 = fmul reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$DM[]_fetch.1846", %"xlm_apply_y_rotation_$QQ[]_fetch.1848"
  %add.282 = fadd reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$Q[][]_fetch.1844", %mul.296
  store double %add.282, double* %"xlm_apply_y_rotation_$Q[][]9", align 1
  %add.283 = add nsw i64 %"xlm_apply_y_rotation_$IND.4", 1
  %add.284 = add i64 %"xlm_apply_y_rotation_$M.2", 1
  %exitcond86.not = icmp eq i64 %add.284, 0
  br i1 %exitcond86.not, label %bb420, label %bb419

bb420:                                            ; preds = %bb419
  %2 = add i64 %"xlm_apply_y_rotation_$L_fetch.1784", %"xlm_apply_y_rotation_$IND.3"
  %add.285 = add i64 %"xlm_apply_y_rotation_$MP.1", 1
  %exitcond87.not = icmp eq i64 %add.285, 0
  br i1 %exitcond87.not, label %bb425.preheader.loopexit, label %bb419.preheader

bb425.preheader.loopexit:                         ; preds = %bb420
  br label %bb425.preheader

bb425.preheader:                                  ; preds = %bb425.preheader.loopexit, %bb408
  %"val$[]16" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 488, double* elementtype(double) bitcast (i8* getelementptr inbounds ([458560 x i8], [458560 x i8]* @ncomxlm_, i64 0, i64 193736) to double*), i64 %"xlm_apply_y_rotation_$L_fetch.1784")
  br label %bb425

bb425:                                            ; preds = %bb425, %bb425.preheader
  %"xlm_apply_y_rotation_$M.3" = phi i64 [ %add.286, %bb425 ], [ %neg.213, %bb425.preheader ]
  %"xlm_apply_y_rotation_$Q[][]15" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %neg.214, i64 8, double* elementtype(double) nonnull %"xlm_apply_y_rotation_$Q[]", i64 %"xlm_apply_y_rotation_$M.3")
  %"xlm_apply_y_rotation_$Q[][]_fetch.1867" = load double, double* %"xlm_apply_y_rotation_$Q[][]15", align 1
  %"val$[][]17" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 -30, i64 8, double* elementtype(double) %"val$[]16", i64 %"xlm_apply_y_rotation_$M.3")
  %"val$[][]_fetch.1870" = load double, double* %"val$[][]17", align 1
  %mul.298 = fmul reassoc ninf nsz arcp contract afn double %"xlm_apply_y_rotation_$Q[][]_fetch.1867", %"val$[][]_fetch.1870"
  store double %mul.298, double* %"xlm_apply_y_rotation_$Q[][]15", align 1
  %add.286 = add nsw i64 %"xlm_apply_y_rotation_$M.3", 1
  %rel.154.not = icmp sgt i64 %add.286, %"xlm_apply_y_rotation_$L_fetch.1784"
  br i1 %rel.154.not, label %bb426.loopexit, label %bb425

bb426.loopexit:                                   ; preds = %bb425
  br label %bb426

bb426:                                            ; preds = %bb426.loopexit, %bb395.bb426_crit_edge
  %add.287.pre-phi = phi i64 [ %.pre, %bb395.bb426_crit_edge ], [ %0, %bb426.loopexit ]
  store i64 %add.287.pre-phi, i64* %"xlm_apply_y_rotation_$L", align 8
  %rel.155.not.not = icmp slt i64 %"xlm_apply_y_rotation_$L_fetch.1784", %"xlm_apply_y_rotation_$LMAX_fetch.1782"
  br i1 %rel.155.not.not, label %bb395, label %bb396.loopexit

bb396.loopexit:                                   ; preds = %bb426
  br label %bb396

bb396:                                            ; preds = %bb396.loopexit, %alloca_13
  ret void
}

; Function Attrs: nounwind uwtable
declare void @xlm_y_rotation_matrix_(i64* noalias dereferenceable(8), double* noalias nocapture readonly dereferenceable(8), double* noalias dereferenceable(8), double* noalias dereferenceable(8)) local_unnamed_addr #1

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
