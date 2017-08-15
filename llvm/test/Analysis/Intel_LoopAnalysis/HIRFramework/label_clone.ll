; Check goto cloning
; TODO: this test is being throttled due to too many nested ifs but not due to presence of goto. Make detection of gotos stronger.
; REQUIRES: asserts
; RUN: opt < %s -hir-post-vec-complete-unroll -hir-cost-model-throttling=0 -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 | FileCheck %s

; CHECK: Dump Before HIR PostVec Complete Unroll 
; CHECK: goto [[LABEL0:L.*]];
; CHECK: goto [[LABEL1:L.*]];
; CHECK-DAG: [[LABEL0]]:
; CHECK-DAG: [[LABEL1]]:

; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK: goto [[LABEL0]]{{.*}}.[[NODE01:.*]];
; CHECK: goto [[LABEL1]]{{.*}}.[[NODE11:.*]];
; CHECK-DAG: <[[NODE01]]>{{.*}}[[LABEL0]]{{.*}}.[[NODE01]]:
; CHECK-DAG: <[[NODE11]]>{{.*}}[[LABEL1]]{{.*}}.[[NODE11]]:

; CHECK-NOT: <[[NODE01]]>{{.*}}[[LABEL0]].[[NODE01]]:
; CHECK-NOT: <[[NODE11]]>{{.*}}[[LABEL1]].[[NODE11]]:

; original loop nodes are reused in last iteration
; CHECK: goto [[LABEL0]];
; CHECK: goto [[LABEL1]];
; CHECK-DAG: [[LABEL0]]:
; CHECK-DAG: [[LABEL1]]:

; ModuleID = 'goto.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal global [5 x i32] zeroinitializer, align 16

define i32 @main(i32 %b) {
entry:
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc15, %do.cond ]
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom
  store i32 %i.0, i32* %arrayidx, align 4
  %cmp = icmp sgt i32 %i.0, 100
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %do.body
  br label %L1

if.end:                                           ; preds = %do.body
  %cmp1 = icmp slt i32 %i.0, %b
  br i1 %cmp1, label %if.then.2, label %if.end.3

if.then.2:                                        ; preds = %if.end
  br label %L2

if.end.3:                                         ; preds = %if.end
  %cmp4 = icmp slt i32 %i.0, %b
  br i1 %cmp4, label %if.then.5, label %if.end.6

if.then.5:                                        ; preds = %if.end.3
  br label %L2.63

if.end.6:                                         ; preds = %if.end.3
  br label %L1

L1:                                               ; preds = %if.end.6, %if.then
  %idxprom7 = sext i32 %i.0 to i64
  %arrayidx8 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom7
  %0 = load i32, i32* %arrayidx8, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx8, align 4
  br label %L2

L2:                                               ; preds = %L1, %if.then.2
  %idxprom9 = sext i32 %i.0 to i64
  %arrayidx10 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom9
  %1 = load i32, i32* %arrayidx10, align 4
  %inc11 = add nsw i32 %1, 1
  store i32 %inc11, i32* %arrayidx10, align 4
  br label %L2.63

L2.63:                                            ; preds = %L2, %if.then.5
  %idxprom12 = sext i32 %i.0 to i64
  %arrayidx13 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom12
  %2 = load i32, i32* %arrayidx13, align 4
  %inc14 = add nsw i32 %2, 1
  store i32 %inc14, i32* %arrayidx13, align 4
  %inc15 = add nsw i32 %i.0, 1
  br label %do.cond

do.cond:                                          ; preds = %L2.63
  %cmp16 = icmp ne i32 %inc15, 2
  br i1 %cmp16, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  %3 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i32 0, i64 0), align 4
  ret i32 %3
}

