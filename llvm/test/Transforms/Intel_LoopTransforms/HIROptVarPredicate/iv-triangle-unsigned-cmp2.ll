; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that Opt Var Predicate can handle unsigned comparison if LHS and RHS
; fit into signed range of the type.

; HIR before optimization
;            BEGIN REGION { }
;                  + DO i1 = 0, 200, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 500, 1   <DO_LOOP>
;                  |   |   if (i1 + 3 >u i2 + 5)
;                  |   |   {
;                  |   |      (%A)[sext.i32.i64(%N) * i1 + i2 + 3 * sext.i32.i64(%N) + 5] = i1 + 3;
;                  |   |   }
;                  |   |   else
;                  |   |   {
;                  |   |      (%A)[sext.i32.i64(%N) * i1 + i2 + 3 * sext.i32.i64(%N) + 5] = i2 + 5;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION


; HIR after optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 200, 1   <DO_LOOP>
; CHECK:           |   %ivcopy = i1 + 3;
; CHECK:           |
; CHECK:           |   + DO i2 = 0, smin(500, (-6 + %ivcopy)), 1   <DO_LOOP>
; CHECK:           |   |   (%A)[sext.i32.i64(%N) * i1 + i2 + 3 * sext.i32.i64(%N) + 5] = i1 + 3;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, -1 * smax(0, (-5 + %ivcopy)) + 500, 1   <DO_LOOP>
; CHECK:           |   |   (%A)[sext.i32.i64(%N) * i1 + i2 + 3 * sext.i32.i64(%N) + smax(0, (-5 + %ivcopy)) + 5] = i2 + smax(0, (-5 + %ivcopy)) + 5;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i32 noundef %M, i32 noundef %N) local_unnamed_addr {
entry:
  %0 = sext i32 %N to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc9
  %indvars.iv32 = phi i64 [ 3, %entry ], [ %indvars.iv.next33, %for.inc9 ]
  %1 = mul nsw i64 %indvars.iv32, %0
  %2 = trunc i64 %indvars.iv32 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc
  %indvars.iv = phi i64 [ 5, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp ugt i64 %indvars.iv32, %indvars.iv
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
  %exitcond.not = icmp eq i64 %indvars.iv.next, 506
  br i1 %exitcond.not, label %for.inc9, label %for.body3

for.inc9:                                         ; preds = %for.inc
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond36.not = icmp eq i64 %indvars.iv.next33, 204
  br i1 %exitcond36.not, label %for.end11, label %for.cond1.preheader

for.end11:                                        ; preds = %for.inc9
  %div = sdiv i32 %N, 2
  %idxprom12 = sext i32 %div to i64
  %arrayidx13 = getelementptr inbounds i32, ptr %A, i64 %idxprom12
  %6 = load i32, ptr %arrayidx13, align 4
  ret i32 %6
}

