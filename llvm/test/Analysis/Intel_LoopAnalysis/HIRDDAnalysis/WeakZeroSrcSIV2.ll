;   for (long int i=1; i <= 100; i++) {
;      B[100] = A[100];
;      A[i] = i; }
; RUN:  opt < %s  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-DAG:  (%A)[100] --> (%A)[i1 + 1] ANTI (<=)
;
; ModuleID = 'WeakZeroSrcSIV2.c'
source_filename = "WeakZeroSrcSIV2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [200 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub(float* nocapture %A, i32 %N) #0 {
entry:
  %arrayidx = getelementptr inbounds float, float* %A, i64 100
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %conv = fptosi float %0 to i32
  store i32 %conv, i32* getelementptr inbounds ([200 x i32], [200 x i32]* @B, i64 0, i64 100), align 16, !tbaa !1
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.08 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %0 = load float, float* %arrayidx, align 4, !tbaa !5
  %conv1 = sitofp i64 %i.08 to float
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %i.08
  store float %conv1, float* %arrayidx2, align 4, !tbaa !5
  %inc = add nuw nsw i64 %i.08, 1
  %exitcond = icmp eq i64 %inc, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12457)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !3, i64 0}
