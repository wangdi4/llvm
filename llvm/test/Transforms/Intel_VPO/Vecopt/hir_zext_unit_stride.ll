; LIT test to check that we do recognize unit stride accesses when the index is
; a zero extended value. It also checks VPValue based generated code matches
; code generated from mixed CG mode.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -aa-pipeline="basic-aa" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
define void @foo(ptr nocapture %arr) {
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:    (<4 x i32>*)(%arr)[i1] = i1 + <i32 0, i32 1, i32 2, i32 3>;
; CHECK-NEXT:  END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.ext = zext i32 %indvars.iv to i64
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.ext
  store i32 %indvars.iv, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
