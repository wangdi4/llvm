; Test for constants mergeable with DDTests

; Check for different types using detailed print
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details -hir-dd-analysis  -hir-dd-analysis-verify=Region | FileCheck %s
; CHECK: %3 = {al:8}(i64*)(%0)[%N + 1]
; CHECK: LINEAR zext.i32.i64(%N + 1)
; CHECK: {al:8}(i64*)(%0)[2] = %3;
; CHECK: (LINEAR bitcast.double*.i64*(%0){def@1})[i64 2] 
; CHECK-DAG: {al:8}(i64*)(%0)[2] --> {al:8}(i64*)(%0)[%N + 1] FLOW (* *)

; # Source Code
; #define ee_u32 unsigned int
; void mat(ee_u32 N, double **C) {
;        ee_u32 i,j;
;        for (i=0; i<N; i++) {
;          for (j=0; j<N; j++) {
;            C[i][2]=C[i][N+1];
;         }}}





target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @_Z17matrix_mul_matrixjPPdS_S_(i32 %N, double** nocapture readonly %C, double* nocapture readnone %A, double* nocapture readnone %B) #0 {
entry:
  %cmp.24 = icmp eq i32 %N, 0
  br i1 %cmp.24, label %for.end.11, label %for.cond.1.preheader.lr.ph

for.cond.1.preheader.lr.ph:                       ; preds = %entry
  %add = add i32 %N, 1
  %idxprom = zext i32 %add to i64
  br label %for.body.3.lr.ph

for.body.3.lr.ph:                                 ; preds = %for.cond.1.preheader.lr.ph, %for.inc.9
  %indvars.iv = phi i64 [ 0, %for.cond.1.preheader.lr.ph ], [ %indvars.iv.next, %for.inc.9 ]
  %arrayidx = getelementptr inbounds double*, double** %C, i64 %indvars.iv
  %0 = load double*, double** %arrayidx, align 8, !tbaa !1
  %arrayidx5 = getelementptr inbounds double, double* %0, i64 %idxprom
  %1 = bitcast double* %arrayidx5 to i64*
  %arrayidx8 = getelementptr inbounds double, double* %0, i64 2
  %2 = bitcast double* %arrayidx8 to i64*
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %j.023 = phi i32 [ 0, %for.body.3.lr.ph ], [ %inc, %for.body.3 ]
  %3 = load i64, i64* %1, align 8, !tbaa !5
  store i64 %3, i64* %2, align 8, !tbaa !5
  %inc = add nuw i32 %j.023, 1
  %exitcond = icmp eq i32 %inc, %N
  br i1 %exitcond, label %for.inc.9, label %for.body.3

for.inc.9:                                        ; preds = %for.body.3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond26 = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond26, label %for.end.11, label %for.body.3.lr.ph

for.end.11:                                       ; preds = %for.inc.9, %entry
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1982)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !3, i64 0}
