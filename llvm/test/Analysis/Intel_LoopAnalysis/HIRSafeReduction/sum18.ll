;    for ( i=0; i< n; i++) {
;        s1  = ( s1 + n2 ) * n3  ;
;    }
;    for ( i=0; i< n; i++) {
;        s1  = ( s0 + n2 ) * n3  ;
;        s2  = ( s1 + n3 ) * n2  ;
;        s3  = ( s2 + n2 ) * n4  ;
;        s0  = ( s3 + n4 ) * n2  ;
;    }
;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK: No Safe Reduction
;
; ModuleID = 'sum18.c'
source_filename = "sum18.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i32 %n2, i32 %n3, i32 %n4, double* nocapture %s) local_unnamed_addr #0 {
entry:
  %cmp41 = icmp sgt i64 %n, 0
  br i1 %cmp41, label %for.body, label %for.end.thread

for.end.thread:                                   ; preds = %entry
  %0 = load double, double* %s, align 8, !tbaa !1
  %add148 = fadd double %0, 0.000000e+00
  store double %add148, double* %s, align 8, !tbaa !1
  br label %for.end16

for.body:                                         ; preds = %entry, %for.body
  %i.043 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %s1.042 = phi i32 [ %mul, %for.body ], [ 0, %entry ]
  %add = add nsw i32 %s1.042, %n2
  %mul = mul nsw i32 %add, %n3
  %inc = add nuw nsw i64 %i.043, 1
  %exitcond45 = icmp eq i64 %inc, %n
  br i1 %exitcond45, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %conv = sitofp i32 %mul to double
  %1 = load double, double* %s, align 8, !tbaa !1
  %add1 = fadd double %conv, %1
  store double %add1, double* %s, align 8, !tbaa !1
  %cmp338 = icmp sgt i64 %n, 0
  br i1 %cmp338, label %for.body5, label %for.end16

for.body5:                                        ; preds = %for.end, %for.body5
  %s0.040 = phi i32 [ %mul13, %for.body5 ], [ 0, %for.end ]
  %i.139 = phi i64 [ %inc15, %for.body5 ], [ 0, %for.end ]
  %add6 = add nsw i32 %s0.040, %n2
  %mul7 = mul nsw i32 %add6, %n3
  %add8 = add nsw i32 %mul7, %n3
  %mul9 = mul nsw i32 %add8, %n2
  %add10 = add nsw i32 %mul9, %n2
  %mul11 = mul nsw i32 %add10, %n4
  %add12 = add nsw i32 %mul11, %n4
  %mul13 = mul nsw i32 %add12, %n2
  %inc15 = add nuw nsw i64 %i.139, 1
  %exitcond = icmp eq i64 %inc15, %n
  br i1 %exitcond, label %for.end16, label %for.body5

for.end16:                                        ; preds = %for.body5, %for.end.thread, %for.end
  %s1.1.lcssa = phi i32 [ %mul, %for.end ], [ 0, %for.end.thread ], [ %mul7, %for.body5 ]
  %conv17 = sitofp i32 %s1.1.lcssa to double
  %2 = load double, double* %s, align 8, !tbaa !1
  %add18 = fadd double %conv17, %2
  store double %add18, double* %s, align 8, !tbaa !1
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17943)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
