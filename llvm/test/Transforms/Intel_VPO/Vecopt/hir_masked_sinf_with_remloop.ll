; LLVM IR generated from testcase below using: icx -S -emit-llvm -Qoption,c,-fveclib=SVML -openmp -restrict -ffast-math -O2
;
;void foo(float* restrict a, float* restrict b) {
;  unsigned int i;
;  for (i = 0; i < N; i++) {
;    if (b[i] > 3)
;      a[i] = sinf(b[i]);
;  }
;}
;
; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -print-after=VPODriverHIR -S  < %s 2>&1 | FileCheck %s
; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR -S  < %s 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR

; Check to see that the main vector loop was vectorized with svml
; CHECK: call <4 x float> @__svml_sinf4_mask

; Check to see that the remainder loop broadcasts the element and uses svml to match the main vector loop.
; CHECK-LABEL: {{then.[0-9]+}}
; CHECK: load float
; CHECK: insertelement <4 x float>
; CHECK-NEXT: shufflevector <4 x float>
; CHECK-NEXT: call <4 x float> @__svml_sinf4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* noalias nocapture %a, float* noalias nocapture readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %cmp1 = fcmp fast ogt float %0, 3.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %call = tail call fast float @sinf(float %0) #3
  %arrayidx5 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %call, float* %arrayidx5, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 131
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nounwind readnone
declare float @sinf(float) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
