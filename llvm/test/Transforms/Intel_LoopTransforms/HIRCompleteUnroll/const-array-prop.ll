; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -hir-complete-unroll-force-constprop 2>&1 < %s | FileCheck %s


; Verify that constant array propagation of @A is propagated to @B in series for pre-vec complete unroll. Also verify that %0 definition is kept since it is liveout in for.end.

; CHECK: Function

; CHECK:    + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:    |   %0 = (@A)[0][i1];
; CHECK:    |   %1 = (@B)[0][%0];
; CHECK:    |   (%C)[%1] = %0;
; CHECK:    + END LOOP


; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK:    (%C)[7] = 3;
; CHECK:    (%C)[6] = 2;
; CHECK:    (%C)[5] = 1;
; CHECK:    %0 = 0;
; CHECK:    (%C)[4] = 0;
; CHECK: END REGION



;Module Before HIR; ModuleID = 'const-array-prop.c'
source_filename = "const-array-prop.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal unnamed_addr constant [4 x i32] [i32 3, i32 2, i32 1, i32 0], align 16
@B = internal unnamed_addr constant [4 x i32] [i32 4, i32 5, i32 6, i32 7], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i32], ptr @A, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %idxprom0 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [4 x i32], ptr @B, i64 0, i64 %idxprom0
  %1 = load i32, ptr %arrayidx2, align 4
  %idxprom1 = sext i32 %1 to i64
  %arrayidx3 = getelementptr inbounds i32, ptr %C, i64 %idxprom1
  store i32 %0, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %liveout = phi i32 [ %0, %for.body ]
  ret void
}

