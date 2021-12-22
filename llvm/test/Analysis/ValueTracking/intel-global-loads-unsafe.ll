; CMPLRLLVM-31967:
; Tests for global-loads-unsafe option, which prevents speculation of global
; loads (moving the load above a guarding condition).

; RUN: opt -S -global-loads-unsafe -simplifycfg -loops -licm %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; CHECK-LABEL: _Z3fooii
; CHECK: entry:
; CHECK-NOT: load i32{{.*}}@g
; CHECK: if.then:
; CHECK-NEXT: load i32{{.*}}@g

; Function Attrs: mustprogress nounwind uwtable
define dso_local i32 @_Z3fooii(i32 %cond, i32 %sum) local_unnamed_addr {
entry:
  %tobool.not = icmp eq i32 %cond, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry

; this load is guarded by the above condition. LLVM would normally want to
; select-convert the cmp/br, and execute the load unconditionally.
  %0 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @g, i64 0, i64 0), align 16

  %add = add nsw i32 %0, %sum
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %sum.addr.0 = phi i32 [ %add, %if.then ], [ %sum, %entry ]
  ret i32 %sum.addr.0
}

; CHECK-LABEL: _Z4foo2ii
; CHECK: entry:
; CHECK-NOT: getelementptr i32{{.*}}@g
; CHECK-NOT: load
; CHECK: for.body:
; CHECK: load i32{{.*}}@g

; Function Attrs: mustprogress nounwind uwtable
define dso_local i32 @_Z4foo2ii(i32 %cond, i32 %sum) local_unnamed_addr {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %sum.addr.0 = phi i32 [ %sum, %entry ], [ %add, %for.body ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %cond
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret i32 %sum.addr.0

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64

; LLVM will normally hoist this load out of the loop. There is no guarantee
; that the loop body will execute, so it depends on whether the load is
; speculatable or not.
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @g, i64 0, i64 0
  %0 = load i32, i32* %arrayidx, align 4

  %div = sdiv i32 %0, %i.0
  %add = add nsw i32 %sum.addr.0, %div
  %inc = add nsw i32 %i.0, 1
  br label %for.cond
}

