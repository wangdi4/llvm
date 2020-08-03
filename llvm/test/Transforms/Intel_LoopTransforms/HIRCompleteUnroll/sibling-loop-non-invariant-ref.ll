; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-post-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that the GEPSavings from A[i2] are zero because it is not invariant in unrolled loop due to the presence of A[i2] in non-candidate sibling loop.

; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   (%A)[i2] = i2;
; |   + END LOOP
; |
; |
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   (%A)[i2] = i2;
; |   + END LOOP
; + END LOOP

; CHECK: GEPSavings: 0

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp30 = icmp sgt i32 %n, 0
  br i1 %cmp30, label %for.body.lr.ph, label %for.end14

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc12
  %i.031 = phi i32 [ 0, %for.body.lr.ph ], [ %inc13, %for.inc12 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.end
  %indvars.iv32 = phi i64 [ 0, %for.end ], [ %indvars.iv.next33, %for.body6 ]
  %arrayidx8 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv32
  %1 = trunc i64 %indvars.iv32 to i32
  store i32 %1, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 10
  br i1 %exitcond34, label %for.inc12, label %for.body6

for.inc12:                                        ; preds = %for.body6
  %inc13 = add nuw nsw i32 %i.031, 1
  %exitcond35 = icmp eq i32 %inc13, %n
  br i1 %exitcond35, label %for.end14.loopexit, label %for.body3.lr.ph

for.end14.loopexit:                               ; preds = %for.inc12
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %entry
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
