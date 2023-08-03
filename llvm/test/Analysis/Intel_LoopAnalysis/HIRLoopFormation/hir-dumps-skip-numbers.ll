; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-cost-model-throttling=0 -disable-output 2>&1 | FileCheck %s --match-full-lines --check-prefix VERBOSE
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-cost-model-throttling=0 -disable-output -hir-details-no-verbose-indent 2>&1 | FileCheck %s --match-full-lines --check-prefix NOVERBOSE

; Verify that <Node number> isn't printed when -hir-details-no-verbose-indent is specified.

; hir-temp-cleanup is added because -print-after= requires a non-analysis pass
; and "-analyze" output from hir-ssa-deconstruction + -hir-framework prints more
; than needed (although could have been used as well)

define i32 @main(ptr %vn, ptr %p)  {
; VERBOSE-LABEL: <0>          BEGIN REGION { }
; VERBOSE-NEXT:  <12>               + DO i1 = 0, %vn.0.vn.0. + -1, 1   <DO_LOOP>
; VERBOSE-NEXT:  <5>                |   (%p)[(1 + sext.i32.i64(%vn.0.vn.0.)) * i1] = i1;
; VERBOSE-NEXT:  <12>               + END LOOP
; VERBOSE-NEXT:  <0>          END REGION

; NOVERBOSE-LABEL: BEGIN REGION { }
; NOVERBOSE-NEXT:        + DO i1 = 0, %vn.0.vn.0. + -1, 1   <DO_LOOP>
; NOVERBOSE-NEXT:        |   (%p)[(1 + sext.i32.i64(%vn.0.vn.0.)) * i1] = i1;
; NOVERBOSE-NEXT:        + END LOOP
; NOVERBOSE-NEXT:  END REGION
entry:
  %vn.0.vn.0. = load volatile i32, ptr %vn, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.025 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %mul1 = mul nsw i32 %i.025, %vn.0.vn.0.
  %add.ptr = getelementptr inbounds i32, ptr %p, i32 %mul1
  %add.ptr2 = getelementptr inbounds i32, ptr %add.ptr, i32 %i.025
  store i32 %i.025, ptr %add.ptr2, align 4
  %inc = add nuw nsw i32 %i.025, 1
  %exitcond = icmp eq i32 %inc, %vn.0.vn.0.
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}
