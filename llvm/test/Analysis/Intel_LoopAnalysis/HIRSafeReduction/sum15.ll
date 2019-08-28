;    for ( i=0; i< n; i++) {
;        s1 =  a[i][s1] +  n;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK: No Safe Reduction
; ModuleID = 'sum15.c'
source_filename = "sum15.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i64 %n2, i64 %n3, double* nocapture %s) local_unnamed_addr #0 {
entry:
  %cmp8 = icmp sgt i64 %n, 0
  br i1 %cmp8, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.010 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %s1.09 = phi i64 [ %add, %for.body ], [ 0, %entry ]
  %arrayidx1 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %i.010, i64 %s1.09
  %0 = load i32, i32* %arrayidx1, align 4, !tbaa !1
  %conv = sext i32 %0 to i64
  %add = add nsw i64 %conv, %n
  %inc = add nuw nsw i64 %i.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %phitmp = sitofp i64 %add to double
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %s1.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %phitmp, %for.end.loopexit ]
  store double %s1.0.lcssa, double* %s, align 8, !tbaa !5
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17927)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !3, i64 0}
