; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that the negative divisor of 32 bit udiv instruction (-2) in incoming
; IR is stored as a big positive number in the int64_t denominator field of
; CanonExpr to correctly represent unsigned division semantics.

; Previous incorrect HIR had this form where we negated the denominator-
; + DO i1 = 0, 127, 1   <DO_LOOP>
; |   (%arr)[i1] = (i1 + -3)/2;
; + END LOOP

; CHECK: + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK: |   (%arr)[i1] = (i1 + -3)/u4294967294;
; CHECK: + END LOOP

; Verify that CG regenerates divisor of -2.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S 2>&1 | FileCheck %s --check-prefix=CG

; CG: loop{{.*}}:
; CG: udiv i32 {{.*}}, -2

define dso_local void @foo(ptr nocapture %arr) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %1 = add i32 %0, -3
  %div = udiv i32 %1, -2
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  store i32 %div, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

