; Run subsequently "interchange, runtimedd, interchange".
; The first interchange will construct a graph for the loop, but
; should fail because of A<->B dependency. The runtimedd should
; invalidate the graph and then the second interchnage should be successful.

; The test is checking DD edges after invalidation.

; RUN: opt < %s -hir-ssa-deconstruction -hir-loop-interchange -hir-dd-analysis -hir-runtime-dd -hir-loop-interchange -hir-dd-analysis -scoped-noalias -analyze -print-after=hir-loop-interchange -hir-dd-analysis-verify=Innermost 2>&1 | FileCheck %s

; CHECK: IR Dump After HIR Loop Interchange
; CHECK: (%A)[i2][i1]

; CHECK: DD graph for function 
; CHECK-DAG: [[SRC1:[0-9]+]]:[[DST1:[0-9]+]] %0 --> %0 FLOW
; CHECK-DAG: [[SRC2:[0-9]+]]:[[DST2:[0-9]+]] (%B)[i1 + i2] --> (%A)[i2][i1] ANTI
; CHECK-DAG: [[SRC3:[0-9]+]]:[[DST3:[0-9]+]] (%A)[i2][i1] --> (%B)[i1 + i2] FLOW

; CHECK: IR Dump After HIR Loop Interchange
; CHECK: (%A)[i1][i2]

; CHECK: DD graph for function 
; CHECK-DAG: [[SRC1]]:[[DST1]] %0 --> %0 FLOW
; CHECK-DAG: (%B)[i1 + i2] --> (%A)[i2][i1] ANTI
; CHECK-DAG: (%A)[i2][i1] --> (%B)[i1 + i2] FLOW
; CHECK-NOT: [[SRC2]]:[[DST2]]
; CHECK-NOT: [[SRC3]]:[[DST3]]

; Source code:
;
; void foo(int A[][100], int B[]) {
; int i, j;
; for (i=0;i<100;i++) {
;   for (j=0;j<100;j++) {
;     A[j][i] = B[j+i];
;   }
; }
; }

; ModuleID = 'dd-invalidate.ll'
source_filename = "dd-invalidate.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo([100 x i32]* %A, i32* %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc8
  %i.02 = phi i32 [ 0, %entry ], [ %inc9, %for.inc8 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc
  %j.01 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %add = add nsw i32 %j.01, %i.02
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom4 = sext i32 %i.02 to i64
  %idxprom5 = sext i32 %j.01 to i64
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %idxprom5
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx6, i64 0, i64 %idxprom4
  store i32 %0, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i32 %j.01, 1
  %cmp2 = icmp slt i32 %inc, 100
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %inc9 = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc9, 100
  br i1 %cmp, label %for.body, label %for.end10

for.end10:                                        ; preds = %for.inc8
  ret void
}

