;
;  This test a fix to  the Wolfe's exact SIV test
;	for (i=0; i<= 21; i++) {
;		A[63  -  3*i] = i+2;
;		A[126 - 6*i] = i +5;
;  DV is expected to be (<=) and not < 
;
; RUN:  opt < %s  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-DAG: (%A)[-3 * i1 + 63] OUTPUT (<=)
;
; ModuleID = 'x.c'
source_filename = "x.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @sub(float* nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.013 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nuw nsw i64 %i.013, 2
  %conv = sitofp i64 %add to float
  %0 = mul nsw i64 %i.013, -3
  %sub = add nsw i64 %0, 63
  %arrayidx = getelementptr inbounds float, float* %A, i64 %sub
  store float %conv, float* %arrayidx, align 4, !tbaa !1
  %add1 = add nuw nsw i64 %i.013, 5
  %conv2 = sitofp i64 %add1 to float
  %1 = mul nsw i64 %i.013, -6
  %sub4 = add nsw i64 %1, 126
  %arrayidx5 = getelementptr inbounds float, float* %A, i64 %sub4
  store float %conv2, float* %arrayidx5, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.013, 1
  %exitcond = icmp eq i64 %inc, 22
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20364)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
