; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that the GEP savings from the conditional store iz zero as it cannot be hoisted out of the unrolled loop.

; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   if (i1 + i2 >u 5)
; |   |   {
; |   |      (%A)[i2] = i1;
; |   |   }
; |   + END LOOP
; + END LOOP


; CHECK: GEPSavings: 0

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body.lr.ph, label %for.end7

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc5, %for.body.lr.ph
  %indvars.iv19 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next20, %for.inc5 ]
  %0 = trunc i64 %indvars.iv19 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.inc ]
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv19
  %cmp4 = icmp ugt i64 %1, 5
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.inc
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond21 = icmp eq i64 %indvars.iv.next20, %wide.trip.count
  br i1 %exitcond21, label %for.end7.loopexit, label %for.body

for.end7.loopexit:                                ; preds = %for.inc5
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
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
