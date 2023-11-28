; RUN: opt < %s -passes=instcombine -S | FileCheck %s
; CHECK-NOT: getelementptr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local global [1 x [4 x i32]] zeroinitializer, align 16

define dso_local void @c() {
entry:
  store i32 0, ptr @a, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %d.0 = phi ptr [ undef, %entry ], [ %arrayidx2, %for.body ]
  %0 = load i32, ptr @a, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %add = add nsw i32 %0, 5
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds [0 x [4 x i32]], ptr @b, i64 0, i64 %idxprom
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [4 x i32], ptr %arrayidx, i64 0, i64 %idxprom1
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @a, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %cmp = icmp eq ptr %d.0, @c
  ret void
}
