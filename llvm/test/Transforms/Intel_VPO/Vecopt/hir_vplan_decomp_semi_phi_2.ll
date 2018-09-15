; Verify that decomposer generates non-empty semi-phis i.e either valid
; induction phi or empty semi-phis later fixed by fixPhiNode.
; This test checks validity of semi-phis for non-reduction code.

; int  foo(int *a, int *b, int N)
; {
;   int t1 = N *2;
;   int i;
;
; #pragma vector always
; #pragma ivdep
;   for (i = 0; i < N; i++) {
;     if (a[i] < i)
;       t1 = a[i];
;     else
;       t1 = a[i+1];
;
;     b[i] = t1;
;   }
;
;   return b[0];
; }

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; CHECK: [[IVPhi:%.*]] = semi-phi i64 0 {{%.*}}
; CHECK: [[SemiPhi:%.*]] = semi-phi [[IfT1:%.*]] [[ElseT1:%.*]]
; CHECK-NEXT: store [[SemiPhi]] {{%.*}}
; CHECK-NOT: {{%.*}} = semi-phi


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable

define dso_local i32 @foo(i32* nocapture readonly %a, i32* nocapture %b, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %N, 0
  br i1 %cmp19, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %.pre, %if.end ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = sext i32 %0 to i64
  %cmp1 = icmp sgt i64 %indvars.iv, %1
  %.pre = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp1, label %if.end, label %if.else

if.else:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds i32, i32* %a, i64 %.pre
  %2 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %for.body, %if.else
  %t1.0 = phi i32 [ %2, %if.else ], [ %0, %for.body ]
  %arrayidx7 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %t1.0, i32* %arrayidx7, align 4, !tbaa !2
  %exitcond = icmp eq i64 %.pre, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !6

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %3 = load i32, i32* %b, align 4, !tbaa !2
  ret i32 %3
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind readonly }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7, !8, !9}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
