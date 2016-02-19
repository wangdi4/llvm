; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the denominator of the blob (%offset /u 2) is not set as the CanonExpr's denominator and merged with the loop index, i.e. the subscript should not look like this: (2 * i1 + %offset)/u2


; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: (%A)[i1 + (%offset /u 2)] = i1;
; CHECK-NEXT: END LOOP


; Function Attrs: nounwind
define void @foo(i32* nocapture %A, i32 %offset, i32 %n) {
entry:
  %div = lshr i32 %offset, 1
  %add.ptr = getelementptr inbounds i32, i32* %A, i32 %div
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %add.ptr, i32 %i.08
  store i32 %i.08, i32* %arrayidx, align 4
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

