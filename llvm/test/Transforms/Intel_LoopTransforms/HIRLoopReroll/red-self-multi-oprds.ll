; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; Verify that the loop with self safe reduction inst can be correctly rerolled.

; CHECK:Function: foo
; CHECK:      + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK:      |   %0 = (@C)[0][2 * i1];
; CHECK:      |   %1 = (@B)[0][2 * i1];
; CHECK:      |   %3 = (@C)[0][2 * i1 + 1];
; CHECK:      |   %4 = (@B)[0][2 * i1 + 1];
; CHECK:      |   %S.036 = %0 + %1 + %S.036  +  %3 + %4;
; CHECK:      + END LOOP

; CHECK:Function: foo
; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:      |   %0 = (@C)[0][i1];
; CHECK:      |   %1 = (@B)[0][i1];
; CHECK:      |   %S.036 = %0 + %S.036  +  %1;
; CHECK:      + END LOOP


;Module Before HIR
; ModuleID = 'store-red-2.c'
source_filename = "store-red-2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add21.lcssa = phi i32 [ %add21, %for.body ]
  ret i32 %add21.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.036 = phi i32 [ 0, %entry ], [ %add21, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 8
  %add = add nsw i32 %1, %0
  %add7 = add nsw i32 %add, %S.036
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx10, align 4
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %2
  %4 = load i32, ptr %arrayidx13, align 4
  %add14 = add nsw i32 %4, %3
  %add21 = add nsw i32 %add7, %add14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}



