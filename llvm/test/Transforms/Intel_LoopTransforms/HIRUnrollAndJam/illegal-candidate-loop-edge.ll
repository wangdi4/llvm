; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam" -debug-only=hir-unroll-and-jam -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that unroll & jam gives up due to this i1 loop cross-iteration edge:
; %i_offset.022 --> %i_offset.022 FLOW (<)

; HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   %0 = (@B)[0][i2];
; |   |   (@A)[0][%i_offset.022][i2] = %0 + 1;
; |   + END LOOP
; |
; |   %i_offset.022 = i1 + 1;
; + END LOOP


; CHECK: Illegal edge found: 
; CHECK-SAME: %i_offset.022 --> %i_offset.022 FLOW (<)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end
  %i_offset.022 = phi i32 [ 5, %entry ], [ %add8, %for.end ]
  %i.021 = phi i32 [ 0, %entry ], [ %add8, %for.end ]
  %idxprom4 = sext i32 %i_offset.022 to i64
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 1
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %idxprom4, i64 %indvars.iv
  store i32 %add, i32* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add8 = add nuw nsw i32 %i.021, 1
  %exitcond23.not = icmp eq i32 %add8, 100
  br i1 %exitcond23.not, label %for.end11, label %for.cond1.preheader

for.end11:                                        ; preds = %for.end
  ret void
}

