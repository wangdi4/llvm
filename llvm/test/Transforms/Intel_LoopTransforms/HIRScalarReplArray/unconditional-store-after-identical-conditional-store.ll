; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl -disable-output < %s 2>&1 | FileCheck %s

; Verify that we remove the empty if after scalar-replacing the conditional
; store and eliminating the redundant temp definition of the scalar-replaced
; temp inside the condition. This is basically equivalent to DSE.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   (@A)[0][i1] = 5;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK:      BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT: |   (@A)[0][i1] = 5;
; CHECK:      + END LOOP


@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %cmp1 = icmp sgt i32 %i.01, 10
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 10, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  store i32 5, ptr %arrayidx, align 4
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}
