; Source code
;
;void sub(float *A,  float *B,  int *M ) {
;  for (int i =0; i< 10000 ; i++){
;    A[i]  += B[M[i]] + 1;
;  }
;}
;
; Test case for ivdep:
; when -hir-dd-test-assume-no-loop-carried-dep=1, DV for the innermost loop for memory refs should be set as  =
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=1 -analyze | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=1 -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK-DAG: (%M)[i1] --> (%A)[i1] ANTI (=) (0)
; CHECK-DAG: (%B)[%0] --> (%A)[i1] ANTI (=) (0)
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @sub(float* nocapture %A, float* nocapture readonly %B, i32* nocapture readonly %M) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %M, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds float, float* %B, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !6
  %add = fadd float %1, 1.000000e+00
  %arrayidx4 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %arrayidx4, align 4, !tbaa !6
  %add5 = fadd float %2, %add
  store float %add5, float* %arrayidx4, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
