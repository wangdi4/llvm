; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the addIV() operation for GEP is performed successfully.
; CHECK: DO i1 = 0, %vn.0.vn.0. + -1
; CHECK-NEXT: (%p)[(1 + %vn.0.vn.0.) * i1] = 1
; CHECK-NEXT: END LOOP


; Function Attrs: nounwind uwtable
define i32 @main(i32* %vn, i32* %p)  {
entry:
  %vn.0.vn.0. = load volatile i32, i32* %vn, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.025 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %mul1 = mul nsw i32 %i.025, %vn.0.vn.0.
  %add.ptr = getelementptr inbounds i32, i32* %p, i32 %mul1
  %add.ptr2 = getelementptr inbounds i32, i32* %add.ptr, i32 %i.025
  store i32 1, i32* %add.ptr2, align 4
  %inc = add nuw nsw i32 %i.025, 1
  %exitcond = icmp eq i32 %inc, %vn.0.vn.0.
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}
