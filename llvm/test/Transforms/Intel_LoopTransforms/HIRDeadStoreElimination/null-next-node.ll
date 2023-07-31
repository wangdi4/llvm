; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-lmm,print<hir>,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that DSE does not compfail while trying to eliminate the redundant store
; ((%A)[0] = %limm) in loop postexit. It was hitting a null next node during
; traversal while trying to get to the load (%1 = (%A)[0]).

; Print Before DSE-
; CHECK: Function:

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %limm = i1;
; CHECK: + END LOOP
; CHECK:   (%A)[0] = %limm;

; CHECK: %1 = (%A)[0];
; CHECK: (%A)[0] = %1 + 1;
; CHECK: ret %1 + 1;
; CHECK: END REGION


; Verify that dead store elimination is able to eliminate the redundant store
; by replacing it with a temp and then propagating that temp into its use.

; Print After DSE-
; CHECK: Function:

; CHECK-NOT: %temp =
; CHECK: (%A)[0] = %limm + 1;


define i32 @foo(ptr %A) {
entry:
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 0
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body3 ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %1 = load i32, ptr %arrayidx, align 4
  %inc6 = add nsw i32 %1, 1
  store i32 %inc6, ptr %arrayidx, align 4
  br label %exit

exit:
  ret i32 %inc6
}
