; RUN: opt -disable-loop-unrolling -passes='default<O2>' -S -disable-hir-vec-dir-insert < %s | FileCheck %s

; Check that disabling of automatic unrolling in the pass builder is honored.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-LABEL: @forced(
; CHECK: = load
; CHECK-NOT: = load

define void @forced(ptr nocapture %a) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret void
}

