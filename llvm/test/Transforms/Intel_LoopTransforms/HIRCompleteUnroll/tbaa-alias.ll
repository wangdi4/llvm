; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we are able to disambiguate between (%A)[i3] and (%F)[%1] using tbaa even though they have the same symbase (because both of them alias with (%C)[i1] which is (char*)).
; This allows use to analyze them as invariant in the unrolled i2 loop. Memory motion can hoist them out of i2 loop later.

; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   (%C)[i1] = i1;
; |
; |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; |   |   |   %1 = (%A)[i3];
; |   |   |   (%F)[%1] = 1.000000e+00;
; |   |   |   (%A)[i3] = i1 + %1;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: GEPSavings: 40

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i8* nocapture %C, i32* nocapture %A, float* nocapture %F, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp37 = icmp sgt i32 %n, 0
  br i1 %cmp37, label %for.body.lr.ph, label %for.end20

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc18
  %indvars.iv41 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next42, %for.inc18 ]
  %0 = trunc i64 %indvars.iv41 to i32
  %conv = trunc i64 %indvars.iv41 to i8
  %arrayidx = getelementptr inbounds i8, i8* %C, i64 %indvars.iv41
  store i8 %conv, i8* %arrayidx, align 1, !tbaa !2
  br label %for.body4

for.body4:                                        ; preds = %for.inc15, %for.body4.lr.ph
  %j.036 = phi i32 [ 0, %for.body4.lr.ph ], [ %inc16, %for.inc15 ]
  br label %for.body8

for.body8:                                        ; preds = %for.body8, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4 ], [ %indvars.iv.next, %for.body8 ]
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx10, align 4, !tbaa !5
  %idxprom11 = sext i32 %1 to i64
  %arrayidx12 = getelementptr inbounds float, float* %F, i64 %idxprom11
  store float 1.000000e+00, float* %arrayidx12, align 4, !tbaa !7
  %add = add nsw i32 %1, %0
  store i32 %add, i32* %arrayidx10, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc15, label %for.body8

for.inc15:                                        ; preds = %for.body8
  %inc16 = add nuw nsw i32 %j.036, 1
  %exitcond40 = icmp eq i32 %inc16, %n
  br i1 %exitcond40, label %for.inc18, label %for.body4

for.inc18:                                        ; preds = %for.inc15
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, %wide.trip.count
  br i1 %exitcond43, label %for.end20.loopexit, label %for.body4.lr.ph

for.end20.loopexit:                               ; preds = %for.inc18
  br label %for.end20

for.end20:                                        ; preds = %for.end20.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 53fda47b976053128a56ec7ec5b3e9cf723c5787) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bab72d07994a2effc7b0cc2c2419332c0f35069c)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !3, i64 0}
