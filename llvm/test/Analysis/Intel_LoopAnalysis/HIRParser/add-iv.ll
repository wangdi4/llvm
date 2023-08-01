; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that the addIV() operation for GEP is performed successfully.

; CHECK: + DO i1 = 0, %vn.0.vn.0. + -1, 1   <DO_LOOP>
; CHECK: |   (%p)[(1 + sext.i32.i64(%vn.0.vn.0.)) * i1] = 1;
; CHECK: + END LOOP

; Function Attrs: nounwind uwtable
define i32 @main(ptr %vn, ptr %p)  {
entry:
  %vn.0.vn.0. = load volatile i32, ptr %vn, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.025 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %mul1 = mul nsw i32 %i.025, %vn.0.vn.0.
  %add.ptr = getelementptr inbounds i32, ptr %p, i32 %mul1
  %add.ptr2 = getelementptr inbounds i32, ptr %add.ptr, i32 %i.025
  store i32 1, ptr %add.ptr2, align 4
  %inc = add nuw nsw i32 %i.025, 1
  %exitcond = icmp eq i32 %inc, %vn.0.vn.0.
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}
