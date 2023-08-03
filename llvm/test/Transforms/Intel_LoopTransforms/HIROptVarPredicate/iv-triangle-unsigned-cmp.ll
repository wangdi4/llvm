; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that Opt Var Predicate can handle unsigned comparison if LHS and RHS
; fit into signed range of the type.

; HIR before optimization
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64(%M) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
;                  |   + DO i2 = 0, sext.i32.i64(%N) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
;                  |   |   if (i1 + 1 >u i2 + 1)
;                  |   |   <RVAL-REG> LINEAR i64 i1 + 1 {sb:2}
;                  |   |   <RVAL-REG> LINEAR i64 i2 + 1 {sb:2}
;                  |   |   {
;                  |   |      (%A)[sext.i32.i64(%N) * i1 + i2 + sext.i32.i64(%N) + 1] = i1 + 1;
;                  |   |   }
;                  |   |   else
;                  |   |   {
;                  |   |      (%A)[sext.i32.i64(%N) * i1 + i2 + sext.i32.i64(%N) + 1] = i2 + 1;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, zext.i32.i64(%M) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
; CHECK:           |   %ivcopy = i1 + 1;
; CHECK:           |   if (%N > 1)
; CHECK:           |   {
; CHECK:           |      + DO i2 = 0, smin((-2 + sext.i32.i64(%N)), (-2 + %ivcopy)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
; CHECK:           |      |   (%A)[sext.i32.i64(%N) * i1 + i2 + sext.i32.i64(%N) + 1] = i1 + 1;
; CHECK:           |      + END LOOP
;                  |
;                  |
; CHECK:           |      + DO i2 = 0, sext.i32.i64(%N) + -1 * smax(0, (-1 + %ivcopy)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
; CHECK:           |      |   (%A)[sext.i32.i64(%N) * i1 + i2 + sext.i32.i64(%N) + smax(0, (-1 + %ivcopy)) + 1] = i2 + smax(0, (-1 + %ivcopy)) + 1;
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i32 noundef %M, i32 noundef %N) local_unnamed_addr {
entry:
  %cmp31 = icmp sgt i32 %M, 1
  br i1 %cmp31, label %for.cond1.preheader.lr.ph, label %for.end11

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp229 = icmp sgt i32 %N, 1
  %0 = sext i32 %N to i64
  %wide.trip.count39 = zext i32 %M to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc9
  %indvars.iv35 = phi i64 [ 1, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next36, %for.inc9 ]
  br i1 %cmp229, label %for.body3.lr.ph, label %for.inc9

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %1 = mul nsw i64 %indvars.iv35, %0
  %2 = trunc i64 %indvars.iv35 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 1, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp ugt i64 %indvars.iv35, %indvars.iv
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  %3 = add nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %3
  store i32 %2, ptr %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body3
  %4 = add nsw i64 %indvars.iv, %1
  %arrayidx8 = getelementptr inbounds i32, ptr %A, i64 %4
  %5 = trunc i64 %indvars.iv to i32
  store i32 %5, ptr %arrayidx8, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond.not, label %for.inc9.loopexit, label %for.body3

for.inc9.loopexit:                                ; preds = %for.inc
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.cond1.preheader
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond40.not = icmp eq i64 %indvars.iv.next36, %wide.trip.count39
  br i1 %exitcond40.not, label %for.end11.loopexit, label %for.cond1.preheader

for.end11.loopexit:                               ; preds = %for.inc9
  br label %for.end11

for.end11:                                        ; preds = %for.end11.loopexit, %entry
  %div = sdiv i32 %N, 2
  %idxprom12 = sext i32 %div to i64
  %arrayidx13 = getelementptr inbounds i32, ptr %A, i64 %idxprom12
  %6 = load i32, ptr %arrayidx13, align 4
  ret i32 %6
}
