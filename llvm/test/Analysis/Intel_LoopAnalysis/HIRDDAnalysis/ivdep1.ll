;  #pragma ivdep
;  for (i=0; i< N; i++) {
;    A[i] = A[i+3] + 2;
;    B[i] = A[i+m] + 1;
; }
; Test case for ivdep:
; expecting dv (=) for all of them, except for small constant distance
; except the the one with constant distance
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s
;
; RUN: opt < %s -opaque-pointers -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

; CHECK: DD graph for function sub:
; CHECK-DAG:  (%A)[i1] --> (%B)[i1] OUTPUT (=)
; CHECK-DAG:  (%A)[i1 + 3] --> (%A)[i1] ANTI (<)
; CHECK-DAG:  (%A)[i1 + 3] --> (%B)[i1] ANTI (=)
;
;Module Before HIR; ModuleID = 'ivdep1.c'
source_filename = "ivdep1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sub(float* nocapture %A, float* nocapture %B, float* nocapture readnone %C, i32 %N, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %N, 0
  br i1 %cmp18, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %m to i64
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = add nuw i64 %indvars.iv, 3
  %2 = and i64 %1, 4294967295
  %arrayidx = getelementptr inbounds float, float* %A, i64 %2
  %3 = load float, float* %arrayidx, align 4, !tbaa !2
  %add1 = fadd float %3, 2.000000e+00
  %arrayidx3 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %add1, float* %arrayidx3, align 4, !tbaa !2
  %4 = add nsw i64 %indvars.iv, %0
  %arrayidx6 = getelementptr inbounds float, float* %A, i64 %4
  %5 = load float, float* %arrayidx6, align 4, !tbaa !2
  %add7 = fadd float %5, 1.000000e+00
  %arrayidx9 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  store float %add7, float* %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !6

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7277ef25cac670b925c89fab076a39a389835fc5) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 372f88114e0e88951a1653bd9fe5cf3cb313b50f)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
