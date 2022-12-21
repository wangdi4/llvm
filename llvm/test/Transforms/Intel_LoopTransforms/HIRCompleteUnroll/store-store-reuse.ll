; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we have GEP savings of 18 (9 * 2) due to redundancy between (%A)[i1] and (%A)[i1 + -1].

; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   (%A)[i1] = 5;
; |   (%A)[i1 + -1] = i1 + %k;
; + END LOOP

; CHECK: GEPSavings: 18

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %k) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 5, i32* %arrayidx, align 4, !tbaa !2
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  %2 = add i32 %1, %k
  store i32 %2, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 33ca482a1b7f763cfc3c669b2ba2ff2ebbb4b09e)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
