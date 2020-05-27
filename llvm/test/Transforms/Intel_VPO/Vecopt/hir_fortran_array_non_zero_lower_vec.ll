; Check that we vectorize array accesses with non-zero lower when the underlying
; HLInst is still valid. VPValue code generation forces mixed CG for now. Check
; for that case as well.
;
; Scalar HIR:
;   DO i1 = 0, 49, 1   <DO_LOOP>
;       %add = (@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:i1 + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)]  +  1.000000e+00;
;       (@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:i1 + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)] = %add;
;   END LOOP
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=2 -print-after=VPlanDriverHIR -hir-details-refs -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=2 -print-after=VPlanDriverHIR -hir-details-refs -disable-output -enable-vp-value-codegen-hir < %s 2>&1 | FileCheck %s
;
; CHECK:      DO i1 = 0, 49, 2   <DO_LOOP> <novectorize>
; CHECK-NEXT:   %add.vec = (<2 x double>*)(@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:i1 + <i64 0, i64 1> + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)]  +  1.000000e+00;
; CHECK-NEXT:   (<2 x double>*)(@mod1_mp_weight_)[0:0:4112([1 x [257 x [2 x double]]]*:0)][1:%"foo_$NG_fetch":4112([1 x [257 x [2 x double]]]:1)][0:i1 + <i64 0, i64 1> + 1:16([257 x [2 x double]]:257)][0:0:8([2 x double]:2)] = %add.vec;
; CHECK-NEXT:  END LOOP
;

@mod1_mp_weight_ = external global [1 x [257 x [2 x double]]], align 8

; Function Attrs: nofree nounwind
define void @foo_(double* noalias nocapture %"foo_$WSUM", i32* noalias nocapture readonly %"foo_$NG") {
alloca:
  %"foo_$NG_fetch" = load i32, i32* %"foo_$NG", align 4
  %int_sext1 = sext i32 %"foo_$NG_fetch" to i64
  %"mod1_mp_weight_[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 4112, double* getelementptr inbounds ([1 x [257 x [2 x double]]], [1 x [257 x [2 x double]]]* @mod1_mp_weight_, i64 0, i64 0, i64 0, i64 0), i64 %int_sext1)
  %"foo_$WSUM.promoted" = load double, double* %"foo_$WSUM", align 8
  br label %bb3

bb3:                                              ; preds = %bb3, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3 ], [ 1, %alloca ]
  %"mod1_mp_weight_[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 16, double* %"mod1_mp_weight_[]", i64 %indvars.iv)
  %"mod1_mp_weight_[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"mod1_mp_weight_[][]", i64 1)
  %"mod1_mp_weight_[][][]_fetch" = load double, double* %"mod1_mp_weight_[][][]", align 8
  %add = fadd fast double %"mod1_mp_weight_[][][]_fetch", 1.000000e+00
  store double %add, double* %"mod1_mp_weight_[][][]", align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 51
  br i1 %exitcond, label %bb1, label %bb3

bb1:                                              ; preds = %bb3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1
