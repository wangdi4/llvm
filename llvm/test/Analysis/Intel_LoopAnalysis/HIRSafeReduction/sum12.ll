;   for (int i=0; i< n; i++) {
;        s  =  c[s] + n;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; CHECK: No Safe Reduction
; ModuleID = 'sum12.c'
source_filename = "sum12.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i32* nocapture %sum, i32 %n) #0 {
entry:
  %cmp6 = icmp sgt i32 %n, 0
  br i1 %cmp6, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  store i32 %s.0.lcssa, i32* %sum, align 4, !tbaa !1
  ret i32 0

for.body:                                         ; preds = %entry, %for.body
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %s.07 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %idxprom = sext i32 %s.07 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @c, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, %n
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12506)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
