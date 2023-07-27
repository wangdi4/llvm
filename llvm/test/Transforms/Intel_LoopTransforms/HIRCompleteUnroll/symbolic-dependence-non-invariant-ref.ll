; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we have GEPSavings of 0 for (%A)[i2] as it cannot be hoisted out of the urolled i1 loop because of dependence with A[%t].

; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   (%A)[%t] = 0;
; |
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   %0 = (%A)[i2];
; |   |   (%A)[i2] = %0 + 1;
; |   + END LOOP
; + END LOOP

; CHECK: GEPSavings: 0

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %A, i32 %n, i32 %t) local_unnamed_addr #0 {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.body.lr.ph, label %for.end9

for.body.lr.ph:                                   ; preds = %entry
  %idxprom = sext i32 %t to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  br label %for.body

for.body:                                         ; preds = %for.inc7, %for.body.lr.ph
  %i.018 = phi i32 [ 0, %for.body.lr.ph ], [ %inc8, %for.inc7 ]
  store i32 0, ptr %arrayidx, align 4, !tbaa !2
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx5, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i32 %i.018, 1
  %exitcond19 = icmp eq i32 %inc8, %n
  br i1 %exitcond19, label %for.end9.loopexit, label %for.body

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 79c50fdec6f9919012732eb5ff19f470f43389fa)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
