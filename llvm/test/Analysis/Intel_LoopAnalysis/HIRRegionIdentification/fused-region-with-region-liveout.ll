; RUN: opt < %s -passes="print<hir-region-identification>" 2>&1 | FileCheck %s

; Verify that both loops are placed in the same region even in the 
; presence of %t which is region liveout from the intermediate bblock.

; CHECK: Region 1
; CHECK: EntryBB: %for.body
; CHECK: Member BBlocks: %for.body, %for.body7, %for.body7.preheader
; CHECK-NOT: Region 2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(i32* %ptr) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv28
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv28 to i32
  %add = add nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv28
  store i32 %add, i32* %arrayidx2, align 4
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, 100
  br i1 %exitcond30, label %for.body7.preheader, label %for.body

for.body7.preheader:                              ; preds = %for.body
  %t = load i32, i32* %ptr
  br label %for.body7

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body7 ], [ 0, %for.body7.preheader ]
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx9, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add10 = add nsw i32 %2, %3
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* @C, i64 0, i64 %indvars.iv
  store i32 %add10, i32* %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup6, label %for.body7

for.cond.cleanup6:                                ; preds = %for.body7
  %t.phi = phi i32 [ %t, %for.body7 ]
  ret void
}



