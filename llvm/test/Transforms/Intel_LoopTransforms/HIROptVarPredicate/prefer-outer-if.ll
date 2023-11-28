; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that HIROptVarPredicate gives preference to outermost 'if' over innermost.

; HIR before optimization
;            BEGIN REGION { }
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 19, 1   <DO_LOOP>
;                  |   |   if (i1 != i2)
;                  |   |   {
;                  |   |      (@a)[0][i1][i2] = i1;
;                  |   |      if (i2 >u 3)
;                  |   |      {
;                  |   |         (@a)[0][i1][i2] = i1 + i2;
;                  |   |      }
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after optimization
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   %ivcopy = i1;
; CHECK:           |
; CHECK:           |   + DO i2 = 0, smin(19, (-1 + %ivcopy)), 1   <DO_LOOP>
; CHECK:           |   |   (@a)[0][i1][i2] = i1;
; CHECK:           |   |   if (i2 >u 3)
; CHECK:           |   |   {
; CHECK:           |   |      (@a)[0][i1][i2] = i1 + i2;
; CHECK:           |   |   }
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |   + DO i2 = 0, -1 * smax(0, (1 + %ivcopy)) + 19, 1   <DO_LOOP>
; CHECK:           |   |   (@a)[0][i1][i2 + smax(0, (1 + %ivcopy))] = i1;
; CHECK:           |   |   if (i2 + smax(0, (1 + %ivcopy)) >u 3)
; CHECK:           |   |   {
; CHECK:           |   |      (@a)[0][i1][i2 + smax(0, (1 + %ivcopy))] = i1 + i2 + smax(0, (1 + %ivcopy));
; CHECK:           |   |   }
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc14
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.inc14 ]
  %0 = trunc i64 %indvars.iv31 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4.not = icmp eq i64 %indvars.iv31, %indvars.iv
  br i1 %cmp4.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [20 x [20 x i32]], ptr @a, i64 0, i64 %indvars.iv31, i64 %indvars.iv
  store i32 %0, ptr %arrayidx6, align 4
  %cmp7 = icmp ugt i64 %indvars.iv, 3
  br i1 %cmp7, label %if.then8, label %for.inc

if.then8:                                         ; preds = %if.then
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv31
  %2 = trunc i64 %1 to i32
  store i32 %2, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then8, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond.not, label %for.inc14, label %for.body3

for.inc14:                                        ; preds = %for.inc
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond33.not = icmp eq i64 %indvars.iv.next32, 20
  br i1 %exitcond33.not, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  ret i32 undef
}

