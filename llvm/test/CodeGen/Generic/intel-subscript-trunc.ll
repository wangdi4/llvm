; RUN: opt < %s -lower-subscript 2>&1 -S | FileCheck %s
; RUN: opt < %s -passes=lower-subscript -S 2>&1 | FileCheck %s

; CHECK: getelementptr inbounds i32, i32* %p, i32 1
; CHECK: %[[T:.*]] = trunc i64 %add to i32
; CHECK: getelementptr inbounds i32, i32* %p, i32 %[[T]]

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32 %n) {
entry:
  %cmp6 = icmp sgt i32 %n, 0
  br i1 %cmp6, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:
  %n64 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:
  ret void

for.body:
  %i.07 = phi i64 [ 0, %for.body.lr.ph ], [ %add, %for.body ]
  %add = add nuw nsw i64 %i.07, 1
  %arrayidx2 = call i32* @llvm.intel.subscript.p0i32.i64.i32.p0i32.i64(i8 0, i64 0, i32 4, i32* %p, i64 1)
  %0 = load i32, i32* %arrayidx2, align 4
  %arrayidx1 = call i32* @llvm.intel.subscript.p0i32.i64.i32.p0i32.i64(i8 0, i64 0, i32 4, i32* %p, i64 %add)
  store i32 %0, i32* %arrayidx1, align 4
  %exitcond = icmp eq i64 %add, %n64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare i32* @llvm.intel.subscript.p0i32.i64.i32.p0i32.i64(i8, i64, i32, i32*, i64)

