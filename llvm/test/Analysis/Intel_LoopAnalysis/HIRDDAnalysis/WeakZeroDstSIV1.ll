;  for (long int i=0; i <= 100; i++) {
;      A[2*i +2] = B[i*i] +1;
;      B[100] = A[2] ; }
; RUN:  opt < %s  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-DAG:  (%A)[2 * i1 + 2] --> (%A)[2] FLOW (<=)

; ModuleID = 'WeakZeroDstSIV1.c'
source_filename = "WeakZeroDstSIV1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [200 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub(float* nocapture %A, i32 %n) #0 {
entry:
  %arrayidx4 = getelementptr inbounds float, float* %A, i64 2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i.012 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = mul nsw i64 %i.012, %i.012
  %arrayidx = getelementptr inbounds [200 x i32], [200 x i32]* @B, i64 0, i64 %mul
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  %conv = sitofp i32 %add to float
  %mul1 = shl i64 %i.012, 1
  %add2 = add nuw nsw i64 %mul1, 2
  %arrayidx3 = getelementptr inbounds float, float* %A, i64 %add2
  store float %conv, float* %arrayidx3, align 4, !tbaa !5
  %1 = load float, float* %arrayidx4, align 4, !tbaa !5
  %conv5 = fptosi float %1 to i32
  store i32 %conv5, i32* getelementptr inbounds ([200 x i32], [200 x i32]* @B, i64 0, i64 100), align 16, !tbaa !1
  %inc = add nuw nsw i64 %i.012, 1
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
