;    for ( i=0; i< n; i++) {
;        s1 = n2 * s1 +  n;
; 			s2 =  s2 * i +  n;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK: No Safe Reduction
; ModuleID = 'sum16.c'
source_filename = "sum16.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i64 %n2, i64 %n3, double* nocapture %s) local_unnamed_addr #0 {
entry:
  %cmp13 = icmp sgt i64 %n, 0
  br i1 %cmp13, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.016 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %s2.015 = phi i64 [ %add2, %for.body ], [ 0, %entry ]
  %s1.014 = phi i64 [ %add, %for.body ], [ 0, %entry ]
  %mul = mul nsw i64 %s1.014, %n2
  %add = add nsw i64 %mul, %n
  %mul1 = mul nsw i64 %i.016, %s2.015
  %add2 = add nsw i64 %mul1, %n
  %inc = add nuw nsw i64 %i.016, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %s2.0.lcssa = phi i64 [ 0, %entry ], [ %add2, %for.body ]
  %s1.0.lcssa = phi i64 [ 0, %entry ], [ %add, %for.body ]
  %add3 = add nsw i64 %s2.0.lcssa, %s1.0.lcssa
  %conv = sitofp i64 %add3 to double
  store double %conv, double* %s, align 8, !tbaa !1
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17927)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
