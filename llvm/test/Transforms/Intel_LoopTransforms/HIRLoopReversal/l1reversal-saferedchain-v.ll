; Sanity Test(s) on HIR Loop Reversal: valid testcase for a safe reduction chain with 2+ instructions
; 
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
; - Decision:   YES, to reverse the loop
;
; 
; *** Source Code ***
;
;float foo(float * restrict A, int n) {
;  float s1 = 0, s2 = 0;
;  for (int i = 0; i <= 10; i++) {
;    s1 = s2 + A[2 * n - i];
;    s2 = s1 + A[n - i];
;  }
;  return s1 + s2 + A[0];
;}
;
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal < %s 2>&1 -disable-output | FileCheck %s
;
; CHECK: IR Dump Before HIR Loop Reversal
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 10, 1   <DO_LOOP>
; CHECK:        |   %add = %s2.019  +  (%A)[-1 * i1 + sext.i32.i64((2 * %n))];
; CHECK:        |   %s2.019 = %add  +  (%A)[-1 * i1 + sext.i32.i64(%n)];
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: IR Dump After HIR Loop Reversal
;
; CHECK: BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 10, 1   <DO_LOOP>
; CHECK:        |   %add = %s2.019  +  (%A)[i1 + sext.i32.i64((2 * %n)) + -10];
; CHECK:        |   %s2.019 = %add  +  (%A)[i1 + sext.i32.i64(%n) + -10];
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define float @foo(float* noalias nocapture readonly %A, i32 %n) local_unnamed_addr #0 {
entry:
  %mul = shl nsw i32 %n, 1
  %0 = sext i32 %n to i64
  %1 = sext i32 %mul to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add5 = fadd float %add, %add4
  %2 = load float, float* %A, align 4, !tbaa !1
  %add7 = fadd float %add5, %2
  ret float %add7

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s2.019 = phi float [ 0.000000e+00, %entry ], [ %add4, %for.body ]
  %3 = sub nsw i64 %1, %indvars.iv
  %arrayidx = getelementptr inbounds float, float* %A, i64 %3
  %4 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %s2.019, %4
  %5 = sub nsw i64 %0, %indvars.iv
  %arrayidx3 = getelementptr inbounds float, float* %A, i64 %5
  %6 = load float, float* %arrayidx3, align 4, !tbaa !1
  %add4 = fadd float %add, %6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20789) (llvm/branches/loopopt 20794)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
