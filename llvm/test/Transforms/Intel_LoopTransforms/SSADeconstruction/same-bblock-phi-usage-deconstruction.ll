; RUN: opt < %s -hir-ssa-deconstruction -S | FileCheck %s

; Check that we insert a liveout copy for the phi %0 which is indirectly used in the same bblock phi %1 ( %add21's SCEV form is: %3 - %4 + %0 ).
; CHECK: %hir.de.ssa.copy0.out = bitcast i32 %0 
; CHECK: %add21 = add i32 %sub18, %hir.de.ssa.copy0.out 


; ModuleID = 'before_ssa.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @main() {
entry:
  %u = alloca [100 x i32], align 16
  %g = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %0 = phi i32 [ 0, %entry ], [ %4, %for.body ]
  %1 = phi i32 [ 0, %entry ], [ %add21, %for.body ]
  %2 = phi i32 [ 0, %entry ], [ %3, %for.body ]
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %u, i64 0, i64 %indvars.iv
  %add9 = add i32 %1, %2
  store i32 %add9, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* %u, i64 0, i64 %indvars.iv.next
  %3 = load i32, i32* %arrayidx14, align 4
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i64 0, i64 %indvars.iv.next
  %4 = load i32, i32* %arrayidx17, align 4
  %sub18 = sub i32 %3, %4
  %arrayidx20 = getelementptr inbounds [100 x i32], [100 x i32]* %g, i64 0, i64 %indvars.iv
  %add21 = add i32 %sub18, %0
  store i32 %add21, i32* %arrayidx20, align 4
  %exitcond146 = icmp eq i64 %indvars.iv.next, 62
  br i1 %exitcond146, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

