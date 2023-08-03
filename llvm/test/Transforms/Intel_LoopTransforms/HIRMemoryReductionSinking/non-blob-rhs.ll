; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; Check that we are not recognizing a reduction here because RHS of the second store is not self-blob.

; Dump Before-

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %or = (%b)[0]  |  %t + 68;
; CHECK: |   (%b)[0] = %or + 1;
; CHECK: + END LOOP

; CHECK-NOT: modified


define void @foo(ptr %b, i64 %n, i32 %t) {
entry:
  br label %for.body

for.body:                                       ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %addm = add nsw i32 %t, 68
  %t14 = load i32, ptr %b, align 4
  %or = or i32 %t14, %addm
  %add = add nsw i32 %or, 1
  store i32 %add, ptr %b, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:
  ret void
}

