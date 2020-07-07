; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Check that redundant loop chunk with UB:smin(-1, (-1 + sext.i32.i64(%n))) will not be generated.

; BEGIN REGION { }
;       + DO i1 = 0, sext.i32.i64(%n) + -1, 1
;       |   (@a)[0][i1] = i1;
;       |   if (i1 == 0)
;       |   {
;       |      (@b)[0][i1] = %n;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK-NOT:   + DO i1 = 0, smin(-1, (-1 + sext.i32.i64(%n))), 1
; CHECK-NOT:   |   (@a)[0][i1] = i1;
; CHECK-NOT:   + END LOOP
;
; CHECK:       if (0 < smin(0, (-1 + sext.i32.i64(%n))) + 1)
; CHECK:       {
; CHECK:          (@a)[0][0] = 0;
; CHECK:          (@b)[0][0] = %n;
; CHECK:       }
;
; CHECK:       + DO i1 = 0, sext.i32.i64(%n) + -2, 1
; CHECK:       |   (@a)[0][i1 + 1] = i1 + 1;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %indvars.iv, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv
  store i32 %n, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

