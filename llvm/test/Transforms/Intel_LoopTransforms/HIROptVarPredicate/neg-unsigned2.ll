; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -S -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that OptVarPredicate pass does not hoist out 'if'
; with unsigned predicate if RHS has negative value.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64(%M) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>  <LEGAL_MAX_TC = 4294967294>
;                  |   if (i1 + 1 >u -5)
;                  |   {
;                  |      (%A)[i1 + 1] = i1 + 1;
;                  |   }
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i32 noundef %M, i32 noundef %N) local_unnamed_addr {
entry:
  %cmp10 = icmp ugt i32 %M, 1
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %M to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 1, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp1 = icmp ugt i64 %indvars.iv, -5
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %idxprom2 = sext i32 %M to i64
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 %idxprom2
  %1 = load i32, ptr %arrayidx3, align 4
  ret i32 %1
}
