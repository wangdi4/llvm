; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-loop-collapse,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -hir-details -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to check that we preserve number of collapsed levels during HIR
; vector code generation.
;
; HIR coming into vectorizer looks like:
;      + DO i64 i1 = 0, 99, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 100>
;      |   %0 = (@arr2)[0][0][i1];
;      |   <RVAL-REG> {al:8}{Collapsed levels:2}(LINEAR ptr @arr2)[i64 0][i64 0][LINEAR i64 i1] inbounds
;      |   
;      |   (@arr1)[0][0][i1] = %0 + 1;
;      |   <LVAL-REG> {al:8}{Collapsed levels:2}(LINEAR ptr @arr1)[i64 0][i64 0][LINEAR i64 i1] inbounds
;      + END LOOP
;
; CHECK:   + DO i64 i1 = 0, 99, 4   <DO_LOOP>  <MAX_TC_EST = 25>  <LEGAL_MAX_TC = 25> <auto-vectorized> <novectorize>
; CHECK:   |   %.vec = (<4 x i64>*)(@arr2)[0][0][i1];
; CHECK:   |   <RVAL-REG> {al:8}{Collapsed levels:2}(<4 x i64>*)(LINEAR ptr @arr2)[i64 0][i64 0][LINEAR i64 i1] inbounds
; CHECK:   |   
; CHECK:   |   (<4 x i64>*)(@arr1)[0][0][i1] = %.vec + 1;
; CHECK:   |   <LVAL-REG> {al:8}{Collapsed levels:2}(<4 x i64>*)(LINEAR ptr @arr1)[i64 0][i64 0][LINEAR i64 i1] inbounds
; CHECK:   + END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr2 = local_unnamed_addr global [10 x [10 x i64]] zeroinitializer, align 16
@arr1 = local_unnamed_addr global [10 x [10 x i64]] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc7
  %l1.017 = phi i64 [ 0, %entry ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %l2.016 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %arrayidx4 = getelementptr inbounds [10 x [10 x i64]], ptr @arr2, i64 0, i64 %l1.017, i64 %l2.016
  %0 = load i64, ptr %arrayidx4, align 8
  %add = add nsw i64 %0, 1
  %arrayidx6 = getelementptr inbounds [10 x [10 x i64]], ptr @arr1, i64 0, i64 %l1.017, i64 %l2.016
  store i64 %add, ptr %arrayidx6, align 8
  %inc = add nuw nsw i64 %l2.016, 1
  %exitcond.not = icmp eq i64 %inc, 10
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i64 %l1.017, 1
  %exitcond18.not = icmp eq i64 %inc8, 10
  br i1 %exitcond18.not, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}
