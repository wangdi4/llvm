; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that we create a liveout copy of linear header phi %i.01 which is
; indirectly liveout of the loop through %i.liveout to handle the live-range
; issue.


; CHECK: + DO i1 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %i.01.out = i1;
; CHECK: |   %1 = (@A)[0][i1];
; CHECK: |   if (%1 < 0)
; CHECK: |   {
; CHECK: |      goto exit.loopexit;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.cond, %entry
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.cond ]
  %0 = zext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @A, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp slt i32 %1, 0
  br i1 %cmp1, label %exit.loopexit, label %for.cond

for.cond:                                         ; preds = %for.body
  %i.liveout = phi i32 [ %i.01, %for.body ]
  %inc = add nuw nsw i32 %i.01, 1
  %cmp = icmp ult i32 %inc, 10
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.cond
  %t.0.lcssa = phi i32 [ %i.liveout, %for.cond ]
  br label %exit

exit.loopexit:                                    ; preds = %for.body
  br label %exit

exit:                                             ; preds = %exit.loopexit, %for.end
  ret void
}
