; RUN: opt < %s -tbaa -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline="basic-aa,tbaa" -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + UNKNOWN LOOP i1
;       |   <i1 = 0>
;       |   for.body:
;       |   (%q)[i1] = 1;
;       |   (%u)[i1] = 1.000000e+00;
;       |   %1 = (%p)[0];
;       |   if (i1 + 1 < %1)
;       |   {
;       |      <i1 = i1 + 1>
;       |      goto for.body;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + UNKNOWN LOOP i1
; CHECK-DAG: %p: *
; CHECK-DAG: %q: *
; CHECK-DAG: %u: (DEF) [i1:*:*]
; CHECK:    + END LOOP
; CHECK: END REGION

source_filename = "unknown-loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture readonly %p, i32* nocapture %q, double* nocapture %u) local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* %p, align 4, !tbaa !2
  %cmp7 = icmp sgt i32 %0, 0
  br i1 %cmp7, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds double, double* %u, i64 %indvars.iv
  store double 1.000000e+00, double* %arrayidx2, align 8, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = load i32, i32* %p, align 4, !tbaa !2
  %2 = sext i32 %1 to i64
  %cmp = icmp slt i64 %indvars.iv.next, %2
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
