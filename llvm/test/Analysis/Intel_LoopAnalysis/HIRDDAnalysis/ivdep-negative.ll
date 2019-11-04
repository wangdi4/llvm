; Source code:
;
;void sub(float *A,  float *B,  int *M ) {
;  for (int i =0; i< 10000 ; i++){
;    A[i]  += A[i-1] + 1;
;  }
;}
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=2 -analyze | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-dd-test-assume-no-loop-carried-dep=2 -disable-output 2>&1 < %s | FileCheck %s
;
; We do not override non-ALL DVs
;
; CHECK-DAG: (%A)[i1] --> (%A)[i1 + -1] FLOW (<) (1)
;
;Module Before HIR
; ModuleID = 't2.c'
source_filename = "t2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @sub(float* nocapture %A, float* nocapture readnone %B, i32* nocapture readnone %M) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds float, float* %A, i64 %0
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %add = fadd float %1, 1.000000e+00
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %arrayidx2, align 4, !tbaa !2
  %add3 = fadd float %2, %add
  store float %add3, float* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
