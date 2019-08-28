;    for(i=0; i<n; i++) {
;        if (A[i] > 1) {
;            tmp = A[i] ;
;        }
;        else {
;            tmp = A[i] + 8;
;        }
;        sum = sum + tmp; }
;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze   -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK:   %sum.017 = %sum.017  +  %1; <Safe Reduction>
;
; ModuleID = 'sum6.c'
source_filename = "sum6.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(float* nocapture readonly %A, i32 %n, i32* nocapture %r) #0 {
entry:
  %cmp15 = icmp sgt i32 %n, 0
  br i1 %cmp15, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.017 = phi float [ %add6, %for.body ], [ 0.000000e+00, %entry ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %cmp1 = fcmp ogt float %0, 1.000000e+00
  %add = fadd float %0, 8.000000e+00
  %1 = select i1 %cmp1, float %0, float %add
  %add6 = fadd float %sum.017, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %phitmp = fptosi float %add6 to i32
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %phitmp, %for.end.loopexit ]
  store i32 %sum.0.lcssa, i32* %r, align 4, !tbaa !5
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12282)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
