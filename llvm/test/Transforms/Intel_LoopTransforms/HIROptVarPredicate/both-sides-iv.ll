; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -S -disable-output < %s 2>&1 | FileCheck %s

; Test checks that HIROptVarPredicate doesn't consider 'if (i1 + 1 != i2 + 1)' as
; a candidate for hoisting out of i1 loop.

; HIR before optimization
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64(%M) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
;                  |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>
;                  |   |   if (i1 + 1 != i2 + 1)
;                  |   |   {
;                  |   |      (%A)[sext.i32.i64(%N) * i1 + i2 + sext.i32.i64(%N) + 1] = i1 + 1;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; CHECK: BEGIN REGION { } 
; CHECK: DO i1
; CHECK: DO i2 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i32 noundef %M, i32 noundef %N) local_unnamed_addr {
entry:
  %cmp22 = icmp sgt i32 %M, 1
  br i1 %cmp22, label %for.cond1.preheader.preheader, label %for.end8

for.cond1.preheader.preheader:                    ; preds = %entry
  %0 = sext i32 %N to i64
  %wide.trip.count32 = zext i32 %M to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc6
  %indvars.iv29 = phi i64 [ 1, %for.cond1.preheader.preheader ], [ %indvars.iv.next30, %for.inc6 ]
  %indvars.iv27 = phi i64 [ 2, %for.cond1.preheader.preheader ], [ %indvars.iv.next28, %for.inc6 ]
  %1 = mul nsw i64 %indvars.iv29, %0
  %2 = trunc i64 %indvars.iv29 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4.not = icmp eq i64 %indvars.iv29, %indvars.iv
  br i1 %cmp4.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body3
  %3 = add nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %3
  store i32 %2, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %indvars.iv27
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.inc
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond33.not = icmp eq i64 %indvars.iv.next30, %wide.trip.count32
  br i1 %exitcond33.not, label %for.end8.loopexit, label %for.cond1.preheader

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  %div = sdiv i32 %N, 2
  %idxprom9 = sext i32 %div to i64
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %idxprom9
  %4 = load i32, ptr %arrayidx10, align 4
  ret i32 %4
}

