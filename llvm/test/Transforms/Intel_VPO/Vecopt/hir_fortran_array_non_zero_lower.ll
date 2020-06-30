; Check that vectorizer handles array accesses with non-zero lower.

;   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;   + DO i1 = 0, 3, 1   <DO_LOOP>
;   |   %add10 = (@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)]
;   |                               [0:i1 + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)]  +  %add10; <Safe Reduction>
;   + END LOOP
;
;   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=2 -print-after=VPlanDriverHIR -hir-details-dims -enable-vp-value-codegen-hir=0 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=2 -print-after=VPlanDriverHIR -hir-details-dims -enable-vp-value-codegen-hir -disable-output < %s 2>&1 | FileCheck %s

; Check that memref and loop was vectorized.
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %red.var = 0.000000e+00;
; CHECK-NEXT:           %.vec = (<2 x double>*)(@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:<i64 0, i64 1> + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)];
; CHECK-NEXT:           %red.var = %.vec  +  %red.var;
; CHECK-NEXT:           %.vec = (<2 x double>*)(@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:<i64 0, i64 1> + 3:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)];
; CHECK-NEXT:           %red.var = %.vec  +  %red.var;
; CHECK-NEXT:           %add10 = @llvm.experimental.vector.reduce.v2.fadd.f64.v2f64(%add10,  %red.var);
; CHECK-NEXT:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@mod1_mp_weight_ = external dso_local local_unnamed_addr global [1 x [257 x [2 x double]]], align 8

; Function Attrs: nofree nounwind
define void @foo_(double* noalias nocapture %"foo_$WSUM", i32* noalias nocapture readonly %"foo_$NG") local_unnamed_addr {
alloca:
  %"foo_$NG_fetch" = load i32, i32* %"foo_$NG", align 4
  %int_sext1 = sext i32 %"foo_$NG_fetch" to i64
  %"mod1_mp_weight_[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 4112, double* getelementptr inbounds ([1 x [257 x [2 x double]]], [1 x [257 x [2 x double]]]* @mod1_mp_weight_, i64 0, i64 0, i64 0, i64 0), i64 %int_sext1)
  %"foo_$WSUM.promoted" = load double, double* %"foo_$WSUM", align 8
  br label %bb3

bb3:                                              ; preds = %bb3, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3 ], [ 1, %alloca ]
  %add10 = phi double [ %add, %bb3 ], [ %"foo_$WSUM.promoted", %alloca ]
  %"mod1_mp_weight_[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* %"mod1_mp_weight_[]", i64 %indvars.iv)
  %"mod1_mp_weight_[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"mod1_mp_weight_[][]", i64 1)
  %"mod1_mp_weight_[][][]_fetch" = load double, double* %"mod1_mp_weight_[][][]", align 8
  %add = fadd fast double %"mod1_mp_weight_[][][]_fetch", %add10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %bb1, label %bb3

bb1:                                              ; preds = %bb3
  %add.lcssa = phi double [ %add, %bb3 ]
  store double %add.lcssa, double* %"foo_$WSUM", align 8
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1
