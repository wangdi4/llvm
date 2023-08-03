; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Verify that we give up on the loop-independant group which starts with a
; conditional store as it will result in undefined scalar-replaced temp
; in the non-taken path for the load.

; CHECK:  Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   %ld = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %ld + 5;
; CHECK: + END LOOP

; CHECK:  Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   %ld = (@A)[0][i1];
; CHECK: |   (@A)[0][i1] = %ld + 5;
; CHECK: + END LOOP

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRScalarReplArray
; CHECK-CHANGED-NOT: Dump After HIRLoopIndependentScalarRepl

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
  %ld = load i32, ptr %arrayidx, align 4
  %add = add i32 %ld, 5
  store i32 %add, ptr %arrayidx, align 4
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}
