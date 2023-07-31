; RUN: opt -hir-create-function-level-region -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s


; Verify that the store to (%A)[5] in the first unknown loop is eliminated by dead store elimination.

; Before DSE

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %indvars.iv.out = i1;
; CHECK: |   (%A)[5] = 0;
; CHECK: |   %ld = (%B)[i1];
; CHECK: |   if (0 != %ld)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body1:
; CHECK: |   (%A)[5] = %indvars.iv.out;
; CHECK: |   %ld1 = (%B)[i1];
; CHECK: |   if (0 != %ld1)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body1;
; CHECK: |   }
; CHECK: + END LOOP


; After DSE
; CHECK: Function

; CHECK-NOT: (%A)[5] = 0;


define void @foo(ptr noalias %A, ptr noalias %B) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 5
  store i32 0, ptr %arrayidx, align 4
  %sext = sext i32 %indvars.iv to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %B, i64 %sext
  %ld = load i32, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 0, %ld
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body1
  br label %for.body1

for.body1:                                         ; preds = %for.body1, %for.end.loopexit
  %indvars.iv1 = phi i32 [ 0, %for.end.loopexit ], [ %indvars.iv.next1, %for.body1 ]
  %arrayidx.1 = getelementptr inbounds i32, ptr %A, i64 5
  store i32 %indvars.iv, ptr %arrayidx.1, align 4
  %sext1 = sext i32 %indvars.iv1 to i64
  %arrayidx2.1 = getelementptr inbounds i32, ptr %B, i64 %sext1
  %ld1 = load i32, ptr %arrayidx2.1, align 4
  %indvars.iv.next1 = add nuw nsw i32 %indvars.iv1, 1
  %exitcond1 = icmp eq i32 0, %ld1
  br i1 %exitcond1, label %for.end.loopexit1, label %for.body1

for.end.loopexit1:                                 ; preds = %for.body1
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

