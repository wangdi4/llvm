; Check goto cloning
; REQUIRES: asserts
; RUN: opt < %s -mem2reg | opt -S -hir-complete-unroll -print-before=hir-complete-unroll -print-after=hir-complete-unroll 2>&1 | FileCheck %s

; CHECK-LABEL: *** IR Dump Before HIR Complete Unroll ***
; CHECK: goto [[LABEL0:L.*]];
; CHECK: goto [[LABEL1:L.*]];
; CHECK-DAG: [[LABEL0]]:
; CHECL-DAG: [[LABEL1]]:

; CHECK-LABEL: *** IR Dump After HIR Complete Unroll ***
; CHECK: goto [[LABEL0]].[[NODE01:.*]].[[NODE02:.*]];
; CHECK: goto [[LABEL1]].[[NODE11:.*]];
; CHECK-DAG: <[[NODE01]]>{{.*}}[[LABEL0]].[[NODE01]].[[NODE02]]:
; CHECK-DAG: <[[NODE11]]>{{.*}}[[LABEL1]].[[NODE11]]:

; CHECK-NOT: <[[NODE01]]>{{.*}}[[LABEL0]].[[NODE01]].[[NODE02]]:
; CHECK-NOT: <[[NODE11]]>{{.*}}[[LABEL1]].[[NODE11]]:

; CHECK: goto [[LABEL0]].[[NODE21:.*]];
; CHECK: goto [[LABEL1]].[[NODE31:.*]];
; CHECK-DAG: <[[NODE21]]>{{.*}}[[LABEL0]].[[NODE21]]:
; CHECK-DAG: <[[NODE31]]>{{.*}}[[LABEL1]].[[NODE31]]:

; ModuleID = 'goto.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal global [5 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 0, i32* %i, align 4
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom
  store i32 %0, i32* %arrayidx, align 4
  %2 = load i32, i32* %i, align 4
  %cmp = icmp eq i32 %2, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %do.body
  br label %L1

if.end:                                           ; preds = %do.body
  %3 = load i32, i32* %i, align 4
  %cmp1 = icmp eq i32 %3, 1
  br i1 %cmp1, label %if.then.2, label %if.end.3

if.then.2:                                        ; preds = %if.end
  br label %L2

if.end.3:                                         ; preds = %if.end
  %4 = load i32, i32* %i, align 4
  %cmp4 = icmp eq i32 %4, 2
  br i1 %cmp4, label %if.then.5, label %if.end.6

if.then.5:                                        ; preds = %if.end.3
  br label %L2.63

if.end.6:                                         ; preds = %if.end.3
  br label %L1

L1:                                               ; preds = %if.end.6, %if.then
  %5 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %5 to i64
  %arrayidx8 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom7
  %6 = load i32, i32* %arrayidx8, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %arrayidx8, align 4
  br label %L2

L2:                                               ; preds = %L1, %if.then.2
  %7 = load i32, i32* %i, align 4
  %idxprom9 = sext i32 %7 to i64
  %arrayidx10 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom9
  %8 = load i32, i32* %arrayidx10, align 4
  %inc11 = add nsw i32 %8, 1
  store i32 %inc11, i32* %arrayidx10, align 4
  br label %L2.63

L2.63:                                               ; preds = %L2, %if.then.5
  %9 = load i32, i32* %i, align 4
  %idxprom12 = sext i32 %9 to i64
  %arrayidx13 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i32 0, i64 %idxprom12
  %10 = load i32, i32* %arrayidx13, align 4
  %inc14 = add nsw i32 %10, 1
  store i32 %inc14, i32* %arrayidx13, align 4
  %11 = load i32, i32* %i, align 4
  %inc15 = add nsw i32 %11, 1
  store i32 %inc15, i32* %i, align 4
  br label %do.cond

do.cond:                                          ; preds = %L2.63
  %12 = load i32, i32* %i, align 4
  %cmp16 = icmp ne i32 %12, 2
  br i1 %cmp16, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  %13 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @A, i32 0, i64 0), align 4
  ret i32 %13
}
