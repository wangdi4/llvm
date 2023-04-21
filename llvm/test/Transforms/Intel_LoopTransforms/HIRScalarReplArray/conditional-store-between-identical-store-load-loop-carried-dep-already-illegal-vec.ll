; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to handle the loop-independant (@A)[0][i1 + 1] group
; with conditional store by moving the store after the if. 
; The existing loop carried lexically backward dependence of the store with load of
; (@A)[0][i1] makes vectorization illegal and can be ignored.


; CHECK: Dump Before

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1 + 1] = (@A)[0][i1];
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1 + 1] = 10;
; CHECK: |   }
; CHECK: |   %add = 5  +  (@A)[0][i1 + 1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %scalarepl = (@A)[0][i1];
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      %scalarepl = 10;
; CHECK: |   }
; CHECK: |   (@A)[0][i1 + 1] = %scalarepl;
; CHECK: |   %add = 5  +  %scalarepl;
; CHECK: + END LOOP




@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %idxprom.1 = add nsw i64 %idxprom, 1
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %ld = load i32, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom.1
  store i32 %ld, ptr %arrayidx1, align 4
  %cmp1 = icmp sgt i32 %i.01, 10
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 10, ptr %arrayidx1, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %ld1 = load i32, ptr %arrayidx1, align 4
  %add = add i32 5, %ld1
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 99 
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %add.lcssa = phi i32 [ %add, %for.inc ]
  ret i32 %add.lcssa
}
