; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to handle the loop-independant group with conditional
; store and a load by moving the unconditional store after the if.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = 5;
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   %add = (@A)[0][i1]  +  5;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %scalarepl = 5;
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      %scalarepl = 10;
; CHECK: |   }
; CHECK: |   (@A)[0][i1] = %scalarepl;
; CHECK: |   %add = %scalarepl  +  5;
; CHECK: + END LOOP


@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  store i32 5, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %i.01, 10
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 10, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %ld = load i32, ptr %arrayidx, align 4
  %add = add i32 %ld, 5
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %add.lcssa = phi i32 [ %add, %for.inc ]
  ret i32 %add.lcssa
}
