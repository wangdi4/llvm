;    for  (i=0; i <= 3-n; i++) {	
;        a[ - 2*n * i +4 ] = a[  -2 *n * i -5 ] +1; 
;   }

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
 
; CHECK: 'HIR Data Dependence Analysis' for function 'sub8'
; CHECK:  (%a)[-2 * %n * i1 + 4] OUTPUT (*)

; ModuleID = 'gcd4.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @sub8(float* nocapture %a, i64 %n) #0 {
entry:
  %cmp.14 = icmp sgt i64 %n, 3
  br i1 %cmp.14, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul = mul nsw i64 %n, -2
  %0 = sub i64 4, %n
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.015 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul1 = mul nsw i64 %mul, %i.015
  %sub2 = add nsw i64 %mul1, -5
  %arrayidx = getelementptr inbounds float, float* %a, i64 %sub2
  %1 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %1, 1.000000e+00
  %add5 = add nsw i64 %mul1, 4
  %arrayidx6 = getelementptr inbounds float, float* %a, i64 %add5
  store float %add, float* %arrayidx6, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.015, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1130)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
