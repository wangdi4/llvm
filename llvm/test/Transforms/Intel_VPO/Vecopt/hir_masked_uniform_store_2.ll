; RUN: opt -S -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir < %s 2>&1 | FileCheck %s
; RUN: opt -S -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -vplan-force-vf=4  < %s 2>&1 | FileCheck %s
; Test to check that we correctly handle store to a uniform location under mask in vp-value code generation
; Check that the loop is vectorized and that we emit a masked store to <&r, &r, &r, &r>
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP>
; CHECK-LABEL: @test
; CHECK:       call void @llvm.masked.scatter{{.*}}(<4 x float> {{.*}}, <4 x float*> <float* @r, float* @r, float* @r, float* @r>, i32 4, <4 x i1> {{.*}}

@S = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@r = common dso_local local_unnamed_addr global float 0.000000e+00, align 4

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @test() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @S, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %cmp1 = fcmp ult float %0, 0.000000e+00
  br i1 %cmp1, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  store float %0, float* @r, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i32 undef
}
