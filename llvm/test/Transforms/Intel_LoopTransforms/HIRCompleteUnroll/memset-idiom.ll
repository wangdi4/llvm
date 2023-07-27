; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that we leave the loop with simple memset loop for idiom recognition/vectorizer pass.

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = 0;
; CHECK: + END LOOP

@A = internal unnamed_addr global [3 x i32] zeroinitializer, align 4

define void @foo() {
entry:
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %gep = getelementptr inbounds [3 x i32], ptr @A, i64 0, i64 %iv
  store i32 0, ptr %gep, align 4
  %iv.inc = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 3
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

