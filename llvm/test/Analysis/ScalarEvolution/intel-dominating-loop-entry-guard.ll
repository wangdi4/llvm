; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that tripcount of the loop does not have smax because dominating
; condition %cmp is identified as the loop entry guard.

; CHECK: Loop %do.body: backedge-taken count is (-1 + %n)


@A = dso_local global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %n, i32 %m) {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %if.then, label %if.end5

if.then:                                          ; preds = %entry
  %cmp1 = icmp sgt i32 %m, 5
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.then
  %0 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 5), align 4
  br label %if.end

if.end:                                           ; preds = %if.then2, %if.then
  br label %do.body

do.body:                                          ; preds = %do.body, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc3, %do.body ]
  %inc3 = add nuw nsw i32 %i.0, 1
  %idxprom = zext i32 %inc3 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %idxprom
  store i32 %i.0, i32* %arrayidx, align 4
  %cmp4 = icmp slt i32 %inc3, %n
  br i1 %cmp4, label %do.body, label %if.end5.loopexit

if.end5.loopexit:                                 ; preds = %do.body
  br label %if.end5

if.end5:                                          ; preds = %if.end5.loopexit, %entry
  ret void
}

